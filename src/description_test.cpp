#include "alfen.h"
#include "em24_e1.h"
#include "optimizer.h"
#include "solaredge.h"
#include "wattnode.h"
#include <cassert>
#include <string>
using namespace modbus_gateway;
struct Sink { std::string value; void print(const char* s) { value += s; } };
int main() {
  static_assert(WattNode::description<WattNode::power_active>().address == 1008);
  Sink out;
  printDeviceDescription<WattNode>(out);
  assert(out.value.find("Device: wattnode") != std::string::npos);
  assert(out.value.find("0x03f0") != std::string::npos);
  auto a = getDeviceDescription<Alfen>();
  auto e = getDeviceDescription<EM24_E1>();
  auto o = getDeviceDescription<Optimizer>();
  auto s = getDeviceDescription<SolarEdge>();
  assert(a.length() && e.length() && o.length() && s.length());
}
