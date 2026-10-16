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
  encService();
 
  float wheelSpeed = (encSpeed(ENC_LEFT) + encSpeed(ENC_RIGHT)) / 2.0f;

  if (imuService()) balanceStep(imuAngle(), imuRate(), imuDt(), wheelSpeed);

  static uint32_t lastLog = 0;
  if (balanceArmed() && millis() - lastLog >= 100) {
      lastLog = millis();

      netLog("sp %.2f  spd %.2f  ang %.2f\n",
            balanceSetpoint(), wheelSpeed, imuAngle());
  }

  currentData.batteryPct = battPercentage();
  currentData.angle      = imuAngle();
  if (!balanceArmed()) displayUpdate(currentData);
}
