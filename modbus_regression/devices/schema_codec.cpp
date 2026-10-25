#include "devices/em24_e1.h"
#include "devices/wattnode.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
using namespace modbus_gateway;

static_assert(EM24_E1::metadata.valid);
static_assert(WattNode::metadata.valid);
static_assert(EM24_E1::metadata.registers.size() == 52);
static_assert(WattNode::metadata.registers.size() == 110);
static_assert(EM24_E1::metadata.blocks.size() == 4);
static_assert(WattNode::metadata.blocks.size() == 8);
static_assert(Device<EM24_E1>::word_count == 98);
static_assert(Device<WattNode>::word_count == 173);
static_assert(EM24_E1::description<EM24_E1::l1_voltage>().address == 0);
static_assert(WattNode::description<WattNode::power_active>().address == 1008);

int main() {
    auto meter = EM24_E1::makeDevice();
    auto wattnode = WattNode::makeDevice(0x12345678u);
    uint16_t wire[2]{};

    // Reference values are Modbus wire words, not values calculated by the codec.
    const uint16_t voltage[] = {0x08d2, 0x0000}; // 2258 raw -> 225.8 V
    assert(meter.writeHoldingRegisters(0, 2, voltage));
    assert(meter.get<EM24_E1::l1_voltage>() == 2258);
    assert(meter.readHoldingRegisters(0, 2, wire));
    assert(wire[0] == voltage[0] && wire[1] == voltage[1]);

    const uint16_t current[] = {0x19ae, 0x0000}; // 6574 raw -> 6.574 A
    assert(meter.writeHoldingRegisters(12, 2, current));
    assert(meter.get<EM24_E1::l1_current>() == 6574);

    const uint16_t negative_power[] = {0xc635, 0xffff}; // -14795 raw
    assert(meter.writeHoldingRegisters(18, 2, negative_power));
    assert(meter.get<EM24_E1::l1_power_active>() == -14795);

    wattnode.set<WattNode::power_active>(1.0f);
    assert(wattnode.readHoldingRegisters(1008, 2, wire));
    assert(wire[0] == 0x0000 && wire[1] == 0x3f80);
    wattnode.set<WattNode::l1_power_active>(-1234.5f);
    assert(wattnode.readHoldingRegisters(1010, 2, wire));
    assert(wire[0] == 0x5000 && wire[1] == 0xc49a);
    assert(wattnode.get<WattNode::serial_number>() == 0x12345678u);
    assert(wattnode.readHoldingRegisters(1700, 2, wire));
    assert(wire[0] == 0x5678 && wire[1] == 0x1234);

    // Reading across a block hole or writing an invalid span must fail.
    assert(!meter.readHoldingRegisters(0x0054, 2, wire));
    const uint16_t invalid[] = {0x1234, 0x5678};
    assert(!meter.writeHoldingRegisters(0x0054, 2, invalid));

    // Every declared register is addressable and every block has its declared width.
    for (const auto& block : EM24_E1::metadata.blocks) {
        for (unsigned n = 0; n < block.words; ++n)
            assert(meter.readHoldingRegisters(block.address + n, 1, wire));
    }
    for (const auto& block : WattNode::metadata.blocks) {
        for (unsigned n = 0; n < block.words; ++n)
            assert(wattnode.readHoldingRegisters(block.address + n, 1, wire));
    }
    std::cout << "PASS: schema, codec, wire-order, defaults and address bounds\n";
}
