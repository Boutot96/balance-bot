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


static void displayBattery(int percentage){
    //char buf[5];
    int fillW = (20 * percentage) / 100;
    //snprintf(buf, sizeof(buf), "%d%%", percentage);               //Uncomment these lines if you want
    //int16_t x = OLED_WIDTH - strlen(buf) * 6 - 30;                //to display the battery percentage 
    //oled.setCursor(x, 0);                                         //as a number in text
    //oled.print(buf);
    oled.drawRect(OLED_WIDTH - 30, 0, 24, 8, SSD1306_WHITE);
    oled.fillRect(OLED_WIDTH - 30 + 24, 2, 2, 4, SSD1306_WHITE);
    oled.fillRect(100, 2, fillW, 4, SSD1306_WHITE);
}

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
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("Balance-Bot");
    displayBattery((int)(current.batteryPct));
    displayIP(current.ip);
    
    //displayTilt(current.angle);
    
    
    oled.display();
}

