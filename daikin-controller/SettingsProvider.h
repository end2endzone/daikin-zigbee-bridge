#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "format_helper.h"

#define SETTINGS_STORE_NAME "settings"

class SettingsProvider
{
public:
  struct Settings {
    String daikin_ip;               // IP of the associated Daikin HEATPUMP on the network
    String wifi_ssid;               // Name/SSID of the WiFi network
    String wifi_password;           // Password  of the WiFi network
  };

private:
  Settings settings;
  Preferences prefs;
  WebServer server;
  bool apMode = false;

public:
  SettingsProvider() : server(80) {}

  void begin() {
    bool success = readSettings(settings);
    if (!success) {
      // Invalid preference settings
      startAPMode();
      setupWebServer();
    } else {
      log_i("Found existing preference settings: %s", toJson(settings).c_str());
    }
  }

  void loop() {
    if (apMode) {
      server.handleClient();
    }
  }

  bool isWifiAccessPointModeEnabled() const {
    return apMode;
  }

  Settings getSettings() {
    return settings;
  }
  const Settings & getSettings() const {
    return settings;
  }

  void factoryReset() {
    log_i("Erasing preferences...");

    prefs.begin(SETTINGS_STORE_NAME, false);
    prefs.clear();              // Erase all keys in this namespace
    prefs.end();

    log_i("Preferences erased. Reboot required.");
    delay(1000);
    ESP.restart();
  }

  static bool isValid(const Settings & s) {
    if (s.daikin_ip.length() == 0)              return false;
    if (s.wifi_ssid.length() == 0)              return false;
    if (s.wifi_password.length() == 0)          return false;
    return true;
  }

  static String toJson(const Settings & s) {
    String output;
    output = strformat("{\"wifi_ssid\":\"%s\", \"wifi_password\":\"%s\", \"daikin_ip\":\"%s\"}",
            s.wifi_ssid.c_str(),
            hidePassword(s.wifi_password).c_str(),
            s.daikin_ip.c_str()
          );
    return output;
  }
  
  static String toJsonFull(const Settings & s) {
    String output;
    output = strformat("{\"wifi_ssid\":\"%s\", \"wifi_password\":\"%s\", \"daikin_ip\":\"%s\"}",
            s.wifi_ssid.c_str(),
            s.wifi_password.c_str(),
            s.daikin_ip.c_str()
          );
    return output;
  }

  static String toString(const Settings & s) {
    String output;
    output = strformat("{\n"
            "  WiFi network SSID:  %s\n"
            "  Daikin heatpump IP: %s\n"
            "}",
            s.wifi_ssid.c_str(),
            s.daikin_ip.c_str()
          );
    return output;
  }
  
  static String toStringDetailed(const Settings & s) {
    String output;
    output = strformat("{\n"
            "  WiFi network SSID:     %s\n"
            "  WiFi network password: %s\n"
            "  Daikin heatpump IP:    %s\n"
            "}",
            s.wifi_ssid.c_str(),
            s.wifi_password.c_str(),
            s.daikin_ip.c_str()
          );
    return output;
  }


private:

  bool readSettings(Settings & settings) {
    prefs.begin(SETTINGS_STORE_NAME, true); // Open in Read-Only mode
    settings.daikin_ip =              prefs.getString("daikin_ip", "");
    settings.wifi_ssid =              prefs.getString("wifi_ssid", "");
    settings.wifi_password =          prefs.getString("wifi_password", "");
    prefs.end();

    return isValid(settings);
  }

  // Return last 6 hex digits of MAC address
  String getMacSuffix() {
    // Get full 48-bit MAC from efuse
    uint64_t mac = ESP.getEfuseMac();

    // Take the lower 24 bits (last 6 hex digits)
    uint32_t suffix = (uint32_t)(mac & 0xFFFFFF);

    char buf[7];  // 6 hex chars + null terminator
    sprintf(buf, "%06lX", suffix);

    return String(buf);
  }

  void startAPMode() {
    apMode = true;

    log_i("No settings saved in Non-Volatile Storage (NVS). Starting Access Point...");
    
    String apName = "DAIKINCTRL-" + getMacSuffix();
    
    WiFi.mode(WIFI_AP); // Switch to AP mode AFTER reading MAC
    WiFi.softAP(apName.c_str());

    String localIp = WiFi.softAPIP().toString(); // usually 192.168.4.1

    log_i("AP started: %s", apName.c_str());
    log_i("AP IP: %s", localIp.c_str());
    log_i("Connect and browse to http://%s/ or http://%s/", apName.c_str(), localIp.c_str());
  }

  void setupWebServer() {
    server.on("/", HTTP_GET, [this]() {
      String html = "<html><head><title>Daikin Controller</title></head><body>"
                    "<h2>Daikin Controller</h2>"
                    "<p>Enter configuration settings:</p>"
                    "<form action='/save' method='POST'>"
                    "WiFi SSID: <input name='wifi_ssid' type='text'></br>"
                    "WiFi password: <input name='wifi_password' type='password'></br>"
                    "Daikin heatpump IP: <input name='daikin_ip' type='text'></br>"
                    "<input type='submit' value='Save'>"
                    "</form>"
                    "</body></html>";
      server.send(200, "text/html", html);
    });

    server.on("/save", HTTP_POST, [this]() {
      if (server.hasArg("daikin_ip") &&
          server.hasArg("wifi_ssid") &&
          server.hasArg("wifi_password")) {
        String daikin_ip = server.arg("daikin_ip");
        String wifi_ssid = server.arg("wifi_ssid");
        String wifi_password = server.arg("wifi_password");

        // Save to Preferences
        prefs.begin(SETTINGS_STORE_NAME, false); // Open in Write mode
        prefs.putString("daikin_ip", daikin_ip);
        prefs.putString("wifi_ssid", wifi_ssid);
        prefs.putString("wifi_password", wifi_password);
        prefs.end();

        // Test
        Settings tmp;
        bool test_read_success = readSettings(tmp);
        String settings_json = SettingsProvider::toJsonFull(tmp);
        if (test_read_success) {
          String response = strformat(
            "<html><body>"
            "<h2>Settings saved:</h2>"
            "<pre>%s</pre>"
            "<p>Rebooting...</p>"
            "</body></html>", settings_json.c_str());
          server.send(200, "text/html", response);

          log_i("Settings saved: %s", settings_json.c_str());

          delay(2000);
          ESP.restart();
        } else {
          String text_response = strformat("Failed to save these settings: %s", settings_json.c_str());
          
          log_w("%s", text_response.c_str());

          server.send(400, "text/plain", text_response);
        }
      } else {
        server.send(400, "text/plain", "Missing IP");
      } 
    });

    server.begin();
    log_i("Web server started");
  }
};
