#pragma once
#include <Arduino.h>

void encSetup();

void encService();

int32_t encCount(int side);

float encSpeed(int side);