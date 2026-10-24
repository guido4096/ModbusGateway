#include "alfen.h"
#include "em24_e1.h"
#include "optimizer.h"
#include "solaredge.h"
#include "wattnode.h"
#include <iostream>
using namespace modbus_gateway;
// Counts and storage size are inferred from the field and block declarations.
static_assert(Alfen::metadata.blocks.size() == 1);
static_assert(Alfen::metadata.registers.size() == 13);
static_assert(Device<Alfen>::word_count == 26);
static_assert(EM24_E1::metadata.blocks.size() == 4);
static_assert(EM24_E1::metadata.registers.size() == 52);
static_assert(Device<EM24_E1>::word_count == 98);
static_assert(Optimizer::metadata.blocks.size() == 1);
static_assert(Device<Optimizer>::word_count == 2);
static_assert(SolarEdge::metadata.blocks.size() == 3);
static_assert(Device<SolarEdge>::word_count == 6);
static_assert(WattNode::metadata.blocks.size() == 8);
static_assert(WattNode::metadata.registers.size() == 110);
static_assert(Device<WattNode>::word_count == 173);

template<typename S> void dump() {
  for (size_t i=0;i<S::metadata.registers.size();++i) {
    auto &r=S::metadata.registers[i];
    auto &b=S::metadata.blocks[r.block];
    std::cout<<S::name<<'\t'<<b.name<<'\t'<<b.address<<'\t'<<i<<'\t'
      <<r.address<<'\t'<<int(r.type)<<'\t'<<int(r.scaling)<<'\t'
      <<int(r.high_word_first)<<'\t'<<r.name<<'\t'<<r.unit<<'\t'<<r.default_bits<<'\n';
  }
}
int main(){dump<Alfen>(); dump<EM24_E1>(); dump<Optimizer>(); dump<SolarEdge>(); dump<WattNode>();}
