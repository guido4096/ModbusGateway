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
    // Register layout is the single source of truth. Blocks and total word
    // storage are counted automatically. All field properties are explicit.
    using RegisterId = e_registers;
    inline static constexpr auto metadata = makeDeviceSchema<RegisterId>(
            block("strategy", 0x0000,
                field<DataType::int32>(optimizer_strategy, Scaling::none, WordOrder::LowFirst, "Requested Power From Inverter", "0 1 2", 0)
            )
        );
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static DeviceType makeDevice();
};
static_assert(Optimizer::metadata.valid, "Invalid Optimizer register schema");

} // namespace modbus_gateway
