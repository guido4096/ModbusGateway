/**
 * @file      definitions.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Definitions to support formalization modbus register definitions
 */
#pragma once

#include <Arduino.h>
#include <vector>

namespace modbus_gateway
{
    enum DataType
    {
        float32,
        int16,
        uint16,
        int32,
        uint32
    };

    enum Scaling
    {
        none = 1,
        ten = 10,
        hundred = 100,
        thousand = 1000
    };
    static uint16_t getScaling(Scaling s)
    {
        switch (s)
        {
        case none:
            return 1;
        case ten:
            return 10;
        case hundred:
            return 100;
        case thousand:
            return 1000;
        }
        return 0;
    }
    static uint16_t getLogScaling(Scaling s)
    {
        switch (s)
        {
        case none:
            return 0;
        case ten:
            return 1;
        case hundred:
            return 2;
        case thousand:
            return 3;
        }
        return 0;
    }
    static uint16_t numberRegisters(DataType d)
    {
        switch (d)
        {
        case float32:
            return 2;
        case int16:
            return 1;
        case uint16:
            return 1;
        case int32:
            return 2;
        case uint32:
            return 2;
        }
        return 0;
    }
    union Value
    {
        static Value _uint32_t(uint32_t v)
        {
            Value r;
            r.ui32 = v;
            return r;
        }
        static Value _int32_t(int32_t v)
        {
            Value r;
            r.i32 = v;
            return r;
        }
        static Value _int16_t(int16_t v)
        {
            Value r;
            r.i16 = v;
            return r;
        }
        static Value _uint16_t(uint16_t v)
        {
            Value r;
            r.ui16 = v;
            return r;
        }
        static Value _float32_t(float v)
        {
            Value r;
            r.f32 = v;
            return r;
        }
        struct
        {
            uint8_t b1;
            uint8_t b2;
            uint8_t b3;
            uint8_t b4;
        };
        struct
        {
            uint16_t w1;
            uint16_t w2;
        };
        struct
        {
            uint16_t w;
        };
        struct
        {
            float f32;
        };
        struct
        {
            uint16_t ui16;
        };
        struct
        {
            int16_t i16;
        };
        struct
        {
            uint32_t ui32;
        };
        struct
        {
            int32_t i32;
        };
    };

    // RegisterReference. Avoid searching by string and instead use indexing
    struct RegisterReference
    {
        String _desc;
        int32_t _block_idx;
        int32_t _register_idx;
    };

    // Register and Block Defintion
    template <typename MODBUS_TYPE>
    struct RegisterDefinition
    {
        using RegisterType = typename MODBUS_TYPE::e_registers;
        const RegisterType _register;
        const DataType _dataType;
        const String _desc;
        const String _unit;
        const Scaling _scaling;
        const Value _default;
    };
    template <typename MODBUS_TYPE>
    struct BlockDefinition
    {
        const String _name;
        const uint16_t _offset;
        const std::vector<RegisterDefinition<MODBUS_TYPE>> _rd;
    };

