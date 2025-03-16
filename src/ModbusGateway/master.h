/**
 * @file      master.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Object that acts as a master to retrieve values from the meter
 */
#pragma once

#include "definitions.h"
#include "ModbusTCP.h"

namespace modbus
{
    template <class MODBUS_TYPE>
    class Master
    {
    public:
        Master(ModbusTCP& tcp, const IPAddress& remote) 
        : _dd(MODBUS_TYPE::getDeviceDescription())
        , _tcp(tcp)
        , _remote(remote)
        {
            for (auto i = _dd._blocks.begin(); i < _dd._blocks.end(); i++)
            {
                BlockValues v (*i, i->_number_reg);
                _blockValues.push_back(v);
            }
            THIS=this;
        }
        using RegisterType = typename MODBUS_TYPE::e_registers;
        float getFloatValue(RegisterType r)
        {
            float f = 0;
            RegisterReference rr = _dd.getRegisterReference(r);
            if (rr._block_idx >= 0 && rr._register_idx >= 0)
            {
                _blockValues[rr._block_idx].getFloatValue(rr, f);
                // Serial.printf("%s=%f\r\n", rr._name, o);
            }
            else
            {
                Serial.printf("Can't find value %s %i %i\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx);
            }
            return f;
        }
        bool readBlockFromMeter(const String& name)
        {
            bool result = false;
            // Avoid growing of transaction list. Allow maximum 50 entries
            if (_transactionList.size() > 50)
                _transactionList.erase(_transactionList.begin(), _transactionList.begin() + (_transactionList.size() - 50));

            if (_tcp.isConnected(_remote))
            {
                modbus::BlockValues* b = getBlockValues(name);
                if (b->_block._number_reg > 0)
                {
                    uint16_t t = _tcp.readIreg(_remote, b->_block._offset, b->_values.data(), b->_block._number_reg, &cbReadIreg);
                    if (t > 0)
                    {
                        result = true;
                        _transactionList.push_back({b, t});
                    }
                }
            }
            else
            {
                bool val = _tcp.connect(_remote, 502);
                Serial.print("tcp.connect=");
                Serial.println(val);
            }

            return result;
        }

        std::vector<BlockValues>              _blockValues;
        const DeviceDescription<MODBUS_TYPE>& _dd;
        bool                                  _dataRead=false;

        private:
        BlockValues *getBlockValues(const String &name)
        {
            for (auto i = _blockValues.begin(); i < _blockValues.end(); i++)
            {
                if (i->_block._name == name)
                    return &(*i);
            }
            return 0;
        }
        static bool cbReadIreg(Modbus::ResultCode event, uint16_t transaction, void *data)
        {
            static bool hasAlreadyTimedOut = false;
            if (event != Modbus::EX_SUCCESS)                     // If transaction got an error
                Serial.printf("Modbus result: %02X %i ", event, transaction); // Display Modbus error code
            else
                hasAlreadyTimedOut = false;

            if (event == Modbus::EX_TIMEOUT)
            {   // If Transaction timeout took place
                if (hasAlreadyTimedOut) {
                Serial.printf("Reset Modbus RTU connection"); 
                THIS->_tcp.disconnect(THIS->_remote); // Close connection to slave and
                THIS->_tcp.dropTransactions(); // Cancel all waiting transactions
                hasAlreadyTimedOut = false;
                }
                else {
                    hasAlreadyTimedOut = true;
                }
            }

            modbus::BlockValues *b = 0;
            for (auto i = THIS->_transactionList.begin(); i < THIS->_transactionList.end(); i++)
            {
                if (i->_transaction == transaction)
                {
                    b = i->_block;
                    THIS->_transactionList.erase(i);
                    break;
                }
            }
            if (b)
            {
                if (event == Modbus::EX_SUCCESS)
                {
                    b->_transaction = transaction;
                    THIS->_dataRead = true;                    
                }
                else
                {
                    Serial.printf("ERROR: Block for transaction %i, blockname=%s\r\n", transaction, b->_block._name.c_str());
                }
            }
            else
            {
                Serial.printf("ERROR: Block for transaction %i not found\r\n", transaction);
            }

            return true;
        }
        // Keep list of transactions, to retrieve the block related to the transaction
        struct Transaction
        {
            modbus::BlockValues *_block;
            uint16_t _transaction;
        };
        static inline Master* THIS = 0;
        std::vector<Transaction> _transactionList;
        ModbusTCP& _tcp;
        IPAddress _remote;
    };
}