#include "homeautomation.h"
#include "templates_switches.h"

#define LOGLEVEL 0
#define LOG_UDP false
#define LOG_TCP false
#define LOG_PREFIX "SWITCH"

#include "sys/macros.h"

#define FIRSTPORT 49153

// https://ullisroboterseite.de/esp8266-Wemo-Emu.html
// http://www.makermusings.com/2015/07/18/virtual-wemo-code-for-amazon-echo/

extern const char *mimeXML;


Switch::Switch(const char *name, bool enable)
{
    add(eSwitch, name, enable);
}

void Switch::startServer()
{
    if (port) return;
    port = FIRSTPORT + myId;
    _hash = hash(_name);
    
    server.on("/upnp/control/basicevent1", HTTP_POST, [&]()
              { HTTP_Control(); });
    server.on("/setup.xml", HTTP_GET, [&]()
              { HTTP_SetupXML(); });
    server.onNotFound([&]()
                      { HTTP_404(); });

    server.begin(port);

}

uint32_t Switch::hash(const char *text)
{
    // http://isthe.com/chongo/tech/comp/fnv/
    uint32_t hash = 2166136261ull;
    const char *p;

    for (p = text; *p; p++)
    {
        hash ^= *p;
        hash *= 16777619ull;
    }
    return hash;
}

void Switch::handleDeviceUDP(IPChar localIP, IPAddress remoteIP, uint16_t remotePort)
{
    PRINTDEF(400);
    PRINTPGM(UDP_RESPONSE, localIP, port, _hash, serial, _hash, serial);
    sendUDPResponse(remoteIP, remotePort, _buf, _sz);
    LOGUDP(1, "%s Sent answer to %s:%u\n", _buf, remoteIP.toString().c_str(), remotePort);
}

void Switch::handleDeviceTCP()
{
    startServer();
    server.handleClient();
};

void Switch::HTTP_404()
{
    server.keepAlive(false);
    server.send(404);
    //LOGTCP(0, "Unknown %s from %s Port %u\n", server.uri().c_str(), port);
}

void Switch::HTTP_SetupXML()
{
    PRINTDEF(800);
    PRINTPGM(TCP_RESPONSE_SETUP, _name, _hash, serial, _hash, serial, state ? 1u : 0);
    server.send(200, mimeXML, _buf, _sz);
    LOGTCP(1, "Sent setup.xml to %s Port %u\n", server.uri().c_str(), port);
}

void Switch::HTTP_Control()
{
    // Alexa arg (get state) is :
    // <?xml version="1.0" encoding="utf-8"?><s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
    // <s:Body><u:GetBinaryState xmlns:u="urn:Belkin:service:basicevent:1"><BinaryState>1</BinaryState></u:GetBinaryState></s:Body></s:Envelope>

    const char *request = server.arg(0).c_str();

    LOGTCP(2, "Request: %s\n", request);

    request = strstr(request, "Body>");
    if (request && strstr_P(request, PSTR("SetBinaryState")))
    {
        LOGTCP(1, "Set state event from %s Port %u\n", server.uri().c_str(), port);

        request = strstr_P(request, PSTR("<BinaryState>"));
        if (request)
        {
            state = request[strlen("<BinaryState>")] == '1';
            HTTP_SendState();
            if (setEvent)
                setEvent(this);
        }
        else
            HTTP_404();
    }
    else if (request && strstr_P(request, PSTR("GetBinaryState")))
    {
        HTTP_SendState();
        if (getEvent)
            getEvent(this);
    }
    else
        HTTP_404();
}

void Switch::HTTP_SendState()
{
    PRINTDEF(sizeof TCP_RESPONSE_STATE);
    PRINTPGM(TCP_RESPONSE_STATE, state ? 1u : 0);
    server.send(200, mimeXML, _buf, _sz);
    LOGTCP(1, "Sent state to %s Port %u\n", server.uri().c_str(), port);
}
