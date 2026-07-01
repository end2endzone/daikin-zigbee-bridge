#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

class PreferencesProvider
{
private:
  Preferences prefs;
  WebServer server;
  String savedIP;
  bool apMode = false;

public:
  PreferencesProvider() : server(80) {}

  void begin() {
    prefs.begin("config", true); // Open in Read-Only mode
    savedIP = prefs.getString("ip", "");
    prefs.end();

    if (savedIP.length() == 0) {
      startAPMode();
      setupWebServer();
    } else {
      log_i("Saved IP found: %s", savedIP.c_str());
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

  String getSavedIP() const {
    return savedIP;
  }

  void factoryReset() {
    log_i("Erasing preferences...");

    prefs.begin("config", false);
    prefs.clear();              // Erase all keys in this namespace
    prefs.end();

    savedIP = "";               // Reset cached value
    apMode = false;             // Stop AP mode if active

    log_i("Preferences erased. Reboot required.");
    delay(1000);
    ESP.restart();
  }
  
private:
  void startAPMode() {
    apMode = true;
    log_i("No saved IP. Starting Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_Config_AP");
    log_i("AP started: ESP32_Config_AP");
    log_i("Connect and browse to http://192.168.4.1");
  }

  void setupWebServer() {
    server.on("/", HTTP_GET, [this]() {
      String html = "<html><body>"
                    "<h2>Enter IP Address</h2>"
                    "<form action='/save' method='POST'>"
                    "IP: <input name='ip' type='text'>"
                    "<input type='submit' value='Save'>"
                    "</form>"
                    "</body></html>";
      server.send(200, "text/html", html);
    });

    server.on("/save", HTTP_POST, [this]() {
      if (server.hasArg("ip")) {
        String ip = server.arg("ip");

        // Save to Preferences
        prefs.begin("config", false); // Open in Write mode
        prefs.putString("ip", ip);
        prefs.end();

        savedIP = ip;

        String html = "<html><body>"
                      "<h2>IP Saved!</h2>"
                      "<p>Rebooting...</p>"
                      "</body></html>";
        server.send(200, "text/html", html);

        log_i("IP saved: %s", ip.c_str());

        delay(2000);
        ESP.restart();
      } else {
        server.send(400, "text/plain", "Missing IP");
      } 
    });

    server.begin();
    log_i("Web server started");
  }
};
