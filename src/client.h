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
        using RegisterType = typename MODBUS_TYPE::e_registers;

        Client(ModbusClientTCP &tcp, const IPAddress &remote)
            : _device(MODBUS_TYPE::getDeviceDescription()), _tcp(tcp), _remote(remote), _transaction(0)
        {
            _tcp.onDataHandler(&Client::handleData);
            _tcp.onErrorHandler(&Client::handleError);
        }

        bool readBlockFromMeter(const String &name)
        {
            bool result = false;
            uint32_t blockindex = _device.GetBlockIndex(name);
            if (_device._dd._bds[blockindex]._number_reg > 0)
            {
                T *t = new T{this, blockindex, _transaction++};
                static_assert(sizeof(T *) == sizeof(uint32_t));
                Error err = _tcp.addRequest(reinterpret_cast<uint32_t>(t), 2, READ_HOLD_REGISTER, _device._dd._bds[blockindex]._offset, _device._dd._bds[blockindex]._number_reg);
                // Serial.printf("readBlockFromMeter Token=%08X\r\n", t);
                if (err != SUCCESS)
                {
                    ModbusError e(err);
                    Serial.printf("Error creating request: %02X - %s\r\n", (int)e, (const char *)e);
                }
                else
                {
                    result = true;
                }
            }
            return result;
        }

        template <typename T>
        friend class DataAccess;

        bool _dataRead = false;
        Device<MODBUS_TYPE> _device;

    private:
        struct T
        {
            Client *_this;
            uint32_t _blockindex;
            uint32_t _transaction;
        };

        // Define an onError handler function to receive error responses
        // Arguments are the error code returned and a user-supplied token to identify the causing request
        static void handleError(Error error, uint32_t token)
        {
            // ModbusError wraps the error code and provides a readable error message for it
            ModbusError me(error);
            T *t = reinterpret_cast<T *>(token);
            Serial.printf("Error response: %02X - %s - %s - %i\r\n", (int)me, (const char *)me, t->_this->_device._dd._bds[t->_blockindex]._name.c_str(), t->_transaction);
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
            // Serial.printf("handleData: serverID=%d, FC=%d, Token=%08X\r\n", response.getServerID(), response.getFunctionCode(), token);

            DataAccess<Client<MODBUS_TYPE>> dataaccess(*t->_this);
            if (t->_blockindex >= 0)
            {
                // Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d, values=%i\r\n", response.getServerID(), response.getFunctionCode(), token, (response.size()-3), (values._values.size()*2) );
                if ((t->_this->_device._dd._bds[t->_blockindex]._number_reg * 2) == (response.size() - 3))
                {
                    auto i = response.begin();
                    i += 3;
                    int index = 0;

                    while (i < response.end())
                    {
                        Value v;
                        v.b2 = *i++;
                        v.b1 = *i++;
                        dataaccess.setRegister(t->_blockindex, index++, v.w);
                    }
                    t->_this->_dataRead = true;
                    dataaccess.setTransaction(t->_blockindex, t->_transaction);
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
        mutable std::timed_mutex _Mutex;
    };
}