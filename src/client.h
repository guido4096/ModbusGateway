/**
 * @file      master.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Object that acts as a master to retrieve values from the meter
 */
#pragma once

#include "mutex"
#include "definitions.h"
#include "ModbusClientTCP.h"

namespace modbus_gateway
{
    template <class MODBUS_TYPE>
    class Client
    {
    public:
        Client(ModbusClientTCP &tcp, const IPAddress &remote, std::timed_mutex& m)
            : _dd(MODBUS_TYPE::getDeviceDescription()), _tcp(tcp), _remote(remote), _transaction(0), _m(m)
        {
            for (auto i = _dd._blocks.begin(); i < _dd._blocks.end(); i++)
            {
                BlockValues v(*i, i->_number_reg);
                _blockValues.push_back(v);
            }
            _tcp.onDataHandler(&Client::handleData);
            _tcp.onErrorHandler(&Client::handleError);
        }

        using RegisterType = typename MODBUS_TYPE::e_registers;
        float getFloatValue(RegisterType r)
        {
            float f = 0;
            RegisterReference rr = _dd.getRegisterReference(r);
            if (rr._block_idx >= 0 && rr._register_idx >= 0)
            {
                f = _blockValues[rr._block_idx].getFloatValue(rr);
            }
            else
            {
                Serial.printf("Can't find value %s %i %i\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx);
            }
            return f;
        }

        bool readBlockFromMeter(const String &name)
        {
            bool result = false;
            modbus_gateway::BlockValues *b = getBlockValues(name);
            if (b->_block._number_reg > 0)
            {
                T *t = new T{this, b, _transaction++};
                static_assert(sizeof(T *) == sizeof(uint32_t));
                Error err = _tcp.addRequest(reinterpret_cast<uint32_t>(t), 2, READ_HOLD_REGISTER, b->_block._offset, b->_block._number_reg);
                if (err != SUCCESS)
                {
                    ModbusError e(err);
                    Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
                }
                else
                {
                    result = true;
                }
            }
            return result;
        }

        String allValuesAsString() const
        {
            String r = "";
            std::unique_lock lock(_m, std::chrono::milliseconds(200));
            if (lock.owns_lock()) {
                for (auto i = _blockValues.begin(); i < _blockValues.end(); i++)
                    r += i->toString();
            } else {
                Serial.println("Client::allValuesAsString: could not lock");
            }
            return r;
        }

        bool _dataRead = false;
        const DeviceDescription<MODBUS_TYPE> &_dd;

    private:
        struct T
        {
            Client *_this;
            modbus_gateway::BlockValues *_block;
            uint32_t _transaction;
        };

        BlockValues *getBlockValues(const String &name)
        {
            for (auto i = _blockValues.begin(); i < _blockValues.end(); i++)
            {
                if (i->_block._name == name)
                    return &(*i);
            }
            return 0;
        }

        // Define an onError handler function to receive error responses
        // Arguments are the error code returned and a user-supplied token to identify the causing request
        static void handleError(Error error, uint32_t token)
        {
            // ModbusError wraps the error code and provides a readable error message for it
            ModbusError me(error);
            T *t = reinterpret_cast<T *>(token);
            Serial.printf("Error response: %02X - %s - %s - %i\r\n", (int)me, (const char *)me, t->_block->_block._name.c_str(), t->_transaction);
            delete t;
        }

        static void handleData(ModbusMessage response, uint32_t token)
        {
            if (token == 0)
            {
                Serial.printf("handleData received null ptr\r\n");
                return;
            }
            T *t = reinterpret_cast<T *>(token);
            modbus_gateway::BlockValues *b = t->_block;

            if (b)
            {
                std::unique_lock lock(t->_this->_m, std::chrono::milliseconds(200));
                if (lock.owns_lock()) {
                    b->_transaction = t->_transaction;
                    // Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d, values=%i\n\r", response.getServerID(), response.getFunctionCode(), token, (response.size()-3), (b->_values.size()*2) );

                    if ((b->_values.size() * 2) == (response.size() - 3))
                    {
                        auto i = response.begin();
                        i += 3;
                        int index = 0;

                        while (i < response.end())
                        {
                            Value v;
                            v.b2 = *i++;
                            v.b1 = *i++;
                            b->_values[uint32_t(index++)] = v.w;
                        }
                        t->_this->_dataRead = true;
                    }
                }
                else {
                    Serial.println("handleData: could not lock");
                }
            }
            else
            {
                Serial.printf("ERROR: Block for transaction %i not found\r\n", t->_transaction);
            }
            delete t;
        }
        uint32_t _transaction;
        ModbusClientTCP &_tcp;
        IPAddress _remote;
        std::vector<BlockValues> _blockValues;
        std::timed_mutex& _m;
    };
}