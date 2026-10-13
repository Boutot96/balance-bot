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

// Bubble level: a rounded "tube" with a bubble that slides with pitch angle.
static void displayTilt(float angle) {
    const int16_t tubeX = 4,  tubeY = 28;     // top-left of the tube
    const int16_t tubeW = 120, tubeH = 14;    // size
    const int16_t r     = tubeH / 2 - 2;      // bubble radius (fits inside the tube)
    const float   range = 45.0f;              // ±45° maps to the tube's ends

    // tube outline + center mark
    oled.drawRoundRect(tubeX, tubeY, tubeW, tubeH, tubeH / 2, SSD1306_WHITE);
    int16_t cx = tubeX + tubeW / 2;
    oled.drawFastVLine(cx, tubeY - 3, 3, SSD1306_WHITE);              // tick above
    oled.drawFastVLine(cx, tubeY + tubeH, 3, SSD1306_WHITE);          // tick below

    // map angle → x position, clamped so the bubble stays inside the tube
    float a = constrain(angle- BALANCE_SETPOINT_DEG, -range, range);
    int16_t travel = tubeW / 2 - r - 3;                                // half the usable length
    int16_t bx = cx + (int16_t)(a / range * travel);
    int16_t by = tubeY + tubeH / 2;

    oled.fillCircle(bx, by, r, SSD1306_WHITE);
}


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
    oled.setCursor(0, 16);
    oled.print(current.imu_status);
    displayBattery((int)(current.batteryPct));
    displayIP(current.ip);
    
    displayTilt(current.angle);
    
    
    oled.display();
}

