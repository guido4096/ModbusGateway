/**
 * @file      server.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Object that acts as a slave to answer requests from the SolarEdge inverter
 */
#pragma once
#include <mutex>
#include <array>
#include <cstdint>
#include <cstdio>
#include <utility>
#include <type_traits>
#include "definitions.h"
#include "ModbusServerRTU.h"

namespace modbus_gateway {
// Detection idiom: a schema may nominate a logical register that mirrors
// the protocol server ID. No such register is required for other devices;
// the binding is resolved by if constexpr at server instantiation.
template<typename T, typename = void>
struct HasServerIdRegister : std::false_type {};
template<typename T>
struct HasServerIdRegister<T, std::void_t<decltype(T::server_id_register)>> : std::true_type {};

/** Modbus server endpoint backed by Device<Schema> raw register words.
 * Register descriptions govern layout and typed application access; FC03,
 * FC06 and FC16 operate on address ranges supplied at runtime by the peer.
 * All request processing uses a DataAccess guard for atomic buffer updates.
 */
template<typename MODBUS_TYPE>
class Server {
public:
    using Schema = MODBUS_TYPE;
    using RegisterType = typename Schema::RegisterId;
    using DeviceType = Device<Schema>;

    // The protocol server ID selects eModbus worker registrations. Factory
    // arguments are independent device-instance defaults (e.g. WattNode
    // serial number), forwarded unchanged to Schema::makeDevice().
    template<typename... DeviceArgs>
    Server(ModbusServer& rtu, uint8_t server_id, DeviceArgs&&... device_args)
        : _device(Schema::makeDevice(std::forward<DeviceArgs>(device_args)...)), _rtu(rtu) {
        _device.setServerId(server_id);
        if constexpr (HasServerIdRegister<Schema>::value) {
            _device.template set<Schema::server_id_register>(server_id);
        }
        // eModbus workers are static function pointers, hence this trampoline.
        // There can be only one live Server instance *per Schema type*; a
        // second instance would redirect all callbacks to the latest object.
        _THIS = this;
        _rtu.registerWorker(server_id, READ_HOLD_REGISTER, &FC03);
        _rtu.registerWorker(server_id, WRITE_HOLD_REGISTER, &FC06);
        _rtu.registerWorker(server_id, WRITE_MULT_REGISTERS, &FC16);
    }

    DeviceType _device;

private:
    // Small static trampolines retain eModbus's required worker signature.
    static ModbusMessage FC03(ModbusMessage req) { return Process(req, READ_HOLD_REGISTER); }
    static ModbusMessage FC06(ModbusMessage req) { return Process(req, WRITE_HOLD_REGISTER); }
    static ModbusMessage FC16(ModbusMessage req) { return Process(req, WRITE_MULT_REGISTERS); }

    // Request coordinates are Modbus *word* addresses. Parse and validate
    // the full PDU before acquiring the device lock or changing any values.
    // FC06 encodes a single value in the field otherwise used as word count.
    static ModbusMessage Process(ModbusMessage request, FunctionCode fc) {
        ModbusMessage response;
        Server* self = _THIS;
        if (!self) return response;
        self->_statistics.recordCall();
        // A rejected request uses a Modbus exception response rather than
        // silently returning truncated data or applying a partial write.
        const auto error = [&](Error code) {
            self->_statistics.recordFailure();
            response.setError(request.getServerID(), request.getFunctionCode(), code);
            return response;
        };
        if (request.size() < 6) return error(ILLEGAL_DATA_VALUE);

        uint16_t address = 0;
        uint16_t count_or_value = 0;
        request.get(2, address);
        request.get(4, count_or_value);
        const uint16_t count = (fc == WRITE_HOLD_REGISTER) ? 1 : count_or_value;
        const uint16_t limit = fc == READ_HOLD_REGISTER ? 125 : 123;
        if (fc == WRITE_HOLD_REGISTER) {
            if (request.size() != 6) return error(ILLEGAL_DATA_VALUE);
        } else if (count == 0 || count > limit) {
            return error(ILLEGAL_DATA_VALUE);
        }
        if (uint32_t(address) + count > 65536u) return error(ILLEGAL_DATA_ADDRESS);
        if (fc == WRITE_MULT_REGISTERS &&
            (request.size() != 7u + 2u*count || request.data()[6] != count*2u))
            return error(ILLEGAL_DATA_VALUE);
        if (fc == READ_HOLD_REGISTER && request.size() != 6)
            return error(ILLEGAL_DATA_VALUE);

        // Stage the complete response/write in a bounded stack buffer. The
        // Device raw-word API validates every address before copying, so a
        // request crossing an undefined block gap has no partial write.
        std::array<uint16_t, 125> words{};
        DataAccess<Server> access(*self);
        if (fc == READ_HOLD_REGISTER) {
            if (!access.readHoldingRegisters(address, count, words.data()))
                return error(ILLEGAL_DATA_ADDRESS);
            response.add(request.getServerID(), request.getFunctionCode(), static_cast<uint8_t>(count*2));
            for (std::size_t i=0; i<count; ++i) response.add(words[i]);
            self->_statistics.recordRead();
            return response;
        }
        if (fc == WRITE_HOLD_REGISTER) {
            words[0] = count_or_value;
        } else {
            for (std::size_t i=0; i<count; ++i)
                request.get(7 + 2*i, words[i]);
        }
        if (!access.writeHoldingRegisters(address, count, words.data()))
            return error(ILLEGAL_DATA_ADDRESS);
        self->_statistics.recordWrite();
        // FC06 echoes address and value; FC16 echoes address and number of words.
        response.add(request.getServerID(), request.getFunctionCode());
        response.add(address);
        response.add(fc == WRITE_HOLD_REGISTER ? count_or_value : count);
        return response;
    }

    Statistics _statistics;
    Log _log;
    template<typename> friend class DataAccess;
    // One callback target per template specialization, not per server ID.
    inline static Server* _THIS = nullptr;
    ModbusServer& _rtu;
    mutable std::timed_mutex _Mutex;
};
} // namespace modbus_gateway
