/**
 * @file      register_schema.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Compile-time Modbus register schemas and validation
 */
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace modbus_gateway {
// A device field describes the *wire representation*, not its engineering
// value. Scaling is applied by engineering-value helpers/diagnostics; typed
// get/set operate on the unscaled native representation.
// WordOrder refers to the significance of the first 16-bit Modbus word,
// independent of the ESP32 CPU byte order.
enum class DataType : uint8_t { float32, int16, uint16, int32, uint32 };
enum class WordOrder : uint8_t { HighFirst, LowFirst };
enum class Scaling : uint16_t { none = 1, ten = 10, hundred = 100, thousand = 1000 };
constexpr uint16_t getScaling(Scaling scaling) { return static_cast<uint16_t>(scaling); }
constexpr uint8_t numberRegisters(DataType type) {
    return (type == DataType::int16 || type == DataType::uint16) ? 1 : 2;
}
constexpr unsigned fractionalDigits(Scaling scaling) {
    switch (scaling) {
    case Scaling::ten: return 1;
    case Scaling::hundred: return 2;
    case Scaling::thousand: return 3;
    default: return 0;
    }
}

template<DataType Type> struct Native;
template<> struct Native<DataType::float32> { using type = float; };
template<> struct Native<DataType::int16> { using type = int16_t; };
template<> struct Native<DataType::uint16> { using type = uint16_t; };
template<> struct Native<DataType::int32> { using type = int32_t; };
template<> struct Native<DataType::uint32> { using type = uint32_t; };

/** A computed logical-register descriptor, indexed directly by RegisterId.
 * `address` is the Modbus wire address; `word_offset` is block-relative and
 * `storage_offset` addresses the flat runtime word array. Multiword fields
 * occupy consecutive words and may be read partially by the protocol layer.
 */
struct RegisterDescription {
    uint16_t address;
    uint16_t block;
    uint16_t word_offset;      // within the block
    uint16_t storage_offset;   // within the device's flat word storage
    uint8_t words;
    DataType type;
    Scaling scaling;
    bool high_word_first;
    const char* name;
    const char* unit;
    uint32_t default_bits;     // raw IEEE754/int bits; dynamic defaults set per instance
};
struct BlockDescription {
    const char* name;
    uint16_t address;
    uint16_t words;
    uint16_t storage_offset;
};

// Small metadata records live in flash/read-only storage; buffers live in the Device.
// No lookup loops are used when a register is identified by its enum value.
// The array extents are part of the type, so no schema containers or
// dynamically allocated descriptions are built at device startup. `valid`
// captures checks performed during constant evaluation; each schema must
// assert it before being used.
enum class SchemaError : uint8_t {
    none, invalid_id, duplicate_id, missing_id, address_overflow,
    storage_overflow, block_overflow, overlapping_blocks, invalid_field
};

template<typename Id, std::size_t RegisterCount, std::size_t BlockCount, std::size_t WordCount>
struct StaticDescription {
    static constexpr std::size_t word_count = WordCount;
    std::array<RegisterDescription, RegisterCount> registers{};
    std::array<BlockDescription, BlockCount> blocks{};
    bool valid = true;
    SchemaError error = SchemaError::none;
    constexpr void reject(SchemaError reason) {
        valid = false;
        if (error == SchemaError::none) {
            error = reason;
        }
    }
};


// C++17 value-and-type schema builder. Widths and array dimensions are
// derived from declarations, not repeated by each device.
// Every field explicitly specifies type, scaling, word order and raw default.
// A field carries its width/type in the C++ type (TypedField<Id, Type>),
// and its per-register metadata in constexpr values. This separation lets
// block() sum field widths without restating register or block counts.
template<typename Id, DataType Type>
struct TypedField {
    using id_type = Id;
    static constexpr DataType data_type = Type;
    static constexpr std::size_t word_count = numberRegisters(Type);
    Id id;
    Scaling scaling;
    WordOrder order;
    const char* name;
    const char* unit;
    uint32_t default_bits;
};

template<DataType Type, typename Id>
constexpr auto field(Id id, Scaling scaling, WordOrder order,
                     const char* name, const char* unit, uint32_t default_bits) {
    return TypedField<Id, Type>{id, scaling, order, name, unit, default_bits};
}

// A block is a contiguous wire address range. Its tuple retains the
// declaration order; the word count is a fold over the declared field types.
// Gaps between blocks are permitted, but gaps inside a block are not.
template<typename... Fields>
struct TypedBlock {
    static constexpr std::size_t word_count = (Fields::word_count + ... + 0);
    const char* name;
    uint32_t address;
    std::tuple<Fields...> fields;
};

template<typename... Fields>
constexpr auto block(const char* name, uint32_t address, Fields... fields) {
    static_assert(sizeof...(Fields) > 0, "A Modbus block must contain fields");
    return TypedBlock<Fields...>{name, address, std::tuple<Fields...>{fields...}};
}

/** Materialize the schema during constant evaluation.
 * Register IDs form dense indexes [0, Id::last); the enumeration does not
 * encode wire addresses. Each field is placed in declaration order within
 * its block, and the resulting descriptor is stored at registers[field.id].
 * Blocks are packed into a separate flat storage array, irrespective of gaps
 * in Modbus addresses.
 *
 * The `assignedIds` bitmap rejects missing/duplicate IDs; bounds and overlap
 * checks reject malformed wire ranges. A schema-level static_assert(valid)
 * turns these checks into build failures.
 */
template<typename Id, typename... Blocks>
constexpr auto makeDeviceSchema(const Blocks&... blockDefinitions) {
    constexpr std::size_t registerCount = static_cast<std::size_t>(Id::last);
    constexpr std::size_t blockCount = sizeof...(Blocks);
    constexpr std::size_t wordCount = (Blocks::word_count + ... + 0);
    StaticDescription<Id, registerCount, blockCount, wordCount> description{};
    std::array<bool, registerCount> assignedIds{};
    std::size_t blockIndex = 0;
    std::size_t storageOffset = 0;

    // These generic lambdas operate on values, but std::apply and the fold
    // expression expand the field types at compilation. They preserve the
    // source declaration order when computing wire and storage offsets.
    auto appendBlock = [&](const auto& blockDefinition) {
        std::size_t blockWordOffset = 0;
        std::apply([&](const auto&... fields) {
            auto appendField = [&](const auto& fieldDefinition) {
                using Field = std::decay_t<decltype(fieldDefinition)>;
                static_assert(std::is_same_v<typename Field::id_type, Id>,
                              "Register ID type does not match schema");
                constexpr auto fieldWordCount = Field::word_count;
                const auto registerIndex = static_cast<std::size_t>(fieldDefinition.id);
                const bool validType = static_cast<unsigned>(Field::data_type) <=
                                       static_cast<unsigned>(DataType::uint32);
                const bool validWidth = fieldWordCount == numberRegisters(Field::data_type);
                const bool validOrder = fieldDefinition.order == WordOrder::HighFirst ||
                                        fieldDefinition.order == WordOrder::LowFirst;
                const bool validScaling = fieldDefinition.scaling == Scaling::none ||
                                          fieldDefinition.scaling == Scaling::ten ||
                                          fieldDefinition.scaling == Scaling::hundred ||
                                          fieldDefinition.scaling == Scaling::thousand;

                if (registerIndex >= registerCount) {
                    description.reject(SchemaError::invalid_id);
                } else if (assignedIds[registerIndex]) {
                    description.reject(SchemaError::duplicate_id);
                } else if (uint64_t(blockDefinition.address) + blockWordOffset + fieldWordCount > 65536) {
                    description.reject(SchemaError::address_overflow);
                } else if (storageOffset + blockWordOffset + fieldWordCount > wordCount ||
                           storageOffset + blockWordOffset > 65535) {
                    description.reject(SchemaError::storage_overflow);
                } else if (!validType || !validWidth || !validOrder || !validScaling) {
                    description.reject(SchemaError::invalid_field);
                } else {
                    assignedIds[registerIndex] = true;
                    description.registers[registerIndex] = RegisterDescription{
                        static_cast<uint16_t>(uint32_t(blockDefinition.address) + blockWordOffset),
                        static_cast<uint16_t>(blockIndex),
                        static_cast<uint16_t>(blockWordOffset),
                        static_cast<uint16_t>(storageOffset + blockWordOffset),
                        static_cast<uint8_t>(fieldWordCount), Field::data_type, fieldDefinition.scaling,
                        fieldDefinition.order == WordOrder::HighFirst,
                        fieldDefinition.name, fieldDefinition.unit, fieldDefinition.default_bits
                    };
                }
                blockWordOffset += fieldWordCount;
            };
            (appendField(fields), ...);
        }, blockDefinition.fields);
        if (blockIndex > 65535 || blockWordOffset > 65535 || blockWordOffset == 0) {
            description.reject(SchemaError::block_overflow);
        }
        if (storageOffset > 65535) {
            description.reject(SchemaError::storage_overflow);
        }
        description.blocks[blockIndex] = BlockDescription{
            blockDefinition.name, static_cast<uint16_t>(blockDefinition.address), static_cast<uint16_t>(blockWordOffset),
            static_cast<uint16_t>(storageOffset)
        };
        storageOffset += blockWordOffset;
        ++blockIndex;
    };
    (appendBlock(blockDefinitions), ...);
    if (storageOffset != wordCount || wordCount > 65536) {
        description.reject(SchemaError::storage_overflow);
    }
    for (bool present : assignedIds) {
        if (!present) {
            description.reject(SchemaError::missing_id);
        }
    }
    for (std::size_t leftIndex = 0; leftIndex < blockCount; ++leftIndex) {
        for (std::size_t rightIndex = leftIndex + 1; rightIndex < blockCount; ++rightIndex) {
            const auto& left = description.blocks[leftIndex];
            const auto& right = description.blocks[rightIndex];
            if (uint32_t(left.address) < uint32_t(right.address) + right.words &&
                uint32_t(right.address) < uint32_t(left.address) + left.words) {
                description.reject(SchemaError::overlapping_blocks);
            }
        }
    }
    return description;
}

// Shared schema API: device headers provide only the register IDs and map.
// Inherited description<R>() is a direct array index, never a name search.
// The schema defines RegisterId and constexpr metadata; sizes are derived.
template<typename Schema> class Device;

// CRTP shares the public schema interface without forcing each device to
// repeat a description<R>() function. Instantiation of its member template
// is deferred until the derived Schema is complete and metadata is defined.
template<typename Schema>
struct SchemaApi {
    using DeviceType = Device<Schema>;

    template<auto Register>
    static constexpr const RegisterDescription& description() {
        static_assert(std::is_same_v<decltype(Register), typename Schema::RegisterId>,
                      "Register ID belongs to another schema");
        constexpr auto registerIndex = static_cast<std::size_t>(Register);
        static_assert(registerIndex < Schema::metadata.registers.size(), "Register index out of range");
        return Schema::metadata.registers[registerIndex];
    }
};

} // namespace modbus_gateway
