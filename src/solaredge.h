/**
 * @file      solaredge.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      SolarEdge Modbus Definitions
 */
#pragma once
#include "definitions.h"

namespace modbus_gateway {

struct SolarEdge : SchemaApi<SolarEdge> {
    // Device identity used by diagnostics; no runtime name argument is needed.
    static constexpr const char* name = "solaredge";
    // Logical register IDs, in index order. These are NOT wire addresses.
    // IDs index metadata.registers[] directly (O(1)); keep names stable.
    enum e_registers {
        ac_power,
        ac_power_sf,
        b1_instantaneous_power,
        dc_power,
        dc_power_sf,
        last
    };
    // ---- Compile-time schema dimensions ----
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 3;
    static constexpr std::size_t word_count = 6;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    // ---- Modbus register map ----
    // Each block starts at its wire address. Subsequent register addresses
    // are derived from preceding widths (1 word for 16-bit, 2 for 32-bit).
    // Field columns: ID, wire type, scale divisor, high-word-first,
    // description, engineering unit, default raw bits.
    // Keep the register order unchanged: it defines the wire layout.
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            // Block pv (base 40083): addresses within this block are contiguous.
            {"pv", 40083, {
                {ac_power, DataType::int16, Scaling::none, true, "AC Power", "W", 0},
                {ac_power_sf, DataType::int16, Scaling::none, true, "AC Power SF", "", 0},
            }},
            // Block battery (base 57716): addresses within this block are contiguous.
            {"battery", 57716, {
                {b1_instantaneous_power, DataType::float32, Scaling::none, true, "Battery 1 Instantaneous Power", "W", 0},
            }},
            // Block dc (base 40100): addresses within this block are contiguous.
            {"dc", 40100, {
                {dc_power, DataType::int16, Scaling::none, true, "DC Power", "W", 0},
                {dc_power_sf, DataType::int16, Scaling::none, true, "DC Power SF", "", 0},
            }},
        });
    }
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static const Metadata metadata;
    static DeviceType makeDevice();
};
// C++17 requires this definition after the schema is complete.
inline constexpr SolarEdge::Metadata SolarEdge::metadata = SolarEdge::makeMetadata();
static_assert(SolarEdge::metadata.valid, "Invalid SolarEdge register schema");

} // namespace modbus_gateway
