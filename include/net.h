#pragma once

// Sets up Wifi, OTA, and Telnet. Call once in setup()
void netSetup();

// Returns true if Wifi is connected, false otherwise
bool netConnected();

// Writes a message to the Telnet console and Serial monitor
void netLog(const char *fmt, ...);

// Answers OTA upload requests. Is ran every loop iteration
void netService();