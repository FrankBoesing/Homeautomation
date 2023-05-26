#include "homeautomation.h"

#define UDP_MULTICAST_IP IPAddress(239, 255, 255, 250)
#define UDP_MULTICAST_PORT 1900

#define LOGLEVEL 0
#define LOG_UDP false
#define LOG_TCP false
#define LOG_PREFIX "AUTOMATION"
#include "sys/macros.h"


struct
{
    const char *search;
    uint devicetype;
} static constexpr const PROGMEM keywords[7] = {
    // don't respond:
    {"urn:agf:", 0},
    {"urn:dial-multiscreen-org", 0},
    // respond:
    {"device:basic:1", eLight},
    {"\"ssdp:discover\"", eLight},
    {"ssdp:all", eSwitch},
    {"urn:Belkin:device:**", eSwitch},
    {"upnp:rootdevice", eLight | eSwitch}};

const char *mimeXML = "text/xml";

/*************************************************************************************/
HomeautomationDevice::HomeautomationDevice() : state(false), myId(0), myDeviceType(eNone)
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    serial = (mac[3] << 16) | (mac[4] << 8) | mac[5];
}

void HomeautomationDevice::handle()
{
    if (!deviceTypes)
        return;

    handleUDP();
    bool lightHandled = false;
    for (const auto &d : devices)
    {
        if (d->myDeviceType & eLight)
        {
            if (!lightHandled)
            {
                d->handleDeviceTCP();
                lightHandled = true;
            }
            continue;
        }
        d->handleDeviceTCP();
    }
}

size_t HomeautomationDevice::add(EdeviceType type, const char *name, bool state)
{

    size_t id = devices.size();
    this->deviceTypes |= type;
    this->myDeviceType = type;
    this->myId = id;
    this->_name = name;
    this->state = state;

    devices.push_back(this);

    return id;
}

inline void HomeautomationDevice::handleUDP()
{

    if (udpServer.localPort() == 0)
    { // Multicast server not started yet.
        udpServer.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
        return;
    }

    const int minlen = 20;
    int len = udpServer.parsePacket();
    if (len < minlen)
        return; // request too short
    if (!deviceTypes)
        return; // no device active, don't respond

    char *c;
    char ch;
    char request[len + 1];

    // copy a few bytes from request:
    int l = udpServer.read(request, minlen);

    c = strtok(request, " ");
    if (!c || strcmp(c, "M-SEARCH") > 0)
        return; // no search request

    //LOGUDP(2, "\n%s\n", c);

    // copy remaining data from packet:
    udpServer.read(&request[l], len - l);
    request[len] = '\0';

    // Parse the remaining header.
    // Find all devicestypes to call or return immediately if unwanted:
    //  find devicetypes to call:

    uint toCall = 0;
    if (strtok(nullptr, "\r\n"))
    {
        while ((c = strtok(nullptr, "\r\n")) && *(c - 3) != '\n' && *c != '\0') // each line:
        {
            LOGUDP(2, "%s\n", c);

            while ((ch = *c++) && ch != ':') // search for ':'
                ;
            while (*c == ' ') // skip spaces
                c++;

            for (const auto &keyword : keywords)
            {
                if (keyword.devicetype == 0)
                {
                    if (strcmp_P(c, keyword.search) >= 0)
                    {
                        LOGUDP(3, "unwanted.\n");
                        return; // Unwanted keyword
                    }
                    continue;
                }
                if ((toCall & keyword.devicetype) != 0)
                    continue; // We have already found the device
                if ((deviceTypes & keyword.devicetype) == 0)
                    continue; // Device is not enabled
                if ((strcmp_P(c, keyword.search) >= 0))
                    toCall |= deviceTypes & keyword.devicetype; // Gotcha
            };
        };
    }
    LOGUDP(1, "-> To Call: %u\n", toCall);

    if (!toCall)
        return;

    // toCall &= eLight | eSwitch; // base types only

    IPChar ip;
    localIP(&ip);
    const IPAddress remoteIp = udpServer.remoteIP();
    const uint16_t remotePort = udpServer.remotePort();

    // a single call for HUE / for Belkin each device
    for (const auto &d : devices)
    {
        if (toCall & d->myDeviceType)
        {
            if (toCall & eLight)
            {
                toCall &= ~eLight; // one call only
                d->handleDeviceUDP(ip, remoteIp, remotePort);
            }
            else if (toCall & eSwitch)
            {
                d->handleDeviceUDP(ip, remoteIp, remotePort);
            }
        }
        if (!toCall)
            break;
    }
}

void HomeautomationDevice::sendUDPResponse(IPAddress remoteIP, uint16_t remotePort, const char *buffer, size_t size)
{
    udpServer.beginPacket(remoteIP, remotePort);
    udpServer.write((const unsigned char *)buffer, size);
    udpServer.endPacket();
    yield();
}

void HomeautomationDevice::localIP(const IPChar *ipc)
{
    IPAddress ip = WiFi.localIP();
    snprintf_P((char *)ipc, sizeof(IPChar), PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
}

// PROGMEM static const char *METHOD[] = {"ANY", "GET", "HEAD", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"};

std::vector<HomeautomationDevice *> HomeautomationDevice::devices;
_WiFiUDP HomeautomationDevice::udpServer;
HomeautomationDevice::TStateEvent HomeautomationDevice::setEvent = nullptr;
HomeautomationDevice::TStateEvent HomeautomationDevice::getEvent = nullptr;
uint8_t HomeautomationDevice::deviceTypes = 0;
uint32_t HomeautomationDevice::serial;
