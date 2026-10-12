#include "battery.h"
#include "config.h"
#include <Arduino.h>

static float batteryVoltage = 0.0f;
static float readings[BATT_SAMPLE_COUNT] = {0.0f};
static int batt_index = 0;

float battVoltage(){

    return batteryVoltage;
}

int battPercentage() {
    float pct = (batteryVoltage - BATT_CUTOFF_VOLTAGE) / (BATT_FULL_VOLTAGE - BATT_CUTOFF_VOLTAGE) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return (int)pct;
}

bool battIsLow() {
    return batteryVoltage < BATT_CUTOFF_VOLTAGE;
}

static float readBatteryOnce() {
  uint32_t sumMv = 0;
  for (int i = 0; i < BATT_ADC_SAMPLES; i++) {                              // One battery reading: averages several ADC samples to reduce noise,
    sumMv += analogReadMilliVolts(BATT_PIN);                                // then converts pin millivolts to battery volts.
  }
  float pinVolts = (sumMv / (float)BATT_ADC_SAMPLES) / 1000.0f;
  return pinVolts * BATT_DIVIDER_RATIO * BATT_CALIBRATION;
}

void battSetup() {
    float temp = 0.0f;
    analogSetPinAttenuation(BATT_PIN, ADC_11db);

    for (float &r : readings) {
        r = readBatteryOnce();                                                                  // read a new battery voltage reading
        temp += r;
    }
    batteryVoltage = temp / BATT_SAMPLE_COUNT;                                                  // average the readings
}

void battService() {
    float reading = 0.0f;
    float sum = 0.0f;
    uint32_t time = millis();
    static uint32_t lastUpdate = 0;
    if (time - lastUpdate < BATT_PERIOD_MS) return;                                             // update every second
    lastUpdate = time;                                                                          // update the last update time
    reading = readBatteryOnce();                                                                // convert to volts
    readings[batt_index] = reading;                                                             // store the reading in the array
    batt_index = (batt_index + 1) % BATT_SAMPLE_COUNT;                                          // increment batt_index and wrap around
    for (float r : readings) {                                                                  //
        sum += r;                                                                               //
    }                                                                                           // averaging readings
    batteryVoltage = sum / BATT_SAMPLE_COUNT;                                                   //
}