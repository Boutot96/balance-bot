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

static TaskHandle_t controlHandle = nullptr;

static void onOtaStart() {
    if (controlHandle) vTaskSuspend(controlHandle);   // control task stops running
    motorStop();                                      // now nothing can restart them
}

static void controlTask(void* param){
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t PERIOD = pdMS_TO_TICKS(IMU_PERIOD_MS);
  uint8_t encDiv = 0;
  uint32_t lastRun = micros();

  for(;;){
    vTaskDelayUntil(&lastWake, PERIOD);     // sleep until next 5ms mark

    uint32_t now = micros();
    periodStats.add(now - lastRun);
    lastRun = now;

    imuStats.start();
    if(++encDiv >= ENC_PERIOD_MS / IMU_PERIOD_MS) {
      encDiv = 0;
      encUpdate();
    }

    imuUpdate();
    float wheelSpeed = (encSpeed(ENC_LEFT) + encSpeed(ENC_RIGHT)) / 2.0f;
    balanceStep(imuAngle(), imuRate(), imuDt(), wheelSpeed);
    imuStats.stop();

  }


}

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
  netOnUploadStart(onOtaStart);
  snprintf(currentData.ip, sizeof(currentData.ip), "%s", netIP());
  encSetup();

  xTaskCreatePinnedToCore(
    controlTask,      // function
    "control",        // name
    4096,             // stack, bytes
    nullptr,          // param
    5,                // priority (loop() is 1)
    &controlHandle,   // handle
    1                 // core
  );

}

void loop() {
    loopStats.start();

    netStats.start();  netService();  netStats.stop();
    battStats.start(); battService(); battStats.stop();

    

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
