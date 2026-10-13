#include "motor.h"
#include "config.h"
#include <Arduino.h>

// Drives one motor. speed: -MOT_PWM_MAX..+MOT_PWM_MAX, already direction-corrected.
// MDD3A: forward = A gets PWM, B is 0.  reverse = A is 0, B gets PWM.  both 0 = stop.
static void driveOne(uint8_t chA, uint8_t chB, int speed) {
  if(speed > 0){
    ledcWrite(chB, 0);
    ledcWrite(chA, speed);
  }
  else{
    ledcWrite(chA, 0);
    ledcWrite(chB, -speed);
  }
}

void motorSetup() {

  pinMode(MOT_PIN_1A, OUTPUT);  digitalWrite(MOT_PIN_1A, LOW);
  pinMode(MOT_PIN_1B, OUTPUT);  digitalWrite(MOT_PIN_1B, LOW);
  pinMode(MOT_PIN_2A, OUTPUT);  digitalWrite(MOT_PIN_2A, LOW);
  pinMode(MOT_PIN_2B, OUTPUT);  digitalWrite(MOT_PIN_2B, LOW);

  ledcSetup(MOT_CH_1A, MOT_PWM_HZ, MOT_PWM_BITS);    // configure channel
  ledcAttachPin(MOT_PIN_1A, MOT_CH_1A);
  ledcSetup(MOT_CH_1B, MOT_PWM_HZ, MOT_PWM_BITS);    
  ledcAttachPin(MOT_PIN_1B, MOT_CH_1B);             
  ledcSetup(MOT_CH_2A, MOT_PWM_HZ, MOT_PWM_BITS);   
  ledcAttachPin(MOT_PIN_2A, MOT_CH_2A);             
  ledcSetup(MOT_CH_2B, MOT_PWM_HZ, MOT_PWM_BITS);    
  ledcAttachPin(MOT_PIN_2B, MOT_CH_2B);             

  motorStop();

}

void motorSet(int left, int right) {
  // 1. clamp both to -MOT_PWM_MAX..+MOT_PWM_MAX   (constrain())
  left  = constrain(left , -MOT_PWM_MAX, MOT_PWM_MAX);
  right = constrain(right, -MOT_PWM_MAX, MOT_PWM_MAX);
  // 2. apply direction signs:  left * MOT1_DIR,  right * MOT2_DIR
  left  *= MOT1_DIR;
  right *= MOT2_DIR;
  // 3. driveOne for motor 1 with its two channels, and motor 2 with its two
  driveOne(MOT_CH_1A, MOT_CH_1B, left);    // motor 1 = left
  driveOne(MOT_CH_2A, MOT_CH_2B, right);   // motor 2 = right
}

void motorStop() {
    ledcWrite(MOT_CH_1A, 0);
    ledcWrite(MOT_CH_2A, 0);
    ledcWrite(MOT_CH_1B, 0);
    ledcWrite(MOT_CH_2B, 0);
}
