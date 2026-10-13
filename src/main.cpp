#include "net.h"
#include "display.h"
#include "battery.h"
#include "imu.h"
#include "motor.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

static DisplayData currentData;

void setup() {
  motorSetup();
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!imuSetup()) {
    snprintf(currentData.imu_status, sizeof(currentData.imu_status), "Failed");
  }
  else {
    snprintf(currentData.imu_status, sizeof(currentData.imu_status), "Successful");
  }

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
  imuService();
  currentData.angle = imuAngle();
  displayUpdate(currentData);

}
