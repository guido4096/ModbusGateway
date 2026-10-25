#pragma once
#include <Arduino.h>
#include <cstdint>
#include <functional>
#include <map>
#include <vector>
enum Error { SUCCESS=0, ILLEGAL_FUNCTION=1, ILLEGAL_DATA_ADDRESS=2,
             ILLEGAL_DATA_VALUE=3, TIMEOUT=0xe0, REQUEST_QUEUE_FULL=0xe1 };
enum FunctionCode { READ_HOLD_REGISTER=3, WRITE_HOLD_REGISTER=6, WRITE_MULT_REGISTERS=16 };
struct ModbusError {
    Error value;
    explicit ModbusError(Error e) : value(e) {}
    operator int() const { return value; }
    operator const char*() const { return "simulated transport error"; }
};
class ModbusMessage {
public:
    ModbusMessage() = default;
    ModbusMessage(std::initializer_list<uint8_t> data) : bytes(data) {}
    std::vector<uint8_t> bytes;
    std::size_t size() { return bytes.size(); }
    const uint8_t* data() { return bytes.data(); }
    uint8_t getServerID() { return size() ? bytes[0] : 0; }
    uint8_t getFunctionCode() { return size() > 1 ? bytes[1] : 0; }
    void get(std::size_t pos, uint16_t& output) const {
        output = uint16_t(bytes.at(pos)) << 8 | bytes.at(pos + 1);
    }
    void add(uint8_t value) { bytes.push_back(value); }
    void add(uint16_t value) { bytes.push_back(value >> 8); bytes.push_back(value); }
    void add(FunctionCode value) { add(static_cast<uint8_t>(value)); }
    template<class T, class U, class... Rest> void add(T a, U b, Rest... rest) {
        add(a); add(b, rest...);
    }
    void setError(uint8_t server, uint8_t function, Error error) {
        bytes = {server, static_cast<uint8_t>(function | 0x80), static_cast<uint8_t>(error)};
    }
};
