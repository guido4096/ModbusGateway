#pragma once
#include "modbus/native_owner.h"

namespace modbus_gateway {
template<class S> struct Server : NativeOwner<S> {
    inline static Server* instance = nullptr;
    Server() { instance = this; }
    template<class Transport, class... Args>
    Server(Transport&, uint8_t id, Args&&... args) {
        instance = this;
        this->_device = S::makeDevice(std::forward<Args>(args)...);
        this->_device.setServerId(id);
    }
};
} // namespace modbus_gateway
