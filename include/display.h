#pragma once

struct DisplayData {
  char ip[16];
  float batteryPct;
  float angle;
  char imu_status[16];
};

enum class EyesMode { Cube /*, Eyes, Text ... later */ };

void eyesSetMode(EyesMode m);

void eyeService(float angle);

bool displaySetup();

void displayUpdate(const DisplayData& data);
