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

        Client(ModbusClientTCP &tcp, const IPAddress &remote, uint16_t tcp_server_id )
            : _device(MODBUS_TYPE::getDeviceDescription()), _tcp(tcp), _remote(remote), _transaction(0), _tcp_server_id(tcp_server_id)
        {
            _tcp.onDataHandler(&Client::handleData);
            _tcp.onErrorHandler(&Client::handleError);
        }

        bool readFromMeter(RegisterType begin, RegisterType end)
        {
            RegisterReference b = _device._dd._rr[begin];
            RegisterReference e = _device._dd._rr[end];
            RegisterDescription rd_b = _device._dd._bds[b._block_idx]._rds[b._register_idx];
            RegisterDescription rd_e = _device._dd._bds[e._block_idx]._rds[e._register_idx];
            return readFromMeter(rd_b, rd_e);
        }

        bool readFromMeter(RegisterDescription b, RegisterDescription e)
        {
            bool result = false;
            if (b._blockNbr == e._blockNbr && b._offset <= e._offset)
            {
                static_assert(sizeof(T *) == sizeof(uint32_t));

                uint16_t start_reg = b._offset;
                uint16_t nbr_reg = e._offset - b._offset + e._number;
                T *t = new T{this, b._blockNbr, start_reg, nbr_reg, _transaction++};

                Error err = _tcp.addRequest(reinterpret_cast<uint32_t>(t), _tcp_server_id, READ_HOLD_REGISTER, start_reg, nbr_reg);
                // Serial.printf("readBlockFromMeter Token=%08X\r\n", t);
                if (err != SUCCESS)
                {
                    char buffer[200];
                    ModbusError e(err);
                    sprintf(buffer, "Error creating request: %02X - %s, offset=%i, nbr_reg=%i, %i", (int)e, (const char *)e, start_reg, nbr_reg, _tcp.pendingRequests());
                    Serial.printf("%s\r\n", buffer);
                    _log.addString(buffer);
                }
                else
                {
                    result = true;
                }
            }
            return result;
        }

        bool readBlockFromMeter(const String &name)
        {
            bool result = false;
            uint32_t blockindex = _device.GetBlockIndex(name);
            if (_device._dd._bds[blockindex]._number_reg > 0)
            {
              result =  readFromMeter( _device._dd._bds[blockindex]._rds.front(), _device._dd._bds[blockindex]._rds.back());
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
            uint32_t _start_reg;
            uint32_t _nbr_reg;
            uint32_t _transaction;
        };

        // Define an onError handler function to receive error responses
        // Arguments are the error code returned and a user-supplied token to identify the causing request
        static void handleError(Error error, uint32_t token)
        {
            // ModbusError wraps the error code and provides a readable error message for it
            ModbusError me(error);
            T *t = reinterpret_cast<T *>(token);
            char buffer[200];
            sprintf(buffer,"Error response: %02X - %s - %s - %i - %i", (int)me, (const char *)me, t->_this->_device._dd._bds[t->_blockindex]._name.c_str(), t->_transaction, t->_this->_tcp.pendingRequests());
            Serial.printf("%s\r\n", buffer);
            t->_this->_log.addString(buffer);
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
                if ((t->_nbr_reg*2) == (response.size() - 3))
                {
                    auto i = response.begin();
                    i += 3;
                    int index = 0+(t->_start_reg - t->_this->_device._dd._bds[t->_blockindex]._offset);

                    while (i < response.end())
                    {
                        Value v;
                        v.b2 = *i++;
                        v.b1 = *i++;
                        dataaccess.setRegisterValue(t->_blockindex, index++, v.w);
                    }
                    t->_this->_dataRead = true;
                    dataaccess.setTransaction(t->_blockindex, t->_transaction);
                }
                else {
                    char buffer[200];
                    sprintf(buffer,"Expected bytes: %i, received bytes: %i", t->_nbr_reg*2, response.size() - 3);
                    Serial.printf("%s\r\n", buffer);
                    t->_this->_log.addString(buffer);
                }
            }
            else
            {
                Serial.printf("ERROR: Block for transaction %i not found\r\n", t->_transaction);
            }
            delete t;
        }

        modbus_gateway::Log _log;
        uint32_t _transaction;
        ModbusClientTCP &_tcp;
        IPAddress _remote;
        uint16_t _tcp_server_id;
        mutable std::timed_mutex _Mutex;
    };
}