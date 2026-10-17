#pragma once
#include <Arduino.h>
#include "net.h"

// ---------- Periodic timer ----------
// Usage:  static Every statusTimer(100);   if (statusTimer.ready()) { ... }
struct Every {
    uint32_t periodMs;
    uint32_t last = 0;

    explicit Every(uint32_t ms) : periodMs(ms) {}

    bool ready() { return ready(periodMs); }

    // Variable-period version, e.g. the cube's angle-based speed
    bool ready(uint32_t ms) {
        uint32_t now = millis();
        if (now - last < ms) return false;
        last += ms;                          // drift-free: schedule from the ideal time
        if (now - last >= ms) last = now;    // fell way behind? resync instead of bursting
        return true;
    }
};

// ---------- Work timer (avg / max / count) ----------
// Usage:  static WorkStats imuStats("imu+bal");
//         imuStats.start(); ...work... imuStats.stop();
//         if (reportTimer.ready()) imuStats.report();
struct WorkStats {
    const char* name;
    uint32_t t0 = 0, maxUs = 0, sumUs = 0, n = 0;

    explicit WorkStats(const char* nm) : name(nm) {}

    void start() { t0 = micros(); }
    void stop() {
        uint32_t d = micros() - t0;
        if (d > maxUs) maxUs = d;
        sumUs += d;
        n++;
    }
    void report() {                          // print, then reset for the next window
        if (n > 0)
            netLog("%s avg %lu  max %lu  n %lu us\n", name,
                   (unsigned long)(sumUs / n), (unsigned long)maxUs, (unsigned long)n);
        maxUs = sumUs = n = 0;
    }
};