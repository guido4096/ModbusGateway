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
    // Register layout is the single source of truth. Blocks and total word
    // storage are counted automatically. All field properties are explicit.
    using RegisterId = e_registers;
    inline static constexpr auto metadata = makeDeviceSchema<RegisterId>(
            block("pv", 40083,
                field<DataType::int16>(ac_power, Scaling::none, WordOrder::HighFirst, "AC Power", "W", 0),
                field<DataType::int16>(ac_power_sf, Scaling::none, WordOrder::HighFirst, "AC Power SF", "", 0)
            ),
            block("battery", 57716,
                field<DataType::float32>(b1_instantaneous_power, Scaling::none, WordOrder::LowFirst, "Battery 1 Instantaneous Power", "W", 0)
            ),
            block("dc", 40100,
                field<DataType::int16>(dc_power, Scaling::none, WordOrder::HighFirst, "DC Power", "W", 0),
                field<DataType::int16>(dc_power_sf, Scaling::none, WordOrder::HighFirst, "DC Power SF", "", 0)
            )
        );
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static DeviceType makeDevice();
};
static_assert(SolarEdge::metadata.valid, "Invalid SolarEdge register schema");

} // namespace modbus_gateway
