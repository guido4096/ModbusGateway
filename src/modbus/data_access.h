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
/** Scope-bound exclusive access to an owner's register values.
 * The guard acquires the owner mutex for its entire lifetime; callers must
 * not retain the object across network operations or other blocking work.
 * The owner grants access to _device and _Mutex through friendship.
 */
template<typename Owner>
class DataAccess {
public:
    using Schema = typename Owner::Schema;
    using RegisterType = typename Schema::RegisterId;

    explicit DataAccess(Owner& owner) : owner_(owner) {
        owner_._Mutex.lock();
    }

    ~DataAccess() {
        owner_._Mutex.unlock();
    }

    DataAccess(const DataAccess&) = delete;
    DataAccess& operator=(const DataAccess&) = delete;

    template<RegisterType Register>
    auto get() const {
        return owner_._device.template get<Register>();
    }

    template<RegisterType Register>
    void set(typename Device<Schema>::template value_type<Register> value) {
        owner_._device.template set<Register>(value);
    }

    String allValuesAsString() const {
        return owner_._device.allValuesAsString();
    }

    bool readHoldingRegisters(uint16_t address, uint16_t count, uint16_t* output) const {
        return owner_._device.readHoldingRegisters(address, count, output);
    }

    bool writeHoldingRegisters(uint16_t address, uint16_t count, const uint16_t* input) {
        return owner_._device.writeHoldingRegisters(address, count, input);
    }

private:
    Owner& owner_;
};

} // namespace modbus_gateway
