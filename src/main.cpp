#include "net.h"
#include <Arduino.h>

constexpr int timeout = 2000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  netSetup();

}

void loop() {
  static uint32_t last = 0;
  static unsigned int tick = 0;
  if (millis() - last >= timeout) {   // has it been 2 s since we last did it?
    last = millis();               // remember when we did it this time
    netLog("Ticks!: %u\n", tick++);  // do it again
  }
  netService();
}
