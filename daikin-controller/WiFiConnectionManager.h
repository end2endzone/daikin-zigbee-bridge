#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPping.h>
#include <SoftTimers.h>
#include "secrets.h"
#include "timer_helper.h"
#include "esp_wifi.h"

#define DNS_PORT                          53
#define WIFI_DISCONNECTED_INTERVAL      5000  //  5.0 seconds
#define WIFI_CONNECTION_TIMEOUT         5000  //  5.0 seconds
#define WIFI_QUICK_PROBE_INTERVAL        300  //  0.3 second
#define WIFI_PROBE_INTERVAL            30000  // 30.0 seconds
#define PROBE_TIMEOUT_MS                3000  // TCP connect timeout (milliseconds)
#define PING_TIMEOUT_S                     1  // ICMP ping timeout (seconds)

static const IPAddress ZERO_IP = IPAddress(0,0,0,0);

class WiFiConnectionManager {
public:
  enum WIFI_STATE {
    DISCONNECTED,   // idle; waiting RECONNECT_DELAY before retrying
    CONNECTING,     // WiFi.begin() called; polling for WL_CONNECTED
    CONNECTED,      // associated; periodic probing underway
    HEALTHY         // probe succesfull; both gateway & DNS checks passing
  };

private:
  SoftTimer eventTimer;
  IPAddress gatewayIP;
  IPAddress dnsIP;
  bool gatewayProbeOK;
  bool dnsProbeOK;
  WIFI_STATE wifi_state;

public:
  static const char * toString(WIFI_STATE state) {
    switch (state) {
      case WIFI_STATE::DISCONNECTED:  return "DISCONNECTED"; break;
      case WIFI_STATE::CONNECTING  :  return "CONNECTING"  ; break;
      case WIFI_STATE::CONNECTED   :  return "CONNECTED"   ; break;
      case WIFI_STATE::HEALTHY     :  return "HEALTHY"     ; break;
      default:
        return "UNKNOWN";
    };
  }

public:
  WiFiConnectionManager() :
    gatewayProbeOK(false),
    dnsProbeOK(false),
    wifi_state(WIFI_STATE::DISCONNECTED) {
  }

  WIFI_STATE getState() const {
    return wifi_state;
  }

  bool inline isHealthy() const {
    return (wifi_state == WIFI_STATE::HEALTHY);
  }

  bool inline isConnected() const {
    return (wifi_state == WIFI_STATE::HEALTHY || wifi_state == WIFI_STATE::CONNECTED);
  }

  IPAddress getGatewayIP() const {
    return gatewayIP;
  };

  IPAddress getDnsIP() const {
    return dnsIP;
  };

  void setup() {
    // Disable the driver's own reconnect logic.
    // Use a state machine for all reconnection decisions.
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);

