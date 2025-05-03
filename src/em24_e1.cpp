/**
 * @file      em24.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      EM24_E1 Modbus Definitions
 */
#include <Arduino.h>
#include <em24_e1.h>
#include <vector>

// Protocol for EM24_E1 register list:
//   https://www.enika.eu/data/files/produkty/energy%20m/CP/em24%20ethernet%20cp.pdf
const modbus_gateway::DeviceDescription<modbus_gateway::EM24_E1> &modbus_gateway::EM24_E1::getDeviceDescription()
{
    static DeviceDescription<EM24_E1> dd =
        DeviceDescription<EM24_E1>::makeDD("em24_e1",
                                        {{"dynamic", 0x0000, {
                                                                 {l1_voltage, DataType::int32, "L1 Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {l2_voltage, DataType::int32, "L2 Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {l3_voltage, DataType::int32, "L3 Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {l12_voltage, DataType::int32, "L1-L2 Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {l23_voltage, DataType::int32, "L2-L3 Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {l31_voltage, DataType::int32, "L3-L1 Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {l1_current, DataType::int32, "L1 Current", "A", Scaling::thousand, Value::_int32_t(0)},
                                                                 {l2_current, DataType::int32, "L2 Current", "A", Scaling::thousand, Value::_int32_t(0)},
                                                                 {l3_current, DataType::int32, "L3 Current", "A", Scaling::thousand, Value::_int32_t(0)},
                                                                 {l1_power_active, DataType::int32, "L1 Power (Active)", "W", Scaling::ten, Value::_int32_t(0)},
                                                                 {l2_power_active, DataType::int32, "L2 Power (Active)", "W", Scaling::ten, Value::_int32_t(0)},
                                                                 {l3_power_active, DataType::int32, "L3 Power (Active)", "W", Scaling::ten, Value::_int32_t(0)},
                                                                 {l1_power_apparent, DataType::int32, "L1 Power (Apparent)", "VA", Scaling::ten, Value::_int32_t(0)},
                                                                 {l2_power_apparent, DataType::int32, "L2 Power (Apparent)", "VA", Scaling::ten, Value::_int32_t(0)},
                                                                 {l3_power_apparent, DataType::int32, "L3 Power (Apparent)", "VA", Scaling::ten, Value::_int32_t(0)},
                                                                 {l1_power_reactive, DataType::int32, "L1 Power (Reactive)", "VAr", Scaling::ten, Value::_int32_t(0)},
                                                                 {l2_power_reactive, DataType::int32, "L2 Power (Reactive)", "VAr", Scaling::ten, Value::_int32_t(0)},
                                                                 {l3_power_reactive, DataType::int32, "L3 Power (Reactive)", "VAr", Scaling::ten, Value::_int32_t(0)},
                                                                 {voltage_ln, DataType::int32, "L-N Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {voltage_ll, DataType::int32, "L-L Voltage", "V", Scaling::ten, Value::_int32_t(0)},
                                                                 {power_active, DataType::int32, "Total Power (Active)", "W", Scaling::ten, Value::_int32_t(0)},
                                                                 {power_apparent, DataType::int32, "Total Power (Apparent)", "VA", Scaling::ten, Value::_int32_t(0)},
                                                                 {power_reactive, DataType::int32, "Total Power (Reactive)", "VAr", Scaling::ten, Value::_int32_t(0)},
                                                                 {l1_power_factor, DataType::int16, "L1 Power Factor", "", Scaling::thousand, Value::_int16_t(0)},
                                                                 {l2_power_factor, DataType::int16, "L2 Power Factor", "", Scaling::thousand, Value::_int16_t(0)},
                                                                 {l3_power_factor, DataType::int16, "L3 Power Factor", "", Scaling::thousand, Value::_int16_t(0)},
                                                                 {total_pf, DataType::int16, "Total Power Factor", "", Scaling::thousand, Value::_int16_t(0)},
                                                                 {phase_sequence, DataType::int16, "Phase Sequence", "", Scaling::none, Value::_int16_t(0)},
                                                                 {frequency, DataType::uint16, "Frequency", "Hz", Scaling::ten, Value::_uint16_t(0)},
                                                             }},
                                         {"energy", 0x0034, {
                                                                {import_energy_active, DataType::int32, "Imported Energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {import_energy_reactive, DataType::int32, "Imported Energy (Reactive)", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                                {demand_power_active, DataType::int32, "Demand Power Active", "W", Scaling::ten, Value::_int32_t(0)},
                                                                {maximum_demand_power_active, DataType::int32, "Maximum Demand Power Active", "W", Scaling::ten, Value::_int32_t(0)},
                                                                {import_energy_active_partial, DataType::int32, "Partial imported Energy (Active))", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {import_energy_reactive_partial, DataType::int32, "Partial imported Energy (Reactive))", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                                {l1_import_energy_active, DataType::int32, "L1 Imported Energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {l2_import_energy_active, DataType::int32, "L2 Imported Energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {l3_import_energy_active, DataType::int32, "L3 Imported Energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {t1_import_energy, DataType::int32, "Tarif 1 imported energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {t2_import_energy, DataType::int32, "Tarif 2 imported energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {t3_import_energy, DataType::int32, "Tarif 3 imported energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {t4_import_energy, DataType::int32, "Tarif 4 imported energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {export_energy_active, DataType::int32, "Exported Energy (Active)", "kWh", Scaling::ten, Value::_int32_t(0)},
                                                                {export_energy_reactive, DataType::int32, "Exported Energy (Reactive)", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                            }},
                                         {"time", 0x005a, {
                                                              {hour, DataType::int32, "Hour", "hour", Scaling::hundred, Value::_int32_t(0)},
                                                          }},
                                         {"tariff", 0x006e, {
                                                                {t1_import_reactive, DataType::int32, "Tarif 1 imported energy (Reactive)", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                                {t2_import_reactive, DataType::int32, "Tarif 2 imported energy (Reactive)", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                                {t3_import_reactive, DataType::int32, "Tarif 3 imported energy (Reactive)", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                                {t4_import_reactive, DataType::int32, "Tarif 4 imported energy (Reactive)", "Kvarh", Scaling::ten, Value::_int32_t(0)},
                                                                {demand_power_apparent, DataType::int32, "Demand Power (Apparent)", "VA", Scaling::ten, Value::_int32_t(0)},
                                                                {maximum_demand_power_apparent, DataType::int32, "Maximum Demand Power (Apparent)", "VA", Scaling::ten, Value::_int32_t(0)},
                                                                {maximum_demand_current, DataType::int32, "Maximum Demand current (Active)", "A", Scaling::thousand, Value::_int32_t(0)},
                                                            }}});
    return dd;
}