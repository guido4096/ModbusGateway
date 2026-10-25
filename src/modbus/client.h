/**
 * @file      client.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Asynchronous Modbus TCP device reader
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
template<typename DeviceSchema>
class Client {
public:
    using Schema = DeviceSchema;
    using RegisterType = typename Schema::RegisterId;
    using DeviceType = Device<Schema>;
    static constexpr std::size_t request_capacity = 32;

    Client(ModbusClientTCP& transport, const IPAddress& remoteAddress, uint16_t tcpPort, uint16_t serverId)
        : _transport(transport), _remoteAddress(remoteAddress), _tcpPort(tcpPort), _serverId(serverId) {
        _transport.onDataHandler([this](ModbusMessage response, uint32_t token) {
            handleData(response, token);
        });
        _transport.onErrorHandler([this](Error error, uint32_t token) {
            handleError(error, token);
        });
    }

    ~Client() {
        // Stop/drain the transport before destruction; then detach its callbacks.
        _transport.onDataHandler(nullptr);
        _transport.onErrorHandler(nullptr);
    }

    void connect() {
        _transport.setTimeout(2000, 10);
        _transport.begin();
        _transport.setTarget(_remoteAddress, _tcpPort);
    }

    // Consume before processing. A callback arriving during processing leaves
    // another notification pending instead of having its flag cleared afterward.
    bool consumeUpdated() {
        return _updated.exchange(false, std::memory_order_acq_rel);
    }

    bool readFromMeter(RegisterType first, RegisterType last) {
        const auto* firstDescription = DeviceType::describe(first);
        const auto* lastDescription = DeviceType::describe(last);
        return firstDescription && lastDescription && readFromMeter(*firstDescription, *lastDescription);
    }

    template<RegisterType First, RegisterType Last>
    bool readFromMeter() {
        return readFromMeter(First, Last);
    }

    // The last logical register may span two words; include its complete width.
    bool readFromMeter(const RegisterDescription& first, const RegisterDescription& last) {
        if (first.block != last.block || first.address > last.address) {
            return false;
        }
        const uint32_t wordCount = uint32_t(last.address) + last.words - first.address;
        if (wordCount == 0 || wordCount > max_read_words) {
            return false;
        }
        return submit(first.block, first.address, static_cast<uint16_t>(wordCount));
    }

    bool readBlockFromMeter(const String& name) {
        const auto& blocks = DeviceType::blocks();
        for (std::size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex) {
            if (name == blocks[blockIndex].name) {
                return readBlockFromMeter(blockIndex);
            }
        }
        return false;
    }

    bool readBlockFromMeter(std::size_t blockIndex) {
        const auto& blocks = DeviceType::blocks();
        if (blockIndex >= blocks.size()) {
            return false;
        }
        const auto& block = blocks[blockIndex];
        return submit(static_cast<uint16_t>(blockIndex), block.address, block.words);
    }

    // Diagnostics synchronize themselves without taking the register lock.
    String getStatistics() { return _statistics.allValuesAsString(); }
    String getLog() { return _log.allValuesAsString(); }
    void logMessage(const String& message) { _log.addString(message); }

private:
    static constexpr uint16_t max_read_words = 125;
    static constexpr std::size_t response_header_bytes = 3;

    // Tokens are sequence numbers, never pointers. A slot is released by its
    // terminal callback or by an enqueue failure. Saturation never allocates.
    struct RequestContext {
        uint32_t token = 0;
        uint16_t blockIndex = 0;
        uint16_t address = 0;
        uint16_t wordCount = 0;
    };

    static bool validRequestRange(uint16_t blockIndex, uint16_t address, uint16_t wordCount) {
        const auto& blocks = DeviceType::blocks();
        if (wordCount == 0 || wordCount > max_read_words || blockIndex >= blocks.size()) {
            return false;
        }
        const auto& block = blocks[blockIndex];
        return address >= block.address &&
               uint32_t(address) + wordCount <= uint32_t(block.address) + block.words;
    }

    bool submit(uint16_t blockIndex, uint16_t address, uint16_t wordCount) {
        if (!validRequestRange(blockIndex, address, wordCount)) {
            return false;
        }
        _statistics.recordCall();

        const auto token = reserveRequest(blockIndex, address, wordCount);
        if (token == 0) {
            _statistics.recordFailure();
            _log.addString("Pending Modbus request capacity reached");
            return false;
        }
        const Error error = _transport.addRequest(token, _serverId, READ_HOLD_REGISTER, address, wordCount);
        if (error != SUCCESS) {
            takeRequest(token);
            _statistics.recordFailure();
            ModbusError modbusError(error);
            char buffer[180];
            snprintf(buffer, sizeof(buffer), "Error creating request: %02X - %s, addr=%u, count=%u",
                     (unsigned)(int)modbusError, (const char*)modbusError, address, wordCount);
            Serial.println(buffer);
            _log.addString(buffer);
            return false;
        }
        _statistics.recordSuccess();
        return true;
    }

    uint32_t reserveRequest(uint16_t blockIndex, uint16_t address, uint16_t wordCount) {
        std::lock_guard<std::mutex> lock(_requestMutex);
        for (auto& slot : _pendingRequests) {
            if (slot.token == 0) {
                slot = {nextRequestToken(), blockIndex, address, wordCount};
                return slot.token;
            }
        }
        return 0;
    }

    RequestContext takeRequest(uint32_t token) {
        std::lock_guard<std::mutex> lock(_requestMutex);
        for (auto& slot : _pendingRequests) {
            if (token != 0 && slot.token == token) {
                const auto request = slot;
                slot = {};
                return request;
            }
        }
        return {};
    }

    // Called only under _requestMutex. Zero denotes a free slot, and a wrapped
    // counter must not reuse a token still owned by another pending request.
    uint32_t nextRequestToken() {
        do {
            ++_lastRequestToken;
        } while (_lastRequestToken == 0 || tokenInUse(_lastRequestToken));
        return _lastRequestToken;
    }

    bool tokenInUse(uint32_t token) const {
        for (const auto& request : _pendingRequests) {
            if (request.token == token) {
                return true;
            }
        }
        return false;
    }

    void handleError(Error error, uint32_t token) {
        const auto request = takeRequest(token);
        if (request.token == 0) {
            return;
        }
        _statistics.recordError();
        ModbusError modbusError(error);
        char buffer[180];
        snprintf(buffer, sizeof(buffer), "Error response: %02X - %s, block=%s, transaction=%lu",
                 (unsigned)(int)modbusError, (const char*)modbusError,
                 DeviceType::blocks()[request.blockIndex].name, (unsigned long)request.token);
        Serial.println(buffer);
        _log.addString(buffer);
    }

    void handleData(ModbusMessage& response, uint32_t token) {
        const auto request = takeRequest(token);
        if (request.token == 0) {
            return;
        }
        if (!validResponse(response, request)) {
            _statistics.recordError();
            _log.addString("Invalid Modbus TCP response length/function/server ID");
            return;
        }
        std::array<uint16_t, max_read_words> words{};
        const uint8_t* payload = response.data() + response_header_bytes;
        for (std::size_t wordIndex = 0; wordIndex < request.wordCount; ++wordIndex) {
            const uint16_t highByte = payload[2 * wordIndex];
            const uint16_t lowByte = payload[2 * wordIndex + 1];
            words[wordIndex] = static_cast<uint16_t>((highByte << 8) | lowByte);
        }
        DataAccess<Client> access(*this);
        if (!access.writeHoldingRegisters(request.address, request.wordCount, words.data())) {
            _statistics.recordError();
            _log.addString("Received data outside device schema");
            return;
        }
        _updated.store(true, std::memory_order_release);
        _statistics.recordDataOk();
    }

    bool validResponse(ModbusMessage& response, const RequestContext& request) const {
        const auto payloadBytes = 2u * request.wordCount;
        return response.size() >= response_header_bytes &&
               response.getServerID() == _serverId &&
               response.getFunctionCode() == READ_HOLD_REGISTER &&
               response.size() == response_header_bytes + payloadBytes &&
               response.data()[2] == payloadBytes;
    }

    template<typename> friend class DataAccess;
    DeviceType _device{};
    Statistics _statistics;
    Log _log;
    std::atomic<bool> _updated{false};
    std::array<RequestContext, request_capacity> _pendingRequests{};
    uint32_t _lastRequestToken = 0;
    std::mutex _requestMutex;
    ModbusClientTCP& _transport;
    IPAddress _remoteAddress;
    uint16_t _tcpPort;
    uint16_t _serverId;
    mutable std::timed_mutex _Mutex;
};
} // namespace modbus_gateway
