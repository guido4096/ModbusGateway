/**
 * @file      modbus_gateway.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Setup and main loop to mediate between SolarEdge and the EnergyMeter
 */
#include "utilities.h" //Board PinMap
#include <Arduino.h>
#include <ETH.h>
#include <SPI.h>
#include <SD.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <queue>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ModbusTCP.h>
#include <ModbusRTU.h>

#include "definitions.h"
#include "slave.h"
#include "master.h"
#include "em24.h"
#include "wattnode.h"
#include "convert_em24_to_wattnode.h"

static bool eth_connected = false;
WebServer server(80);

// What is the name of the device
// Passed as MACRO through a build_flag in secrets.ini
// #define DEVICENAME "devicename"

// What is the password for OTA updates
// Passed as MACRO through a build_flag in secrets.ini
// #define OTA_PASSWORD "password"

// TCP Master REMOTE ip address & RTU Slave SLAVE_ID are coming from secrets.ini
// #define REMOTE "192.168.1.2"
// #define SLAVE_ID 2

// TCP Master
IPAddress remote()
{
    IPAddress a;
    a.fromString(REMOTE);
    return a;
}
ModbusTCP tcp;
modbus::Master<modbus::EM24> meter(tcp, remote());

// TCP Slave
ModbusRTU rtu;
modbus::Slave<modbus::WattNode> wattnode(rtu, SLAVE_ID);

// Converter mapping
modbus::ConvertEM24ToWattNode converter(meter, wattnode);

// How thr RS485 port is connected to pins
#define BOARD_485_TX 33
#define BOARD_485_RX 32
#define Serial485 Serial2

// Room for 3 timed events
unsigned long prevTime1;
unsigned long prevTime2;
unsigned long prevTime3;

void handleRoot()
{
    String r = "\
    <a href=\"./wattnode\">WattNode values</a><br/>\
    <a href=\"./meter\">Meter values</a><br/>\
    <a href=\"./description\">Description of WattNode and Meter device</a><br/>\
    ";
    server.send(200, "text/html", r.c_str());
}

void handleMeter()
{
    String r = meter.allValueAsString();
    server.send(200, "text/plain", r.c_str());
}

void handleWattnode()
{
    String r = wattnode.allValueAsString();
    server.send(200, "text/plain", r.c_str());
}

void handleDescription()
{
    String r;
    r += wattnode._dd.GetDescriptions();
    r += meter._dd.GetDescriptions();
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

    if (MDNS.begin(DEVICENAME))
    {
        Serial.println("MDNS responder started");
    }

    // Setup HTTP server
    server.on("/", handleRoot);
    server.on("/description", handleDescription);
    server.on("/meter", handleMeter);
    server.on("/wattnode", handleWattnode);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
    tcp.client();
    bool val = tcp.connect(remote(), 502);
    Serial.print("tcp.connect=");
    Serial.println(val);

    // Setup timers to allow tracking elapsed time
    prevTime1 = millis() - 5000; // trigger timers immediately at startup
    prevTime2 = prevTime1;
    prevTime3 = prevTime1;

    // Start the 485 serial bus
    Serial485.begin(9600, SERIAL_8N1, BOARD_485_RX, BOARD_485_TX);
    rtu.begin(&Serial485);

    // Print the setup of the modbus devices
    Serial.print(wattnode._dd.GetDescriptions());
    Serial.print(meter._dd.GetDescriptions());

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

    // The Modbus Master object tends to return timeouts if creating too many requests and not giving time to process them
    // Hence only create one request per loop with a following 20 ms delay
    // To implement this, a static _joblist is maintained. You can add blocks to be retrieved and
    // they will be processed one at a time.
    static std::queue<String> _joblist;

    // check for updates
    ArduinoOTA.handle();

    // Handle HTTP requests
    server.handleClient();

    // use elapsed time to know when to add a new job
    unsigned long currTime = millis();
    if (currTime - prevTime1 >= 500) // Instantaneous variables, update regularly
    {
        _joblist.push("dynamic");
        prevTime1 = currTime;
    }
    if (currTime - prevTime2 >= 1000) // Updated every second
    {
        _joblist.push("energy");
        prevTime2 = currTime;
    }
    if (currTime - prevTime3 >= 4700)
    { // This hardly ever changes
        _joblist.push("time");
        _joblist.push("tariff");
        prevTime3 = currTime;
    }
    // process only one job per loop to avoid timeouts
    if (_joblist.size() > 0)
    {
        String b = _joblist.front();
        _joblist.pop();
        meter.readBlockFromMeter(b);
    }
    // process tcp and rtu tasks
    tcp.task();
    rtu.task();

    // Received data from the meter and it is now stored in the meter object
    // Copy and convert this data to the wattnode object
    if (meter._dataRead)
    {
        converter.CopyDataFromMasterToSlave();
        meter._dataRead = false;
    }

    // delay 20 miliseconds to allow background tasks to finish
    delay(20); // allow the cpu to switch to other tasks
}