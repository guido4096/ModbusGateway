/**
 * @file      server.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Modbus server backed by a device register schema
 */
#pragma once
#include <array>
#include <cstdint>
#include <mutex>
#include <type_traits>
#include <utility>
#include "modbus/definitions.h"
#include "ModbusServer.h"

namespace modbus_gateway {
// Some schemas mirror the protocol server ID in a logical register. Resolve
// that optional binding at compile time; other schemas need no such register.
template<typename Schema, typename = void>
struct HasServerIdRegister : std::false_type {};

template<typename Schema>
struct HasServerIdRegister<Schema, std::void_t<decltype(Schema::server_id_register)>>
    : std::true_type {};

// FC03, FC06 and FC16 expose Device's raw words. Typed application access is
// provided separately through DataAccess; protocol requests never apply scaling.
template<typename DeviceSchema>
class Server {
public:
    using Schema = DeviceSchema;
    using RegisterType = typename Schema::RegisterId;
    using DeviceType = Device<Schema>;

    // Factory arguments belong to the device instance, independently of its
    // protocol ID (for example, a serial number forwarded to makeDevice()).
    template<typename... DeviceArgs>
    Server(ModbusServer& transport, uint8_t serverId, DeviceArgs&&... deviceArgs)
        : _device(Schema::makeDevice(std::forward<DeviceArgs>(deviceArgs)...)),
          _transport(transport) {
        _device.setServerId(serverId);
        if constexpr (HasServerIdRegister<Schema>::value) {
            _device.template set<Schema::server_id_register>(serverId);
        }
        registerWorkers(serverId);
    }

    ~Server() {
        // Stop request processing before destruction: an in-flight callback
        // still refers to this instance even after its worker is unregistered.
        if (!_registered) {
            return;
        }
        for (auto function : {READ_HOLD_REGISTER, WRITE_HOLD_REGISTER, WRITE_MULT_REGISTERS}) {
            _transport.unregisterWorker(_device.serverId(), function);
        }
    }

    bool registered() const { return _registered; }

    // Diagnostics synchronize themselves without taking the register lock.
    String getStatistics() { return _statistics.allValuesAsString(); }
    String getLog() { return _log.allValuesAsString(); }
    void logMessage(const String& message) { _log.addString(message); }

private:
    static constexpr uint16_t max_read_words = 125;
    static constexpr uint16_t max_write_words = 123;
    static constexpr std::size_t request_header_bytes = 6;

    struct Request {
        uint16_t address = 0;
        uint16_t wordCount = 0;
        // FC06 carries a value where FC03/FC16 carry the register count.
        uint16_t valueOrCount = 0;
    };

    void registerWorkers(uint8_t serverId) {
        // Reject occupied endpoints rather than replacing another instance.
        for (auto function : {READ_HOLD_REGISTER, WRITE_HOLD_REGISTER, WRITE_MULT_REGISTERS}) {
            if (_transport.isServerFor(serverId, function)) {
                Serial.println("Modbus server ID/function already registered");
                return;
            }
        }
        _transport.registerWorker(serverId, READ_HOLD_REGISTER,
            [this](ModbusMessage message) { return processRequest(message, READ_HOLD_REGISTER); });
        _transport.registerWorker(serverId, WRITE_HOLD_REGISTER,
            [this](ModbusMessage message) { return processRequest(message, WRITE_HOLD_REGISTER); });
        _transport.registerWorker(serverId, WRITE_MULT_REGISTERS,
            [this](ModbusMessage message) { return processRequest(message, WRITE_MULT_REGISTERS); });
        _registered = true;
    }

    ModbusMessage processRequest(ModbusMessage& message, FunctionCode function) {
        _statistics.recordCall();

        Request request;
        const Error error = validateRequest(message, function, request);
        if (error != SUCCESS) {
            return errorResponse(message, error);
        }
        if (function == READ_HOLD_REGISTER) {
            return readRegisters(message, request);
        }
        return writeRegisters(message, function, request);
    }

    // Validate the complete frame before taking the register lock. Retain the
    // validation order so malformed requests keep their existing exception code.
    static Error validateRequest(ModbusMessage& message, FunctionCode function, Request& request) {
        if (message.size() < request_header_bytes) {
            return ILLEGAL_DATA_VALUE;
        }
        message.get(2, request.address);
        message.get(4, request.valueOrCount);
        request.wordCount = function == WRITE_HOLD_REGISTER ? 1 : request.valueOrCount;

        const uint16_t limit = function == READ_HOLD_REGISTER ? max_read_words : max_write_words;
        if (function == WRITE_HOLD_REGISTER) {
            if (message.size() != request_header_bytes) {
                return ILLEGAL_DATA_VALUE;
            }
        } else if (request.wordCount == 0 || request.wordCount > limit) {
            return ILLEGAL_DATA_VALUE;
        }
        if (uint32_t(request.address) + request.wordCount > 65536u) {
            return ILLEGAL_DATA_ADDRESS;
        }
        if (function == WRITE_MULT_REGISTERS &&
            (message.size() != 7u + 2u * request.wordCount ||
             message.data()[6] != 2u * request.wordCount)) {
            return ILLEGAL_DATA_VALUE;
        }
        if (function == READ_HOLD_REGISTER && message.size() != request_header_bytes) {
            return ILLEGAL_DATA_VALUE;
        }
        return SUCCESS;
    }

    ModbusMessage readRegisters(ModbusMessage& message, const Request& request) {
        std::array<uint16_t, max_read_words> words{};
        DataAccess<Server> access(*this);
        if (!access.readHoldingRegisters(request.address, request.wordCount, words.data())) {
            return errorResponse(message, ILLEGAL_DATA_ADDRESS);
        }

        ModbusMessage response;
        response.add(message.getServerID(), message.getFunctionCode(),
                     static_cast<uint8_t>(request.wordCount * 2));
        for (std::size_t wordIndex = 0; wordIndex < request.wordCount; ++wordIndex) {
            response.add(words[wordIndex]);
        }
        _statistics.recordRead();
        return response;
    }

    ModbusMessage writeRegisters(ModbusMessage& message, FunctionCode function, const Request& request) {
        // Stage all words and validate their full address range before any
        // register changes. Requests crossing an undefined gap cannot partly apply.
        std::array<uint16_t, max_read_words> words{};
        DataAccess<Server> access(*this);
        if (function == WRITE_HOLD_REGISTER) {
            words[0] = request.valueOrCount;
        } else {
            for (std::size_t wordIndex = 0; wordIndex < request.wordCount; ++wordIndex) {
                message.get(7 + 2 * wordIndex, words[wordIndex]);
            }
        }
        if (!access.writeHoldingRegisters(request.address, request.wordCount, words.data())) {
            return errorResponse(message, ILLEGAL_DATA_ADDRESS);
        }
        _statistics.recordWrite();

        // FC06 echoes the written value; FC16 echoes the number of words.
        ModbusMessage response;
        response.add(message.getServerID(), message.getFunctionCode());
        response.add(request.address);
        response.add(function == WRITE_HOLD_REGISTER ? request.valueOrCount : request.wordCount);
        return response;
    }

    ModbusMessage errorResponse(ModbusMessage& request, Error error) {
        _statistics.recordFailure();
        ModbusMessage response;
        response.setError(request.getServerID(), request.getFunctionCode(), error);
        return response;
    }

    template<typename> friend class DataAccess;
    Statistics _statistics;
    Log _log;
    DeviceType _device;
    bool _registered = false;
    ModbusServer& _transport;
    mutable std::timed_mutex _Mutex;
};
} // namespace modbus_gateway
