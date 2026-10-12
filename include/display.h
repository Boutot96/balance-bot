#pragma once

struct DisplayData {
  char ip[16];
  float batteryPct;
  float angle;

};

bool displaySetup();

void displayUpdate(const DisplayData& data);
