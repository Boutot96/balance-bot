#pragma once
enum class BalState { Disarmed, Armed };

void balanceSetup();

void balanceStep(float angle, float rate, float dt);

bool balanceArmed();

float balanceSetpoint();

float balanceOutput();

void balanceSetGains(float kp, float ki, float kd);