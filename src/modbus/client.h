/**
 * @file      client.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Object that acts as a master to retrieve values from the meter
 */
#pragma once
#include <array>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <cstdio>
#include "modbus/definitions.h"
#include "ModbusClientTCP.h"

namespace modbus_gateway {
// The caller owns the transport and must keep it and this client alive until
// accepted requests have completed. One client owns the callback pair per transport.
template<class MODBUS_TYPE>
class Client {
public:
    using Schema = MODBUS_TYPE;
    using RegisterType = typename Schema::RegisterId;
    using DeviceType = Device<Schema>;
    static constexpr std::size_t request_capacity = 32;

    Client(ModbusClientTCP& tcp, const IPAddress& remote, uint16_t tcp_port, uint16_t tcp_server_id)
        : _tcp(tcp), _remote(remote), _tcp_port(tcp_port), _tcp_server_id(tcp_server_id) {
        _tcp.onDataHandler([this](ModbusMessage response, uint32_t token) { handleData(response, token); });
        _tcp.onErrorHandler([this](Error error, uint32_t token) { handleError(error, token); });
    }
    ~Client() {
        // Stop/drain the transport before destruction; then detach its callbacks.
        _tcp.onDataHandler(nullptr);
        _tcp.onErrorHandler(nullptr);
    }
    void connect() {
        _tcp.setTimeout(2000, 10);
        _tcp.begin();
        _tcp.setTarget(_remote, _tcp_port);
    }

    // Consume before processing. A callback arriving during processing leaves
    // another notification pending instead of having its flag cleared afterward.
    bool consumeUpdated() { return _updated.exchange(false, std::memory_order_acq_rel); }

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
        for (std::size_t i = 0; i < blocks.size(); ++i)
            if (name == blocks[i].name) return readBlockFromMeter(i);
        return false;
    }
    bool readBlockFromMeter(std::size_t index) {
        const auto& blocks = DeviceType::blocks();
        if (index >= blocks.size()) return false;
        const auto& b = blocks[index];
        return submit(static_cast<uint16_t>(index), b.address, b.words);
    }

private:
    // Tokens are sequence numbers, never pointers. Slots are released on every
    // terminal callback and synchronous enqueue failure. Saturation rejects a
    // request without allocating; capacity is independent of the transport queue.
    struct RequestContext {
        uint32_t token = 0;
        uint16_t block_index = 0;
        uint16_t start = 0;
        uint16_t count = 0;
    };
    uint32_t reserveRequest(uint16_t block, uint16_t address, uint16_t count) {
        std::lock_guard<std::mutex> lock(_requestMutex);
        for (auto& slot : _requests) {
            if (slot.token) continue;
            bool collision;
            do {
                ++_transaction;
                collision = _transaction == 0;
                for (const auto& used : _requests)
                    if (used.token == _transaction) collision = true;
            } while (collision);
            slot = {_transaction, block, address, count};
            return slot.token;
        }
        return 0;
    }
    RequestContext takeRequest(uint32_t token) {
        std::lock_guard<std::mutex> lock(_requestMutex);
        for (auto& slot : _requests) {
            if (!token || slot.token != token) continue;
            const auto result = slot;
            slot = {};
            return result;
        }
        return {};
    }
    bool submit(uint16_t block, uint16_t address, uint16_t count) {
        const auto& blocks = DeviceType::blocks();
        if (!count || count > 125 || block >= blocks.size()) return false;
        const auto& b = blocks[block];
        if (address < b.address || uint32_t(address) + count > uint32_t(b.address) + b.words)
            return false;
        _statistics.recordCall();
        const auto token = reserveRequest(block, address, count);
        if (!token) {
            _statistics.recordFailure();
            _log.addString("Pending Modbus request capacity reached");
            return false;
        }
        const Error err = _tcp.addRequest(token, _tcp_server_id, READ_HOLD_REGISTER, address, count);
        if (err != SUCCESS) {
            takeRequest(token);
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
        return true;
    }
    void handleError(Error error, uint32_t token) {
        const auto ctx = takeRequest(token);
        if (!ctx.token) return;
        _statistics.recordError();
        ModbusError e(error);
        char buffer[180];
        snprintf(buffer, sizeof(buffer), "Error response: %02X - %s, block=%s, transaction=%lu",
                 (unsigned)(int)e, (const char*)e,
                 DeviceType::blocks()[ctx.block_index].name, (unsigned long)ctx.token);
        Serial.println(buffer);
        _log.addString(buffer);
    }
    void handleData(ModbusMessage& response, uint32_t token) {
        const auto ctx = takeRequest(token);
        if (!ctx.token) return;
        constexpr std::size_t header = 3;
        if (response.size() < header || response.getServerID() != _tcp_server_id ||
            response.getFunctionCode() != READ_HOLD_REGISTER ||
            response.size() != header + 2u * ctx.count || response.data()[2] != ctx.count * 2u) {
            _statistics.recordError();
            _log.addString("Invalid Modbus TCP response length/function/server ID");
            return;
        }
        std::array<uint16_t, 125> words{};
        const uint8_t* p = response.data() + header;
        for (std::size_t i = 0; i < ctx.count; ++i)
            words[i] = static_cast<uint16_t>((uint16_t(p[2*i]) << 8) | p[2*i+1]);
        DataAccess<Client> access(*this);
        if (!access.writeHoldingRegisters(ctx.start, ctx.count, words.data())) {
            _statistics.recordError();
            _log.addString("Received data outside device schema");
            return;
        }
        _updated.store(true, std::memory_order_release);
        _statistics.recordDataOk();
    }

    template<typename> friend class DataAccess;
    DeviceType _device{};
    Statistics _statistics;
    Log _log;
    std::atomic<bool> _updated{false};
    std::array<RequestContext, request_capacity> _requests{};
    uint32_t _transaction = 0;
    std::mutex _requestMutex;
    ModbusClientTCP& _tcp;
    IPAddress _remote;
    uint16_t _tcp_port;
    uint16_t _tcp_server_id;
    mutable std::timed_mutex _Mutex;
};
} // namespace modbus_gateway
