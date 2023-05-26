#include "homeautomation.h"

#define LOGLEVEL 0
#define LOG_UDP false
#define LOG_TCP false
#define LOG_PREFIX "LIGHT"
#include "sys/macros.h"

extern const char *stateStr[2];
static const char PROGMEM DEVICE_JSON[] =
    ",\"bri\":%d";
static const char PROGMEM STATE_RESPONSE[] =
    ",{\"success\":{\"/lights/%u/state/bri\":%u}}";

Light_Brightness::Light_Brightness(const char *name, bool enable, uint8_t brightness) : Light(name, enable)
{
    //    Light::add(eLight_brightness, name, enable);
    myDeviceType = eLight_brightness;
    setBrightness(brightness);
}

void Light_Brightness::setBrightnessJson(const char *body)
{
    const char *val = JsonGetValue(body, "bri");
    if (val)
        setBrightness(atoi(val));
}

size_t Light_Brightness::HTTP_HandleAPi_PUT_status(const char *body, char *buf, size_t size)
{
    size_t sz;
    sz = Light::HTTP_HandleAPi_PUT_status(body, buf, size);
    setBrightnessJson(body);

    sz += snprintf_P(buf + sz, size - sz,
                     STATE_RESPONSE,
                     myId + serial, brightness);
    return sz;
}

size_t Light_Brightness::HTTP_HandleApi_GET_status(char *buf, size_t size)
{
    size_t sz;
    sz = Light::HTTP_HandleApi_GET_status(buf, size);
    sz += snprintf_P(buf + sz, size - sz,
                     DEVICE_JSON,
                     brightness);
    return sz;
}