    class RegisterDescription
    {
    public:
        String toString(const uint16_t *r) const
        {
            String result = "unknown";
            char buf[40] = {0};
            Value v;
            switch (_dataType)
            {
            case float32:
                v.w1 = r[0];
                v.w2 = r[1];
                sprintf(buf, "%.1f", v.f32 / getScaling(_scaling));
                result = buf;
                break;
            case int16:
                v.w1 = r[0];
                if (_scaling == none)
                    sprintf(buf, "%i", v.i16);
                else
                    sprintf(buf, "%.*f", getLogScaling(_scaling), float(v.i16) / getScaling(_scaling));
                result = buf;
                break;
            case uint16:
                v.w1 = r[0];
                if (_scaling == none)
                    sprintf(buf, "%u", v.ui16);
                else
                    sprintf(buf, "%.*f", getLogScaling(_scaling), float(v.ui16) / getScaling(_scaling));
                result = buf;
                break;
            case int32:
                v.w1 = r[0];
                v.w2 = r[1];
                if (_scaling == none)
                    sprintf(buf, "%i", v.i32);
                else
                    sprintf(buf, "%.*f", getLogScaling(_scaling), float(v.i32) / getScaling(_scaling));
                result = buf;
                break;
            case uint32:
                v.w1 = r[0];
                v.w2 = r[1];
                if (_scaling == none)
                    sprintf(buf, "%u", v.ui32);
                else
                    sprintf(buf, "%.*f", getLogScaling(_scaling), float(v.ui32) / getScaling(_scaling));
                result = buf;
                break;
            }

            return result;
        }
        float toFloat32(const uint16_t *r) const
        {
            float result = 0;
            Value v;
            switch (_dataType)
            {
            case float32:
                v.w1 = r[0];
                v.w2 = r[1];
                result = round(float(v.f32) * getScaling(_scaling)) / getScaling(_scaling);
                break;
            case int16:
                v.w1 = r[0];
                result = round(float(v.i16) * getScaling(_scaling)) / getScaling(_scaling);
                break;
            case uint16:
                v.w1 = r[0];
                result = round(float(v.ui16) * getScaling(_scaling)) / getScaling(_scaling);
                break;
            case int32:
                v.w1 = r[0];
                v.w2 = r[1];
                result = round(float(v.i32) * getScaling(_scaling)) / getScaling(_scaling);
                break;
            case uint32:
                v.w1 = r[0];
                v.w2 = r[1];
                result = round(float(v.ui32) * getScaling(_scaling)) / getScaling(_scaling);
                break;
            }
            return result;
        }

        const uint16_t _offset;
        const uint16_t _blockNbr;
        const uint8_t _number;
        const DataType _dataType;
        const String _desc;
        const String _unit;
        const Scaling _scaling;
        const Value _default;

    private:
        template <typename MODBUS_TYPE>
        friend class DeviceDescription;
        RegisterDescription(uint16_t offset, uint16_t blockNbr, uint8_t number, DataType r_type, String desc, String unit, Scaling scaling, Value d)
            : _offset(offset), _blockNbr(blockNbr), _number(number), _dataType(r_type), _desc(desc), _unit(unit), _scaling(scaling), _default(d)
        {
        }
    };

    // Block
    class BlockDescription
    {
    public:
        ~BlockDescription() {}

        const String _name;
        const std::vector<RegisterDescription> _rds;
        const uint16_t _offset = 0;
        const uint16_t _number_reg = 0;

    private:
        template <typename MODBUS_TYPE>
        friend class DeviceDescription;
        BlockDescription(const String &name, const std::vector<RegisterDescription> &rds, uint16_t offset, uint16_t number_reg)
            : _name(name), _rds(rds), _offset(offset), _number_reg(number_reg)
        {
        }
    };

    // Full description of the blocks and registers of a device, with a check that the registers in a block are contiguous
    template <typename MODBUS_TYPE>
    class DeviceDescription
    {
    public:
        using RegisterType = typename MODBUS_TYPE::e_registers;
        static DeviceDescription makeDD(const char *name, const std::vector<BlockDefinition<MODBUS_TYPE>> &blocks)
        {
            std::vector<BlockDescription> bl;
            std::vector<RegisterReference> rr;
            rr.resize(RegisterType::last);

            uint16_t blockNbr = 0;
            for (auto i = blocks.begin(); i < blocks.end(); i++)
            {
                BlockDescription b = makeBlock(i->_name, blockNbr, i->_offset, i->_rd, rr);
                blockNbr++;
                bl.push_back(b);
            }
            return DeviceDescription(name, bl, rr);
        }

