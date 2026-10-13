/**
 * @file      server.h
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
        // Byte offset of the register data inside a Modbus PDU: the first two
        // bytes are the function code and the register address, the next two are
        // the register count. Only used to decode write requests (FC06/FC16).
        static const uint16_t REQUEST_DATA_OFFSET = 7;

        static ModbusMessage Process(ModbusMessage request, FunctionCode fc)
        {
            uint16_t address;       // requested register address
            uint16_t words;         // requested number of registers
            ModbusMessage response; // response message to be sent back

            Server<MODBUS_TYPE> *self = _THIS; // static worker has no instance pointer
            self->_statistics.recordCall();

            // get request values
            request.get(2, address);
            request.get(4, words);

            DataAccess<Server<MODBUS_TYPE>> dataaccess(*self);

            if (fc == WRITE_MULT_REGISTERS || fc == WRITE_HOLD_REGISTER)
            {
                // Log the raw request bytes to make protocol debugging easier
                char hexstr[200];
                uint16_t i = 0;
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

            // Find the block that fully covers the requested address range
            const auto &blocks = self->_device._dd._bds;
            int32_t block_index = -1;
            for (auto block = blocks.begin(); block < blocks.end(); block++)
            {
                if (block->_offset <= address && (block->_offset + block->_number_reg) >= (address + words))
                {
                    block_index = block - blocks.begin();
                    break;
                }
            }

            // Did we find the block?
            if (block_index >= 0)
            {
                const uint16_t block_offset = blocks[block_index]._offset;

                if (fc == READ_HOLD_REGISTER)
                {
                    self->_statistics.recordRead();
                    // Looks okay. Set up message with serverID, FC and length of data
                    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
                    // Fill response with requested data
                    for (uint16_t i = address; i < address + words; ++i)
                    {
                        uint16_t v = dataaccess.getRegisterValue(block_index, i - block_offset);
                        response.add(v);
                    }
                    // char buffer[200];
                    // sprintf(buffer, "Response: serverID=%d, FC=%d, start=%d length=%d block=%s", response.getServerID(), response.getFunctionCode(), address, words, blocks[block_index]._name.c_str());
                    // dataaccess.logMessage(buffer);
                }
                if (fc == WRITE_MULT_REGISTERS)
                {
                    self->_statistics.recordWrite();
                    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
                    for (uint16_t i = address; i < address + words; ++i)
                    {
                        uint16_t v = 0;
                        request.get(REQUEST_DATA_OFFSET + (i - address) * 2, v);
                        dataaccess.setRegisterValue(block_index, i - block_offset, v);

                        // char buffer[200];
                        // sprintf(buffer, "%04x", v);
                        // dataaccess.logMessage(buffer);
                    }
                    char buffer[200];
                    sprintf(buffer, "Writing: serverID=%d, FC=%d, start=%d length=%d block=%s", request.getServerID(), request.getFunctionCode(), address, words, blocks[block_index]._name.c_str());
                    dataaccess.logMessage(buffer);
                }
            }
            else
            {
                // No, either address or words are outside the limits. Set up error response.
                self->_statistics.recordFailure();
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                char buffer[200];
                sprintf(buffer, "Error: serverID=%d, FC=%d, length=%d", response.getServerID(), response.getFunctionCode(), response.size());
                dataaccess.logMessage(buffer);
            }
            return response;
        }
        Statistics _statistics;
        modbus_gateway::Log _log;
        template <typename T>
        friend class DataAccess;
        inline static Server<MODBUS_TYPE> *_THIS;
        ModbusServer &_rtu;
        mutable std::timed_mutex _Mutex;
    };

}
