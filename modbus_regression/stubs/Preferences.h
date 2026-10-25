#pragma once
#include <Arduino.h>
#include "native_lock_state.h"
#include <map>

class Preferences {
public:
    bool begin(const char* name, bool) { space = name; return true; }
    int32_t getInt(const char* key, int32_t fallback) {
        const auto pos = values.find(space + "/" + key);
        return pos == values.end() ? fallback : pos->second;
    }
    void putInt(const char* key, int32_t value) {
        assert(native_device_locks == 0);
        values[space + "/" + key] = value;
    }
    inline static std::map<String, int32_t> values;
    String space;
};
