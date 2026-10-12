#include "net.h"
#include "display.h"
#include "battery.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

static DisplayData currentData;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);

  battSetup();

  displaySetup();

  netSetup();
  snprintf(currentData.ip, sizeof(currentData.ip), "%s", netIP());

}

void loop() {
  netService();
  currentData.angle = 0.0f; // TODO: get angle from IMU
  battService();
  currentData.batteryPct = battPercentage();
  displayUpdate(currentData);

}
