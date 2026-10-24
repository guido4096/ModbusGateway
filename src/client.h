/**
 * @file      client.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Object that acts as a master to retrieve values from the meter
 */
#pragma once
#include <mutex>
#include <memory>
#include <cstdint>
#include <cstdio>
#include "definitions.h"
#include "ModbusClientTCP.h"

namespace modbus_gateway {
template<class MODBUS_TYPE>
class Client {
public:
    using Schema = MODBUS_TYPE;
    using RegisterType = typename Schema::RegisterId;
    using DeviceType = Device<Schema>;

    Client(ModbusClientTCP& tcp, const IPAddress& remote, uint16_t tcp_port, uint16_t tcp_server_id)
        : _tcp(tcp), _remote(remote), _tcp_port(tcp_port), _tcp_server_id(tcp_server_id) {
        _tcp.onDataHandler(&Client::handleData);
        _tcp.onErrorHandler(&Client::handleError);
    }
    void connect() {
        _tcp.setTimeout(2000, 10);
        _tcp.begin();
        _tcp.setTarget(_remote, _tcp_port);
    }

    // IDs map directly into the constexpr register array; no name search.
    bool readFromMeter(RegisterType first, RegisterType last) {
        const auto* a = DeviceType::describe(first);
        const auto* b = DeviceType::describe(last);
        return a && b && readFromMeter(*a, *b);
    }
    template<RegisterType First, RegisterType Last>
    bool readFromMeter() { return readFromMeter(First, Last); }

    bool readFromMeter(const RegisterDescription& first, const RegisterDescription& last) {
        if (first.block != last.block || first.address > last.address) return false;
        const uint32_t count = uint32_t(last.address) + last.words - first.address;
        if (!count || count > 125) return false;
        return submit(first.block, first.address, static_cast<uint16_t>(count));
    }
    bool readBlockFromMeter(const String& name) {
        const auto& blocks = DeviceType::blocks();
        for (std::size_t i = 0; i < blocks.size(); ++i) {
            if (name == blocks[i].name)
                return blocks[i].words > 0 && blocks[i].words <= 125 &&
                       submit(static_cast<uint16_t>(i), blocks[i].address, blocks[i].words);
        }
        return false;
    }
    // No string search for callers that already have a block index.
    bool readBlockFromMeter(std::size_t index) {
        const auto& blocks = DeviceType::blocks();
        if (index >= blocks.size()) return false;
        const auto& b = blocks[index];
        return b.words > 0 && b.words <= 125 && submit(static_cast<uint16_t>(index), b.address, b.words);
    }

    template<typename> friend class DataAccess;
    bool _dataRead = false;
    DeviceType _device{};

private:
    struct RequestContext {
        Client* owner;
        uint16_t block_index;
        uint16_t start;
        uint16_t count;
        uint32_t transaction;
    };

    bool submit(uint16_t block, uint16_t address, uint16_t count) {
#if defined(ARDUINO_ARCH_ESP32)
        static_assert(sizeof(void*) == sizeof(uint32_t),
                      "This ModbusClientTCP callback token requires a 32-bit pointer");
#endif
        auto context = std::unique_ptr<RequestContext>(new RequestContext{
            this, block, address, count, _transaction++});
        const auto token = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(context.get()));
        _statistics.recordCall();
        const Error err = _tcp.addRequest(token, _tcp_server_id, READ_HOLD_REGISTER, address, count);
        if (err != SUCCESS) {
            _statistics.recordFailure();
            ModbusError e(err);
            char buffer[180];
            snprintf(buffer, sizeof(buffer), "Error creating request: %02X - %s, addr=%u, count=%u",
                     (unsigned)(int)e, (const char*)e, address, count);
            Serial.println(buffer);
            _log.addString(buffer);
            return false;
        }
        _statistics.recordSuccess();
        context.release(); // callback owns it after a successful enqueue
        return true;
    }
    static void handleError(Error error, uint32_t token) {
        std::unique_ptr<RequestContext> ctx(reinterpret_cast<RequestContext*>(static_cast<uintptr_t>(token)));
        if (!ctx) return;
        Client& self = *ctx->owner;
        self._statistics.recordError();
        ModbusError e(error);
        char buffer[180];
        snprintf(buffer, sizeof(buffer), "Error response: %02X - %s, block=%s, transaction=%lu",
                 (unsigned)(int)e, (const char*)e,
                 DeviceType::blocks()[ctx->block_index].name, (unsigned long)ctx->transaction);
        Serial.println(buffer);
        self._log.addString(buffer);
    }
    static void handleData(ModbusMessage response, uint32_t token) {
        std::unique_ptr<RequestContext> ctx(reinterpret_cast<RequestContext*>(static_cast<uintptr_t>(token)));
        if (!ctx) return;
        Client& self = *ctx->owner;
        // Library response: server ID, function code, byte count, big-endian payload.
        constexpr std::size_t header = 3;
        if (response.size() < header || response.getFunctionCode() != READ_HOLD_REGISTER ||
            response.size() != header + 2u * ctx->count ||
            response.data()[2] != ctx->count * 2u) {
            self._statistics.recordError();
            self._log.addString("Invalid Modbus TCP response length/function");
            return;
        }
        std::array<uint16_t, 125> words{};
        const uint8_t* p = response.data() + header;
        for (std::size_t i = 0; i < ctx->count; ++i)
            words[i] = static_cast<uint16_t>((uint16_t(p[2*i]) << 8) | p[2*i+1]);
        DataAccess<Client> access(self);
        if (!access.writeHoldingRegisters(ctx->start, ctx->count, words.data())) {
            self._statistics.recordError();
            self._log.addString("Received data outside device schema");
            return;
        }
        self._dataRead = true;
        self._statistics.recordDataOk();
    }

    Statistics _statistics;
    Log _log;
    uint32_t _transaction = 0;
    ModbusClientTCP& _tcp;
    IPAddress _remote;
    uint16_t _tcp_port;
    uint16_t _tcp_server_id;
    mutable std::timed_mutex _Mutex;
};
} // namespace modbus_gateway
