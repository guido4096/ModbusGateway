/**
 * @file      optimizer.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Optimizer Modbus Definitions
 */
#pragma once
#include "definitions.h"

namespace modbus_gateway {

struct Optimizer : SchemaApi<Optimizer> {
    // Device identity used by diagnostics; no runtime name argument is needed.
    static constexpr const char* name = "optimizer";
    // Logical register IDs, in index order. These are NOT wire addresses.
    // IDs index metadata.registers[] directly (O(1)); keep names stable.
    enum e_registers {
        optimizer_strategy,
        last
    };
    // ---- Compile-time schema dimensions ----
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 1;
    static constexpr std::size_t word_count = 2;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    // ---- Modbus register map ----
    // Each block starts at its wire address. Subsequent register addresses
    // are derived from preceding widths (1 word for 16-bit, 2 for 32-bit).
    // Field columns: ID, wire type, scale divisor, high-word-first,
    // description, engineering unit, default raw bits.
    // Keep the register order unchanged: it defines the wire layout.
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            // Block strategy (base 0x0000): addresses within this block are contiguous.
            {"strategy", 0x0000, {
                {optimizer_strategy, DataType::int32, Scaling::none, true, "Requested Power From Inverter", "0 1 2", 0},
            }},
        });
    }
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static const Metadata metadata;
    static DeviceType makeDevice();
};
// C++17 requires this definition after the schema is complete.
inline constexpr Optimizer::Metadata Optimizer::metadata = Optimizer::makeMetadata();
static_assert(Optimizer::metadata.valid, "Invalid Optimizer register schema");

} // namespace modbus_gateway
