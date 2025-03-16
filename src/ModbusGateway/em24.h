/**
 * @file      em24.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      EM24 Modbus Definitions
 */
#pragma once

#include <Arduino.h>
#include <definitions.h>
#include <vector>
namespace modbus
{
    class EM24
    {
    public:
        static const DeviceDescription<EM24> &getDeviceDescription();

        // All defines registers. See the definition of each register in the .cpp file
        enum e_registers
        {
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
    };

}
