#include "plugin/devices/optimizer.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace modbus_gateway;

static_assert(Optimizer::metadata.valid);
static_assert(Optimizer::metadata.registers.size() == 1);
static_assert(Optimizer::metadata.blocks.size() == 1);
static_assert(Optimizer::description<Optimizer::optimizer_strategy>().address == 0);

int main() {
    auto optimizer = Optimizer::makeDevice();
    uint16_t words[2]{};
    optimizer.set<Optimizer::optimizer_strategy>(2);
    assert(optimizer.get<Optimizer::optimizer_strategy>() == 2);
    assert(optimizer.readHoldingRegisters(0, 2, words));
    assert(words[0] == 2 && words[1] == 0);
    for (const auto& block : Optimizer::metadata.blocks)
        for (unsigned n=0; n<block.words; ++n)
            assert(optimizer.readHoldingRegisters(block.address+n, 1, words));
    std::cout << "PASS: plugin-specific Optimizer schema and wire words\n";
}
