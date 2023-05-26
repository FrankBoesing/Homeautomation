#include "homeautomation.h"

#define LOGLEVEL 0
#define LOG_UDP false
#define LOG_TCP false
#define LOG_PREFIX "LIGHT"
#include "sys/macros.h"

extern const char *stateStr[2];
static const char PROGMEM DEVICE_JSON[] =
    ",\"hue\":%u,\"sat\":%u,\"colormode\": \"hs\"";
static const char PROGMEM STATE_RESPONSE[] =
    ",{\"success\":{\"/lights/%u/state/hue\":%u}}"
    ",{\"success\":{\"/lights/%u/state/sat\":%u}}";

Light_BriHueSat::Light_BriHueSat(const char *name, bool enable, uint8_t brightness, uint16_t hue, uint8_t sat) : Light_Brightness(name, enable, brightness)
{
    myDeviceType = eLight_brihuesat;
    setHue(hue);
    setSat(sat);
}

void Light_BriHueSat::setHueSatJson(const char *body)
{
    const char *val;

    val = JsonGetValue(body, "hue");
    if (val)
        setHue(atoi(val));

    val = JsonGetValue(body, "sat");
    if (val)
        setSat(atoi(val));
}

size_t Light_BriHueSat::HTTP_HandleAPi_PUT_status(const char *body, char *buf, size_t size)
{
    size_t sz;
    sz = Light_Brightness::HTTP_HandleAPi_PUT_status(body, buf, size);
    setHueSatJson(body);

    sz += snprintf_P(buf + sz, size - sz,
                     STATE_RESPONSE,
                     myId + serial, hue,
                     myId + serial, sat);
    return sz;
}

size_t Light_BriHueSat::HTTP_HandleApi_GET_status(char *buf, size_t size)
{
    size_t sz;
    sz = Light_Brightness::HTTP_HandleApi_GET_status(buf, size);
    sz += snprintf_P(buf + sz, size - sz,
                     DEVICE_JSON,
                     hue, sat);
    return sz;
}
