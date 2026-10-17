#include "net.h"
#include "display.h"
#include "battery.h"
#include "imu.h"
#include "motor.h"
#include "config.h"
#include "balance.h"
#include "encoder.h"
#include "timing.h"
#include <Arduino.h>
#include <Wire.h>

static DisplayData currentData;

static WorkStats loopStats("loop"),  netStats("net"),   battStats("batt"),
                 encStats("enc"),    imuStats("imu+bal"), periodStats("period"),
                 eyesStats("eyes"),  statusStats("status"), reportStats("report");
static Every reportTimer(2000);

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
    loopStats.start();

    netStats.start();  netService();  netStats.stop();
    battStats.start(); battService(); battStats.stop();
    encStats.start();  encService();  encStats.stop();

    float wheelSpeed = (encSpeed(ENC_LEFT) + encSpeed(ENC_RIGHT)) / 2.0f;

    imuStats.start();
    if (imuService()) {
        static uint32_t lastRun = 0;
        uint32_t now = micros();
        if (lastRun) periodStats.add(now - lastRun);   // gap between control runs
        lastRun = now;
        balanceStep(imuAngle(), imuRate(), imuDt(), wheelSpeed);
        imuStats.stop();
    }

    eyesStats.start(); eyeService(imuAngle()); eyesStats.stop();

    currentData.batteryPct = battPercentage();
    currentData.angle      = imuAngle();
    if (!balanceArmed()) {
        statusStats.start(); displayUpdate(currentData); statusStats.stop();
    }

    loopStats.stop();

    if (reportTimer.ready()) {
        reportStats.start();
        netLog("----\n");
        loopStats.report();  periodStats.report(); imuStats.report();
        netStats.report();   battStats.report();   encStats.report();
        eyesStats.report();  statusStats.report(); displayReport();
        reportStats.report();     // prints the PREVIOUS report's time
        reportStats.stop();       // records THIS report's time
    }
}
