#include "display.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1, 400000, 400000);
static bool             ready = false;   // did the screen start?
static DisplayData      current;         // latest data from main


static void displayIP(const char* ip){
    oled.setCursor(0, 48);
    oled.println(ip);

}

//static void displayBattery();

//static void displayTilt(float angle);


bool displaySetup(){
    if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, true, false)){
        return false;
    }
    oled.stopscroll();
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.print("Balance-Bot");
    oled.display();
    ready = true;
    return ready;
}

void displayUpdate(const DisplayData& data){
    static uint32_t lastUpdate = 0;
    if(!ready) return;
    if(millis() - lastUpdate < DISPLAY_PERIOD_MS) return;
    lastUpdate = millis();
    current = data;

    displayIP(current.ip);
    //displayBattery(current.batteryPct);
    //displayTilt(current.angle);
    
    
    oled.display();
}

