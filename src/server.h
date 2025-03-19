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
    template <class MODBUS_TYPE>
    class Server
    {
    public:
        Server(ModbusServerRTU &rtu, uint8_t slaveId, std::timed_mutex& m) 
        : _dd(MODBUS_TYPE::getDeviceDescription()), _rtu(rtu), _m(m)
        {
            for (auto i = _dd._blocks.begin(); i < _dd._blocks.end(); i++)
            {
                BlockValues bv(*i, i->_number_reg);
                modbus_gateway::Value v;
                int index =0;
                for (auto j = i->_registers.begin(); j < i->_registers.end(); j++) {
                    switch (j->_number)
                    {
                    case 1:
                        bv._values[index++] = j->_default.w;
                        break;
                    case 2:
                        bv._values[index++] = j->_default.w1;
                        bv._values[index++] = j->_default.w2;
                        break;
                    }
                }
                _blockValues.push_back(bv);
            }
            _THIS = this;
            _rtu.registerWorker(slaveId, READ_HOLD_REGISTER, &FC03);
        }

        using RegisterType = typename MODBUS_TYPE::e_registers;
        void setFloatValue(RegisterType r, float i)
        {
            RegisterReference rr = _dd.getRegisterReference(r);
            if (rr._block_idx >= 0 && rr._register_idx >= 0)
            {
                _blockValues[rr._block_idx].setFloatValue(rr, i);
                //Serial.printf("setFloatValue %s %i %i %i %i=%f\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx, r._blockNbr, r._offset, i);
            }
            else
            {
                Serial.printf("modbus_gateway::ConvertEM24ToWattNode::setFloatValue invalid reference for register %s %i %i\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx);
            }
        }

        String getValueAsString(const Register &r) const
        {
            char buf[200] = {0};
            Value v = getValue(r);
            switch (r._dataType)
            {
            case DataType::float32:
                sprintf(buf, "%.1f", v.f32);
                break;
            case DataType::uint32:
                sprintf(buf, "%u", v.ui32);
                break;
            case DataType::int16:
                sprintf(buf, "%i", v.i16);
                break;
            case DataType::uint16:
                sprintf(buf, "%u", v.ui16);
                break;
            default:
                sprintf(buf, "not converted");
            }
            return buf;
        }

        modbus_gateway::Value getValue(const Register &r) const
        {
            modbus_gateway::Value v;
            switch (r._number)
            {
            case 1:
                v.w = _blockValues[r._blockNbr]._values[r._offset-_blockValues[r._blockNbr]._block._offset];
                break;
            case 2:
                v.w1 = _blockValues[r._blockNbr]._values[r._offset-_blockValues[r._blockNbr]._block._offset];
                v.w2 = _blockValues[r._blockNbr]._values[r._offset-_blockValues[r._blockNbr]._block._offset+1];
                break;
            }
            return v;
        }

        String allValuesAsString() const
        {
            String r = "";
            std::unique_lock lock(_m, std::chrono::milliseconds(200));
            if (lock.owns_lock()) {
                char buf[200];
                for (auto i = _dd._blocks.begin(); i < _dd._blocks.end(); i++)
                {
                    sprintf(buf, "Block %s\r\n", i->_name.c_str());
                    r += buf;
                    for (auto j = i->_registers.begin(); j < i->_registers.end(); j++)
                    {
                        String v = getValueAsString(*j);
                        sprintf(buf, "  %s=%s %s\r\n", j->_desc.c_str(), v.c_str(), j->_unit);
                        r += buf;
                    }
                }
            }
            else {
                Serial.println("Server::allValuesAsString: could not lock");
            }
            return r;
        }

        const DeviceDescription<MODBUS_TYPE> &_dd;
    private:
        ModbusServerRTU &_rtu;

        static ModbusMessage FC03(ModbusMessage request) {
            uint16_t address;           // requested register address
            uint16_t words;             // requested number of registers
            ModbusMessage response;     // response message to be sent back
          
            // get request values
            request.get(2, address);
            request.get(4, words);
            //Serial.printf("FC03 address:%i words:%i\r\n", address, words);

            // Find the block
            BlockValues* bv = 0;
            for (auto i = _THIS->_blockValues.begin(); i < _THIS->_blockValues.end(); i++)
            {
                if (i->_block._offset <= address && (i->_block._offset + i->_block._number_reg) >= (address+words))
                {
                    bv = &(*i);
                    break;
                }
            }

            // Did we find the block?
            
            if (bv) {
              std::unique_lock lock(_THIS->_m, std::chrono::milliseconds(200));
              if (lock.owns_lock()) {
                // Looks okay. Set up message with serverID, FC and length of data
                response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
                // Fill response with requested data
                for (uint16_t i = address; i < address + words; ++i) {
                    response.add(uint16_t(bv->_values[i-bv->_block._offset]));
                }
                //Serial.printf("Response: serverID=%d, FC=%d, length=%d block=%s\n\r", response.getServerID(), response.getFunctionCode(), response.size(), bv->_block._name.c_str());
                }
                else {
                    Serial.println("FC03: could not lock");
                    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
                    Serial.printf("Error: serverID=%d, FC=%d, length=%d\n\r", response.getServerID(), response.getFunctionCode(), response.size());
                }
            } else {
              // No, either address or words are outside the limits. Set up error response.
              response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
              Serial.printf("Error: serverID=%d, FC=%d, length=%d\n\r", response.getServerID(), response.getFunctionCode(), response.size());
            }
            return response;
          }

          std::vector<BlockValues> _blockValues;
          inline static Server<MODBUS_TYPE>* _THIS;
          std::timed_mutex& _m;
    };
}
