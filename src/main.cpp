/**
 * @file      modbus_gateway.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Board, network, HTTP and OTA host for the selected gateway plugin
 */
#include "utilities.h" //Board PinMap
#include <Arduino.h>
#include <ETH.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "application/plugin.h"
#include "time.h"

static bool eth_connected = false;
WebServer server(80);

// NTP Server
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// What is the name of the device
// Passed as MACRO through a build_flag in secrets.ini
// #define DEVICENAME "devicename"

// What is the password for OTA updates
// Passed as MACRO through a build_flag in secrets.ini
// #define OTA_PASSWORD "password"

void handleRoot()
{
    String r = "<a href=\"info\">Info</a><br/>";
    modbus_gateway::plugin::appendRootLinks(r);
    server.send(200, "text/html", r.c_str());
}

void handleInfo()
{
    String r = "ESP-IDF version is: " + String(esp_get_idf_version()) + "\r\n";
    r += String("FlashSize = ") + String(ESP.getFlashChipSize()) + " bytes\r\n";

    server.send(200, "text/plain", r.c_str());
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void WiFiEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname(DEVICENAME);
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    WiFi.onEvent(WiFiEvent);

#ifdef ETH_POWER_PIN
    pinMode(ETH_POWER_PIN, OUTPUT);
    digitalWrite(ETH_POWER_PIN, HIGH);
#endif

    // Setup the ETHERNET device
    if (!ETH.begin(ETH_ADDR, ETH_PHY_POWER, ETH_MDC_PIN,
                   ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE))
    {
        Serial.println("ETH start Failed!");
    }

    while (!eth_connected)
    {
        Serial.println("Wait for network connect ...");
        delay(500);
    }

    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    if (MDNS.begin(DEVICENAME))
    {
        Serial.println("MDNS responder started");
    }

    // Setup HTTP server
    server.on("/", handleRoot);
    server.on("/info", handleInfo);
    server.onNotFound(handleNotFound);

    modbus_gateway::plugin::setup(server);
    server.begin();
    Serial.println("HTTP server started");

    // OTA
    ArduinoOTA.setHostname(DEVICENAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.onStart([]()
                       {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type); })
        .onEnd([]()
               { Serial.println("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
        .onError([](ota_error_t error)
                 {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
    ArduinoOTA.begin();
    Serial.println("ArduinoOTA started");
    Serial.print("FlashSize = ");
    Serial.print(ESP.getFlashChipSize());
    Serial.println("bytes.");
}

void loop()
{
    // check for updates
    ArduinoOTA.handle();

    // Handle HTTP requests
    server.handleClient();

    if (modbus_gateway::plugin::loop(millis()))
        taskYIELD();
}
