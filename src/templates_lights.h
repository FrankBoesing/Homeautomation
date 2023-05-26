#include <Arduino.h>

static const char PROGMEM UDP_RESPONSE[] =
    "HTTP/1.1 200 OK\r\n"
    "EXT:\r\n"
    "CACHE-CONTROL: max-age=100\r\n"                            // SSDP_INTERVAL
    "LOCATION: http://%s:80/description.xml\r\n"                // !Portnumber is mandatory! Alexa wants it.
    "SERVER: FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0\r\n"     // _modelName, _modelNumber
    "hue-bridgeid: %s\r\n"                                      //
    "ST: urn:schemas-upnp-org:device:basic:1\r\n"               // _deviceType
    "USN: uuid:2f402f80-da50-11e1-9b23-%s::upnp:rootdevice\r\n" // _uuid::_deviceType
    "\r\n";

static const char PROGMEM DESCRIPTION[] =
    "<?xml version=\"1.0\" ?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion><major>1</major><minor>0</minor></specVersion>"
    "<URLBase>http://%s:80/</URLBase>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:basic:1</deviceType>"
    "<friendlyName>Philips hue (%s)</friendlyName>"
    "<manufacturer>Royal Philips Electronics</manufacturer>"
    "<manufacturerURL>http://www.philips.com</manufacturerURL>"
    "<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
    "<modelName>Philips hue bridge 2012</modelName>"
    "<modelNumber>929000226503</modelNumber>"
    //"<modelURL>http://www.meethue.com</modelURL>"
    "<serialNumber>%s</serialNumber>"
    "<UDN>uuid:2f402f80-da50-11e1-9b23-%s</UDN>"
    // "<presentationURL>index.html</presentationURL>"
    "</device>"
    "</root>";

static const char PROGMEM DEVICETYPE[] =
    "[{\"success\":{\"username\":\"%s\"}}]";

static const char PROGMEM STATE_RESPONSE[] =
    "{\"success\":{\"/lights/%u/state/on\":%s}}";

// Working with gen1 and gen3, ON/OFF/%, gen3 requires TCP port 80
static const char PROGMEM DEVICE_JSON_HEAD[] =
    "{"
    "\"type\":\"Extended color light\","
    "\"name\":\"%s\","
    "\"uniqueid\":\"%s:00:%02X-%02X\","
    "\"modelid\":\"LCT015\","
    "\"manufacturername\":\"Philips\","
    "\"productname\":\"E4\","
    "\"state\":{";

static const char PROGMEM DEVICE_JSON[] = "\"on\":%s";

static const char PROGMEM DEVICE_JSON_TAIL[] =
    ","
    "\"mode\":\"homeautomation\","
    "\"reachable\":true"
    "},"
    "\"capabilities\":{"
    "\"certified\":false,"
    "\"streaming\":{\"renderer\":true,\"proxy\":false}"
    "}"
    ",\"swversion\":\"5.105.0.21169\""
    "}";

// Use shorter description template when listing all devices
static const char PROGMEM DEVICE_JSON_SHORT[] =
    "%c"      // ","
    "\"%u\":" // device number
    "{"
    "\"type\":\"Extended color light\","
    "\"name\":\"%s\","
    "\"uniqueid\":\"%s:00:%02X-%02X\""
    "}";
