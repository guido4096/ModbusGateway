/**
 * @file      solaredge.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      SolarEdge Modbus Definitions
 */
// Compile-time schema migration: metadata is constexpr; device values remain runtime state.
#pragma once
#include "definitions.h"
namespace modbus_gateway {
struct SolarEdge {
    static constexpr const char* name = "solaredge";
    enum e_registers {
        ac_power,
        ac_power_sf,
        b1_instantaneous_power,
        dc_power,
        dc_power_sf,
        last
    };
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 3;
    static constexpr std::size_t word_count = 6;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            {"pv", 40083, {
                {ac_power, DataType::int16, Scaling::none, true, "AC Power", "W", static_cast<uint32_t>(0)},
                {ac_power_sf, DataType::int16, Scaling::none, true, "AC Power SF", "", static_cast<uint32_t>(0)},
            }},
            {"battery", 57716, {
                {b1_instantaneous_power, DataType::float32, Scaling::none, true, "Battery 1 Instantaneous Power", "W", static_cast<uint32_t>(0)},
            }},
            {"dc", 40100, {
                {dc_power, DataType::int16, Scaling::none, true, "DC Power", "W", static_cast<uint32_t>(0)},
                {dc_power_sf, DataType::int16, Scaling::none, true, "DC Power SF", "", static_cast<uint32_t>(0)},
            }},
        });
    }
    static const Metadata metadata;
    using DeviceType = Device<SolarEdge>;
    static DeviceType makeDevice();
    template<RegisterId R>
    static constexpr const RegisterDescription& description();
};
inline constexpr SolarEdge::Metadata SolarEdge::metadata = SolarEdge::makeMetadata();
static_assert(SolarEdge::metadata.valid, "Invalid SolarEdge register schema");
template<SolarEdge::RegisterId R>
constexpr const RegisterDescription& SolarEdge::description() {
    constexpr auto index = static_cast<std::size_t>(R);
    static_assert(index < register_count, "Register index out of range");
    return metadata.registers[index];
}
} // namespace modbus_gateway
