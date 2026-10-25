/**
 * @file      data_access.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Scope-bound synchronized device access
 */
#pragma once
#include "modbus/device.h"

namespace modbus_gateway {
// Optional lock-scoped access. The owner must expose _Mutex and _device.
/** Scope-bound exclusive access to an owner's device and diagnostics.
 * The guard acquires the owner mutex for its entire lifetime; callers must
 * not retain the object across network operations or other blocking work.
 * The owner provides _device, _Mutex, _log and _statistics as friends.
 */
template<typename Owner>
class DataAccess {
public:
    using Schema = typename Owner::Schema;
    using RegisterType = typename Schema::RegisterId;
    explicit DataAccess(Owner& owner) : owner_(owner) { owner_._Mutex.lock(); }
    ~DataAccess() { owner_._Mutex.unlock(); }
    DataAccess(const DataAccess&) = delete;
    DataAccess& operator=(const DataAccess&) = delete;

    template<RegisterType R>
    auto get() const { return owner_._device.template get<R>(); }
    template<RegisterType R>
    void set(typename Device<Schema>::template value_type<R> v) {
        owner_._device.template set<R>(v);
    }
    String allValuesAsString() const { return owner_._device.allValuesAsString(); }
    String getStatistics() { return owner_._statistics.allValuesAsString(); }
    String getLog() { return owner_._log.allValuesAsString(); }
    void logMessage(const String& message) { owner_._log.addString(message); }
    bool readHoldingRegisters(uint16_t addr, uint16_t count, uint16_t* dst) const {
        return owner_._device.readHoldingRegisters(addr,count,dst);
    }
    bool writeHoldingRegisters(uint16_t addr, uint16_t count, const uint16_t* src) {
        return owner_._device.writeHoldingRegisters(addr,count,src);
    }
private:
    Owner& owner_;
};

} // namespace modbus_gateway
