/**
 * @file      device.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Fixed-size Modbus register storage and access
 */
#pragma once
#include <Arduino.h>
#include <cstdio>
#include "modbus/register_codec.h"

namespace modbus_gateway {
/** Mutable state belonging to one device instance.
 * All current register words are held in one fixed-size array. Descriptors
 * remain in Schema::metadata (immutable), and the constructor copies the
 * declared raw defaults into the appropriate word positions. Runtime-only
 * values such as a device serial number may then be installed by a factory.
 */
template<typename Schema>
class Device {
public:
    using RegisterType = typename Schema::RegisterId;
    static constexpr auto& metadata = Schema::metadata;
    static constexpr std::size_t word_count = decltype(Schema::metadata)::word_count;

    Device() {
        for (std::size_t i=0; i<metadata.registers.size(); ++i) {
            const auto& d = metadata.registers[i];
            storeBits(words_.data()+d.storage_offset, d.words,
                      d.high_word_first, d.default_bits);
        }
    }

    // The protocol server ID is instance state, independent of register layout.
    void setServerId(uint8_t id) { server_id_ = id; }
    uint8_t serverId() const { return server_id_; }

    template<RegisterType R>
    using value_type = typename Native<metadata.registers[static_cast<std::size_t>(R)].type>::type;

    template<RegisterType R>
    static constexpr const RegisterDescription& description() {
        constexpr auto i = static_cast<std::size_t>(R);
        static_assert(i < Schema::metadata.registers.size(), "Register ID out of range");
        return metadata.registers[i];
    }

    // Every descriptor property used by the typed access path is known at
    // instantiation time. No runtime metadata lookup or codec dispatch occurs.
    template<RegisterType R>
    value_type<R> get() const {
        constexpr auto d = description<R>();
        return decodeTyped<d.type, d.high_word_first>(
            words_.data() + d.storage_offset);
    }

    template<RegisterType R>
    void set(value_type<R> v) {
        constexpr auto d = description<R>();
        encodeTyped<d.type, d.high_word_first>(
            words_.data() + d.storage_offset, v);
    }

    // Dynamic ID lookup is also constant time: enum value -> descriptor index.
    // Only wire-address lookup requires walking the block ranges.
    static const RegisterDescription* describe(RegisterType id) {
        auto i = static_cast<std::size_t>(id);
        return i < metadata.registers.size() ? &metadata.registers[i] : nullptr;
    }

    // Address-based protocol requests traverse blocks (B is typically small).
    // A request crossing a hole fails, rather than fabricating zeros.
    // Validate the *entire* requested range before copying any words. A
    // request may begin midway through a 32-bit field, as Modbus addresses
    // words rather than logical C++ values. Holes between blocks are invalid.
    bool readHoldingRegisters(uint16_t address, uint16_t count, uint16_t* output) const {
        if (!output || count == 0 || uint32_t(address)+count > 65536) return false;
        for (uint32_t n=0; n<count; ++n) {
            auto pos = findWord(uint32_t(address)+n);
            if (pos == invalid_position) return false;
        }
        for (uint32_t n=0; n<count; ++n)
            output[n] = words_[findWord(uint32_t(address)+n)];
        return true;
    }
    // Writes are likewise all-or-nothing with respect to address validity.
    // This raw protocol path does not apply scaling or field-level permissions.
    bool writeHoldingRegisters(uint16_t address, uint16_t count, const uint16_t* input) {
        if (!input || count == 0 || uint32_t(address)+count > 65536) return false;
        for (uint32_t n=0; n<count; ++n)
            if (findWord(uint32_t(address)+n) == invalid_position) return false;
        for (uint32_t n=0; n<count; ++n)
            words_[findWord(uint32_t(address)+n)] = input[n];
        return true;
    }
    // Format the current register values using the schema descriptions and scaling.
    String allValuesAsString() const {
        String result;
        char buf[96];
        for (std::size_t block_index = 0; block_index < metadata.blocks.size(); ++block_index) {
            const auto& block = metadata.blocks[block_index];
            result += "Block ";
            result += block.name;
            result += "\r\n";
            for (const auto& r : metadata.registers) {
                if (r.block != block_index) continue;
                const uint32_t bits = loadBits(words_.data() + r.storage_offset,
                                               r.words, r.high_word_first);
                const double scale = static_cast<double>(getScaling(r.scaling));
                const int decimals = static_cast<int>(fractionalDigits(r.scaling));
                switch (r.type) {
                case DataType::float32:
                    snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(fromBits(bits)) / scale);
                    break;
                case DataType::int16: {
                    const int16_t v = static_cast<int16_t>(static_cast<uint16_t>(bits));
                    if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v) / scale);
                    else snprintf(buf, sizeof(buf), "%d", static_cast<int>(v));
                    break;
                }
                case DataType::uint16: {
                    const uint16_t v = static_cast<uint16_t>(bits);
                    if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v) / scale);
                    else snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(v));
                    break;
                }
                case DataType::int32: {
                    int32_t v;
                    std::memcpy(&v, &bits, sizeof(v));
                    if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v) / scale);
                    else snprintf(buf, sizeof(buf), "%ld", static_cast<long>(v));
                    break;
                }
                case DataType::uint32:
                    if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(bits) / scale);
                    else snprintf(buf, sizeof(buf), "%lu", static_cast<unsigned long>(bits));
                    break;
                }
                result += "  ";
                result += r.name;
                result += "=";
                result += buf;
                if (r.unit && *r.unit) {
                    result += " ";
                    result += r.unit;
                }
                result += "\r\n";
            }
        }
        return result;
    }

    const uint16_t* rawWords() const { return words_.data(); }
    static constexpr const auto& blocks() { return metadata.blocks; }

private:
    static constexpr std::size_t invalid_position = static_cast<std::size_t>(-1);
    // Convert a wire address to a flat storage offset. Each disjoint block
    // has its own wire base; its words are packed contiguously in words_.
    static std::size_t findWord(uint32_t address) {
        for (const auto& b : metadata.blocks)
            if (address >= b.address && address < uint32_t(b.address)+b.words)
                return b.storage_offset + address - b.address;
        return invalid_position;
    }
    std::array<uint16_t, word_count> words_{};
    uint8_t server_id_ = 0;
};

} // namespace modbus_gateway
