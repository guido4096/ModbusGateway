/**
 * @file      em24_e1.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      EM24 Modbus Definitions
 */
// Compile-time schema migration: metadata is constexpr; device values remain runtime state.
#pragma once
#include "definitions.h"
namespace modbus_gateway {
struct EM24_E1 {
    static constexpr const char* name = "em24_e1";
    enum e_registers {
        l1_voltage,
        l2_voltage,
        l3_voltage,
        l12_voltage,
        l23_voltage,
        l31_voltage,
        l1_current,
        l2_current,
        l3_current,
        l1_power_active,
        l2_power_active,
        l3_power_active,
        l1_power_apparent,
        l2_power_apparent,
        l3_power_apparent,
        l1_power_reactive,
        l2_power_reactive,
        l3_power_reactive,
        voltage_ln,
        voltage_ll,
        power_active,
        power_apparent,
        power_reactive,
        l1_power_factor,
        l2_power_factor,
        l3_power_factor,
        total_pf,
        phase_sequence,
        frequency,
        import_energy_active,
        import_energy_reactive,
        demand_power_active,
        maximum_demand_power_active,
        import_energy_active_partial,
        import_energy_reactive_partial,
        l1_import_energy_active,
        l2_import_energy_active,
        l3_import_energy_active,
        t1_import_energy,
        t2_import_energy,
        t3_import_energy,
        t4_import_energy,
        export_energy_active,
        export_energy_reactive,
        hour,
        t1_import_reactive,
        t2_import_reactive,
        t3_import_reactive,
        t4_import_reactive,
        demand_power_apparent,
        maximum_demand_power_apparent,
        maximum_demand_current,
        last
    };
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 4;
    static constexpr std::size_t word_count = 98;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            {"dynamic", 0x0000, {
                {l1_voltage, DataType::int32, Scaling::ten, true, "L1 Voltage", "V", static_cast<uint32_t>(0)},
                {l2_voltage, DataType::int32, Scaling::ten, true, "L2 Voltage", "V", static_cast<uint32_t>(0)},
                {l3_voltage, DataType::int32, Scaling::ten, true, "L3 Voltage", "V", static_cast<uint32_t>(0)},
                {l12_voltage, DataType::int32, Scaling::ten, true, "L1-L2 Voltage", "V", static_cast<uint32_t>(0)},
                {l23_voltage, DataType::int32, Scaling::ten, true, "L2-L3 Voltage", "V", static_cast<uint32_t>(0)},
                {l31_voltage, DataType::int32, Scaling::ten, true, "L3-L1 Voltage", "V", static_cast<uint32_t>(0)},
                {l1_current, DataType::int32, Scaling::thousand, true, "L1 Current", "A", static_cast<uint32_t>(0)},
                {l2_current, DataType::int32, Scaling::thousand, true, "L2 Current", "A", static_cast<uint32_t>(0)},
                {l3_current, DataType::int32, Scaling::thousand, true, "L3 Current", "A", static_cast<uint32_t>(0)},
                {l1_power_active, DataType::int32, Scaling::ten, true, "L1 Power (Active)", "W", static_cast<uint32_t>(0)},
                {l2_power_active, DataType::int32, Scaling::ten, true, "L2 Power (Active)", "W", static_cast<uint32_t>(0)},
                {l3_power_active, DataType::int32, Scaling::ten, true, "L3 Power (Active)", "W", static_cast<uint32_t>(0)},
                {l1_power_apparent, DataType::int32, Scaling::ten, true, "L1 Power (Apparent)", "VA", static_cast<uint32_t>(0)},
                {l2_power_apparent, DataType::int32, Scaling::ten, true, "L2 Power (Apparent)", "VA", static_cast<uint32_t>(0)},
                {l3_power_apparent, DataType::int32, Scaling::ten, true, "L3 Power (Apparent)", "VA", static_cast<uint32_t>(0)},
                {l1_power_reactive, DataType::int32, Scaling::ten, true, "L1 Power (Reactive)", "VAr", static_cast<uint32_t>(0)},
                {l2_power_reactive, DataType::int32, Scaling::ten, true, "L2 Power (Reactive)", "VAr", static_cast<uint32_t>(0)},
                {l3_power_reactive, DataType::int32, Scaling::ten, true, "L3 Power (Reactive)", "VAr", static_cast<uint32_t>(0)},
                {voltage_ln, DataType::int32, Scaling::ten, true, "L-N Voltage", "V", static_cast<uint32_t>(0)},
                {voltage_ll, DataType::int32, Scaling::ten, true, "L-L Voltage", "V", static_cast<uint32_t>(0)},
                {power_active, DataType::int32, Scaling::ten, true, "Total Power (Active)", "W", static_cast<uint32_t>(0)},
                {power_apparent, DataType::int32, Scaling::ten, true, "Total Power (Apparent)", "VA", static_cast<uint32_t>(0)},
                {power_reactive, DataType::int32, Scaling::ten, true, "Total Power (Reactive)", "VAr", static_cast<uint32_t>(0)},
                {l1_power_factor, DataType::int16, Scaling::thousand, true, "L1 Power Factor", "", static_cast<uint32_t>(0)},
                {l2_power_factor, DataType::int16, Scaling::thousand, true, "L2 Power Factor", "", static_cast<uint32_t>(0)},
                {l3_power_factor, DataType::int16, Scaling::thousand, true, "L3 Power Factor", "", static_cast<uint32_t>(0)},
                {total_pf, DataType::int16, Scaling::thousand, true, "Total Power Factor", "", static_cast<uint32_t>(0)},
                {phase_sequence, DataType::int16, Scaling::none, true, "Phase Sequence", "", static_cast<uint32_t>(0)},
                {frequency, DataType::uint16, Scaling::ten, true, "Frequency", "Hz", static_cast<uint32_t>(0)},
            }},
            {"energy", 0x0034, {
                {import_energy_active, DataType::int32, Scaling::ten, true, "Imported Energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {import_energy_reactive, DataType::int32, Scaling::ten, true, "Imported Energy (Reactive)", "Kvarh", static_cast<uint32_t>(0)},
                {demand_power_active, DataType::int32, Scaling::ten, true, "Demand Power Active", "W", static_cast<uint32_t>(0)},
                {maximum_demand_power_active, DataType::int32, Scaling::ten, true, "Maximum Demand Power Active", "W", static_cast<uint32_t>(0)},
                {import_energy_active_partial, DataType::int32, Scaling::ten, true, "Partial imported Energy (Active))", "kWh", static_cast<uint32_t>(0)},
                {import_energy_reactive_partial, DataType::int32, Scaling::ten, true, "Partial imported Energy (Reactive))", "Kvarh", static_cast<uint32_t>(0)},
                {l1_import_energy_active, DataType::int32, Scaling::ten, true, "L1 Imported Energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {l2_import_energy_active, DataType::int32, Scaling::ten, true, "L2 Imported Energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {l3_import_energy_active, DataType::int32, Scaling::ten, true, "L3 Imported Energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {t1_import_energy, DataType::int32, Scaling::ten, true, "Tarif 1 imported energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {t2_import_energy, DataType::int32, Scaling::ten, true, "Tarif 2 imported energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {t3_import_energy, DataType::int32, Scaling::ten, true, "Tarif 3 imported energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {t4_import_energy, DataType::int32, Scaling::ten, true, "Tarif 4 imported energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {export_energy_active, DataType::int32, Scaling::ten, true, "Exported Energy (Active)", "kWh", static_cast<uint32_t>(0)},
                {export_energy_reactive, DataType::int32, Scaling::ten, true, "Exported Energy (Reactive)", "Kvarh", static_cast<uint32_t>(0)},
            }},
            {"time", 0x005a, {
                {hour, DataType::int32, Scaling::hundred, true, "Hour", "hour", static_cast<uint32_t>(0)},
            }},
            {"tariff", 0x006e, {
                {t1_import_reactive, DataType::int32, Scaling::ten, true, "Tarif 1 imported energy (Reactive)", "Kvarh", static_cast<uint32_t>(0)},
                {t2_import_reactive, DataType::int32, Scaling::ten, true, "Tarif 2 imported energy (Reactive)", "Kvarh", static_cast<uint32_t>(0)},
                {t3_import_reactive, DataType::int32, Scaling::ten, true, "Tarif 3 imported energy (Reactive)", "Kvarh", static_cast<uint32_t>(0)},
                {t4_import_reactive, DataType::int32, Scaling::ten, true, "Tarif 4 imported energy (Reactive)", "Kvarh", static_cast<uint32_t>(0)},
                {demand_power_apparent, DataType::int32, Scaling::ten, true, "Demand Power (Apparent)", "VA", static_cast<uint32_t>(0)},
                {maximum_demand_power_apparent, DataType::int32, Scaling::ten, true, "Maximum Demand Power (Apparent)", "VA", static_cast<uint32_t>(0)},
                {maximum_demand_current, DataType::int32, Scaling::thousand, true, "Maximum Demand current (Active)", "A", static_cast<uint32_t>(0)},
            }},
        });
    }
    static const Metadata metadata;
    using DeviceType = Device<EM24_E1>;
    static DeviceType makeDevice();
    template<RegisterId R>
    static constexpr const RegisterDescription& description();
};
inline constexpr EM24_E1::Metadata EM24_E1::metadata = EM24_E1::makeMetadata();
static_assert(EM24_E1::metadata.valid, "Invalid EM24_E1 register schema");
template<EM24_E1::RegisterId R>
constexpr const RegisterDescription& EM24_E1::description() {
    constexpr auto index = static_cast<std::size_t>(R);
    static_assert(index < register_count, "Register index out of range");
    return metadata.registers[index];
}
} // namespace modbus_gateway
