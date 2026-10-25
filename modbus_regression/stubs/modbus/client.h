#pragma once
#include "modbus/native_owner.h"
#include <vector>

namespace modbus_gateway {
template<class S> struct Client : NativeOwner<S> {
    inline static Client* instance = nullptr;
    template<class... Args> explicit Client(Args&&...) { instance = this; }
    void connect() { ++connections; }
    bool readBlockFromMeter(const String& name) { blocks.push_back(name); return true; }
    bool consumeUpdated() { bool updated = _dataRead; _dataRead = false; return updated; }
    unsigned connections = 0;
    bool _dataRead = false;
    std::vector<String> blocks;
};
} // namespace modbus_gateway
