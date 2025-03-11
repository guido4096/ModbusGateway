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
    // Register
    struct Register
    {
        union value_t
        {
            static value_t _uint32_t(uint32_t v)
            {
                value_t r;
                r.ui32 = v;
                return r;
            }
            static value_t _int32_t(int32_t v)
            {
                value_t r;
                r.i32 = v;
                return r;
            }
            static value_t _int16_t(int16_t v)
            {
                value_t r;
                r.i16 = v;
                return r;
            }
            static value_t _uint16_t(uint16_t v)
            {
                value_t r;
                r.ui16 = v;
                return r;
            }
            static value_t _float32_t(float v)
            {
                value_t r;
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
                uint16_t i16;
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

        enum data_type_e
        {
            float32,
            int16,
            uint16,
            int32,
            uint32
        };
        Register(const String &name, uint16_t offset, uint8_t number, data_type_e r_type, String desc, String unit, uint32_t scaling, value_t d)
            : _name(name), _offset(offset), _number(number), _r_type(r_type), _desc(desc), _unit(unit), _scaling(scaling), _default(d)
        {
        }
        String toString(const uint16_t *r) const
        {
            String result = "unknown";
            char buf[40] = {0};
            value_t v;
            switch (_r_type)
            {
            case float32:
                v.w1 = r[0];
                v.w2 = r[1];
                sprintf(buf, "%f", v.f32 / _scaling);
                result = buf;
                break;
            case int16:
                v.w1 = r[0];
                if (_scaling == 1)
                    sprintf(buf, "%i", v.i16);
                else
                    sprintf(buf, "%.*f", lround(log10(_scaling)), float(v.i16) / _scaling);
                result = buf;
                break;
            case uint16:
                v.w1 = r[0];
                if (_scaling == 1)
                    sprintf(buf, "%u", v.ui16);
                else
                    sprintf(buf, "%.*f", lround(log10(_scaling)), float(v.ui16) / _scaling);
                result = buf;
                break;
            case int32:
                v.w1 = r[0];
                v.w2 = r[1];
                if (_scaling == 1)
                    sprintf(buf, "%i", v.i32);
                else
                    sprintf(buf, "%.*f", lround(log10(_scaling)), float(v.i32) / _scaling);
                result = buf;
                break;
            case uint32:
                v.w1 = r[0];
                v.w2 = r[1];
                if (_scaling == 1)
                    sprintf(buf, "%u", v.ui32);
                else
                    sprintf(buf, "%.*f", lround(log10(_scaling)), float(v.ui32) / _scaling);
                result = buf;
                break;
            }

            return result;
        }
        float toFloat32(const uint16_t *r) const
        {
            float result = 0;
            value_t v;
            switch (_r_type)
            {
            case float32:
                v.w1 = r[0];
                v.w2 = r[1];
                result = round(float(v.f32) * _scaling) / _scaling;
                break;
            case int16:
                v.w1 = r[0];
                result = round(float(v.i16) * _scaling) / _scaling;
                break;
            case uint16:
                v.w1 = r[0];
                result = round(float(v.ui16) * _scaling) / _scaling;
                break;
            case int32:
                v.w1 = r[0];
                v.w2 = r[1];
                result = round(float(v.i32) * _scaling) / _scaling;
                break;
            case uint32:
                v.w1 = r[0];
                v.w2 = r[1];
                result = round(float(v.ui32) * _scaling) / _scaling;
                break;
            }
            return result;
        }


        const String _name;
        const uint16_t _offset;
        const uint8_t _number;
        const data_type_e _r_type;
        const String _desc;
        const String _unit;
        const uint32_t _scaling;
        const value_t _default;        
    };

    // RegisterBlock
    class RegisterBlock
    {
    public:
        ~RegisterBlock() {}
        static RegisterBlock makeBlock(const char *name, const std::vector<Register> &registers)
        {
            String errors;
            char buf[200];
            sprintf(buf, "Check register list %s \r\n", name); 
            errors.concat(buf); 
            
            std::vector<Register> r;
            int offset = 0;
            int number = 0;
            if (registers.size() == 0)
            {
                sprintf(buf, "ERROR: Block of size 0");
                errors.concat(buf);
            }
            else 
            {
                offset = registers[0]._offset;
                int expected_offset = offset;
                for (auto i = registers.begin(); i < registers.end(); i++)
                {
                    if (i->_offset != expected_offset)
                    {
                        sprintf(buf, "ERROR: Offset %i doesn't start at expected Offset %i. Skipping the rest of the registers.\r\n", i->_offset, offset); 
                        errors.concat(buf);
                        break;
                    }
                    r.push_back(*i);
                    number += i->_number;
                    expected_offset += i->_number;
                }
            }
            return RegisterBlock(name, errors, r, offset, number);
        }
        const String _errors;
        const String _name;
        const std::vector<Register> _registers;
        const uint16_t _offset = 0;
        const uint16_t _number_reg = 0;
    private:
        RegisterBlock(const char *name, const String& errors, const std::vector<Register> &registers, uint16_t offset, uint16_t number_reg)
        : _name(name)
        , _errors(errors)
        , _registers(registers)
        , _offset(offset)
        , _number_reg(number_reg)
        {
        }        
    };

// RegisterReference. Avoid searching by string and instead use indexing
#define REGISTER_REFERENCE(name)                                                                         \
static const DeviceDescription::RegisterReference &name()                                                \
{                                                                                                        \
    static DeviceDescription::RegisterReference rr = getDeviceDescription().getRegisterReference(#name); \
    return rr;                                                                                           \
}

    // Full description of the blocks and registers of a device, with a check that the registers in a block are contiguous
    class DeviceDescription
    {
    public:
        struct RegisterReference
        {
            const char *_name;
            int32_t _block_idx;
            int32_t _register_idx;
        };

        DeviceDescription(const char* name, const std::vector<RegisterBlock> &blocks) : _blocks(blocks), _name(name) {}
        // Get an index based reference to a register
        RegisterReference getRegisterReference(const char *name) const
        {
            for (auto i = _blocks.begin(); i < _blocks.end(); i++)
            {
                for (auto j = i->_registers.begin(); j < i->_registers.end(); j++)
                {
                    if (j->_name == name)
                    {
                        return {name, i - _blocks.begin(), j - i->_registers.begin()};
                    }
                }
            }
            Serial.printf("Can't find register %s\r\n", name);
            return {name, -1, -1};
        }
        // Get a description of the device
        String GetDescriptions() const
        {
            String result;
            char buf[200];
            sprintf(buf, "Device: %s\r\n", _name.c_str());
            result+=buf;
            for (auto i = _blocks.begin(); i < _blocks.end(); i++)
            {
                sprintf(buf, "  %s", i->_errors.c_str());
                result+=buf;
                sprintf(buf, "  Block: %s, offset=%i, numreg=%i\r\n", i->_name.c_str(), i->_offset, i->_number_reg);
                result+=buf;
                for (auto j = i->_registers.begin(); j < i->_registers.end(); j++)
                {
                    sprintf(buf, "    Register: %s\r\n", j->_name.c_str());
                    result+=buf;
                }
                sprintf(buf, "\r\n");
                result+=buf;
            }
            return result;
        }

        const String _name;
        const std::vector<RegisterBlock> _blocks;
    };

    // BlockValues. This contains values for a block.
    struct BlockValues
    {
        BlockValues(const RegisterBlock& block, int32_t _number_reg) : _block(block), _transaction(0) {_values.resize(_number_reg);}

        bool getFloatValue(const DeviceDescription::RegisterReference &rr, float &o) const
        {
            const Register &r = _block._registers[rr._register_idx];
            o = r.toFloat32(&(_values[r._offset - _block._offset])) / r._scaling;
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

        const RegisterBlock& _block;
        std::vector<uint16_t> _values;
        uint16_t _transaction;
    };
}
