#include "teleop.h"
#include "config.h"
#include <stdint.h>


static portMUX_TYPE teleMux = portMUX_INITIALIZER_UNLOCKED;

struct teleopMessage{
    float v;
    float w;
    uint32_t time;
};

static teleopMessage teleopM;

void teleopSet(float v, float w){
    portENTER_CRITICAL(&teleMux);
    teleopM.v = v;
    teleopM.w = w;
    teleopM.time = millis();
    portEXIT_CRITICAL(&teleMux);
}

void teleopGet(float& v, float& w){
    uint32_t t;
    portENTER_CRITICAL(&teleMux);
    v = teleopM.v;
    w = teleopM.w;
    t = teleopM.time;
    portEXIT_CRITICAL(&teleMux);
    if(millis() - t > TELEOP_TIMEOUT_MS){
        v = 0;
        w = 0;
    }
    if (!isfinite(v) || !isfinite(w)) { v = 0; w = 0; }
    v = constrain(v, -1.0f, 1.0f);
    w = constrain(w, -1.0f, 1.0f);

}