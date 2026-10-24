/**
 * @file      alfen.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Alfen Modbus Definitions
 */
#pragma once
#include "definitions.h"

namespace modbus_gateway {

struct Alfen : SchemaApi<Alfen> {
    // Device identity used by diagnostics; no runtime name argument is needed.
    static constexpr const char* name = "alfen";
    // Logical register IDs, in index order. These are NOT wire addresses.
    // IDs index metadata.registers[] directly (O(1)); keep names stable.
    enum e_registers {
        current_l1,
        current_l2,
        current_l3,
        current_sum,
        power_factor_l1,
        power_factor_l2,
        power_factor_l3,
        power_factor_sum,
        frequency,
        real_power_l1,
        real_power_l2,
        real_power_l3,
        real_power_sum,
        last
    };
    // ---- Compile-time schema dimensions ----
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 1;
    static constexpr std::size_t word_count = 26;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    // ---- Modbus register map ----
    // Each block starts at its wire address. Subsequent register addresses
    // are derived from preceding widths (1 word for 16-bit, 2 for 32-bit).
    // Field columns: ID, wire type, scale divisor, high-word-first,
    // description, engineering unit, default raw bits.
    // Keep the register order unchanged: it defines the wire layout.
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            // Block dynamic (base 320): addresses within this block are contiguous.
            {"dynamic", 320, {
                {current_l1, DataType::float32, Scaling::none, true, "Current Phase L1", "A", 0},
                {current_l2, DataType::float32, Scaling::none, true, "Current Phase L2", "A", 0},
                {current_l3, DataType::float32, Scaling::none, true, "Current Phase L3", "A", 0},
                {current_sum, DataType::float32, Scaling::none, true, "Current Sum", "W", 0},
                {power_factor_l1, DataType::float32, Scaling::none, true, "Power Factor L1", "", 0},
                {power_factor_l2, DataType::float32, Scaling::none, true, "Power Factor L2", "", 0},
                {power_factor_l3, DataType::float32, Scaling::none, true, "Power Factor L3", "", 0},
                {power_factor_sum, DataType::float32, Scaling::none, true, "Power Factor Sum", "", 0},
                {frequency, DataType::float32, Scaling::none, true, "Frequency", "Hz", 0},
                {real_power_l1, DataType::float32, Scaling::none, true, "Real Power Phase L1", "W", 0},
                {real_power_l2, DataType::float32, Scaling::none, true, "Real Power Phase L2", "W", 0},
                {real_power_l3, DataType::float32, Scaling::none, true, "Real Power Phase L3", "W", 0},
                {real_power_sum, DataType::float32, Scaling::none, true, "Real Power Sum", "W", 0},
            }},
        });
    }
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static const Metadata metadata;
    static DeviceType makeDevice();
};
// C++17 requires this definition after the schema is complete.
inline constexpr Alfen::Metadata Alfen::metadata = Alfen::makeMetadata();
static_assert(Alfen::metadata.valid, "Invalid Alfen register schema");

} // namespace modbus_gateway
