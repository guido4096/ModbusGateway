#include "plugin/convert_em24_e1_to_wattnode.h"
#include "devices/em24_e1.h"
#include "devices/wattnode.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>

int main() {
    using namespace modbus_gateway;
    Client<EM24_E1> meter;
    Server<WattNode> wattnode;
    meter._device.set<EM24_E1::l1_voltage>(2258);
    meter._device.set<EM24_E1::l1_current>(6574);
    meter._device.set<EM24_E1::l1_power_active>(-14795);
    meter._device.set<EM24_E1::power_active>(-12340);
    meter._device.set<EM24_E1::import_energy_active>(12000);
    meter._device.set<EM24_E1::export_energy_active>(3000);
    ConvertEM24_E1ToWattNode(meter, wattnode).CopyDataFromMasterToSlave();
    auto near = [](float a, float b) { return std::fabs(a-b) < 0.0005f; };
    // EM24 quantities are raw fixed-point integers, WattNode outputs IEEE floats.
    assert(near(wattnode._device.get<WattNode::l1n_voltage>(), 225.8f));
    assert(near(wattnode._device.get<WattNode::l1_current>(), 6.574f));
    assert(near(wattnode._device.get<WattNode::l1_power_active>(), -1479.5f));
    assert(near(wattnode._device.get<WattNode::power_active>(), -1234.0f));
    uint16_t wire[2]{};
    assert(wattnode._device.readHoldingRegisters(1008, 2, wire));
    // -1234.0f = 0xC49A4000; low word first in WattNode.
    assert(wire[0] == 0x4000 && wire[1] == 0xc49a);
    std::cout << "PASS: compiled EM24-to-WattNode conversion and output wire words\n";
}
