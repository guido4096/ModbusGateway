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
#include "plugin/battery_inputs.h"
#include "devices/alfen.h"
#include "devices/solaredge.h"
#include "plugin/devices/optimizer.h"
#include <Preferences.h>
#include "ModbusServerTCPasync.h"

namespace modbus_gateway {
namespace {
void appendExtraStatistics(String& output);
void appendExtraDescriptions(String& output);
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

WiFiClient alfenClient;
WiFiClient solaredgeClient;
Preferences preferences;
unsigned long previousPoll;
constexpr unsigned long POLL_INTERVAL = 200;

// Alfen master
IPAddress remote_alfen()
{
    IPAddress a;
    a.fromString(REMOTE_ALFEN);
    return a;
}
ModbusClientTCP tcp_alfen(alfenClient, 10);
modbus_gateway::Client<modbus_gateway::Alfen> alfen(tcp_alfen, remote_alfen(), TCP_ALFEN_PORT, TCP_ALFEN_SERVER_ID);

// SolarEdge master
IPAddress remote_solaredge()
{
    IPAddress a;
    a.fromString(REMOTE_SOLAREDGE);
    return a;
}
ModbusClientTCP tcp_solaredge(solaredgeClient, 10);
modbus_gateway::Client<modbus_gateway::SolarEdge> solaredge(tcp_solaredge, remote_solaredge(), TCP_SOLAREDGE_PORT, TCP_SOLAREDGE_SERVER_ID);

ModbusServerTCPasync tcp_server;
Server<Optimizer> optimizer(tcp_server, 3);

void handleMeter()
{
    String response;
    {
        DataAccess<Client<EM24_E1>> access(meter);
        response = access.allValuesAsString();
    }
    webServer->send(200, "text/plain", response.c_str());
}

void handleWattnode()
{
    String response;
    {
        DataAccess<Server<WattNode>> access(wattnode);
        response = access.allValuesAsString();
    }
    webServer->send(200, "text/plain", response.c_str());
}

void handleStatistics()
{
    String response = "WattNode (RTU server)\r\n";
    response += wattnode.getStatistics();
    response += "\r\nMeter (TCP client)\r\n";
    response += meter.getStatistics();
    appendExtraStatistics(response);
    webServer->send(200, "text/plain", response.c_str());
}

void handleDescription()
{
    String r;
    r += modbus_gateway::getDeviceDescription<modbus_gateway::WattNode>();
    r += modbus_gateway::getDeviceDescription<modbus_gateway::EM24_E1>();
    appendExtraDescriptions(r);
    webServer->send(200, "text/plain", r.c_str());
}

void handleLogMeter()
{
    const String response = meter.getLog();
    webServer->send(200, "text/plain", response.c_str());
}

void handleLogWattnode()
{
    const String response = wattnode.getLog();
    webServer->send(200, "text/plain", response.c_str());
}

void handleAlfen()
{
    String response;
    {
        DataAccess<Client<Alfen>> access(alfen);
        response = access.allValuesAsString();
    }
    webServer->send(200, "text/plain", response.c_str());
}

void handleSolarEdge()
{
    String response;
    {
        DataAccess<Client<SolarEdge>> access(solaredge);
        response = access.allValuesAsString();
    }
    webServer->send(200, "text/plain", response.c_str());
}

void handleOptimizer()
{
    String response;
    {
        DataAccess<Server<Optimizer>> access(optimizer);
        response = access.allValuesAsString();
    }
    webServer->send(200, "text/plain", response.c_str());
}

void handleOptimizer0()
{
    String response;
    {
        DataAccess<Server<Optimizer>> access(optimizer);
        access.set<Optimizer::optimizer_strategy>(0);
        response = "OK\n" + access.allValuesAsString();
    }
    preferences.putInt("opt_strat", 0);
    webServer->send(200, "text/plain", response.c_str());
}

void handleOptimizer1()
{
    String response;
    {
        DataAccess<Server<Optimizer>> access(optimizer);
        access.set<Optimizer::optimizer_strategy>(1);
        response = "OK\n" + access.allValuesAsString();
    }
    preferences.putInt("opt_strat", 1);
    webServer->send(200, "text/plain", response.c_str());
}

void handleOptimizer2()
{
    String response;
    {
        DataAccess<Server<Optimizer>> access(optimizer);
        access.set<Optimizer::optimizer_strategy>(2);
        response = "OK\n" + access.allValuesAsString();
    }
    preferences.putInt("opt_strat", 2);
    webServer->send(200, "text/plain", response.c_str());
}

void handleStatus()
{
    int32_t strategy;
    {
        DataAccess<Server<Optimizer>> access(optimizer);
        strategy = access.get<Optimizer::optimizer_strategy>();
    }
    String response = "{\"circuit\": " + String(static_cast<uint32_t>(strategy)) + " }";
    webServer->send(200, "application/json", response.c_str());
}

void handleLogAlfen()
{
    const String response = alfen.getLog();
    webServer->send(200, "text/plain", response.c_str());
}

void handleLogSolarEdge()
{
    const String response = solaredge.getLog();
    webServer->send(200, "text/plain", response.c_str());
}

void handleLogOptimizer()
{
    const String response = optimizer.getLog();
    webServer->send(200, "text/plain", response.c_str());
}

void appendExtraStatistics(String& output) {
    output += "\r\nOptimizer (TCP server)\r\n";
    output += optimizer.getStatistics();
    output += "\r\nAlfen (TCP client)\r\n";
    output += alfen.getStatistics();
    output += "\r\nSolarEdge (TCP client)\r\n";
    output += solaredge.getStatistics();
}

void appendExtraDescriptions(String& output) {
    output += getDeviceDescription<Alfen>();
    output += getDeviceDescription<SolarEdge>();
    output += getDeviceDescription<Optimizer>();
}

} // namespace

namespace battery_optimizer {
Inputs readInputs() {
    DataAccess<Server<Optimizer>> o(optimizer);
    DataAccess<Client<SolarEdge>> s(solaredge);
    DataAccess<Client<Alfen>> a(alfen);
    return {o.get<Optimizer::optimizer_strategy>(), a.get<Alfen::real_power_sum>(),
            s.get<SolarEdge::ac_power>(), s.get<SolarEdge::ac_power_sf>(),
            s.get<SolarEdge::b1_instantaneous_power>()};
}

void logAdjustment(const char* message) {
    optimizer.logMessage(message);
}
} // namespace battery_optimizer

namespace plugin {
void setup(WebServer& server) {
    webServer = &server;
    server.on("/meter", handleMeter);
    server.on("/wattnode", handleWattnode);
    server.on("/statistics", handleStatistics);
    server.on("/description", handleDescription);
    server.on("/logmeter", handleLogMeter);
    server.on("/logwattnode", handleLogWattnode);
    preferences.begin("modbusgateway", false);
    server.on("/alfen", handleAlfen);
    server.on("/solaredge", handleSolarEdge);
    server.on("/optimizer", handleOptimizer);
    server.on("/logalfen", handleLogAlfen);
    server.on("/logsolaredge", handleLogSolarEdge);
    server.on("/logoptimizer", handleLogOptimizer);
    server.on("/optimizer0", handleOptimizer0);
    server.on("/optimizer1", handleOptimizer1);
    server.on("/optimizer2", handleOptimizer2);
    server.on("/status", handleStatus);
    alfen.connect();
    solaredge.connect();
    const int32_t strategy = preferences.getInt("opt_strat", 0);
    {
        DataAccess<Server<Optimizer>> o(optimizer);
        o.set<Optimizer::optimizer_strategy>(strategy);
    }
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "Starting Strategy: %i", static_cast<int>(strategy));
    optimizer.logMessage(buffer);
    tcp_server.start(504, 4, 100);
    previousPoll = millis() - 10000;
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

    if (currTime - previousPoll >= POLL_INTERVAL) {
        int32_t strategy;
        {
            DataAccess<Server<Optimizer>> o(optimizer);
            strategy = o.get<Optimizer::optimizer_strategy>();
        }
        if (strategy == 1) {
            solaredge.readBlockFromMeter("dc");
            solaredge.readBlockFromMeter("pv");
            solaredge.readBlockFromMeter("battery");
        }
        alfen.readBlockFromMeter("dynamic");
        previousPoll = currTime;
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
    output +=
        "<a href=\"alfen\">Alfen values</a><br/>"
        "<a href=\"solaredge\">SolarEdge values</a><br/>"
        "<a href=\"optimizer\">Optimizer values</a><br/>"
        "<a href=\"logalfen\">Log Alfen messages</a><br/>"
        "<a href=\"logsolaredge\">Log SolarEdge messages</a><br/>"
        "<a href=\"logoptimizer\">Log Optimizer messages</a><br/>"
        "<a href=\"optimizer0\">Optimizer 0</a><br/>"
        "<a href=\"optimizer1\">Optimizer 1</a><br/>"
        "<a href=\"optimizer2\">Optimizer 2</a><br/>";
}
} // namespace plugin
} // namespace modbus_gateway
