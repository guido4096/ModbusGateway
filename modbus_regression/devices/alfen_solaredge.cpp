#include "devices/alfen.h"
#include "devices/solaredge.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace modbus_gateway;

static_assert(Alfen::metadata.valid);
static_assert(SolarEdge::metadata.valid);
static_assert(Alfen::metadata.registers.size() == 13);
static_assert(SolarEdge::metadata.registers.size() == 5);
static_assert(Alfen::metadata.blocks.size() == 1);
static_assert(SolarEdge::metadata.blocks.size() == 3);
static_assert(SolarEdge::description<SolarEdge::ac_power>().address == 40083);

int main() {
    auto alfen = Alfen::makeDevice();
    auto solaredge = SolarEdge::makeDevice();
    uint16_t words[2]{};

    // Alfen power uses IEEE 754 in high-word-first Modbus order.
    alfen.set<Alfen::real_power_sum>(3000.0f);
    assert(alfen.get<Alfen::real_power_sum>() == 3000.0f);
    constexpr auto power = Alfen::description<Alfen::real_power_sum>();
    assert(alfen.readHoldingRegisters(power.address, 2, words));
    assert(words[0] == 0x453b && words[1] == 0x8000); // 3000.0f = 0x453B8000

    // SolarEdge AC power and scale factor are independent signed 16-bit values.
    const uint16_t ac[] = {0xf830, 0xfffe}; // -2000, scale 10^-2
    assert(solaredge.writeHoldingRegisters(40083, 2, ac));
    assert(solaredge.get<SolarEdge::ac_power>() == -2000);
    assert(solaredge.get<SolarEdge::ac_power_sf>() == -2);
    solaredge.set<SolarEdge::b1_instantaneous_power>(-500.0f);
    assert(solaredge.readHoldingRegisters(57716, 2, words));
    assert(words[0] == 0x0000 && words[1] == 0xc3fa); // -500.0f = 0xC3FA0000

    for (const auto& block : Alfen::metadata.blocks)
        for (unsigned n=0; n<block.words; ++n)
            assert(alfen.readHoldingRegisters(block.address+n, 1, words));
    for (const auto& block : SolarEdge::metadata.blocks)
        for (unsigned n=0; n<block.words; ++n)
            assert(solaredge.readHoldingRegisters(block.address+n, 1, words));
    std::cout << "PASS: Alfen/SolarEdge schemas, codecs and wire words\n";
}
