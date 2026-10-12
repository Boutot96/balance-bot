#pragma once

constexpr char HOSTNAME[] = "balance-bot";
constexpr int TEL_SERVER = 23;
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 19;
constexpr int WIFI_TIMEOUT_MS = 10000;


constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;
constexpr int OLED_ADDR = 0x3C;
constexpr int DISPLAY_PERIOD_MS = 100;

constexpr int BATT_PIN = 34;
constexpr float BATT_DIVIDER_RATIO = 4.0f; // voltage divider ratio for battery voltage measurement
constexpr float BATT_CALIBRATION = 1.0f; // calibration factor for battery voltage measurement
constexpr float BATT_CUTOFF_VOLTAGE = 6.8f; // voltage at which the battery is considered empty
constexpr float BATT_FULL_VOLTAGE = 8.4f; // voltage at which the battery is considered full
constexpr int BATT_PERIOD_MS = 1000; // period for battery monitoring in milliseconds
constexpr int BATT_SAMPLE_COUNT = 25; // number of samples to average for battery voltage measurement
constexpr int BATT_ADC_SAMPLES = 16; // number of ADC samples to average for battery voltage measurement