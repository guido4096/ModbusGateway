#pragma once
#include "modbus/definitions.h"
#include "native_lock_state.h"

namespace modbus_gateway {
template<class S> struct NativeOwner {
    using Schema = S;
    Device<S> _device;
    NativeDeviceMutex _Mutex;
    Log _log;
    Statistics _statistics;
    String getStatistics() { return _statistics.allValuesAsString(); }
    String getLog() { return _log.allValuesAsString(); }
    void logMessage(const String& message) { _log.addString(message); }
};
} // namespace modbus_gateway
