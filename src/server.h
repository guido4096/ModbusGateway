/**
 * @file      slave.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Object that acts as a slave to answer requests from the SolarEdge inverter
 */
#pragma once

#include "mutex"
#include "definitions.h"
#include "ModbusServerRTU.h"

namespace modbus_gateway
{
    template <typename MODBUS_TYPE>
    class Server
    {
    public:
        using RegisterType = typename MODBUS_TYPE::e_registers;

        Server(ModbusServer &rtu, uint8_t rtuServerId, uint32_t serialNumber)
            : _device(MODBUS_TYPE::getDeviceDescription(rtuServerId, serialNumber)), _rtu(rtu)
        {
            _THIS = this;
            _rtu.registerWorker(rtuServerId, READ_HOLD_REGISTER, &FC03);
            _rtu.registerWorker(rtuServerId, WRITE_HOLD_REGISTER, &FC06);
            _rtu.registerWorker(rtuServerId, WRITE_MULT_REGISTERS, &FC16);
        }
        Device<MODBUS_TYPE> _device;

    private:
        static ModbusMessage FC06(ModbusMessage request)
        {
            return Process(request, WRITE_HOLD_REGISTER);
        }
        static ModbusMessage FC16(ModbusMessage request)
        {
            return Process(request, WRITE_MULT_REGISTERS);
        }
        static ModbusMessage FC03(ModbusMessage request)
        {
            return Process(request, READ_HOLD_REGISTER);
        }
        static ModbusMessage Process(ModbusMessage request, FunctionCode fc)
        {
            uint16_t address;       // requested register address
            uint16_t words;         // requested number of registers
            ModbusMessage response; // response message to be sent back

            // get request values
            request.get(2, address);
            request.get(4, words);
            // Serial.printf("FC03 address:%i words:%i\r\n", address, words);
            DataAccess<Server<MODBUS_TYPE>> dataaccess(*_THIS);

            if (fc == WRITE_MULT_REGISTERS || fc == WRITE_HOLD_REGISTER)
            {
                uint16_t i = 0;
                char hexstr[200];
                for (i = 0; i < request.size() && i < 200 / 3; i++)
                {
                    uint8_t b = request.data()[i];
                    sprintf(hexstr + i * 3, "%02x ", b);
                }
                hexstr[i * 3] = 0;
                char buffer[200];
                sprintf(buffer, "Writing: serverID=%d, FC=%d, start=%d, length=%d, size=%d, b=%s", request.getServerID(), request.getFunctionCode(), address, words, request.size(), hexstr);
                dataaccess.logMessage(buffer);
            }
            // Find the block
            int32_t block_index = -1;
            for (auto i = _THIS->_device._dd._bds.begin(); i < _THIS->_device._dd._bds.end(); i++)
            {
                if (i->_offset <= address && (i->_offset + i->_number_reg) >= (address + words))
                {
                    block_index = i - _THIS->_device._dd._bds.begin();
                    break;
                }
            }

            // Did we find the block?
            if (block_index >= 0)
            {
                if (fc == READ_HOLD_REGISTER)
                {
                    // Looks okay. Set up message with serverID, FC and length of data
                    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
                    // Fill response with requested data
                    for (uint16_t i = address; i < address + words; ++i)
                    {
                        uint16_t v = dataaccess.getRegisterValue(block_index, i - _THIS->_device._dd._bds[block_index]._offset);
                        response.add(v);
                    }
                    char buffer[200];
                    sprintf(buffer, "Response: serverID=%d, FC=%d, start=%d length=%d block=%s", response.getServerID(), response.getFunctionCode(), address, words, _THIS->_device._dd._bds[block_index]._name.c_str());
                    // Serial.printf("%s\r\n", buffer);
                    dataaccess.logMessage(buffer);
                }
                if (fc == WRITE_MULT_REGISTERS)
                {
                    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
                    for (uint16_t i = address; i < address + words; ++i)
                    {
                        uint16_t v = 0;
                        request.get(7 + (i - address) * 2, v);
                        dataaccess.setRegisterValue(block_index, i - _THIS->_device._dd._bds[block_index]._offset, v);
                        char buffer[200];
                        sprintf(buffer, "%04x", v);
                        dataaccess.logMessage(buffer);

                        //  request.get()
                        //  response.add(v);
                    }
                    char buffer[200];
                    sprintf(buffer, "Writing: serverID=%d, FC=%d, start=%d length=%d block=%s", request.getServerID(), request.getFunctionCode(), address, words, _THIS->_device._dd._bds[block_index]._name.c_str());
                    // Serial.printf("%s\r\n", buffer);
                    dataaccess.logMessage(buffer);
                }
            }
            else
            {
                // No, either address or words are outside the limits. Set up error response.
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                char buffer[200];
                sprintf(buffer, "Error: serverID=%d, FC=%d, length=%d", response.getServerID(), response.getFunctionCode(), response.size());
                // Serial.printf("%s\r\n", buffer);
                dataaccess.logMessage(buffer);
            }
            return response;
        }
        modbus_gateway::Log _log;
        template <typename T>
        friend class DataAccess;
        inline static Server<MODBUS_TYPE> *_THIS;
        ModbusServer &_rtu;
        mutable std::timed_mutex _Mutex;
    };

}
