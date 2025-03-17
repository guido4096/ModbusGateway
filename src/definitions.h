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

namespace modbus
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
        String  _desc;
        int32_t _block_idx;
        int32_t _register_idx;
    };

    // Register and Block Defintion
    template <typename MODBUS_TYPE>
    struct RegisterDefinition
    {
        using RegisterType = typename MODBUS_TYPE::e_registers;
        const RegisterType _register;
        const DataType     _dataType;
        const String       _desc;
        const String       _unit;
        const Scaling      _scaling;
        const Value        _default;
    };
    template <typename MODBUS_TYPE>
    struct BlockDefinition
    {
        const String   _name;
        const uint16_t _offset;
        const std::vector<RegisterDefinition<MODBUS_TYPE>> _rd;
    };

    class Register
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
                sprintf(buf, "%f", v.f32 / getScaling(_scaling));
                result = buf;
                break;
            case int16:
                v.w1 = r[0];
                if (_scaling == none)
                    sprintf(buf, "%i", v.i16);
                else
                    sprintf(buf, "%.*f", lround(getLogScaling(_scaling)), float(v.i16) / getScaling(_scaling));
                result = buf;
                break;
            case uint16:
                v.w1 = r[0];
                if (_scaling == none)
                    sprintf(buf, "%u", v.ui16);
                else
                    sprintf(buf, "%.*f", lround(getLogScaling(_scaling)), float(v.ui16) / getScaling(_scaling));
                result = buf;
                break;
            case int32:
                v.w1 = r[0];
                v.w2 = r[1];
                if (_scaling == none)
                    sprintf(buf, "%i", v.i32);
                else
                    sprintf(buf, "%.*f", lround(getLogScaling(_scaling)), float(v.i32) / getScaling(_scaling));
                result = buf;
                break;
            case uint32:
                v.w1 = r[0];
                v.w2 = r[1];
                if (_scaling == none)
                    sprintf(buf, "%u", v.ui32);
                else
                    sprintf(buf, "%.*f", lround(getLogScaling(_scaling)), float(v.ui32) / getScaling(_scaling));
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
        const uint8_t _number;
        const DataType _dataType;
        const String _desc;
        const String _unit;
        const Scaling _scaling;
        const Value _default;

    private:
        template <typename MODBUS_TYPE>
        friend class DeviceDescription;
        Register(uint16_t offset, uint8_t number, DataType r_type, String desc, String unit, Scaling scaling, Value d)
            : _offset(offset), _number(number), _dataType(r_type), _desc(desc), _unit(unit), _scaling(scaling), _default(d)
        {
        }
    };

    // Block
    class Block
    {
    public:
        ~Block() {}

        const String _name;
        const std::vector<Register> _registers;
        const uint16_t _offset = 0;
        const uint16_t _number_reg = 0;

    private:
        template <typename MODBUS_TYPE>
        friend class DeviceDescription;
        Block(const String &name, const std::vector<Register> &registers, uint16_t offset, uint16_t number_reg)
            : _name(name), _registers(registers), _offset(offset), _number_reg(number_reg)
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
            std::vector<Block> bl;
            std::vector<RegisterReference> rr;
            rr.resize(RegisterType::last);

            uint16_t blockNbr = 0;
            for (auto i = blocks.begin(); i < blocks.end(); i++)
            {
                Block b = makeBlock(i->_name, blockNbr, i->_offset, i->_rd, rr);
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
            for (auto i = _blocks.begin(); i < _blocks.end(); i++)
            {
                sprintf(buf, "  Block: %s, offset=0x%04x (%06u), numreg=%i\r\n", i->_name.c_str(), i->_offset, i->_offset, i->_number_reg);
                result += buf;
                for (auto j = i->_registers.begin(); j < i->_registers.end(); j++)
                {
                    sprintf(buf, "    Register 0x%04x (%06u): %s\r\n", j->_offset, j->_offset, j->_desc.c_str());
                    result += buf;
                }
                sprintf(buf, "\r\n");
                result += buf;
            }
            return result;
        }

        RegisterReference getRegisterReference(RegisterType r) const
        {
            return _rr[r];
        }

        const String _name;
        const std::vector<Block> _blocks;
        const std::vector<RegisterReference> _rr;

    private:
        DeviceDescription(const char *name, const std::vector<Block> &blocks, std::vector<RegisterReference> rr) : _blocks(blocks), _name(name), _rr(rr) {}
        static Block makeBlock(const String &name, uint16_t blockNbr, uint16_t offset, const std::vector<RegisterDefinition<MODBUS_TYPE>> &registers, std::vector<RegisterReference> &rr)
        {
            std::vector<Register> rl;
            int number = 0;

            uint16_t r_offset = offset;
            for (auto i = registers.begin(); i < registers.end(); i++)
            {
                rr[int32_t(i->_register)] = RegisterReference{i->_desc, blockNbr, i - registers.begin()};
                Register r(r_offset, numberRegisters(i->_dataType), i->_dataType, i->_desc, i->_unit, i->_scaling, i->_default);
                rl.push_back(r);
                number += numberRegisters(i->_dataType);
                r_offset += numberRegisters(i->_dataType);
            }

            return Block(name, rl, offset, number);
        }
    };

    // BlockValues. This contains values for a block.
    struct BlockValues
    {
        BlockValues(const Block &block, int32_t _number_reg) : _block(block), _transaction(0) { _values.resize(_number_reg); }

        bool getFloatValue(const RegisterReference &rr, float &o) const
        {
            const Register &r = _block._registers[rr._register_idx];
            o = r.toFloat32(&(_values[r._offset - _block._offset])) / getScaling(r._scaling);
            return true;
        }
        String toString() const
        {
            String result;
            char buf[200] = {0};

            // Transaction id
            sprintf(buf, "TransactionID=%i\r\n", _transaction);
            result = result += buf;
            sprintf(buf, "Block %s\r\n", _block._name.c_str());
            result = result += buf;

            int d = 0;
            for (auto i = _block._registers.begin(); i < _block._registers.end(); i++)
            {
                String value = i->toString(&(_values[d]));
                sprintf(buf, "  %s=%s %s\n", i->_desc.c_str(), value.c_str(), i->_unit);
                result += buf;
                d += i->_number;
            }
            return result;
        }

        const Block &_block;
        std::vector<uint16_t> _values;
        uint16_t _transaction;
    };
}
