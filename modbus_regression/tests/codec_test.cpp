#include "alfen.h"
#include "em24_e1.h"
#include "optimizer.h"
#include "solaredge.h"
#include "wattnode.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace modbus_gateway;
int main() {
  auto w = WattNode::makeDevice(0x12345678u);
  w.set<WattNode::power_active>(1.0f);
  uint16_t words[4]{};
  assert(w.readHoldingRegisters(1008,2,words));
  assert(words[0]==0x3f80 && words[1]==0x0000);
  assert(w.get<WattNode::power_active>() == 1.0f);
  w.set<WattNode::l1_power_active>(-1234.5f);
  assert(w.readHoldingRegisters(1010,2,words));
  assert(words[0]==0xc49a && words[1]==0x5000); // IEEE754 -1234.5
  assert(w.get<WattNode::serial_number>()==0x12345678u);
  auto a = Alfen::makeDevice();
  a.set<Alfen::current_l1>(1.0f);
  assert(a.readHoldingRegisters(320,2,words));
  assert(words[0]==0x0000 && words[1]==0x3f80);
  auto e=EM24_E1::makeDevice();
  const uint16_t raw[2]{0xffff,0xff9c}; // -100, signed 32-bit (=> -10 V after /10)
  assert(e.writeHoldingRegisters(0,2,raw));
  assert(e.get<EM24_E1::l1_voltage>()==-100);
  assert(!e.readHoldingRegisters(0x0054,2,words)); // crosses hole before energy block
  assert(e.readHoldingRegisters(0,2,words));
  assert(words[0]==0xffff && words[1]==0xff9c);
  auto o=Optimizer::makeDevice();
  o.set<Optimizer::optimizer_strategy>(-1);
  assert(o.readHoldingRegisters(0,2,words));
  assert(words[0]==0xffff && words[1]==0xffff);
  uint16_t input[2]{0x1234,0x5678};
  assert(!o.writeHoldingRegisters(1,2,input));
  assert(o.get<Optimizer::optimizer_strategy>()==-1); // rejected write atomic
  std::cout << "PASS native codec, scaled raw values, word orders, bounds, defaults\n";
}
