#pragma once
#include "modbus/definitions.h"
#include <mutex>

namespace modbus_gateway {
template<class S> struct NativeOwner {
    using Schema = S;
    Device<S> _device;
    std::mutex _Mutex;
    Log _log;
    Statistics _statistics;
};
} // namespace modbus_gateway
