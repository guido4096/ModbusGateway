/**
 * @file      em24.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      EM24 Modbus Definitions
 */
#include <Arduino.h>
#include <em24.h>
#include <vector>

// Protocol for EM24 register list: 
//   https://www.enika.eu/data/files/produkty/energy%20m/CP/em24%20ethernet%20cp.pdf
const modbus::DeviceDescription &modbus::EM24::getDeviceDescription()
{

    static modbus::DeviceDescription dd("em24",
        std::vector<modbus::RegisterBlock>{
            modbus::RegisterBlock::makeBlock("dynamic",
                                  std::vector<Register>{
                                      modbus::Register("l1_voltage", 0x0000, 2, modbus::Register::data_type_e::int32, "L1 Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_voltage", 0x0002, 2, modbus::Register::data_type_e::int32, "L2 Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_voltage", 0x0004, 2, modbus::Register::data_type_e::int32, "L3 Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l12_voltage", 0x0006, 2, modbus::Register::data_type_e::int32, "L1-L2 Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l23_voltage", 0x0008, 2, modbus::Register::data_type_e::int32, "L2-L3 Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l31_voltage", 0x000a, 2, modbus::Register::data_type_e::int32, "L3-L1 Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_current", 0x000c, 2, modbus::Register::data_type_e::int32, "L1 Current", "A", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_current", 0x000e, 2, modbus::Register::data_type_e::int32, "L2 Current", "A", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_current", 0x0010, 2, modbus::Register::data_type_e::int32, "L3 Current", "A", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_power_active", 0x0012, 2, modbus::Register::data_type_e::int32, "L1 Power (Active)", "W", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_power_active", 0x0014, 2, modbus::Register::data_type_e::int32, "L2 Power (Active)", "W", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_power_active", 0x0016, 2, modbus::Register::data_type_e::int32, "L3 Power (Active)", "W", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_power_apparent", 0x0018, 2, modbus::Register::data_type_e::int32, "L1 Power (Apparent)", "VA", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_power_apparent", 0x001a, 2, modbus::Register::data_type_e::int32, "L2 Power (Apparent)", "VA", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_power_apparent", 0x001c, 2, modbus::Register::data_type_e::int32, "L3 Power (Apparent)", "VA", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_power_reactive", 0x001e, 2, modbus::Register::data_type_e::int32, "L1 Power (Reactive)", "VAr", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_power_reactive", 0x0020, 2, modbus::Register::data_type_e::int32, "L2 Power (Reactive)", "VAr", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_power_reactive", 0x0022, 2, modbus::Register::data_type_e::int32, "L3 Power (Reactive)", "VAr", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("voltage_ln", 0x0024, 2, modbus::Register::data_type_e::int32, "L-N Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("voltage_ll", 0x0026, 2, modbus::Register::data_type_e::int32, "L-L Voltage", "V", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("power_active", 0x0028, 2, modbus::Register::data_type_e::int32, "Total Power (Active)", "W", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("power_apparent", 0x002a, 2, modbus::Register::data_type_e::int32, "Total Power (Apparent)", "VA", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("power_reactive", 0x002c, 2, modbus::Register::data_type_e::int32, "Total Power (Reactive)", "VAr", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_power_factor", 0x002e, 1, modbus::Register::data_type_e::int16, "L1 Power Factor", "", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_power_factor", 0x002f, 1, modbus::Register::data_type_e::int16, "L2 Power Factor", "", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_power_factor", 0x0030, 1, modbus::Register::data_type_e::int16, "L3 Power Factor", "", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("total_pf", 0x0031, 1, modbus::Register::data_type_e::int16, "Total Power Factor", "", 1000, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("phase_sequence", 0x0032, 1, modbus::Register::data_type_e::int16, "Phase Sequence", "", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("frequency", 0x0033, 1, modbus::Register::data_type_e::uint16, "Frequency", "Hz", 10, modbus::Register::value_t::_float32_t(0)),
                                  }),
            modbus::RegisterBlock::makeBlock("energy",
                                  std::vector<Register>{
                                      modbus::Register("import_energy_active", 0x0034, 2, modbus::Register::data_type_e::int32, "Imported Energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("import_energy_reactive", 0x0036, 2, modbus::Register::data_type_e::int32, "Imported Energy (Reactive)", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("demand_power_active", 0x0038, 2, modbus::Register::data_type_e::int32, "Demand Power Active", "W", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("maximum_demand_power_active", 0x003a, 2, modbus::Register::data_type_e::int32, "Maximum Demand Power Active", "W", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("import_energy_active_partial", 0x003c, 2, modbus::Register::data_type_e::int32, "Partial imported Energy (Active))", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("import_energy_reactive_partial", 0x003e, 2, modbus::Register::data_type_e::int32, "Partial imported Energy (Reactive))", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_import_energy_active", 0x0040, 2, modbus::Register::data_type_e::int32, "L1 Imported Energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_import_energy_active", 0x0042, 2, modbus::Register::data_type_e::int32, "L2 Imported Energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_import_energy_active", 0x0044, 2, modbus::Register::data_type_e::int32, "L3 Imported Energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t1_import_energy", 0x0046, 2, modbus::Register::data_type_e::int32, "Tarif 1 imported energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t2_import_energy", 0x0048, 2, modbus::Register::data_type_e::int32, "Tarif 2 imported energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t3_import_energy", 0x004a, 2, modbus::Register::data_type_e::int32, "Tarif 3 imported energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t4_import_energy", 0x004c, 2, modbus::Register::data_type_e::int32, "Tarif 4 imported energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("export_energy_active", 0x004e, 2, modbus::Register::data_type_e::int32, "Exported Energy (Active)", "kWh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("export_energy_reactive", 0x0050, 2, modbus::Register::data_type_e::int32, "Exported Energy (Reactive)", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                  }),
            modbus::RegisterBlock::makeBlock("time",
                                  std::vector<Register>{
                                      modbus::Register("hour", 0x005a, 2, modbus::Register::data_type_e::int32, "Hour", "hour", 100, modbus::Register::value_t::_float32_t(0)),
                                  }),
            modbus::RegisterBlock::makeBlock("tariff",
                                  std::vector<Register>{
                                      modbus::Register("t1_import_reactive", 0x006e, 2, modbus::Register::data_type_e::int32, "Tarif 1 imported energy (Reactive)", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t2_import_reactive", 0x0070, 2, modbus::Register::data_type_e::int32, "Tarif 2 imported energy (Reactive)", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t3_import_reactive", 0x0072, 2, modbus::Register::data_type_e::int32, "Tarif 3 imported energy (Reactive)", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("t4_import_reactive", 0x0074, 2, modbus::Register::data_type_e::int32, "Tarif 4 imported energy (Reactive)", "Kvarh", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("demand_power_apparent", 0x0076, 2, modbus::Register::data_type_e::int32, "Demand Power (Apparent)", "VA", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("maximum_demand_power_apparent", 0x0078, 2, modbus::Register::data_type_e::int32, "Maximum Demand Power (Apparent)", "VA", 10, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("maximum_demand_current", 0x007a, 2, modbus::Register::data_type_e::int32, "Maximum Demand current (Active)", "A", 1000, modbus::Register::value_t::_float32_t(0)),
                                  })});
    return dd;
}