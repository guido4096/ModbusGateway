/**
 * @file      modbus_gateway.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Setup and main loop to mediate between SolarEdge and the EnergyMeter
 */
#include <Arduino.h>
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include <ETHClass2.h> //Is to use the modified ETHClass
#define ETH ETH2
#else
#include <ETH.h>
#endif
#include <SPI.h>
#include <SD.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <queue>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ModbusTCP.h>
#include <ModbusRTU.h>

#include "utilities.h" //Board PinMap
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
    String r = "";
    for (auto i = meter._blockValues.begin(); i < meter._blockValues.end(); i++)
        r += i->toString();

    server.send(200, "text/plain", r.c_str());
}

void handleMeter()
{
    String r = "";
    for (auto i = meter._blockValues.begin(); i < meter._blockValues.end(); i++)
        r += i->toString();

    server.send(200, "text/plain", r.c_str());
}

void handleWattnode()
{
    String r = "";
    char buf[200];
    for (auto i = wattnode._dd._blocks.begin(); i < wattnode._dd._blocks.end(); i++)
    {
        sprintf(buf, "Block %s\r\n", i->_name.c_str());
        r += buf;
        for (auto j = i->_registers.begin(); j < i->_registers.end(); j++)
        {
            switch (j->_dataType)
            {
            case modbus::DataType::float32:
            {
                float v = wattnode.getFloatValue(*j);
                sprintf(buf, "  %s=%.1f %s\r\n", j->_desc.c_str(), v, j->_unit);
                r += buf;
            }
            break;
            case modbus::DataType::uint32:
            {
                uint32_t v = wattnode.getUint32(*j);
                sprintf(buf, "  %s=%i %s\r\n", j->_desc.c_str(), v, j->_unit);
                r += buf;
            }
            break;
            case modbus::DataType::int16:
            {
                int16_t v = wattnode.getInt16(*j);
                sprintf(buf, "  %s=%i %s\r\n", j->_desc.c_str(), v, j->_unit);
                r += buf;
            }
            break;
            case modbus::DataType::uint16:
            {
                uint16_t v = wattnode.getUint16(*j);
                sprintf(buf, "  %s=%u %s\r\n", j->_desc.c_str(), v, j->_unit);
                r += buf;
            }
            break;            default:
            {
                sprintf(buf, "  %s=not converted %s\r\n", j->_desc.c_str(), j->_unit);
                r += buf;
            }
            break;
            }
        }
    }
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

    // Use static ip address config
    // IPAddress local_ip(192, 168, 3, 235);
    // IPAddress gateway(192, 168, 3, 1);
    // IPAddress subnet(255, 255, 255, 0);
    // IPAddress dns (192, 168, 3, 1);

    /*ETH.config( local_ip,
                gateway,
                subnet,
                dns
                // IPAddress dns1 = (uint32_t)0x00000000,
                // IPAddress dns2 = (uint32_t)0x00000000
              );*/

    // Setup the ETHERNET device
#if CONFIG_IDF_TARGET_ESP32
    if (!ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN,
                   ETH_MDIO_PIN, ETH_RESET_PIN, ETH_CLK_MODE))
    {
        Serial.println("ETH start Failed!");
    }
#else
    if (!ETH.begin(ETH_PHY_W5500, 1, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN,
                   SPI3_HOST,
                   ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN))
    {
        Serial.println("ETH start Failed!");
    }
#endif

    while (!eth_connected)
    {
        Serial.println("Wait for network connect ...");
        delay(500);
        ETH.printInfo(Serial);
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

#if defined(ESP32) || defined(ESP8266)
    rtu.begin(&Serial485);
#else
    rtu.begin(&Serial485);
    // rtu.begin(&Serial, RXTX_PIN);  //or use RX/TX direction control pin (if required)
    rtu.setBaudrate(9600);
#endif

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