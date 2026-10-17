#include "encoder.h"
#include "config.h"
#include "timing.h"

static Every encTimer(ENC_PERIOD_MS);

static volatile int32_t counts[2]       = {0,0};
static int32_t          lastCounts[2]   = {0, 0};   // counts at the last speed update
static float            speeds[2]       = {0.0f,0.0f};
static uint32_t lastRun = 0;

static portMUX_TYPE encMux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR encLeftISR(){
    if(digitalRead(ENC1_A) == digitalRead(ENC1_B))  counts[ENC_LEFT]--;
    else                                            counts[ENC_LEFT]++;
}

static void IRAM_ATTR encRightISR(){
    if(digitalRead(ENC2_A) == digitalRead(ENC2_B))  counts[ENC_RIGHT]--;
    else                                            counts[ENC_RIGHT]++;
}

void encSetup(){
    pinMode(ENC1_A, INPUT_PULLUP);
    pinMode(ENC1_B, INPUT_PULLUP);
    pinMode(ENC2_A, INPUT_PULLUP);
    pinMode(ENC2_B, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC1_A), encLeftISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC2_A), encRightISR, CHANGE);
    lastRun = micros();
}

void encUpdate(){
    uint32_t nowUs = micros();
    float dt = (nowUs - lastRun) / 1e6f;   // real seconds since last update
    if (dt <= 0.0f) return;
    lastRun = nowUs;
    const int   dirs[2] = { ENC1_DIR, ENC2_DIR };

        for (int side = 0; side < 2; side++) {

            int32_t now   = encCount(side);
            int32_t delta = now - lastCounts[side];      // how far it moved
            lastCounts[side] = now;                      // remember for next time

            float rps = (delta / dt) / ENC_COUNTS_PER_REV;   // revolutions per second
            rps *= dirs[side];                                 // forward = positive

            speeds[side] = 0.7f * speeds[side] + 0.3f * rps;   // smooth
        }
}

void encService() {
    if (encTimer.ready()) encUpdate();
}

int32_t encCount(int side)  { 
    int32_t copy = 0;
    portENTER_CRITICAL(&encMux);
    copy = counts[side];
    portEXIT_CRITICAL(&encMux);
    return copy;
}

float encSpeed(int side)    { return speeds[side]; }
