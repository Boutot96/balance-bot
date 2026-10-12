#pragma once


void battSetup();   // initializes the battery monitoring system. It should be called once during the setup phase of the program.

void  battService();  // periodically checks the battery status and updates the battery percentage. It should be called in the main loop of the program. 

float battVoltage();  // returns the current battery voltage. 

int  battPercentage();  // returns the current battery percentage as 0 - 100. 

bool battIsLow();  // checks if the battery is low. returns true if the battery is considered low, and false otherwise.