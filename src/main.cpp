#include "net.h"
#include "display.h"
#include "battery.h"
#include "imu.h"
#include "motor.h"
#include "config.h"
#include "balance.h"
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

}

void loop() {
  netService();
  battService();

  //if (imuService()) balanceStep(imuAngle(), imuRate(), imuDt());

  currentData.batteryPct = battPercentage();
  currentData.angle      = imuAngle();
  if (!balanceArmed()) displayUpdate(currentData);
}
