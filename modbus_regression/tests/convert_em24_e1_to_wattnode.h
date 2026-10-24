#pragma once
#include "alfen.h"
#include "em24_e1.h"
#include "optimizer.h"
#include "solaredge.h"
#include "wattnode.h"
#include <mutex>
#include <cmath>
namespace modbus_gateway {
template<class S> struct FakeOwner {using Schema=S; Device<S> _device; std::mutex _Mutex; struct Logger{ void addString(const String&) {} } _log;};
template<class S> struct Client:FakeOwner<S> {};
template<class S> struct Server:FakeOwner<S> {};
template<auto R, class O> float legacyFloat(const DataAccess<O>& access) {
 using S=typename O::Schema;
 auto raw=access.template get<R>();
 constexpr auto d=S::template description<R>();
 float scale=static_cast<float>(getScaling(d.scaling));
 return std::round(static_cast<float>(raw)*scale)/scale/scale;
}
template<auto R, class O> int16_t legacyI16(const DataAccess<O>& access) {
 using S=typename O::Schema;
 constexpr auto d=S::template description<R>();
 return static_cast<int16_t>(access.template get<R>() / getScaling(d.scaling));
}
template<auto R, class O> int32_t legacyI32(const DataAccess<O>& access) {
 using S=typename O::Schema;
 constexpr auto d=S::template description<R>();
 return static_cast<int32_t>(access.template get<R>() / getScaling(d.scaling));
}
class ConvertEM24_E1ToWattNode {
public:
 Client<EM24_E1>& _meter; Server<WattNode>& _wattnode; Server<Optimizer>& _optimizer; Client<SolarEdge>& _solaredge; Client<Alfen>& _alfen;
 ConvertEM24_E1ToWattNode(Client<EM24_E1>& a,Server<WattNode>& b,Server<Optimizer>& c,Client<SolarEdge>& d,Client<Alfen>& e):_meter(a),_wattnode(b),_optimizer(c),_solaredge(d),_alfen(e){}
 void CopyDataFromMasterToSlave();
};
}
