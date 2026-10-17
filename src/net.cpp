#include "net.h"
#include "config.h"
#include "secrets.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <stdarg.h>
#include <WiFiUdp.h>
static WiFiUDP udp;

static char ipText[16] = "";   

static WiFiServer telnetServer(TEL_SERVER);
static WiFiClient telnetClient;
static void (*uploadStartCallback)() = nullptr;

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
    ArduinoOTA.onStart([]() {
        if (uploadStartCallback) uploadStartCallback();   // e.g. stop motors
            netLog("OTA update starting...\n");
        });
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
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len <= 0) return;
    if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;   // message was cut off; send what fit

    if (telnetClient && telnetClient.connected()) {
        udp.beginPacket(telnetClient.remoteIP(), 4210);
        udp.write((const uint8_t*)buf, len);
        udp.endPacket();
    }
    // else: drop it rather than block
}

const char* netIP() {
    snprintf(ipText, sizeof(ipText), "%s", WiFi.localIP().toString().c_str());
    return ipText;
}

void netOnUploadStart(void (*callback)()) {
  uploadStartCallback = callback;
}