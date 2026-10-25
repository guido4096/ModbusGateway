#include "application/plugin.h"
#include "devices/em24_e1.h"
#include "devices/wattnode.h"
#include "modbus/client.h"
#include "modbus/server.h"
#include <WebServer.h>
#include <ModbusServerRTU.h>
#include <cassert>
#include <iostream>

int main() {
    using namespace modbus_gateway;
    WebServer web;
    plugin::setup(web);
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
    assert(!plugin::loop(99));
    assert(plugin::loop(100));
    assert(meter.blocks.back() == "dynamic" && meter.blocks.size() == 5);
    assert(plugin::loop(2000));
    assert(meter.blocks.size() == 7 && meter.blocks.back() == "energy");
    assert(plugin::loop(10000));
    assert(meter.blocks.size() == 11 && meter.blocks.back() == "tariff");

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
    std::cout << "PASS: production plugin setup, polling, conversion and web routes\n";
}
