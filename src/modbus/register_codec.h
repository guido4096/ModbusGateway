/**
 * @file      register_codec.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Modbus word encoding and decoding
 */
#pragma once
#include <cstring>
#include "modbus/register_schema.h"

namespace modbus_gateway {
// Modbus carries 16-bit words. Explicitly assemble/disassemble the 32-bit
// bit pattern so host endianness cannot silently change the wire word order.
// memcpy preserves float bit patterns without inactive-union-member access.
inline uint32_t toBits(float x) {
    static_assert(sizeof(float)==4, "IEEE754 float32 required");
    uint32_t bits; std::memcpy(&bits, &x, 4); return bits;
}
inline float fromBits(uint32_t bits) {
    float value; std::memcpy(&value, &bits, 4); return value;
}
inline uint32_t loadBits(const uint16_t* words, uint8_t count, bool highFirst) {
    if (count == 1) return words[0];
    return highFirst ? (uint32_t(words[0]) << 16) | words[1]
                     : (uint32_t(words[1]) << 16) | words[0];
}
inline void storeBits(uint16_t* words, uint8_t count, bool highFirst, uint32_t bits) {
    if (count == 1) { words[0] = uint16_t(bits); return; }
    uint16_t hi = uint16_t(bits >> 16), lo = uint16_t(bits);
    words[0] = highFirst ? hi : lo;
    words[1] = highFirst ? lo : hi;
}

/**
 * Compile-time codec for typed register access.
 * Type and word order are non-type template arguments: there is no runtime
 * RegisterDescription parameter, type switch, or word-order branch. Runtime
 * Modbus address requests continue to use the existing raw-word path.
 *
 * Keep the independent known-wire-word regression tests: self round-trips
 * alone cannot detect a consistently reversed word order.
 */
template<DataType Type, bool HighFirst>
inline typename Native<Type>::type decodeTyped(const uint16_t* words) {
    using T = typename Native<Type>::type;
    if constexpr (Type == DataType::int16) {
        // Converting an unsigned word outside the signed range is implementation-defined;
        // the ESP32 toolchain uses the expected two's-complement interpretation.
        return static_cast<int16_t>(words[0]);
    } else if constexpr (Type == DataType::uint16) {
        return words[0];
    } else {
        const uint32_t bits = loadBits(words, 2, HighFirst);
        if constexpr (Type == DataType::float32) return fromBits(bits);
        else return static_cast<T>(bits);
    }
}

template<DataType Type, bool HighFirst>
inline void encodeTyped(uint16_t* words, typename Native<Type>::type value) {
    if constexpr (Type == DataType::float32) {
        storeBits(words, 2, HighFirst, toBits(value));
    } else if constexpr (Type == DataType::int16 || Type == DataType::uint16) {
        words[0] = static_cast<uint16_t>(value);
    } else {
        storeBits(words, 2, HighFirst, static_cast<uint32_t>(value));
    }
}

// Generic runtime codecs are used where the descriptor is selected at
// runtime (for example diagnostic formatting). Typed get/set below instead
// instantiate decodeTyped/encodeTyped with compile-time codec parameters.
template<typename T>
T decode(const uint16_t* data, const RegisterDescription& r) {
    uint32_t bits = loadBits(data, r.words, r.high_word_first);
    if constexpr (std::is_same_v<T,float>) {
        float raw = r.type == DataType::float32 ? fromBits(bits) :
                    r.type == DataType::int16 ? float(int16_t(uint16_t(bits))) :
                    r.type == DataType::uint16 ? float(uint16_t(bits)) :
                    r.type == DataType::int32 ? float(int32_t(bits)) : float(bits);
        return raw / getScaling(r.scaling);
    } else {
        // Exact integral access only; typed callers select the declared type.
        if constexpr (std::is_same_v<T,int16_t>) return int16_t(uint16_t(bits));
        else return static_cast<T>(bits);
    }
}

template<typename T>
void encode(uint16_t* data, const RegisterDescription& r, T value) {
    uint32_t bits = 0;
    if constexpr (std::is_same_v<T,float>) bits = toBits(value);
    else bits = static_cast<uint32_t>(value);
    storeBits(data, r.words, r.high_word_first, bits);
}

} // namespace modbus_gateway
