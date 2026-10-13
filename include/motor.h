#pragma once

void motorSetup(); // Set motors pins to LOW and configure PWM. Must be called first thing in setup()

void motorSet(int left, int right); // Sets left and right motor speeds

void motorStop(); // Shut off motors. Safe to call anytime.