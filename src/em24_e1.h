/**
 * @file      em24_e1.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      EM24 Modbus Definitions
 */
#pragma once
#include "definitions.h"

namespace modbus_gateway {

struct EM24_E1 : SchemaApi<EM24_E1> {
    // Device identity used by diagnostics; no runtime name argument is needed.
    static constexpr const char* name = "em24_e1";
    // Logical register IDs, in index order. These are NOT wire addresses.
    // IDs index metadata.registers[] directly (O(1)); keep names stable.
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
    // ---- Compile-time schema dimensions ----
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 4;
    static constexpr std::size_t word_count = 98;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    // ---- Modbus register map ----
    // Each block starts at its wire address. Subsequent register addresses
    // are derived from preceding widths (1 word for 16-bit, 2 for 32-bit).
    // Field columns: ID, wire type, scale divisor, high-word-first,
    // description, engineering unit, default raw bits.
    // EM24 uses LOW 16-bit word first for 32-bit values. The original
    // firmware used a host-endian union: its wordorder=true had the
    // opposite effective meaning on ESP32. Hence false is intentional.
    // Keep register order unchanged: it defines the wire layout.
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            // Block dynamic (base 0x0000): addresses within this block are contiguous.
            {"dynamic", 0x0000, {
                {l1_voltage, DataType::int32, Scaling::ten, false, "L1 Voltage", "V", 0},
                {l2_voltage, DataType::int32, Scaling::ten, false, "L2 Voltage", "V", 0},
                {l3_voltage, DataType::int32, Scaling::ten, false, "L3 Voltage", "V", 0},
                {l12_voltage, DataType::int32, Scaling::ten, false, "L1-L2 Voltage", "V", 0},
                {l23_voltage, DataType::int32, Scaling::ten, false, "L2-L3 Voltage", "V", 0},
                {l31_voltage, DataType::int32, Scaling::ten, false, "L3-L1 Voltage", "V", 0},
                {l1_current, DataType::int32, Scaling::thousand, false, "L1 Current", "A", 0},
                {l2_current, DataType::int32, Scaling::thousand, false, "L2 Current", "A", 0},
                {l3_current, DataType::int32, Scaling::thousand, false, "L3 Current", "A", 0},
                {l1_power_active, DataType::int32, Scaling::ten, false, "L1 Power (Active)", "W", 0},
                {l2_power_active, DataType::int32, Scaling::ten, false, "L2 Power (Active)", "W", 0},
                {l3_power_active, DataType::int32, Scaling::ten, false, "L3 Power (Active)", "W", 0},
                {l1_power_apparent, DataType::int32, Scaling::ten, false, "L1 Power (Apparent)", "VA", 0},
                {l2_power_apparent, DataType::int32, Scaling::ten, false, "L2 Power (Apparent)", "VA", 0},
                {l3_power_apparent, DataType::int32, Scaling::ten, false, "L3 Power (Apparent)", "VA", 0},
                {l1_power_reactive, DataType::int32, Scaling::ten, false, "L1 Power (Reactive)", "VAr", 0},
                {l2_power_reactive, DataType::int32, Scaling::ten, false, "L2 Power (Reactive)", "VAr", 0},
                {l3_power_reactive, DataType::int32, Scaling::ten, false, "L3 Power (Reactive)", "VAr", 0},
                {voltage_ln, DataType::int32, Scaling::ten, false, "L-N Voltage", "V", 0},
                {voltage_ll, DataType::int32, Scaling::ten, false, "L-L Voltage", "V", 0},
                {power_active, DataType::int32, Scaling::ten, false, "Total Power (Active)", "W", 0},
                {power_apparent, DataType::int32, Scaling::ten, false, "Total Power (Apparent)", "VA", 0},
                {power_reactive, DataType::int32, Scaling::ten, false, "Total Power (Reactive)", "VAr", 0},
                {l1_power_factor, DataType::int16, Scaling::thousand, true, "L1 Power Factor", "", 0},
                {l2_power_factor, DataType::int16, Scaling::thousand, true, "L2 Power Factor", "", 0},
                {l3_power_factor, DataType::int16, Scaling::thousand, true, "L3 Power Factor", "", 0},
                {total_pf, DataType::int16, Scaling::thousand, true, "Total Power Factor", "", 0},
                {phase_sequence, DataType::int16, Scaling::none, true, "Phase Sequence", "", 0},
                {frequency, DataType::uint16, Scaling::ten, true, "Frequency", "Hz", 0},
            }},
            // Block energy (base 0x0034): addresses within this block are contiguous.
            {"energy", 0x0034, {
                {import_energy_active, DataType::int32, Scaling::ten, false, "Imported Energy (Active)", "kWh", 0},
                {import_energy_reactive, DataType::int32, Scaling::ten, false, "Imported Energy (Reactive)", "Kvarh", 0},
                {demand_power_active, DataType::int32, Scaling::ten, false, "Demand Power Active", "W", 0},
                {maximum_demand_power_active, DataType::int32, Scaling::ten, false, "Maximum Demand Power Active", "W", 0},
                {import_energy_active_partial, DataType::int32, Scaling::ten, false, "Partial imported Energy (Active))", "kWh", 0},
                {import_energy_reactive_partial, DataType::int32, Scaling::ten, false, "Partial imported Energy (Reactive))", "Kvarh", 0},
                {l1_import_energy_active, DataType::int32, Scaling::ten, false, "L1 Imported Energy (Active)", "kWh", 0},
                {l2_import_energy_active, DataType::int32, Scaling::ten, false, "L2 Imported Energy (Active)", "kWh", 0},
                {l3_import_energy_active, DataType::int32, Scaling::ten, false, "L3 Imported Energy (Active)", "kWh", 0},
                {t1_import_energy, DataType::int32, Scaling::ten, false, "Tarif 1 imported energy (Active)", "kWh", 0},
                {t2_import_energy, DataType::int32, Scaling::ten, false, "Tarif 2 imported energy (Active)", "kWh", 0},
                {t3_import_energy, DataType::int32, Scaling::ten, false, "Tarif 3 imported energy (Active)", "kWh", 0},
                {t4_import_energy, DataType::int32, Scaling::ten, false, "Tarif 4 imported energy (Active)", "kWh", 0},
                {export_energy_active, DataType::int32, Scaling::ten, false, "Exported Energy (Active)", "kWh", 0},
                {export_energy_reactive, DataType::int32, Scaling::ten, false, "Exported Energy (Reactive)", "Kvarh", 0},
            }},
            // Block time (base 0x005a): addresses within this block are contiguous.
            {"time", 0x005a, {
                {hour, DataType::int32, Scaling::hundred, false, "Hour", "hour", 0},
            }},
            // Block tariff (base 0x006e): addresses within this block are contiguous.
            {"tariff", 0x006e, {
                {t1_import_reactive, DataType::int32, Scaling::ten, false, "Tarif 1 imported energy (Reactive)", "Kvarh", 0},
                {t2_import_reactive, DataType::int32, Scaling::ten, false, "Tarif 2 imported energy (Reactive)", "Kvarh", 0},
                {t3_import_reactive, DataType::int32, Scaling::ten, false, "Tarif 3 imported energy (Reactive)", "Kvarh", 0},
                {t4_import_reactive, DataType::int32, Scaling::ten, false, "Tarif 4 imported energy (Reactive)", "Kvarh", 0},
                {demand_power_apparent, DataType::int32, Scaling::ten, false, "Demand Power (Apparent)", "VA", 0},
                {maximum_demand_power_apparent, DataType::int32, Scaling::ten, false, "Maximum Demand Power (Apparent)", "VA", 0},
                {maximum_demand_current, DataType::int32, Scaling::thousand, false, "Maximum Demand current (Active)", "A", 0},
            }},
        });
    }
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static const Metadata metadata;
    static DeviceType makeDevice();
};
// C++17 requires this definition after the schema is complete.
inline constexpr EM24_E1::Metadata EM24_E1::metadata = EM24_E1::makeMetadata();
static_assert(EM24_E1::metadata.valid, "Invalid EM24_E1 register schema");

} // namespace modbus_gateway
