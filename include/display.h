#pragma once

struct DisplayData {
  char ip[16];
  float batteryPct;
  float angle;
  char imu_status[16];
};

bool displaySetup();

void displayUpdate(const DisplayData& data);
