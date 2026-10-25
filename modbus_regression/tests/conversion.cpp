#include "em24_e1.h"
#include "wattnode.h"
#include "alfen.h"
#include "solaredge.h"
#include "optimizer.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>

namespace modbus_gateway {
// Native owners expose the same Device/DataAccess interface as the transport wrappers.
// Only networking is replaced; the compiled conversion body remains unchanged.
template<class S> struct NativeOwner {
    using Schema = S;
    Device<S> _device;
    std::mutex _Mutex;
    struct Log { void addString(const String&) {} } _log;
};
template<class S> struct Client : NativeOwner<S> {};
template<class S> struct Server : NativeOwner<S> {};
class ConvertEM24_E1ToWattNode {
public:
    ConvertEM24_E1ToWattNode(Client<EM24_E1>& meter, Server<WattNode>& wattnode,
                             Server<Optimizer>& optimizer, Client<SolarEdge>& solaredge,
                             Client<Alfen>& alfen)
        : _meter(meter), _wattnode(wattnode), _optimizer(optimizer),
          _solaredge(solaredge), _alfen(alfen) {}
    void CopyDataFromMasterToSlave();
private:
    Client<EM24_E1>& _meter;
    Server<WattNode>& _wattnode;
    Server<Optimizer>& _optimizer;
    Client<SolarEdge>& _solaredge;
    Client<Alfen>& _alfen;
};
}

// Engineering-unit conversion used by the current typed register API.
namespace {
template<modbus_gateway::EM24_E1::RegisterId R, typename Access>
float engineering(const Access& access) {
    constexpr auto& descriptor = modbus_gateway::EM24_E1::description<R>();
    return static_cast<float>(access.template get<R>()) /
           modbus_gateway::getScaling(descriptor.scaling);
}
}

// run.py inserts the body of CopyDataFromMasterToSlave from project src/ here.
// The method under test is never reimplemented in the test suite.
#include "conversion_body.inc"

int main() {
    using namespace modbus_gateway;
    Client<EM24_E1> meter;
    Server<WattNode> wattnode;
    Server<Optimizer> optimizer;
    Client<SolarEdge> solaredge;
    Client<Alfen> alfen;
    meter._device.set<EM24_E1::l1_voltage>(2258);
    meter._device.set<EM24_E1::l1_current>(6574);
    meter._device.set<EM24_E1::l1_power_active>(-14795);
    meter._device.set<EM24_E1::power_active>(-12340);
    meter._device.set<EM24_E1::import_energy_active>(12000);
    meter._device.set<EM24_E1::export_energy_active>(3000);
    ConvertEM24_E1ToWattNode converter(meter, wattnode, optimizer, solaredge, alfen);
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
    // Meter raw power -12340 corresponds to -1234 W; each phase is -1479.5 W.
    // Strategy 2 keeps all 3000 W of charging consumption on the grid.
    alfen._device.set<Alfen::real_power_sum>(3000.0f);
    optimizer._device.set<Optimizer::optimizer_strategy>(2);
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -4234.0f));
    assert(near(wattnode._device.get<WattNode::l1_power_active>(), -2479.5f));

    // Strategy 1: 2000 W inverter AC, 0 W battery, 3000 W charging.
    // House = max(0, -1234 + 2000 - 3000) = 0 W.
    // PV covers 2000 W of charging; the remaining 1000 W comes from grid.
    optimizer._device.set<Optimizer::optimizer_strategy>(1);
    solaredge._device.set<SolarEdge::ac_power>(2000);
    solaredge._device.set<SolarEdge::ac_power_sf>(0);
    solaredge._device.set<SolarEdge::b1_instantaneous_power>(0.0f);
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -2234.0f));
    assert(near(wattnode._device.get<WattNode::l1_power_active>(), -1479.5f - 1000.0f/3.0f));

    // Strategy 0 returns to the unmodified meter measurements.
    optimizer._device.set<Optimizer::optimizer_strategy>(0);
    converter.CopyDataFromMasterToSlave();
    assert(near(wattnode._device.get<WattNode::power_active>(), -1234.0f));
    std::cout << "PASS: compiled conversion (strategies 0, 1, 2) and output wire words\n";
}
