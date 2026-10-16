#include "motor.h"
#include "config.h"
#include "net.h"
#include "battery.h"
#include "imu.h"
#include "balance.h"
#include <Arduino.h>

enum class BalState { Disarmed, Armed };

static BalState balState     = BalState::Disarmed;
static uint32_t uprightSince = 0;
static float integral = 0.0f; 
static uint32_t satSince = 0;
static float speedI = 0.0f;
static float shift = 0.0f;
static float setpoint = BALANCE_SETPOINT_DEG;

static void disarm(const char* reason) {
  motorStop();
  balState     = BalState::Disarmed;
  uprightSince = 0;
  satSince     = 0;
  shift        = 0.0f; 
  setpoint     = BALANCE_SETPOINT_DEG;
  netLog("DISARMED: %s\n", reason);
}

bool balanceArmed(){
    return balState == BalState::Armed;
}

void balanceStep(float angle, float rate, float dt, float wheelSpeed){    
    float tiltErr = angle - BALANCE_SETPOINT_DEG;   // safety: distance from upright

    switch (balState) {
    case BalState::Disarmed:
      motorStop();
      if (fabsf(tiltErr) < ARM_WINDOW_DEG && !battIsLow()) {
        if (uprightSince == 0) uprightSince = millis();
        else if (millis() - uprightSince >= ARM_HOLD_MS) {
          balState = BalState::Armed;
          integral = 0.0f;
          speedI = 0.0f;
          netLog("ARMED\n");
        }
      } 
      else {
        uprightSince = 0;                    // left the window, so restart the hold timer
      }
      break;

    case BalState::Armed: {
      if (fabsf(tiltErr) > DISARM_DEG) { disarm("fell");        break; }
      if (battIsLow())               { disarm("battery low"); break; }
        
      float speedErr = -wheelSpeed; 
      speedI += speedErr * dt;
      speedI = constrain(speedI, -SPD_I_LIMIT, SPD_I_LIMIT);
      shift = constrain(SPD_SIGN * (SPD_KP * speedErr + SPD_KI * speedI), -SPD_MAX_TILT, SPD_MAX_TILT);


      setpoint = BALANCE_SETPOINT_DEG + shift;
      float error = angle - setpoint; 
      integral += error * dt;
      integral  = constrain(integral, -BAL_I_LIMIT, BAL_I_LIMIT);

      float out = BAL_SIGN * (BAL_KP * error + BAL_KI * integral + BAL_KD * rate);

      // pinned at full power too long = it can't recover, so give up
      if (fabsf(out) >= MOT_PWM_MAX) { if (!satSince) satSince = millis(); }
      else                           { satSince = 0; }
      if (satSince && millis() - satSince > 300) { disarm("saturated"); break; }

      motorSet((int)out, (int)out);
      break;
      }
    }
}

float balanceSetpoint() { return setpoint; }
float balanceShift()    { return shift; }