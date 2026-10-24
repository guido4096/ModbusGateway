/**
 * @file      optimizer.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Optimizer Modbus Definitions
 */
// Compile-time schema migration: metadata is constexpr; device values remain runtime state.
#pragma once
#include "definitions.h"
namespace modbus_gateway {
struct Optimizer {
    static constexpr const char* name = "optimizer";
    enum e_registers {
        optimizer_strategy,
        last
    };
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 1;
    static constexpr std::size_t word_count = 2;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            {"strategy", 0x0000, {
                {optimizer_strategy, DataType::int32, Scaling::none, true, "Requested Power From Inverter", "0 1 2", static_cast<uint32_t>(0)},
            }},
        });
    }
    static const Metadata metadata;
    using DeviceType = Device<Optimizer>;
    static DeviceType makeDevice();
    template<RegisterId R>
    static constexpr const RegisterDescription& description();
};
inline constexpr Optimizer::Metadata Optimizer::metadata = Optimizer::makeMetadata();
static_assert(Optimizer::metadata.valid, "Invalid Optimizer register schema");
template<Optimizer::RegisterId R>
constexpr const RegisterDescription& Optimizer::description() {
    constexpr auto index = static_cast<std::size_t>(R);
    static_assert(index < register_count, "Register index out of range");
    return metadata.registers[index];
}
} // namespace modbus_gateway
