#include <Arduino.h>

static const char PROGMEM UDP_RESPONSE[] =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=86400\r\n"
    "DATE: Fri, 15 Apr 2016 04:56:29 GMT\r\n"
    "EXT:\r\n"
    "LOCATION: http://%s:%u/setup.xml\r\n"
    "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    "01-NLS: %08x-736d-4b93-bf03-835149%06x\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "ST: urn:Belkin:device:**\r\n"
    //"USN: uuid:Socket-1_0-38323636-4558-4dda-9188-cda0e6%06x-%02x::urn:Belkin:device:**\r\n"
    "USN: uuid:Socket-1_0-%08X%06X::urn:Belkin:device:**\r\n"
    "X-User-Agent: redsonic\r\n"
    "\r\n";

static const char PROGMEM TCP_RESPONSE_SETUP[] =
    "<?xml version=\"1.0\"?>"
    "<root>"
    "<device>"
    "<deviceType>urn:FrankB:device:controllee:1</deviceType>"
    "<friendlyName>%s</friendlyName>"
    "<manufacturer>Belkin International Inc.</manufacturer>"
    "<modelName>ESP</modelName>"
    "<modelNumber>1.0</modelNumber>"
    "<modelDescription>Belkin Plugin Socket 1.0</modelDescription>"
    //"<UDN>uuid:Socket-1_0-38323636-4558-4dda-9188-cda0e6%06x-%02x</UDN>"
    "<UDN>uuid:Socket-1_0-%08X%06X</UDN>"
    "<serialNumber>%08X%06X</serialNumber>"
    "<binaryState>%u</binaryState>"
    "<serviceList>"
    "<service>"
    "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
    "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
    "<controlURL>/upnp/control/basicevent1</controlURL>"
    "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
    "<SCPDURL>/eventservice.xml</SCPDURL>"
    "</service>"
    "</serviceList>"
    "</device>"
    "</root>\r\n"
    "\r\n";

static const char PROGMEM TCP_RESPONSE_STATE[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "<s:Body><u:GetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>%u</BinaryState>"
    "</u:GetBinaryStateResponse></s:Body></s:Envelope>";