    // Set bandwidth to 20MHz or 40MHz
    // Options: WIFI_BW_HT20 (20MHz), WIFI_BW_HT40 (40MHz)
    esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20); 
  
    log_i("Bandwidth set to 20MHz");

    // Setup device in a disconnected state for a long period.
    wifi_state = WIFI_STATE::DISCONNECTED;

    // This will trigger connection immediately on next loop().
    timer_disable(eventTimer);
  }

  void loop() {
    switch (wifi_state) {
      case WIFI_STATE::DISCONNECTED:
        // Wait RECONNECT_DELAY, then kick off a new association.
        if (eventTimer.getTimeOutTime() == 0 || (eventTimer.getTimeOutTime() != 0 && eventTimer.hasTimedOut())) {
          // DISCONNECTED --> CONNECTING

          // Time to try/retry the connection
          log_i("WiFi connecting to '%s'.", SECRET_WIFI_SSID);
          WiFi.disconnect(true);
          WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);

          // Start a timer to detect connection timeout or to delay reconnecting.
          eventTimer.setTimeOutTime(WIFI_CONNECTION_TIMEOUT);
          eventTimer.reset();

          // Change state
          changeState(WIFI_STATE::CONNECTING);
        }
        break;

      case WIFI_STATE::CONNECTING:
        // Poll WiFi.status() every tick until associated or timed out.
        // WiFi.begin() is asynchronous; we must not call it again here.
        if (WiFi.status() == WL_CONNECTED) {
          // CONNECTING --> CONNECTED

          log_i("WiFi connected.");
          readNetworkConfig();

          // Start probe timer.
          eventTimer.setTimeOutTime(WIFI_QUICK_PROBE_INTERVAL);
          eventTimer.reset();

          // Change state
          changeState(WIFI_STATE::CONNECTED);
        }
        else if (eventTimer.getTimeOutTime() != 0 && eventTimer.hasTimedOut()) {
          // CONNECTING --> DISCONNECTED
          log_w("WiFi connection timed out.");
          toDisconnectedState();
        }
        break;

      case WIFI_STATE::CONNECTED:
        // Poll WiFi.status() every tick; drop back if the link is lost.
        if (WiFi.status() != WL_CONNECTED) {
          // CONNECTED --> DISCONNECTED
          log_w("WiFi connection is lost.");
          toDisconnectedState();
        }
        else if (eventTimer.getTimeOutTime() != 0 && eventTimer.hasTimedOut()) {
          // Time to probe
          probe();

          // Start timer to know when to probe next
          eventTimer.setTimeOutTime(WIFI_PROBE_INTERVAL);
          eventTimer.reset();

          if (gatewayProbeOK && dnsProbeOK) {
            // CONNECTED --> HEALTHY

            // Change state
            changeState(WIFI_STATE::HEALTHY);
          }
        }
        break;

      case WIFI_STATE::HEALTHY:
        // Poll WiFi.status() every tick; drop back if the link is lost.
        if (WiFi.status() != WL_CONNECTED) {
          // HEALTHY --> DISCONNECTED
          log_w("WiFi healthy connection is lost.");
          toDisconnectedState();
        }
        else if (eventTimer.getTimeOutTime() != 0 && eventTimer.hasTimedOut()) {
          // Time to probe again
          probe();

          if (!gatewayProbeOK || !dnsProbeOK) {
            // HEALTHY --> CONNECTED
            log_w("WiFi probes failed; downgrading.");

            // Change state
            changeState(WIFI_STATE::CONNECTED);
          }

          // Reset the timer regardless of outcome so probing continues
          eventTimer.reset();
        }
        break;
    }
  }

private:
  void toDisconnectedState() {
    // Reset healthy markers
    gatewayProbeOK = false;
    dnsProbeOK = false;

    // Reset IPs
    gatewayIP = ZERO_IP;
    dnsIP = ZERO_IP;

    // Start timer to stay in disconnected state for a while.
    eventTimer.setTimeOutTime(WIFI_DISCONNECTED_INTERVAL);
    eventTimer.reset();

    // Change state
    changeState(WIFI_STATE::DISCONNECTED);
  }

  void readNetworkConfig() {
    gatewayIP = WiFi.gatewayIP();
    dnsIP     = WiFi.dnsIP(0);

    log_i("WiFi MAC     : %s", WiFi.macAddress().c_str());
    log_i("WiFi IP      : %s", WiFi.localIP().toString().c_str());
    log_i("WiFi Gateway : %s", gatewayIP.toString().c_str());
    log_i("WiFi DNS     : %s", dnsIP.toString().c_str());
  }

  void changeState(WIFI_STATE new_state) {
    if (new_state == wifi_state)
      return;

    log_i("WiFi state: %s --> %s", toString(wifi_state), toString(new_state));
    wifi_state = new_state;
  }

  bool checkGateway() {
    bool ok = Ping.ping(gatewayIP, 1) > 0;
    if (!ok) {
      log_e("*** ICMP ping to WiFi gateway %s has failed.", gatewayIP.toString().c_str());
      return false;
    }

    log_i("ICMP ping to WiFi gateway %s : %.2fms", gatewayIP.toString().c_str(), Ping.averageTime());
    return ok;
  }

  bool checkDns() {
    WiFiClient client;
    client.setTimeout(PROBE_TIMEOUT_MS);
    bool ok = client.connect(dnsIP, DNS_PORT);
    if (ok) client.stop();

    if (!ok) {
      log_e("*** TCP connection to DNS %s has failed.", dnsIP.toString().c_str());
      return false;
    }

    log_i("TCP connection to DNS %s : success.", dnsIP.toString().c_str());
    return ok;
  }

  void probe() {
    gatewayProbeOK = checkGateway();
    dnsProbeOK = checkDns();
  }
};
