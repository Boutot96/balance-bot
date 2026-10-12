#include "net.h"
#include "config.h"
#include "secrets.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <stdarg.h>

static WiFiServer telnetServer(TEL_SERVER);
static WiFiClient telnetClient;

void netSetup() {

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    
    if (WiFi.waitForConnectResult(WIFI_TIMEOUT_MS) != WL_CONNECTED) {
        return;
    }
   
    netLog("Ready\nIP address: %s\n", WiFi.localIP().toString().c_str());


    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASS);
    ArduinoOTA.onStart([]() { netLog("OTA update starting...\n"); });
    ArduinoOTA.onEnd([]()   { netLog("\nOTA done, rebooting.\n"); });
    ArduinoOTA.onError([](ota_error_t e) { netLog("OTA error %u\n", e); });
    ArduinoOTA.begin();

    telnetServer.begin();
    telnetServer.setNoDelay(true);
}

bool netConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void netService() {
    if(!netConnected()) return;

    ArduinoOTA.handle();

    if (telnetServer.hasClient()) {
        if (telnetClient && telnetClient.connected()) telnetClient.stop();
            telnetClient = telnetServer.accept();
            telnetClient.println("Connected to balancebot");
        }
    
}

void netLog(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Serial.print(buf);
    if(telnetClient && telnetClient.connected()) telnetClient.print(buf);
}
