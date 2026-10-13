#include "imu.h"
#include "config.h"
#include "net.h"
#include <Wire.h>

// ---- MPU6050 registers ----
constexpr uint8_t REG_PWR_MGMT_1   = 0x6B;
constexpr uint8_t REG_CONFIG       = 0x1A;
constexpr uint8_t REG_GYRO_CONFIG  = 0x1B;
constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;   // first of 14 data registers

// ---- module state ----
static float    gyroOffset = 0.0f;   // raw gx while still (from calibration)
static float    rate       = 0.0f;   // pitch rate, deg/s
static float    angle      = 0.0f;   // pitch angle, deg
static uint32_t lastMicros = 0;      // time of last filter update

// ---- helpers ----
static bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

static int16_t read16() {
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return (int16_t)((hi << 8) | lo);
}

static bool readRaw(int16_t& ax, int16_t& ay, int16_t& az,
                    int16_t& gx, int16_t& gy, int16_t& gz) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(REG_ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(IMU_ADDR, (uint8_t)14) != 14) return false;
  ax = read16(); ay = read16(); az = read16();
  read16();                                   // temperature, unused
  gx = read16(); gy = read16(); gz = read16();
  return true;
}

// ---- public ----
bool imuSetup() {
    if (!writeReg(REG_PWR_MGMT_1,   0x00))     return false;    // wake
    delay(100); // wait for the IMU to wake up
    if (!writeReg(REG_CONFIG,       0x03))     return false;    // DLPF ~44 Hz
    if (!writeReg(REG_GYRO_CONFIG,  1 << 3))   return false;    // ±500 dps
    if (!writeReg(REG_ACCEL_CONFIG, 1 << 3))   return false;    // ±4 g

    int16_t ax, ay, az, gx, gy, gz;

    // gyro calibration: robot must be still
    float sum = 0.0f;
    for (int i = 0; i < IMU_CAL_SAMPLES; i++) {
        if (!readRaw(ax, ay, az, gx, gy, gz)) return false;
        sum += gx;
        delay(5);
    }
    gyroOffset = sum / IMU_CAL_SAMPLES;

    // start the filter from the accelerometer's angle
    if (!readRaw(ax, ay, az, gx, gy, gz)) return false;
    angle = atan2f(-ay, az) * (180.0f/3.14159f);
    lastMicros = micros();
    return true;

}

bool imuService() {
  // ---- filter: every IMU_PERIOD_MS ----
    uint32_t now = micros();
    if (now - lastMicros >= IMU_PERIOD_MS * 1000UL) {
        float dt = (now - lastMicros) / 1e6f;     // measured, seconds
        lastMicros = now;

        int16_t ax, ay, az, gx, gy, gz;
        if (readRaw(ax, ay, az, gx, gy, gz)) {
            rate = (gx - gyroOffset) / IMU_GYRO_LSB_PER_DPS * IMU_GYRO_SIGN;
            float accelAngle = atan2f(-ay, az) * (180.0f/3.1519f);
            angle = IMU_ALPHA * (angle + rate * dt) + (1.0f - IMU_ALPHA) * accelAngle;
        }
        return true;
    }
    return false;
}

float imuAngle() { return angle; }
float imuRate() { return rate; }