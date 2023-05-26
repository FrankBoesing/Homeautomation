// #pragma once

/**************************************************************************************************/
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h> // for the webserver

typedef WiFiUDP _WiFiUDP;

class _WebServer : public ESP8266WebServer
{
public:
    explicit _WebServer(int port = 80) : ESP8266WebServer(port){};
};

/**************************************************************************************************/
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>

class _WiFiUDP : public WiFiUDP
{
public:
    inline uint8_t beginMulticast(IPAddress interfaceAddr [[maybe_unused]], IPAddress multicast, uint16_t port)
    {
        _localPort = port;
        return WiFiUDP::beginMulticast(multicast, port);
    }
    inline uint16_t localPort() { return _localPort; }

private:
    uint16_t _localPort;
};

class _WebServer : public WebServer
{
public:
    explicit _WebServer(int port = 80) : WebServer(port){};

    using WebServer::send;
    void send(int code, const char *content_type, const char *content, size_t contentLength) // missing
    {
        String header;
        if (contentLength == 0)
        {
            log_w("content length is zero");
        }
        _prepareHeader(header, code, content_type, contentLength);
        _currentClientWrite(header.c_str(), header.length());
        if (contentLength)
            sendContent(content);
    }

    void send(int code, const char *content_type, const char *content) // better
    {
        send(code, content_type, content, strlen(content));
    }

    void keepAlive(bool) {};

    using WebServer::send_P;
    void send_P(int code, PGM_P content_type, PGM_P content)
    {
        String header;
        size_t contentLength;
        char type[32];

        if (content != NULL)
            contentLength = strlen_P(content);
        else
            contentLength = 0;

        memccpy_P((void *)type, (PGM_VOID_P)content_type, 0, sizeof(type));

        _prepareHeader(header, code, (const char *)type, contentLength);
        _currentClientWrite(header.c_str(), header.length());

        if (contentLength)
            sendContent_P(content);
    }

    bool chunkedResponseModeStart_P(int code, PGM_P content_type)
    {
        if (_currentVersion == 0) // no chunk mode in HTTP/1.0
            return false;
        setContentLength(CONTENT_LENGTH_UNKNOWN);
        send_P(code, content_type, "");
        return true;
    }
    bool chunkedResponseModeStart(int code, const char *content_type)
    {
        return chunkedResponseModeStart_P(code, content_type);
    }

    bool chunkedResponseModeStart(int code, const String &content_type)
    {
        return chunkedResponseModeStart_P(code, content_type.c_str());
    }

    void chunkedResponseFinalize()
    {
        sendContent(emptyString);
    }
};
#endif
