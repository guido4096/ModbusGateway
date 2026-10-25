#include "application/plugin.h"
#include "devices/em24_e1.h"
#include "devices/wattnode.h"
#include "modbus/client.h"
#include "modbus/server.h"
#include "devices/alfen.h"
#include "devices/solaredge.h"
#include "plugin/devices/optimizer.h"
#include <Preferences.h>
#include <ModbusServerTCPasync.h>
#include <WebServer.h>
#include <ModbusServerRTU.h>
#include <cassert>
#include <iostream>

int main() {
    using namespace modbus_gateway;
    WebServer web;
    Preferences::values["modbusgateway/opt_strat"] = 2;
    plugin::setup(web);
    auto& optimizer = *Server<Optimizer>::instance;
    auto& alfen = *Client<Alfen>::instance;
    auto& solaredge = *Client<SolarEdge>::instance;
    assert(optimizer._device.get<Optimizer::optimizer_strategy>() == 2);
    assert(alfen.connections == 1 && solaredge.connections == 1);
    assert(ModbusServerTCPasync::last_port == 504);
    assert(ModbusServerTCPasync::last_clients == 4);
    assert(ModbusServerTCPasync::last_timeout == 100);
    for (const char* route : {"/alfen", "/solaredge", "/optimizer", "/optimizer0", "/optimizer1", "/optimizer2", "/status", "/logalfen", "/logsolaredge", "/logoptimizer"})
        assert(web.routes.count(route) == 1);
    for (const char* route : {"/meter", "/wattnode", "/statistics", "/description", "/logmeter", "/logwattnode"})
        assert(web.routes.count(route) == 1);
    auto& meter = *Client<EM24_E1>::instance;
    auto& wattnode = *Server<WattNode>::instance;
    assert(meter.connections == 1);
    assert(ModbusServerRTU::starts == 1);
    assert(wattnode._device.serverId() == 2);
    assert(wattnode._device.get<WattNode::serial_number>() == 0x12345678u);
    assert(plugin::loop(0));
    assert((meter.blocks == std::vector<String>{"dynamic", "energy", "time", "tariff"}));
    assert(alfen.blocks.size() == 1 && solaredge.blocks.empty());
    assert(!plugin::loop(99));
    assert(plugin::loop(100));
    assert(meter.blocks.back() == "dynamic" && meter.blocks.size() == 5);
    assert(plugin::loop(2000));
    assert(meter.blocks.size() == 7 && meter.blocks.back() == "energy");
    assert(plugin::loop(10000));
    assert(meter.blocks.size() == 11 && meter.blocks.back() == "tariff");

    web.routes.at("/optimizer0")();
    assert(Preferences::values.at("modbusgateway/opt_strat") == 0);
    assert(web.code == 200 && web.response.find("OK") != String::npos);
    meter._device.set<EM24_E1::power_active>(-12340);
    meter._dataRead = true;
    plugin::loop(10001);
    assert(!meter._dataRead);
    assert(wattnode._device.get<WattNode::power_active>() == -1234.0f);
    uint16_t wire[2]{};
    assert(wattnode._device.readHoldingRegisters(1008, 2, wire));
    assert(wire[0] == 0x4000 && wire[1] == 0xc49a);
    web.routes.at("/description")();
    assert(web.code == 200 && web.response.find("wattnode") != String::npos);
    assert(web.response.find("em24_e1") != String::npos);
    String links;
    plugin::appendRootLinks(links);
    assert(links.find("meter") != String::npos);
    web.routes.at("/optimizer1")();
    assert(Preferences::values.at("modbusgateway/opt_strat") == 1);
    plugin::loop(10200);
    assert((solaredge.blocks == std::vector<String>{"dc", "pv", "battery"}));
    alfen._device.set<Alfen::real_power_sum>(3000.0f);
    solaredge._device.set<SolarEdge::ac_power>(2000);
    solaredge._device.set<SolarEdge::ac_power_sf>(0);
    meter._dataRead = true;
    plugin::loop(10201);
    assert(wattnode._device.get<WattNode::power_active>() == -2234.0f);
    web.routes.at("/optimizer2")();
    assert(Preferences::values.at("modbusgateway/opt_strat") == 2);
    meter._dataRead = true;
    plugin::loop(10202);
    assert(wattnode._device.get<WattNode::power_active>() == -4234.0f);
    web.routes.at("/status")();
    assert(web.content_type == "application/json");
    assert(web.response == "{\"circuit\": 2 }");
    web.routes.at("/optimizer0")();
    meter._dataRead = true;
    plugin::loop(10203);
    assert(wattnode._device.get<WattNode::power_active>() == -1234.0f);
    for (auto& route : web.routes) {
        route.second();
        assert(web.code == 200);
        assert(native_device_locks == 0);
    }
    std::cout << "PASS: production plugin setup, polling, conversion and web routes\n";
}
