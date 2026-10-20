#include "display.h"
#include "config.h"
#include "timing.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Every     frameTimer(CUBE_PERIOD_MS);

static WorkStats chunkStats("oled chunk");
static WorkStats drawStats("drawCube");


static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1, 400000, 400000);
static Adafruit_SSD1306 oled2(OLED_WIDTH, OLED_HEIGHT, &Wire1,  -1, 400000, 400000);
static Adafruit_SSD1306 oled3(OLED_WIDTH, OLED_HEIGHT, &Wire1,  -1, 400000, 400000);
static bool             ready = false;   // did the screen start?
static DisplayData      current;         // latest data from main


static EyesMode eyesMode = EyesMode::Cube;
static int8_t chunk = -1; 

void eyesSetMode(EyesMode m) { eyesMode = m; }

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

// Function for controlling oled screen
static void oled_control(const DisplayData& data){
    
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

// Function for displaying a spinning cube
struct Vec3 { float x, y, z; };
static const Vec3 CUBE_VERTS[8] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},   // 0-3: back face  (z = -1)
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1},   // 4-7: front face (z = +1)
};
struct Edge { uint8_t a, b; };
static const Edge CUBE_EDGES[12] = { {0,4}, {1,5}, {2,6}, {3,7}, {0,1}, {1,2}, {2,3}, {3,0}, {4,5},{5,6},{6,7},{7,4} };

static void drawCube(){
    static float angleA = 0.0f, angleB = 0.0f;   // static: keeps its value between frames
    angleA += 0.16f;
    angleB += 0.12f;
    if (angleA > TWO_PI) angleA -= TWO_PI;
    if (angleB > TWO_PI) angleB -= TWO_PI;

    int16_t px[8], py[8];
    
    float cA = cosf(angleA);
    float sA = sinf(angleA);
    float cB = cosf(angleB);
    float sB = sinf(angleB);

    for (int i = 0; i < 8; i++) {
        float x = CUBE_VERTS[i].x;
        float y = CUBE_VERTS[i].y;
        float z = CUBE_VERTS[i].z;

        
        // 1. rotate around one axis by angleA 
        float x1 = x * cA - y *sA;
        float y1 = x * sA + y * cA;
        float z1 = z;
        // 2. rotate around another axis by angleB
        float x2 = x1 * cB - z1 *sB;
        float y2 = y1;
        float z2 = x1 * sB + z1 * cB;
        // 3. project: px[i] = ..., py[i] = ...
        px[i] = 64 + (int16_t)(y2 * CUBE_SCALE / (x2 + CUBE_DIST));
        py[i] = 32 - (int16_t)(z2 * CUBE_SCALE / (x2 + CUBE_DIST));

    }

    oled2.clearDisplay();
    for (const Edge& e : CUBE_EDGES) {
        oled2.drawLine(px[e.a], py[e.a], px[e.b], py[e.b], SSD1306_WHITE);
    }

}

// Send a list of command bytes to an SSD1306
static void oledCmd(TwoWire& bus, uint8_t addr, const uint8_t* cmds, size_t n) {
  bus.beginTransmission(addr);
  bus.write(0x00);                  // control byte: commands follow
  bus.write(cmds, n);
  bus.endTransmission();
}

// Send chunk 0..31 of the display buffer (32 bytes each, 4 per page)
static void oledSendChunk(Adafruit_SSD1306& d, TwoWire& bus, uint8_t addr, uint8_t chunk) {
  uint8_t page = chunk / 4;         // 0..7
  uint8_t col  = (chunk % 4) * 32;  // 0, 32, 64, 96

  const uint8_t win[] = { 0x21, col, (uint8_t)(col + 31),   // column range
                          0x22, page, page };                // page range
  oledCmd(bus, addr, win, sizeof(win));

  bus.beginTransmission(addr);
  bus.write(0x40);                  // control byte: pixel data follows
  bus.write(d.getBuffer() + page * 128 + col, 32);
  bus.endTransmission();
}

void eyeService(float angle){
    if(!ready) return;

    if(chunk >= 0){
        chunkStats.start();
        oledSendChunk(oled2, Wire1, OLED_ADDR2, chunk);
        chunkStats.stop();
        chunk++;
        if(chunk == 32) chunk = -1;
        return;
    }    

    float a = fabsf(constrain(angle - BALANCE_SETPOINT_DEG, -40, 40)) / 10;
    a = constrain(a, 1, 4);
    if(frameTimer.ready(CUBE_PERIOD_MS*a)) {drawStats.start(); drawCube(); drawStats.stop(); chunk = 0;}
    
}

bool displaySetup(){
    if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, true, false)){
        return false;
    }
    if(!oled2.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR2, true, false)){
        return false;
    }
    if(!oled3.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, true, false)){
        return false;
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.print("Balance-Bot");
    oled.display();

    oled2.clearDisplay();
    oled2.setTextSize(1);
    oled2.setTextColor(SSD1306_WHITE);
    oled2.setCursor(0, 0);
    oled2.print("Balance-Bot: Eyes-1");
    oled2.display();

    oled3.clearDisplay();
    oled3.setTextSize(1);
    oled3.setTextColor(SSD1306_WHITE);
    oled3.setCursor(0, 0);
    oled3.print("Balance-Bot: Eyes-2");
    oled3.display();

    ready = true;
    return ready;
}

void displayUpdate(const DisplayData& data){
    if (!ready) return;

    static uint32_t lastStatus = 0;

    if (millis() - lastStatus >= DISPLAY_PERIOD_MS) {          // status: 10 Hz
        lastStatus = millis();
        oled_control(data);
    }

}

void displayReport() { chunkStats.report(); drawStats.report(); }