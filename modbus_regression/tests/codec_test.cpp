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
  const uint16_t raw[2]{0xff9c,0xffff}; // EM24 low-word-first -100 (=> -10 V after /10)
  assert(e.writeHoldingRegisters(0,2,raw));
  assert(e.get<EM24_E1::l1_voltage>()==-100);
  assert(e.readHoldingRegisters(0,2,words));
  assert(words[0]==0xff9c && words[1]==0xffff);
  // Captured corrupted readings reversed back into plausible EM24 values.
  const uint16_t voltage_sample[2]{0x08d2,0x0000}; // raw 2258 => 225.8 V
  assert(e.writeHoldingRegisters(0,2,voltage_sample));
  assert(e.get<EM24_E1::l1_voltage>() == 2258);
  const uint16_t current_sample[2]{0x19ae,0x0000}; // raw 6574 => 6.574 A
  assert(e.writeHoldingRegisters(12,2,current_sample));
  assert(e.get<EM24_E1::l1_current>() == 6574);
  const uint16_t power_sample[2]{0xc635,0xffff}; // raw -14795 => -1479.5 W
  assert(e.writeHoldingRegisters(18,2,power_sample));
  assert(e.get<EM24_E1::l1_power_active>() == -14795);
  assert(!e.readHoldingRegisters(0x0054,2,words)); // crosses hole before energy block
  assert(e.readHoldingRegisters(0,2,words));
  assert(words[0]==0x08d2 && words[1]==0x0000);
  auto o=Optimizer::makeDevice();
  o.set<Optimizer::optimizer_strategy>(-1);
  assert(o.readHoldingRegisters(0,2,words));
  assert(words[0]==0xffff && words[1]==0xffff);
  uint16_t input[2]{0x1234,0x5678};
  assert(!o.writeHoldingRegisters(1,2,input));
  assert(o.get<Optimizer::optimizer_strategy>()==-1); // rejected write atomic
  std::cout << "PASS native codec, scaled raw values, word orders, bounds, defaults\n";
}
