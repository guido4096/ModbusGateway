#pragma once
#include "Transport.h"
class ModbusClientTCP {
public:
    struct Request { uint32_t token; uint8_t server; uint16_t address; uint16_t count; };
    std::function<void(ModbusMessage, uint32_t)> data_handler;
    std::function<void(Error, uint32_t)> error_handler;
    std::vector<Request> pending;
    Error enqueue_error = SUCCESS;
    bool onDataHandler(decltype(data_handler) f) { data_handler = f; return true; }
    bool onErrorHandler(decltype(error_handler) f) { error_handler = f; return true; }
    void setTimeout(int, int) {}
    void begin() {}
    void setTarget(IPAddress, uint16_t) {}
    Error addRequest(uint32_t token, uint8_t server, uint8_t, uint16_t address, uint16_t count) {
        if (enqueue_error != SUCCESS) return enqueue_error;
        pending.push_back({token, server, address, count});
        return SUCCESS;
    }
    void respond(ModbusMessage message, std::size_t index = 0) {
        auto token = pending.at(index).token;
        pending.erase(pending.begin() + index);
        data_handler(message, token);
    }
    void fail(Error error = TIMEOUT) {
        auto token = pending.at(0).token;
        pending.erase(pending.begin());
        error_handler(error, token);
    }
};
