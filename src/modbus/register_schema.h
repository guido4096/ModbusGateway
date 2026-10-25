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
enum class Scaling : uint16_t { none=1, ten=10, hundred=100, thousand=1000 };
constexpr uint16_t getScaling(Scaling s) { return static_cast<uint16_t>(s); }
constexpr uint8_t numberRegisters(DataType d) {
    return (d == DataType::int16 || d == DataType::uint16) ? 1 : 2;
}
constexpr unsigned fractionalDigits(Scaling s) {
    return s == Scaling::ten ? 1 : s == Scaling::hundred ? 2 : s == Scaling::thousand ? 3 : 0;
}

template<DataType T> struct Native;
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
template<typename Id, std::size_t N, std::size_t B, std::size_t Words>
struct StaticDescription {
    static constexpr std::size_t word_count = Words;
    std::array<RegisterDescription, N> registers{};
    std::array<BlockDescription, B> blocks{};
    bool valid = true;
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
    uint16_t address;
    std::tuple<Fields...> fields;
};

template<typename... Fields>
constexpr auto block(const char* name, uint16_t address, Fields... fields) {
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
 * The `assigned` bitmap rejects missing/duplicate IDs; bounds and overlap
 * checks reject malformed wire ranges. A schema-level static_assert(valid)
 * turns these checks into build failures.
 */
template<typename Id, typename... Blocks>
constexpr auto makeDeviceSchema(const Blocks&... blocks) {
    constexpr std::size_t N = static_cast<std::size_t>(Id::last);
    constexpr std::size_t B = sizeof...(Blocks);
    constexpr std::size_t Words = (Blocks::word_count + ... + 0);
    StaticDescription<Id, N, B, Words> m{};
    std::array<bool, N> assigned{};
    std::size_t blockNo = 0, storage = 0;

    // These generic lambdas operate on values, but std::apply and the fold
    // expression expand the field types at compilation. They preserve the
    // source declaration order when computing wire and storage offsets.
    auto appendBlock = [&](const auto& b) {
        std::size_t offset = 0;
        std::apply([&](const auto&... fs) {
            auto appendField = [&](const auto& f) {
                using F = std::decay_t<decltype(f)>;
                static_assert(std::is_same_v<typename F::id_type, Id>,
                              "Register ID type does not match schema");
                constexpr auto width = F::word_count;
                const auto index = static_cast<std::size_t>(f.id);
                if (index >= N || assigned[index] ||
                    uint32_t(b.address) + offset + width > 65536 ||
                    storage + offset + width > Words) {
                    m.valid = false;
                } else {
                    assigned[index] = true;
                    m.registers[index] = RegisterDescription{
                        static_cast<uint16_t>(uint32_t(b.address) + offset),
                        static_cast<uint16_t>(blockNo),
                        static_cast<uint16_t>(offset),
                        static_cast<uint16_t>(storage + offset),
                        static_cast<uint8_t>(width), F::data_type, f.scaling,
                        f.order == WordOrder::HighFirst,
                        f.name, f.unit, f.default_bits
                    };
                }
                offset += width;
            };
            (appendField(fs), ...);
        }, b.fields);
        m.blocks[blockNo] = BlockDescription{
            b.name, b.address, static_cast<uint16_t>(offset),
            static_cast<uint16_t>(storage)
        };
        storage += offset;
        ++blockNo;
    };
    (appendBlock(blocks), ...);
    if (storage != Words) m.valid = false;
    for (bool present : assigned) if (!present) m.valid = false;
    for (std::size_t i=0; i<B; ++i)
        for (std::size_t j=i+1; j<B; ++j) {
            const auto& a=m.blocks[i]; const auto& b=m.blocks[j];
            if (uint32_t(a.address)<uint32_t(b.address)+b.words &&
                uint32_t(b.address)<uint32_t(a.address)+a.words)
                m.valid=false;
        }
    return m;
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

    template<auto R>
    static constexpr const RegisterDescription& description() {
        static_assert(std::is_same_v<decltype(R), typename Schema::RegisterId>,
                      "Register ID belongs to another schema");
        constexpr auto index = static_cast<std::size_t>(R);
        static_assert(index < Schema::metadata.registers.size(), "Register index out of range");
        return Schema::metadata.registers[index];
    }
};

} // namespace modbus_gateway
