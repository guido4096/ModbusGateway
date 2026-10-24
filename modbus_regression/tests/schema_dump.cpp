#include "alfen.h"
#include "em24_e1.h"
#include "optimizer.h"
#include "solaredge.h"
#include "wattnode.h"
#include <iostream>
using namespace modbus_gateway;
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
