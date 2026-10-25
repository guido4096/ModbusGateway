#pragma once
struct ModbusServerRTU {
    explicit ModbusServerRTU(int) {}
    template<class T> void begin(T&) { ++starts; }
    inline static unsigned starts = 0;
};
