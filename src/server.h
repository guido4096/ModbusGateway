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

        Server(ModbusServerRTU &rtu, uint8_t slaveId)
            : _device(MODBUS_TYPE::getDeviceDescription()), _rtu(rtu)
        {
            _THIS = this;
            _rtu.registerWorker(slaveId, READ_HOLD_REGISTER, &FC03);
        }
        Device<MODBUS_TYPE> _device;

    private:

        static ModbusMessage FC03(ModbusMessage request)
        {
            uint16_t address;       // requested register address
            uint16_t words;         // requested number of registers
            ModbusMessage response; // response message to be sent back

            // get request values
            request.get(2, address);
            request.get(4, words);
            // Serial.printf("FC03 address:%i words:%i\r\n", address, words);
            DataAccess<Server<MODBUS_TYPE>> dataaccess(*_THIS);

            // Find the block
            Values *bv = 0;
            for (auto i = dataaccess.GetValues().begin(); i < dataaccess.GetValues().end(); i++)
            {
                if (i->_block._offset <= address && (i->_block._offset + i->_block._number_reg) >= (address + words))
                {
                    bv = &(*i);
                    break;
                }
            }

            // Did we find the block?
            if (bv)
            {
                // Looks okay. Set up message with serverID, FC and length of data
                response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
                // Fill response with requested data
                for (uint16_t i = address; i < address + words; ++i)
                {
                    response.add(uint16_t(bv->_values[i - bv->_block._offset]));
                }
                // Serial.printf("Response: serverID=%d, FC=%d, length=%d block=%s\n\r", response.getServerID(), response.getFunctionCode(), response.size(), bv->_block._name.c_str());
            }
            else
            {
                // No, either address or words are outside the limits. Set up error response.
                response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                Serial.printf("Error: serverID=%d, FC=%d, length=%d\n\r", response.getServerID(), response.getFunctionCode(), response.size());
            }
            return response;
        }
        template <typename T>
        friend class DataAccess;
        inline static Server<MODBUS_TYPE> *_THIS;
        ModbusServerRTU &_rtu;
        mutable std::timed_mutex _Mutex;
    };

}
