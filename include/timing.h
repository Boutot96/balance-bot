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
    uint32_t t0 = 0, minUs = UINT32_MAX, maxUs = 0, sumUs = 0, n = 0;

    explicit WorkStats(const char* nm) : name(nm) {}

    void start() { t0 = micros(); }
    void stop()  { add(micros() - t0); }

    void add(uint32_t d) {               // record any measurement directly
        if (d < minUs) minUs = d;
        if (d > maxUs) maxUs = d;
        sumUs += d;
        n++;
    }
    void report() {
        if (n > 0)
            netLog("%-12s avg %5lu  min %5lu  max %5lu  n %5lu us\n", name,
                   (unsigned long)(sumUs / n), (unsigned long)minUs,
                   (unsigned long)maxUs, (unsigned long)n);
        minUs = UINT32_MAX; maxUs = sumUs = n = 0;
    }
};