        // Get a description of the device
        String GetDescriptions() const
        {
            String result;
            char buf[200];
            sprintf(buf, "Device: %s\r\n", _name.c_str());
            result += buf;
            for (auto i = _bds.begin(); i < _bds.end(); i++)
            {
                sprintf(buf, "  Block: %s, offset=0x%04x (%06u), numreg=%i\r\n", i->_name.c_str(), i->_offset, i->_offset, i->_number_reg);
                result += buf;
                for (auto j = i->_rds.begin(); j < i->_rds.end(); j++)
                {
                    sprintf(buf, "    Register 0x%04x (%06u): %s\r\n", j->_offset, j->_offset, j->_desc.c_str());
                    result += buf;
                }
                sprintf(buf, "\r\n");
                result += buf;
            }
            return result;
        }

        const String _name;
        const std::vector<BlockDescription> _bds;
        const std::vector<RegisterReference> _rr;

    private:
        DeviceDescription(const char *name, const std::vector<BlockDescription> &bds, std::vector<RegisterReference> rr) : _bds(bds), _name(name), _rr(rr) {}
        static BlockDescription makeBlock(const String &name, uint16_t blockNbr, uint16_t offset, const std::vector<RegisterDefinition<MODBUS_TYPE>> &registers, std::vector<RegisterReference> &rr)
        {
            std::vector<RegisterDescription> rl;
            int number = 0;

            uint16_t r_offset = offset;
            for (auto i = registers.begin(); i < registers.end(); i++)
            {
                rr[int32_t(i->_register)] = RegisterReference{i->_desc, blockNbr, i - registers.begin()};
                RegisterDescription r(r_offset, blockNbr, numberRegisters(i->_dataType), i->_dataType, i->_desc, i->_unit, i->_scaling, i->_default);
                rl.push_back(r);
                number += numberRegisters(i->_dataType);
                r_offset += numberRegisters(i->_dataType);
            }

            return BlockDescription(name, rl, offset, number);
        }
    };

    // Block. This contains values for a block.
    class Block
    {
    public:
        Block(const BlockDescription &bd, int32_t _number_reg) : _bd(bd), _transaction(0) { _registers.resize(_number_reg); }

        const BlockDescription &_bd;

    private:
        String allValuesAsString() const
        {
            String result;
            char buf[200] = {0};

            // Transaction id
            sprintf(buf, "TransactionID=%i\r\n", _transaction);
            result = result += buf;
            sprintf(buf, "Block %s\r\n", _bd._name.c_str());
            result = result += buf;

            for (auto i = _bd._rds.begin(); i < _bd._rds.end(); i++)
            {
                String value = i->toString(&(_registers[i->_offset - _bd._offset]));
                sprintf(buf, "  %s=%s %s\n", i->_desc.c_str(), value.c_str(), i->_unit);
                result += buf;
            }
            return result;
        }
        float getFloatValue(const RegisterDescription &r) const
        {
            return r.toFloat32(&(_registers[r._offset - _bd._offset])) / getScaling(r._scaling);
        }
        float getFloatValue(const RegisterReference &rr) const
        {
            const RegisterDescription &r = _bd._rds[rr._register_idx];
            return getFloatValue(r);
        }
        void setFloatValue(const RegisterReference &rr, float f)
        {
            const RegisterDescription &r = _bd._rds[rr._register_idx];
            Value v = Value::_float32_t(f);
            _registers[r._offset - _bd._offset] = v.w1;
            _registers[r._offset - _bd._offset + 1] = v.w2;
        }
        template <typename T>
        friend class Device;

        std::vector<uint16_t> _registers;
        uint16_t _transaction;
    };

