
#pragma once
#define _HOMEAUTOMATION_H_

#include <Arduino.h>
#include <vector>
#include <functional>
#include "sys/compat.h"

#define __NONULL__ __attribute__((nonnull))

enum EdeviceType : uint8_t
{
    eNone = 0,
    eSwitch = 1,
    eLight = 2,
    eLight_brightness = eLight | 4,
    eLight_brihuesat = eLight_brightness | 8
};

/********************************************************************************************/
/********************************************************************************************/

class HomeautomationDevice
{
protected:
    HomeautomationDevice();
    HomeautomationDevice(const HomeautomationDevice &) = delete;
    void operator=(const HomeautomationDevice &) = delete;

    typedef char IPChar[16];
    typedef std::function<void(HomeautomationDevice *)> TStateEvent;

    static std::vector<HomeautomationDevice *> devices;
    struct __attribute__((packed))
    {
        const char *_name;
        bool state;
        uint8_t myId;
        EdeviceType myDeviceType;
    };

    static uint32_t serial;
    static TStateEvent setEvent;
    static TStateEvent getEvent;

    size_t add(EdeviceType, const char *name, bool state);
    static void handle();
    static void sendUDPResponse(IPAddress remoteIP, uint16_t remotePort, const char *buffer, size_t size) __NONULL__;
    static void localIP(const IPChar *ipc) __NONULL__;

private:
    static _WiFiUDP udpServer;  // common UDP server
    static uint8_t deviceTypes; // enabled device types

    inline static void handleUDP();
    virtual void handleDeviceUDP(IPChar localIP [[maybe_unused]], IPAddress remoteIP [[maybe_unused]], uint16_t remotePort [[maybe_unused]]){};
    inline virtual void handleDeviceTCP(){};

public:
    inline EdeviceType type() const { return this->myDeviceType; }
    inline size_t id() const { return this->myId; }
    inline const char *name() const { return this->_name; }
    inline bool getState() const { return this->state; }
    inline void setState(bool s) { this->state = s; }

    virtual inline uint8_t getBrightness() { return 0; }
    virtual inline void setBrightness(uint8_t) {}
    virtual inline uint16_t getHue() { return 0; }
    virtual inline void setHue(uint16_t){};
    virtual inline uint8_t getSat() { return 0; }
    virtual inline void setSat(uint8_t){};
};

/********************************************************************************************/
/********************************************************************************************/

class Homeautomation : protected HomeautomationDevice
{
public:
    inline void handle() { HomeautomationDevice::handle(); };
    inline void onGetState(TStateEvent fn) { getEvent = fn; }
    inline void onSetState(TStateEvent fn) { setEvent = fn; }

    inline size_t size() { return devices.size(); }
    inline HomeautomationDevice &operator[](size_t index) const
    {
        auto &ref = *devices[index];
        return ref;
    };
};

/********************************************************************************************/
/********************************************************************************************/

class Light : public HomeautomationDevice
{
public:
    explicit Light(const char *_name, bool _enable = false);

protected:
    Light(){};
    static _WebServer server;
    static char macstr[18];
    static char shortmacstr[13]; // "abcdbeefabcd"
    static const constexpr char *username = &shortmacstr[6];

    inline static void HTTP_Description();
    inline static void HTTP_HandleApi();
    inline static bool HTTP_HandleApi_POST(const char *body) __NONULL__;
    inline static bool HTTP_HandleApi_PUT(int id, const char *body) __NONULL__;
    inline static bool HTTP_HandleApi_GET();       // List all devices
    inline static bool HTTP_HandleApi_GET(int id); // List one device

    virtual size_t HTTP_HandleAPi_PUT_status(const char *body, char *buf, size_t size) __NONULL__;
    virtual size_t HTTP_HandleApi_GET_status(char *buf, size_t size) __NONULL__;

    static int idToIndex(const char *sid) __NONULL__;
    static const char *JsonGetValue(const char *Jsons, const char *name) __NONULL__;
    void setStateJson(const char *body) __NONULL__;

    inline void startServer();
    virtual void handleDeviceUDP(IPChar localIP, IPAddress remoteIP, uint16_t remotePort) final override;
    virtual void handleDeviceTCP() final override;
};

/********************************************************************************************/
/********************************************************************************************/

class Light_Brightness : public Light
{
public:
    explicit Light_Brightness(const char *name, bool enable = false, uint8_t brightness = 254);
    inline virtual uint8_t getBrightness() { return this->brightness; }
    inline virtual void setBrightness(uint8_t brightness) { this->brightness = constrain(brightness, 1u, 254u); }

protected:
    uint8_t brightness;

    void setBrightnessJson(const char *body);
    virtual size_t HTTP_HandleAPi_PUT_status(const char *body, char *buf, size_t size) override;
    virtual size_t HTTP_HandleApi_GET_status(char *buf, size_t size) override;

private:
};

/********************************************************************************************/
/********************************************************************************************/

class Light_BriHueSat : public Light_Brightness
{
public:
    explicit Light_BriHueSat(const char *name, bool enable = false, uint8_t brightness = 254, uint16_t hue = 0, uint8_t sat = 0);
    inline virtual uint16_t getHue() { return this->hue; }
    inline virtual void setHue(uint16_t hue) { this->hue = hue; }
    inline virtual uint8_t getSat() { return this->sat; }
    inline virtual void setSat(uint8_t sat) { this->sat = sat; }

protected:
    void setHueSatJson(const char *body);
    virtual size_t HTTP_HandleAPi_PUT_status(const char *body, char *buf, size_t size) override;
    virtual size_t HTTP_HandleApi_GET_status(char *buf, size_t size) override;

private:
    uint16_t hue;
    uint8_t sat;
};

/********************************************************************************************/
/********************************************************************************************/

class Switch : public HomeautomationDevice
{
public:
    explicit Switch(const char *name, bool enable = false);

protected:
    void HTTP_SetupXML();
    void HTTP_Control();
    void HTTP_SendState();
    void HTTP_404();
    static uint32_t hash(const char *text);

private:
    uint32_t _hash;
    _WebServer server;
    uint16_t port;

    inline void startServer();
    virtual void handleDeviceUDP(IPChar localIP, IPAddress remoteIP, uint16_t remotePort) final override;
    inline virtual void handleDeviceTCP() final override;
};

#undef __NONULL__
