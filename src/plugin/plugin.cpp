/**
 * @file      plugin.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Device-specific gateway implementation
 */
#include "application/plugin.h"
#include "plugin/convert_em24_e1_to_wattnode.h"
#include "ModbusClientTCP.h"
#include "ModbusServerRTU.h"
#include "RTUutils.h"
#include <WebServer.h>
#include <WiFi.h>

namespace modbus_gateway {
namespace {
WebServer* webServer = nullptr;
WiFiClient meterClient;
// TCP Master
IPAddress remote()
{
    IPAddress a;
    a.fromString(REMOTE);
    return a;
}
ModbusClientTCP tcp(meterClient, 10);
modbus_gateway::Client<modbus_gateway::EM24_E1> meter(tcp, remote(), TCP_PORT, TCP_SERVER_ID);

// TCP Slave
ModbusServerRTU rtu(1000);
modbus_gateway::Server<modbus_gateway::WattNode> wattnode(rtu, RTU_SERVER_ID, SERIAL_NUMBER);

// Converter mapping
modbus_gateway::ConvertEM24_E1ToWattNode converter(meter, wattnode);

// How the RS485 port is connected to pins
#define BOARD_485_TX 33
#define BOARD_485_RX 32
#define Serial485 Serial2

// Polling periods in milliseconds. Each register block is scheduled independently.
static constexpr unsigned long INTERVAL_DYNAMIC = 100;
static constexpr unsigned long INTERVAL_ENERGY = 2000;
static constexpr unsigned long INTERVAL_SLOW = 10000;

unsigned long prevTime1;
unsigned long prevTime2;
unsigned long prevTime3;

void handleMeter()
{
    modbus_gateway::DataAccess<modbus_gateway::Client<modbus_gateway::EM24_E1>> m(meter);
    String r = m.allValuesAsString();
    webServer->send(200, "text/plain", r.c_str());
}

void handleWattnode()
{
    modbus_gateway::DataAccess<modbus_gateway::Server<modbus_gateway::WattNode>> wn(wattnode);
    String r = wn.allValuesAsString();
    webServer->send(200, "text/plain", r.c_str());
}

void handleStatistics()
{
    String r;
    {
        modbus_gateway::DataAccess<modbus_gateway::Server<modbus_gateway::WattNode>> wn(wattnode);
        r += "WattNode (RTU server)\r\n";
        r += wn.getStatistics();
    }
    {
        modbus_gateway::DataAccess<modbus_gateway::Client<modbus_gateway::EM24_E1>> m(meter);
        r += "\r\nMeter (TCP client)\r\n";
        r += m.getStatistics();
    }

    webServer->send(200, "text/plain", r.c_str());
}

void handleDescription()
{
    String r;
    r += modbus_gateway::getDeviceDescription<modbus_gateway::WattNode>();
    r += modbus_gateway::getDeviceDescription<modbus_gateway::EM24_E1>();

    webServer->send(200, "text/plain", r.c_str());
}

void handleLogMeter()
{
    modbus_gateway::DataAccess<modbus_gateway::Client<modbus_gateway::EM24_E1>> m(meter);

    String r = m.getLog();
    webServer->send(200, "text/plain", r.c_str());
}

void handleLogWattnode()
{
    modbus_gateway::DataAccess<modbus_gateway::Server<modbus_gateway::WattNode>> wn(wattnode);

    String r = wn.getLog();
    webServer->send(200, "text/plain", r.c_str());
}

} // namespace

namespace plugin {
void setup(WebServer& server) {
    webServer = &server;
    server.on("/meter", handleMeter);
    server.on("/wattnode", handleWattnode);
    server.on("/statistics", handleStatistics);
    server.on("/description", handleDescription);
    server.on("/logmeter", handleLogMeter);
    server.on("/logwattnode", handleLogWattnode);
    meter.connect();
    prevTime1 = millis() - 10000;
    prevTime2 = prevTime1;
    prevTime3 = prevTime1;
    RTUutils::prepareHardwareSerial(Serial485);
    Serial485.begin(9600, SERIAL_8N1, BOARD_485_RX, BOARD_485_TX);
    rtu.begin(Serial485);
    Serial.print(getDeviceDescription<WattNode>());
    Serial.print(getDeviceDescription<EM24_E1>());
}

bool loop(unsigned long currTime) {
    bool jobScheduled = false;
    if (currTime - prevTime1 >= INTERVAL_DYNAMIC) // Instantaneous variables, update regularly
    {
        meter.readBlockFromMeter("dynamic");
        prevTime1 = currTime;
        jobScheduled = true;
    }
    if (currTime - prevTime2 >= INTERVAL_ENERGY) // Updated every two seconds
    {
        meter.readBlockFromMeter("energy");
        prevTime2 = currTime;
        jobScheduled = true;
    }
    if (currTime - prevTime3 >= INTERVAL_SLOW) // This hardly ever changes
    {

        meter.readBlockFromMeter("time");
        meter.readBlockFromMeter("tariff");
        prevTime3 = currTime;
        jobScheduled = true;
    }

    if (meter.consumeUpdated()) {
        converter.CopyDataFromMasterToSlave();
    }
    return jobScheduled;
}

void appendRootLinks(String& output) {
    output +=
        "<a href=\"wattnode\">WattNode values</a><br/>"
        "<a href=\"meter\">Meter values</a><br/>"
        "<a href=\"description\">Description of devices</a><br/>"
        "<a href=\"logmeter\">Log Meter messages</a><br/>"
        "<a href=\"logwattnode\">Log WattNode messages</a><br/>"
        "<a href=\"statistics\">Statistics</a><br/>";
}
} // namespace plugin
} // namespace modbus_gateway
