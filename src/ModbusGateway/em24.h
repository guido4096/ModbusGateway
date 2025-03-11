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
        static const DeviceDescription& getDeviceDescription();

        REGISTER_REFERENCE(l1_voltage);
        REGISTER_REFERENCE(l2_voltage);
        REGISTER_REFERENCE(l3_voltage);
        REGISTER_REFERENCE(l12_voltage);
        REGISTER_REFERENCE(l23_voltage);
        REGISTER_REFERENCE(l31_voltage);
        REGISTER_REFERENCE(l1_current);
        REGISTER_REFERENCE(l2_current);
        REGISTER_REFERENCE(l3_current);
        REGISTER_REFERENCE(l1_power_active);
        REGISTER_REFERENCE(l2_power_active);
        REGISTER_REFERENCE(l3_power_active);
        REGISTER_REFERENCE(l1_power_apparent);
        REGISTER_REFERENCE(l2_power_apparent);
        REGISTER_REFERENCE(l3_power_apparent);
        REGISTER_REFERENCE(l1_power_reactive);
        REGISTER_REFERENCE(l2_power_reactive);
        REGISTER_REFERENCE(l3_power_reactive);
        REGISTER_REFERENCE(voltage_ln);
        REGISTER_REFERENCE(voltage_ll);
        REGISTER_REFERENCE(power_active);
        REGISTER_REFERENCE(power_apparent);
        REGISTER_REFERENCE(power_reactive);
        REGISTER_REFERENCE(l1_power_factor);
        REGISTER_REFERENCE(l2_power_factor);
        REGISTER_REFERENCE(l3_power_factor);
        REGISTER_REFERENCE(total_pf);
        REGISTER_REFERENCE(phase_sequence);
        REGISTER_REFERENCE(frequency);

        REGISTER_REFERENCE(import_energy_active);
        REGISTER_REFERENCE(import_energy_reactive);
        REGISTER_REFERENCE(demand_power_active);
        REGISTER_REFERENCE(maximum_demand_power_active);
        REGISTER_REFERENCE(import_energy_active_partial);
        REGISTER_REFERENCE(import_energy_reactive_partial);
        REGISTER_REFERENCE(l1_import_energy_active);
        REGISTER_REFERENCE(l2_import_energy_active);
        REGISTER_REFERENCE(l3_import_energy_active);
        REGISTER_REFERENCE(t1_import_energy);
        REGISTER_REFERENCE(t2_import_energy);
        REGISTER_REFERENCE(t3_import_energy);
        REGISTER_REFERENCE(t4_import_energy);
        REGISTER_REFERENCE(export_energy_active);
        REGISTER_REFERENCE(export_energy_reactive);

        REGISTER_REFERENCE(hour);

        REGISTER_REFERENCE(t1_import_reactive);
        REGISTER_REFERENCE(t2_import_reactive);
        REGISTER_REFERENCE(t3_import_reactive);
        REGISTER_REFERENCE(t4_import_reactive);
        REGISTER_REFERENCE(demand_power_apparent);
        REGISTER_REFERENCE(maximum_demand_power_apparent);
        REGISTER_REFERENCE(maximum_demand_current);
    };

}
