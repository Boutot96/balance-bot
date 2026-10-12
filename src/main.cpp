#include "net.h"
#include "display.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

static DisplayData currentData;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);

  displaySetup();

  netSetup();

}

void loop() {
  netService();
  currentData.batteryPct = 0.0f; // TODO: get battery percentage
  currentData.angle = 0.0f; // TODO: get angle from IMU
  snprintf(currentData.ip, sizeof(currentData.ip), "%s", netIP());
  displayUpdate(currentData);

}
