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
#include <algorithm>
#include "modbus/register_codec.h"
#include "modbus/device_diagnostics.h"

namespace modbus_gateway {
// One instance owns mutable raw words; its schema owns immutable descriptions.
// Access to this storage is synchronized by the enclosing Client/Server owner.
template<typename Schema>
class Device {
public:
    static_assert(Schema::metadata.valid, "Invalid device register schema");
    using RegisterType = typename Schema::RegisterId;
    static constexpr auto& metadata = Schema::metadata;
    static constexpr std::size_t word_count = decltype(Schema::metadata)::word_count;

    Device() {
        for (const auto& registerDescription : metadata.registers) {
            storeBits(words_.data() + registerDescription.storage_offset,
                      registerDescription.words, registerDescription.high_word_first,
                      registerDescription.default_bits);
        }
    }

    // Protocol identity is instance state, independent of the register layout.
    void setServerId(uint8_t id) { server_id_ = id; }
    uint8_t serverId() const { return server_id_; }

    template<RegisterType Register>
    static constexpr const RegisterDescription& description() {
        return Schema::template description<Register>();
    }

    template<RegisterType Register>
    using value_type = typename Native<
        metadata.registers[static_cast<std::size_t>(Register)].type>::type;

    // Type, word order and storage offset are constants for each register ID.
    // These calls return raw native values; engineering scaling is separate.
    template<RegisterType Register>
    value_type<Register> get() const {
        constexpr auto registerDescription = description<Register>();
        return decodeTyped<registerDescription.type, registerDescription.high_word_first>(
            words_.data() + registerDescription.storage_offset);
    }

    template<RegisterType Register>
    void set(value_type<Register> value) {
        constexpr auto registerDescription = description<Register>();
        encodeTyped<registerDescription.type, registerDescription.high_word_first>(
            words_.data() + registerDescription.storage_offset, value);
    }

    // Runtime IDs still index directly; only wire addresses need a block search.
    static const RegisterDescription* describe(RegisterType id) {
        const auto registerIndex = static_cast<std::size_t>(id);
        return registerIndex < metadata.registers.size() ? &metadata.registers[registerIndex] : nullptr;
    }

    // Modbus addresses words, so a range may include part of a 32-bit value.
    // Check the entire range before copying: a gap cannot cause a partial result.
    bool readHoldingRegisters(uint16_t address, uint16_t count, uint16_t* output) const {
        if (!output || !validAddressRange(address, count)) {
            return false;
        }
        const auto start = contiguousRangeStart(address, count);
        if (start != invalid_position) {
            std::copy_n(words_.data() + start, count, output);
            return true;
        }
        if (!allWordsDefined(address, count)) {
            return false;
        }
        for (uint32_t wordIndex = 0; wordIndex < count; ++wordIndex) {
            output[wordIndex] = words_[storagePosition(uint32_t(address) + wordIndex)];
        }
        return true;
    }

    // Raw protocol writes apply no scaling or field permissions. Address
    // validation remains all-or-nothing, including ranges crossing block gaps.
    bool writeHoldingRegisters(uint16_t address, uint16_t count, const uint16_t* input) {
        if (!input || !validAddressRange(address, count)) {
            return false;
        }
        const auto start = contiguousRangeStart(address, count);
        if (start != invalid_position) {
            std::copy_n(input, count, words_.data() + start);
            return true;
        }
        if (!allWordsDefined(address, count)) {
            return false;
        }
        for (uint32_t wordIndex = 0; wordIndex < count; ++wordIndex) {
            words_[storagePosition(uint32_t(address) + wordIndex)] = input[wordIndex];
        }
        return true;
    }

    String allValuesAsString() const {
        return formatDeviceValues(metadata, words_.data());
    }

    const uint16_t* rawWords() const { return words_.data(); }
    static constexpr const auto& blocks() { return metadata.blocks; }

private:
    static constexpr std::size_t invalid_position = static_cast<std::size_t>(-1);

    static bool validAddressRange(uint16_t address, uint16_t count) {
        return count != 0 && uint32_t(address) + count <= 65536;
    }

    // A single block maps directly to contiguous storage. The general path
    // below also handles adjacent wire blocks declared in a different order.
    static std::size_t contiguousRangeStart(uint16_t address, uint16_t count) {
        for (const auto& block : metadata.blocks) {
            if (address >= block.address &&
                uint32_t(address) + count <= uint32_t(block.address) + block.words) {
                return block.storage_offset + address - block.address;
            }
        }
        return invalid_position;
    }

    static bool allWordsDefined(uint16_t address, uint16_t count) {
        for (uint32_t wordIndex = 0; wordIndex < count; ++wordIndex) {
            if (storagePosition(uint32_t(address) + wordIndex) == invalid_position) {
                return false;
            }
        }
        return true;
    }

    static std::size_t storagePosition(uint32_t address) {
        for (const auto& block : metadata.blocks) {
            if (address >= block.address && address < uint32_t(block.address) + block.words) {
                return block.storage_offset + address - block.address;
            }
        }
        return invalid_position;
    }

    std::array<uint16_t, word_count> words_{};
    uint8_t server_id_ = 0;
};
} // namespace modbus_gateway
