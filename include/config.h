#pragma once
#include <stdint.h>

// ----- Network Configuration -----
constexpr char HOSTNAME[] = "balance-bot";
constexpr int TEL_SERVER = 23;
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 19;
constexpr int WIFI_TIMEOUT_MS = 10000;

// ----- Display Configuration -----
constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;
constexpr int OLED_ADDR = 0x3C;
constexpr int OLED_ADDR2 = 0x3D;
constexpr int DISPLAY_PERIOD_MS = 100;
constexpr int CUBE_PERIOD_MS = 30;
constexpr float CUBE_SCALE = 80.0f;
constexpr float CUBE_DIST  = 4.0f;


// ----- Battery Configuration -----
constexpr int BATT_PIN = 34;
constexpr float BATT_DIVIDER_RATIO = 4.0f; // voltage divider ratio for battery voltage measurement
constexpr float BATT_CALIBRATION = 1.0f; // calibration factor for battery voltage measurement
constexpr float BATT_CUTOFF_VOLTAGE = 6.8f; // voltage at which the battery is considered empty
constexpr float BATT_FULL_VOLTAGE = 8.4f; // voltage at which the battery is considered full
constexpr int BATT_PERIOD_MS = 1000; // period for battery monitoring in milliseconds
constexpr int BATT_SAMPLE_COUNT = 25; // number of samples to average for battery voltage measurement
constexpr int BATT_ADC_SAMPLES = 16; // number of ADC samples to average for battery voltage measurement


// ----- IMU (MPU6050) -----
constexpr uint8_t  IMU_ADDR             = 0x68;    // I2C address (0x69 if AD0 is pulled high)
constexpr uint32_t IMU_PERIOD_MS        = 5;       // filter update period (200 Hz)
constexpr float    IMU_ALPHA            = 0.98f;   // complementary filter: weight on the gyro
constexpr float    IMU_GYRO_SIGN        = -1.0f;   // flips pitch rate so it matches the accel angle direction
constexpr int      IMU_CAL_SAMPLES      = 500;     // gyro readings averaged at startup (robot must be still)

// Scales MUST match the range settings written in imuSetup():
//   gyro  ±500 dps → 65.5 LSB per deg/s   (raw / 65.5  = deg/s)
//   accel ±4 g     → 8192 LSB per g       (raw / 8192  = g)
constexpr float    IMU_GYRO_LSB_PER_DPS = 65.5f;
constexpr float    IMU_ACCEL_LSB_PER_G  = 8192.0f;



// ----- Motors -----
constexpr int MOT_PIN_1A = 32;
constexpr int MOT_PIN_1B = 33;
constexpr int MOT_PIN_2A = 25;
constexpr int MOT_PIN_2B = 26;
constexpr uint32_t MOT_PWM_HZ    = 20000;   // 20 kHz: above hearing, so no motor whine
constexpr uint8_t  MOT_PWM_BITS  = 8;       // duty 0–255
constexpr int      MOT_PWM_MAX   = (1 << MOT_PWM_BITS) - 1;   // 255, derived so it can't drift
constexpr int MOT_DEADBAND = 18;   // ~80% of measured 30 (first wheel movement)

// LEDC channels (core 2.x assigns PWM by channel, one per pin)
constexpr uint8_t  MOT_CH_1A     = 0;
constexpr uint8_t  MOT_CH_1B     = 1;
constexpr uint8_t  MOT_CH_2A     = 2;
constexpr uint8_t  MOT_CH_2B     = 3;

// Direction signs so +speed = forward on both wheels (from the direction test)
constexpr int      MOT1_DIR      = -1;
constexpr int      MOT2_DIR      = +1;


// ----- Balance -----
constexpr float    ARM_WINDOW_DEG = 3.0f;    // must be this close to setpoint to arm
constexpr uint32_t ARM_HOLD_MS    = 500;     // ...for this long
constexpr float    DISARM_DEG     = 16.0f;   // beyond this, give up (it's fallen)
constexpr float    BAL_KP         = 13.0f;   // start low
constexpr float    BAL_KD         = 0.5f;    // add after Kp
constexpr int      BAL_SIGN       = -1;      // flip if wheels drive the wrong way
constexpr float    BAL_KI      = 0.0f;    // start small
constexpr float    BAL_I_LIMIT = 50.0f;   // anti-windup: max |integral|
constexpr float    BALANCE_SETPOINT_DEG = -4.5f;

// ----- Speed -----
constexpr float SPD_KP        = 5.0f;   // degrees of lean per rev/s; start small
constexpr float SPD_KI        = 1.0f;   // leave at 0 until P works
constexpr float SPD_I_LIMIT   = 5.0f;   // clamp on the speed integral
constexpr float SPD_MAX_TILT  = 6.0f;   // max degrees the speed loop may shift the setpoint
constexpr int   SPD_SIGN      = -1;     // flip if it accelerates instead of braking


// ----- Encoders -----
constexpr int ENC1_A = 16;   // left wheel (driver side)
constexpr int ENC1_B = 18;
constexpr int ENC2_A = 4;    // right wheel (passenger side)
constexpr int ENC2_B = 17;
constexpr int ENC1_DIR = +1;
constexpr int ENC2_DIR = +1;   // mirrored motor: adjust after testing
constexpr uint32_t ENC_PERIOD_MS = 20;
constexpr int ENC_LEFT  = 0;
constexpr int ENC_RIGHT = 1;
constexpr float ENC_COUNTS_PER_REV = 340.0f;   // A both edges

// ------ Teleop ------
constexpr uint32_t TELEOP_TIMEOUT_MS = 300;
constexpr float MAX_DRIVE_SPEED = 1.0f;
constexpr float MAX_TURN_PWM = 30.0f;
constexpr float MAX_ACCEL = 1.0f;