    template <typename MODBUS_TYPE>
    class Device
    {
    public:
        using RegisterType = typename MODBUS_TYPE::e_registers;
        Device(const DeviceDescription<MODBUS_TYPE> &dd) : _dd(dd)
        {
            for (auto i = _dd._bds.begin(); i < _dd._bds.end(); i++)
            {
                Block b(*i, i->_number_reg);
                modbus_gateway::Value v;
                for (auto j = i->_rds.begin(); j < i->_rds.end(); j++)
                {
                    switch (j->_number)
                    {
                    case 1:
                        b._registers[j->_offset - i->_offset] = j->_default.w;
                        break;
                    case 2:
                        b._registers[j->_offset - i->_offset] = j->_default.w1;
                        b._registers[j->_offset - i->_offset + 1] = j->_default.w2;
                        break;
                    }
                }
                _blocks.push_back(b);
            }
        }
        uint32_t GetBlockIndex(const String &name) const
        {
            for (auto i = _dd._bds.begin(); i < _dd._bds.end(); i++)
            {
                if (i->_name == name)
                    return i - _dd._bds.begin();
            }
            return -1;
        }
        const DeviceDescription<MODBUS_TYPE> &_dd;

    private:
        // Plain uint16_t values access
        void setRegister(uint32_t block_idx, uint32_t val_index, uint16_t val)
        {
            _blocks[block_idx]._registers[val_index] = val;
        }
        uint16_t getRegister(uint32_t block_idx, uint32_t val_index)
        {
            return _blocks[block_idx]._registers[val_index];
        }
        void setFloatValue(RegisterType r, float i)
        {
            RegisterReference rr = _dd._rr[r];
            if (rr._block_idx >= 0 && rr._register_idx >= 0)
            {
                _blocks[rr._block_idx].setFloatValue(rr, i);
                // Serial.printf("setFloatValue %s %i %i %i %i=%f\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx, r._blockNbr, r._offset, i);
            }
            else
            {
                Serial.printf("modbus_gateway::ConvertEM24ToWattNode::setFloatValue invalid reference for register %s %i %i\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx);
            }
        }
        float getFloatValue(RegisterType r)
        {
            float f = 0;
            RegisterReference rr = _dd._rr[r];
            if (rr._block_idx >= 0 && rr._register_idx >= 0)
            {
                f = _blocks[rr._block_idx].getFloatValue(rr);
            }
            else
            {
                Serial.printf("Can't find value %s %i %i\r\n", rr._desc.c_str(), rr._block_idx, rr._register_idx);
            }
            return f;
        }
        void setTransaction(uint32_t block_idx, uint32_t t)
        {
            _blocks[block_idx]._transaction - t;
        }
        String allValuesAsString() const
        {
            String r = "";
            for (auto i = _blocks.begin(); i < _blocks.end(); i++)
                r += i->allValuesAsString();
            return r;
        }
        template <typename T>
        friend class DataAccess;
        std::vector<Block> _blocks;
    };

    // You should only access data through DataAccess. DataAccess takes care of
    // Thread-Safety by locking and unlocking the mutex when it comes in and out of scope
    // Instantiate this class to modify or retrieve values, but do not keep it around
    // as it locks the mutex for thread synchronization
    template <typename MODBUS_TYPE>
    class DataAccess
    {
    public:
        using RegisterType = typename MODBUS_TYPE::RegisterType;
        DataAccess(MODBUS_TYPE &s) : _s(s)
        {
            _s._Mutex.lock();
        }
        ~DataAccess()
        {
            _s._Mutex.unlock();
        }
        DataAccess& operator=(const DataAccess&) = delete;
        DataAccess(const DataAccess&) = delete;
        DataAccess() = delete;

        // Plain uint16_t values access
        void setRegister(uint32_t block_idx, uint32_t val_index, uint16_t val)
        {
            _s._device.setRegister(block_idx, val_index, val);
        }
        uint16_t getRegister(uint32_t block_idx, uint32_t val_index)
        {
            return _s._device.getRegister(block_idx, val_index);
        }

        void setTransaction(uint32_t block_idx, uint32_t t)
        {
            _s._device.setTransaction(block_idx, t);
        }

        String allValuesAsString() const
        {
            return _s._device.allValuesAsString();
        }

        void setFloatValue(RegisterType r, float i)
        {
            _s._device.setFloatValue(r, i);
        }
        float getFloatValue(RegisterType r)
        {
            return _s._device.getFloatValue(r);
        }

    private:
        MODBUS_TYPE &_s;
    };

}
