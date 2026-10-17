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

struct LogMsg { char text[96]; };
static QueueHandle_t logQueue;

// logger task: sleeps until a message arrives, then sends it
static void loggerTask(void*) {
    LogMsg m;
    for (;;) {
        xQueueReceive(logQueue, &m, portMAX_DELAY);   // block until something arrives
        // udp.beginPacket / write / endPacket — the ONLY place UDP is used
        if (telnetClient && telnetClient.connected()) {
            udp.beginPacket(telnetClient.remoteIP(), 4210);
            udp.write((const uint8_t*)m.text, strlen(m.text));
            udp.endPacket();
        }
        // else: drop it rather than block
    }
}

void netSetup() {
    logQueue = xQueueCreate(16, sizeof(LogMsg));
    xTaskCreatePinnedToCore(loggerTask, "logger", 4096, nullptr, 1, nullptr, 0);
    
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
    if (!logQueue) return;                                 // not started yet
    LogMsg m;
    va_list args;
    va_start(args, fmt);
    vsnprintf(m.text, sizeof(m.text), fmt, args);          // always null-terminated
    va_end(args);
    xQueueSend(logQueue, &m, 0);                           // full? drop it, never wait
}

const char* netIP() {
    snprintf(ipText, sizeof(ipText), "%s", WiFi.localIP().toString().c_str());
    return ipText;
}

void netOnUploadStart(void (*callback)()) {
  uploadStartCallback = callback;
}