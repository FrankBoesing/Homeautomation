#include "homeautomation.h"
#include "templates_lights.h"

#define LOGLEVEL 0
#define LOG_UDP false
#define LOG_TCP false
#define LOG_PREFIX "LIGHT"
#include "sys/macros.h"

extern const char *mimeXML;
static const char *mimeJson = "application/json";

static const char *stateStr[2] = {"false", "true"};

static const char PROGMEM MAC[] = "%02X:%02X:%02X:%02X:%02X:%02X";
static const char PROGMEM SHORTMAC[] = "%02x%02x%02x%02x%02x%02x";

_WebServer Light::server(80);
char Light::macstr[18] = "";
char Light::shortmacstr[13];

Light::Light(const char *name, bool enable)
{
    add(eLight, name, enable);
}

void Light::startServer()
{
    if (macstr[0])
        return;
        
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf_P(macstr, sizeof macstr, MAC, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf_P(shortmacstr, sizeof shortmacstr, SHORTMAC, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    server.onNotFound(HTTP_HandleApi); // use "not found" to handle the api.
    server.on("/description.xml", HTTP_GET, HTTP_Description);
    server.begin();
}

const char *Light::JsonGetValue(const char *Jsonstring, const char *name)
{
    const char *p = Jsonstring;
    const size_t ln = strlen(name);
    for (;;)
    {
        p = strstr(p, name);
        if (!p)
            break;

        if ((p != Jsonstring) && ('"' == *(p - 1)) && ('"' == *(p + ln)))
        {
            // search ":" behind name
            p = strchr(p + 1, ':');
            if (!p)
                break;

            // skip spaces:
            p++;
            while (('\0' != *p) && (' ' == *p))
                p++;

            return p;
        }
        p++;
    }
    return nullptr;
}

void Light::handleDeviceUDP(IPChar localIP, IPAddress remoteIP, uint16_t remotePort)
{
    PRINTDEF(sizeof UDP_RESPONSE + sizeof(IPChar) + 2 * sizeof shortmacstr);
    PRINTPGM(UDP_RESPONSE, localIP, shortmacstr, shortmacstr);

    sendUDPResponse(remoteIP, remotePort, _buf, _sz);
    LOGUDP(1, "Sent answer to %s:%u\n", remoteIP.toString().c_str(), remotePort);
}

void Light::handleDeviceTCP()
{
    server.handleClient();
}

void Light::HTTP_Description()
{
    IPChar ip;
    localIP(&ip);

    PRINTDEF(sizeof DESCRIPTION + 2 * sizeof ip + 2 * sizeof shortmacstr);
    PRINTPGM(DESCRIPTION, ip, ip, shortmacstr, shortmacstr);

    server.send(200, mimeXML, _buf);

    LOGTCP(1, "Sent descrition.xml to %s\n", DBGTCPInfo(server).c_str());
}

int Light::idToIndex(const char *sid)
{
    int id = atoi(sid);

    if (id >= (int)serial)
        id -= (int)serial;

    if (id >= (int)devices.size())
        id = -1;

    return id;
}

bool Light::HTTP_HandleApi_POST(const char *body)
{

    if (!JsonGetValue(body, "devicetype"))
        return false;

    PRINTDEF(sizeof DEVICETYPE + 6);
    PRINTPGM(DEVICETYPE, username);
    server.send(200, mimeJson, _buf);

    LOGTCP(1, "%s: Sent login data\n", server->client().remoteIP().toString().c_str());
    return true;
}

bool Light::HTTP_HandleApi_PUT(int id, const char *body)
{
    if (id < 0)
        return false;

    const auto &e = devices[id];

    PRINTDEF(250);
    _buf[_sz] = '[';
    _sz++;
    _sz += static_cast<Light *>(e)->HTTP_HandleAPi_PUT_status(body, _buf + _sz, sizeof _buf - _sz);
    _buf[_sz] = ']';
    _sz++;

    server.send(200, mimeJson, _buf, _sz);

    if (setEvent)
        setEvent(e);

    LOGTCP(1, "%s: Sent PUT answer\n", server->client().remoteIP().toString().c_str());
    return true;
}

bool Light::HTTP_HandleApi_GET()
{
    char buf[200];
    int sz;

    server.chunkedResponseModeStart(200, mimeJson);
    server.sendContent("{", 1);

    char delim = ' ';
    uint i = 0;
    for (const auto &e : devices)
    {
        if ((e->type() & eLight) == 0)
            continue;
        sz = snprintf_P(buf, sizeof buf,
                        DEVICE_JSON_SHORT,
                        delim,
                        i + serial,
                        e->name(),
                        macstr, (uint)e->type(), i);
        server.sendContent(buf, sz);
        delim = ',';
        i++;
    }
    server.sendContent("}", 1);
    server.chunkedResponseFinalize();
    LOGTCP(1, "Sent all device info\n");
    return true;
}

bool Light::HTTP_HandleApi_GET(int id)
{
    if (id < 0)
        return false;

    char buf[200];
    int sz;

    const auto &e = devices[id];

    if ((e->type() & eLight) == 0)
        return false; // keine lampe

    server.chunkedResponseModeStart(200, mimeJson);
    sz = snprintf_P(buf, sizeof buf, DEVICE_JSON_HEAD,
                    e->name(),
                    macstr, (uint)e->type(), id);
    server.sendContent(buf, sz);

    sz = static_cast<Light *>(e)->HTTP_HandleApi_GET_status(buf, sizeof buf);

    server.sendContent(buf, sz);
    server.sendContent_P(DEVICE_JSON_TAIL);
    server.chunkedResponseFinalize();

    if (getEvent)
        getEvent(e);

    LOGTCP(1, "Sent device info '%s'\n", e.name());
    return true;
}

void Light::setStateJson(const char *body)
{
    const char *val = JsonGetValue(body, "on");
    state = (strncmp(val, "false", 5) == 0) ? 0 : 1;
}

size_t Light::HTTP_HandleAPi_PUT_status(const char *body, char *buf, size_t size)
{
    const char *sstate = JsonGetValue(body, "on");
    state = (strncmp(sstate, "false", 5) == 0) ? 0 : 1;

    return snprintf_P(buf, size,
                      STATE_RESPONSE,
                      myId + serial,
                      stateStr[state ? 1 : 0]);
}

size_t Light::HTTP_HandleApi_GET_status(char *buf, size_t size)
{
    return snprintf_P(buf, size, DEVICE_JSON,
                      stateStr[state ? 1 : 0]);
}

void Light::HTTP_HandleApi()
{
    const int szUriParts = 5;
    const char *uriParts[szUriParts];
    const char *body;

#if 1
    char *uri = (char *)server.uri().c_str();
#else
    char uri[server.uri().length() + 1];
    memccpy(uri, server.uri().c_str(), 0, server.uri().length() + 1);
#endif

    // parse Uri
    // http://(0:)api/(1:)user/(2:)lights/(3:)12345/(4:)state
    uint cnt = 0;
    const char *p = strtok(uri, "/");

    while (p && cnt < szUriParts)
    {
        uriParts[cnt++] = p;
        p = strtok(nullptr, "/");
    }

    if (cnt < 1 || strcmp(uriParts[0], "api") != 0)
        goto notFound;

    // check user:
    if (cnt >= 2 && strcmp(uriParts[1], username) != 0)
        goto notFound;

    // check part "lights":
    if (cnt >= 3 && strcmp(uriParts[2], "lights") != 0)
        goto notFound;

    body = server.arg(F("plain")).c_str();

    switch (server.method())
    {

    case HTTP_PUT: // PUT /api/b591e0/lights/11899361/state body:{"on":false/true}
    {
        if (cnt == 5 && (strcmp(uriParts[4], "state") == 0) &&
            HTTP_HandleApi_PUT(idToIndex(uriParts[3]), body))
            return;
    }
    break;

    case HTTP_GET:
        if (cnt == 3 && HTTP_HandleApi_GET()) // GET /api/b591e0/lights
            return;
        else if (cnt == 4 && HTTP_HandleApi_GET(idToIndex(uriParts[3]))) // GET /api/b591e0/lights/11899361
            return;
        break;

    case HTTP_POST: // POST /api body:{"devicetype": "Echo"}

        if (cnt == 1 && HTTP_HandleApi_POST(body))
            return;
        break;

    default:
        break;
    }

notFound:

    server.keepAlive(false);
    server.send(404);
    LOGTCP(0, "Not found\n");
}
