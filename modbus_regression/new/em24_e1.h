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
    // Register layout is the single source of truth. Blocks and total word
    // storage are counted automatically. All field properties are explicit.
    using RegisterId = e_registers;
    inline static constexpr auto metadata = makeDeviceSchema<RegisterId>(
            block("dynamic", 0x0000,
                field<DataType::int32>(l1_voltage, Scaling::ten, WordOrder::LowFirst, "L1 Voltage", "V", 0),
                field<DataType::int32>(l2_voltage, Scaling::ten, WordOrder::LowFirst, "L2 Voltage", "V", 0),
                field<DataType::int32>(l3_voltage, Scaling::ten, WordOrder::LowFirst, "L3 Voltage", "V", 0),
                field<DataType::int32>(l12_voltage, Scaling::ten, WordOrder::LowFirst, "L1-L2 Voltage", "V", 0),
                field<DataType::int32>(l23_voltage, Scaling::ten, WordOrder::LowFirst, "L2-L3 Voltage", "V", 0),
                field<DataType::int32>(l31_voltage, Scaling::ten, WordOrder::LowFirst, "L3-L1 Voltage", "V", 0),
                field<DataType::int32>(l1_current, Scaling::thousand, WordOrder::LowFirst, "L1 Current", "A", 0),
                field<DataType::int32>(l2_current, Scaling::thousand, WordOrder::LowFirst, "L2 Current", "A", 0),
                field<DataType::int32>(l3_current, Scaling::thousand, WordOrder::LowFirst, "L3 Current", "A", 0),
                field<DataType::int32>(l1_power_active, Scaling::ten, WordOrder::LowFirst, "L1 Power (Active)", "W", 0),
                field<DataType::int32>(l2_power_active, Scaling::ten, WordOrder::LowFirst, "L2 Power (Active)", "W", 0),
                field<DataType::int32>(l3_power_active, Scaling::ten, WordOrder::LowFirst, "L3 Power (Active)", "W", 0),
                field<DataType::int32>(l1_power_apparent, Scaling::ten, WordOrder::LowFirst, "L1 Power (Apparent)", "VA", 0),
                field<DataType::int32>(l2_power_apparent, Scaling::ten, WordOrder::LowFirst, "L2 Power (Apparent)", "VA", 0),
                field<DataType::int32>(l3_power_apparent, Scaling::ten, WordOrder::LowFirst, "L3 Power (Apparent)", "VA", 0),
                field<DataType::int32>(l1_power_reactive, Scaling::ten, WordOrder::LowFirst, "L1 Power (Reactive)", "VAr", 0),
                field<DataType::int32>(l2_power_reactive, Scaling::ten, WordOrder::LowFirst, "L2 Power (Reactive)", "VAr", 0),
                field<DataType::int32>(l3_power_reactive, Scaling::ten, WordOrder::LowFirst, "L3 Power (Reactive)", "VAr", 0),
                field<DataType::int32>(voltage_ln, Scaling::ten, WordOrder::LowFirst, "L-N Voltage", "V", 0),
                field<DataType::int32>(voltage_ll, Scaling::ten, WordOrder::LowFirst, "L-L Voltage", "V", 0),
                field<DataType::int32>(power_active, Scaling::ten, WordOrder::LowFirst, "Total Power (Active)", "W", 0),
                field<DataType::int32>(power_apparent, Scaling::ten, WordOrder::LowFirst, "Total Power (Apparent)", "VA", 0),
                field<DataType::int32>(power_reactive, Scaling::ten, WordOrder::LowFirst, "Total Power (Reactive)", "VAr", 0),
                field<DataType::int16>(l1_power_factor, Scaling::thousand, WordOrder::HighFirst, "L1 Power Factor", "", 0),
                field<DataType::int16>(l2_power_factor, Scaling::thousand, WordOrder::HighFirst, "L2 Power Factor", "", 0),
                field<DataType::int16>(l3_power_factor, Scaling::thousand, WordOrder::HighFirst, "L3 Power Factor", "", 0),
                field<DataType::int16>(total_pf, Scaling::thousand, WordOrder::HighFirst, "Total Power Factor", "", 0),
                field<DataType::int16>(phase_sequence, Scaling::none, WordOrder::HighFirst, "Phase Sequence", "", 0),
                field<DataType::uint16>(frequency, Scaling::ten, WordOrder::HighFirst, "Frequency", "Hz", 0)
            ),
            block("energy", 0x0034,
                field<DataType::int32>(import_energy_active, Scaling::ten, WordOrder::LowFirst, "Imported Energy (Active)", "kWh", 0),
                field<DataType::int32>(import_energy_reactive, Scaling::ten, WordOrder::LowFirst, "Imported Energy (Reactive)", "Kvarh", 0),
                field<DataType::int32>(demand_power_active, Scaling::ten, WordOrder::LowFirst, "Demand Power Active", "W", 0),
                field<DataType::int32>(maximum_demand_power_active, Scaling::ten, WordOrder::LowFirst, "Maximum Demand Power Active", "W", 0),
                field<DataType::int32>(import_energy_active_partial, Scaling::ten, WordOrder::LowFirst, "Partial imported Energy (Active))", "kWh", 0),
                field<DataType::int32>(import_energy_reactive_partial, Scaling::ten, WordOrder::LowFirst, "Partial imported Energy (Reactive))", "Kvarh", 0),
                field<DataType::int32>(l1_import_energy_active, Scaling::ten, WordOrder::LowFirst, "L1 Imported Energy (Active)", "kWh", 0),
                field<DataType::int32>(l2_import_energy_active, Scaling::ten, WordOrder::LowFirst, "L2 Imported Energy (Active)", "kWh", 0),
                field<DataType::int32>(l3_import_energy_active, Scaling::ten, WordOrder::LowFirst, "L3 Imported Energy (Active)", "kWh", 0),
                field<DataType::int32>(t1_import_energy, Scaling::ten, WordOrder::LowFirst, "Tarif 1 imported energy (Active)", "kWh", 0),
                field<DataType::int32>(t2_import_energy, Scaling::ten, WordOrder::LowFirst, "Tarif 2 imported energy (Active)", "kWh", 0),
                field<DataType::int32>(t3_import_energy, Scaling::ten, WordOrder::LowFirst, "Tarif 3 imported energy (Active)", "kWh", 0),
                field<DataType::int32>(t4_import_energy, Scaling::ten, WordOrder::LowFirst, "Tarif 4 imported energy (Active)", "kWh", 0),
                field<DataType::int32>(export_energy_active, Scaling::ten, WordOrder::LowFirst, "Exported Energy (Active)", "kWh", 0),
                field<DataType::int32>(export_energy_reactive, Scaling::ten, WordOrder::LowFirst, "Exported Energy (Reactive)", "Kvarh", 0)
            ),
            block("time", 0x005a,
                field<DataType::int32>(hour, Scaling::hundred, WordOrder::LowFirst, "Hour", "hour", 0)
            ),
            block("tariff", 0x006e,
                field<DataType::int32>(t1_import_reactive, Scaling::ten, WordOrder::LowFirst, "Tarif 1 imported energy (Reactive)", "Kvarh", 0),
                field<DataType::int32>(t2_import_reactive, Scaling::ten, WordOrder::LowFirst, "Tarif 2 imported energy (Reactive)", "Kvarh", 0),
                field<DataType::int32>(t3_import_reactive, Scaling::ten, WordOrder::LowFirst, "Tarif 3 imported energy (Reactive)", "Kvarh", 0),
                field<DataType::int32>(t4_import_reactive, Scaling::ten, WordOrder::LowFirst, "Tarif 4 imported energy (Reactive)", "Kvarh", 0),
                field<DataType::int32>(demand_power_apparent, Scaling::ten, WordOrder::LowFirst, "Demand Power (Apparent)", "VA", 0),
                field<DataType::int32>(maximum_demand_power_apparent, Scaling::ten, WordOrder::LowFirst, "Maximum Demand Power (Apparent)", "VA", 0),
                field<DataType::int32>(maximum_demand_current, Scaling::thousand, WordOrder::LowFirst, "Maximum Demand current (Active)", "A", 0)
            )
        );
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static DeviceType makeDevice();
};
static_assert(EM24_E1::metadata.valid, "Invalid EM24_E1 register schema");

} // namespace modbus_gateway
