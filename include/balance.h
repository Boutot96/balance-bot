#pragma once


void balanceSetup();

void balanceStep(float angle, float rate, float dt, float wheelSpeed);

bool balanceArmed();

float balanceOutput();

void balanceSetGains(float kp, float ki, float kd);

float balanceSetpoint();
float balanceWantSpeed();
float balanceTargetSpeed();
float balanceShift();