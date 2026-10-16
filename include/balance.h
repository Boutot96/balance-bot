#pragma once


void balanceSetup();

void balanceStep(float angle, float rate, float dt, float wheelSpeed);

bool balanceArmed();

float balanceSetpoint();

float balanceOutput();

void balanceSetGains(float kp, float ki, float kd);