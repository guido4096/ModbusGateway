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
    // Register layout is the single source of truth. Blocks and total word
    // storage are counted automatically. All field properties are explicit.
    using RegisterId = e_registers;
    inline static constexpr auto metadata = makeDeviceSchema<RegisterId>(
            block("dynamic", 320,
                field<DataType::float32>(current_l1, Scaling::none, WordOrder::HighFirst, "Current Phase L1", "A", 0),
                field<DataType::float32>(current_l2, Scaling::none, WordOrder::HighFirst, "Current Phase L2", "A", 0),
                field<DataType::float32>(current_l3, Scaling::none, WordOrder::HighFirst, "Current Phase L3", "A", 0),
                field<DataType::float32>(current_sum, Scaling::none, WordOrder::HighFirst, "Current Sum", "W", 0),
                field<DataType::float32>(power_factor_l1, Scaling::none, WordOrder::HighFirst, "Power Factor L1", "", 0),
                field<DataType::float32>(power_factor_l2, Scaling::none, WordOrder::HighFirst, "Power Factor L2", "", 0),
                field<DataType::float32>(power_factor_l3, Scaling::none, WordOrder::HighFirst, "Power Factor L3", "", 0),
                field<DataType::float32>(power_factor_sum, Scaling::none, WordOrder::HighFirst, "Power Factor Sum", "", 0),
                field<DataType::float32>(frequency, Scaling::none, WordOrder::HighFirst, "Frequency", "Hz", 0),
                field<DataType::float32>(real_power_l1, Scaling::none, WordOrder::HighFirst, "Real Power Phase L1", "W", 0),
                field<DataType::float32>(real_power_l2, Scaling::none, WordOrder::HighFirst, "Real Power Phase L2", "W", 0),
                field<DataType::float32>(real_power_l3, Scaling::none, WordOrder::HighFirst, "Real Power Phase L3", "W", 0),
                field<DataType::float32>(real_power_sum, Scaling::none, WordOrder::HighFirst, "Real Power Sum", "W", 0)
            )
        );
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static DeviceType makeDevice();
};
static_assert(Alfen::metadata.valid, "Invalid Alfen register schema");

} // namespace modbus_gateway
