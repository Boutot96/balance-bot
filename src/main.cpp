#include "net.h"
#include "display.h"
#include "battery.h"
#include "imu.h"
#include "motor.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

static DisplayData currentData;

// --- needs seperat module -----

enum class BalState { Disarmed, Armed };
static BalState balState     = BalState::Disarmed;
static uint32_t uprightSince = 0;
static float integral = 0.0f; 
static uint32_t satSince = 0;

static void disarm(const char* reason) {
  motorStop();
  balState     = BalState::Disarmed;
  uprightSince = 0;
  satSince     = 0;
  netLog("DISARMED: %s\n", reason);
}          // with balState / uprightSince

static void balanceStep() {
  float error = imuAngle() - BALANCE_SETPOINT_DEG;
  

  switch (balState) {
    case BalState::Disarmed:
      motorStop();
      if (fabsf(error) < ARM_WINDOW_DEG && !battIsLow()) {
        if (uprightSince == 0) uprightSince = millis();
        else if (millis() - uprightSince >= ARM_HOLD_MS) {
          balState = BalState::Armed;
          integral = 0.0f;
          netLog("ARMED\n");
        }
      } else {
        uprightSince = 0;                    // left the window, so restart the hold timer
      }
      break;

    case BalState::Armed: {
      if (fabsf(error) > DISARM_DEG) { disarm("fell");        break; }
      if (battIsLow())               { disarm("battery low"); break; }

      const float dt = IMU_PERIOD_MS / 1000.0f;
      integral += error * dt;
      integral  = constrain(integral, -BAL_I_LIMIT, BAL_I_LIMIT);

      float out = BAL_SIGN * (BAL_KP * error + BAL_KI * integral + BAL_KD * imuRate());

      // pinned at full power too long = it can't recover, so give up
      if (fabsf(out) >= MOT_PWM_MAX) { if (!satSince) satSince = millis(); }
      else                           { satSince = 0; }
      if (satSince && millis() - satSince > 300) { disarm("saturated"); break; }

      motorSet((int)out, (int)out);
      break;
    }
  }
}

// -----------------------

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

  if (imuService()) balanceStep();

  currentData.batteryPct = battPercentage();
  currentData.angle      = imuAngle();
  if (balState == BalState::Disarmed) displayUpdate(currentData);
}
