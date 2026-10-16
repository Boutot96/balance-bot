#include "net.h"
#include "display.h"
#include "battery.h"
#include "imu.h"
#include "motor.h"
#include "config.h"
#include "balance.h"
#include "encoder.h"
#include <Arduino.h>
#include <Wire.h>

static DisplayData currentData;

void setup() {
  motorSetup();
  motorSet(150, 150);
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
  netOnUploadStart(motorStop);
  snprintf(currentData.ip, sizeof(currentData.ip), "%s", netIP());
  encSetup();
}

void loop() {
  netService();
  battService();
  //if (imuService()) balanceStep(imuAngle(), imuRate(), imuDt());
  encService();

  static uint32_t lastLog = 0;
  static uint32_t stop = 0;
  if (millis() - lastLog >= 200) {
      lastLog = millis();
      netLog("L %.2f  R %.2f rev/s\n", encSpeed(ENC_LEFT), encSpeed(ENC_RIGHT));
  }
  if (millis() - stop >= 25000) {
    motorStop();
  }
  currentData.batteryPct = battPercentage();
  currentData.angle      = imuAngle();
  if (!balanceArmed()) displayUpdate(currentData);
}
