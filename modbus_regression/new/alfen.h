/**
 * @file      alfen.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Alfen Modbus Definitions
 */
// Compile-time schema migration: metadata is constexpr; device values remain runtime state.
#pragma once
#include "definitions.h"
namespace modbus_gateway {
struct Alfen {
    static constexpr const char* name = "alfen";
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
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 1;
    static constexpr std::size_t word_count = 26;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            {"dynamic", 320, {
                {current_l1, DataType::float32, Scaling::none, false, "Current Phase L1", "A", static_cast<uint32_t>(0)},
                {current_l2, DataType::float32, Scaling::none, false, "Current Phase L2", "A", static_cast<uint32_t>(0)},
                {current_l3, DataType::float32, Scaling::none, false, "Current Phase L3", "A", static_cast<uint32_t>(0)},
                {current_sum, DataType::float32, Scaling::none, false, "Current Sum", "W", static_cast<uint32_t>(0)},
                {power_factor_l1, DataType::float32, Scaling::none, false, "Power Factor L1", "", static_cast<uint32_t>(0)},
                {power_factor_l2, DataType::float32, Scaling::none, false, "Power Factor L2", "", static_cast<uint32_t>(0)},
                {power_factor_l3, DataType::float32, Scaling::none, false, "Power Factor L3", "", static_cast<uint32_t>(0)},
                {power_factor_sum, DataType::float32, Scaling::none, false, "Power Factor Sum", "", static_cast<uint32_t>(0)},
                {frequency, DataType::float32, Scaling::none, false, "Frequency", "Hz", static_cast<uint32_t>(0)},
                {real_power_l1, DataType::float32, Scaling::none, false, "Real Power Phase L1", "W", static_cast<uint32_t>(0)},
                {real_power_l2, DataType::float32, Scaling::none, false, "Real Power Phase L2", "W", static_cast<uint32_t>(0)},
                {real_power_l3, DataType::float32, Scaling::none, false, "Real Power Phase L3", "W", static_cast<uint32_t>(0)},
                {real_power_sum, DataType::float32, Scaling::none, false, "Real Power Sum", "W", static_cast<uint32_t>(0)},
            }},
        });
    }
    static const Metadata metadata;
    using DeviceType = Device<Alfen>;
    static DeviceType makeDevice();
    template<RegisterId R>
    static constexpr const RegisterDescription& description();
};
inline constexpr Alfen::Metadata Alfen::metadata = Alfen::makeMetadata();
static_assert(Alfen::metadata.valid, "Invalid Alfen register schema");
template<Alfen::RegisterId R>
constexpr const RegisterDescription& Alfen::description() {
    constexpr auto index = static_cast<std::size_t>(R);
    static_assert(index < register_count, "Register index out of range");
    return metadata.registers[index];
}
} // namespace modbus_gateway
