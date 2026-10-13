#pragma once

bool imuSetup();  // Initializes the IMU (MPU6050) sensor

bool imuService();  // Periodically updates the IMU readings and calculates the pitch angle

float imuAngle();  // Retrieves the current pitch angle calculated from the IMU readings

float imuRate();  // Retrieves the current pitch rate (angular velocity) from the IMU readings