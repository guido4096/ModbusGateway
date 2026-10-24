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
// Optional schema hook. Schemas without an exposed ID register need no declaration.
template<typename T, typename = void>
struct HasServerIdRegister : std::false_type {};
template<typename T>
struct HasServerIdRegister<T, std::void_t<decltype(T::server_id_register)>> : std::true_type {};

template<typename MODBUS_TYPE>
class Server {
public:
    using Schema = MODBUS_TYPE;
    using RegisterType = typename Schema::RegisterId;
    using DeviceType = Device<Schema>;

    // Modbus server ID belongs to the transport; factory arguments belong to the device.
    template<typename... DeviceArgs>
    Server(ModbusServer& rtu, uint8_t server_id, DeviceArgs&&... device_args)
        : _device(Schema::makeDevice(std::forward<DeviceArgs>(device_args)...)), _rtu(rtu) {
        _device.setServerId(server_id);
        if constexpr (HasServerIdRegister<Schema>::value) {
            _device.template set<Schema::server_id_register>(server_id);
        }
        _THIS = this; // Matches original library's static worker API: one instance per Schema.
        _rtu.registerWorker(server_id, READ_HOLD_REGISTER, &FC03);
        _rtu.registerWorker(server_id, WRITE_HOLD_REGISTER, &FC06);
        _rtu.registerWorker(server_id, WRITE_MULT_REGISTERS, &FC16);
    }

    DeviceType _device;

private:
    static ModbusMessage FC03(ModbusMessage req) { return Process(req, READ_HOLD_REGISTER); }
    static ModbusMessage FC06(ModbusMessage req) { return Process(req, WRITE_HOLD_REGISTER); }
    static ModbusMessage FC16(ModbusMessage req) { return Process(req, WRITE_MULT_REGISTERS); }

    static ModbusMessage Process(ModbusMessage request, FunctionCode fc) {
        ModbusMessage response;
        Server* self = _THIS;
        if (!self) return response;
        self->_statistics.recordCall();
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
    inline static Server* _THIS = nullptr;
    ModbusServer& _rtu;
    mutable std::timed_mutex _Mutex;
};
} // namespace modbus_gateway
