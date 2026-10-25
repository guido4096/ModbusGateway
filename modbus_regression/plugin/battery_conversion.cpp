#include "plugin/convert_em24_e1_to_wattnode.h"
#include "plugin/battery_inputs.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {
modbus_gateway::battery_optimizer::Inputs inputs{};
std::vector<std::string> messages;
}

// Substitute transport snapshots and log storage; the converter and power hook
// are compiled directly from production sources.
namespace modbus_gateway::battery_optimizer {
Inputs readInputs() { return inputs; }
void logAdjustment(const char* message) { messages.emplace_back(message); }
}

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
    ConvertEM24_E1ToWattNode converter(meter, wattnode);
    converter.CopyDataFromMasterToSlave();
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
    // Additional input devices affect only the optimizer strategies.
    // Meter raw power -12340 corresponds to -1234 W; phase L1 is -1479.5 W.
    // Strategy 2 keeps all 3000 W of charging consumption on the grid.
    inputs.charging_power = 3000.0f;
    inputs.strategy = 2;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -4234.0f));
    assert(near(wattnode._device.get<WattNode::l1_power_active>(), -2479.5f));

    // Strategy 1: 2000 W inverter AC, 0 W battery, 3000 W charging.
    // House = max(0, -1234 + 2000 - 3000) = 0 W.
    // PV covers 2000 W of charging; the remaining 1000 W comes from grid.
    inputs.strategy = 1;
    inputs.ac_power = 2000;
    inputs.ac_scale_factor = 0;
    inputs.battery_power = 0.0f;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -2234.0f));
    assert(near(wattnode._device.get<WattNode::l1_power_active>(), -1479.5f - 1000.0f/3.0f));

    // Strategy 0 returns to the unmodified meter measurements.
    inputs.strategy = 0;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -1234.0f));
    assert(messages.size() == 2); // strategies 1 and 2 log; normal does not
    assert(wattnode._device.get<WattNode::energy_active>() == 1500.0f);

    // Negative PV estimate: battery discharge must not count as PV production.
    inputs.strategy = 1;
    inputs.ac_power = 20000;
    inputs.ac_scale_factor = -1;
    inputs.battery_power = -3000.0f;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -4234.0f));
    assert(wattnode._device.readHoldingRegisters(1008, 2, wire));
    assert(wire[0] == 0x5000 && wire[1] == 0xc584); // -4234.0f, low first
    assert(near(wattnode._device.get<WattNode::l2_power_active>(), -1000.0f));
    assert(near(wattnode._device.get<WattNode::l3_power_active>(), -1000.0f));

    // House gets PV first: 5000 W AC, 3000 W charging and 1234 W grid
    // consumption leave 1766 W PV for the car, requiring 1234 W grid power.
    meter._device.set<EM24_E1::power_active>(12340);
    inputs.ac_power = 5000;
    inputs.ac_scale_factor = 0;
    inputs.battery_power = 0;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), 0.0f));
    assert(wattnode._device.get<WattNode::energy_active>() == 1500.0f);

    // All car power is covered by PV, including PV sent into the battery.
    meter._device.set<EM24_E1::power_active>(-12340);
    inputs.battery_power = 2000;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -1234.0f));

    // An unknown strategy retains the existing unmodified-power behavior.
    inputs.strategy = 99;
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -1234.0f));
    std::cout << "PASS: compiled conversion (strategies 0, 1, 2) and output wire words\n";
}
