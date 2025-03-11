/**
 * @file      wattnode.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      WattNode Modbus Definitions used by SolarEdge
 */
#pragma once


#include <Arduino.h>
#include <definitions.h>
#include <vector>

namespace modbus
{

    class WattNode
    {      
        public:
        static const DeviceDescription& getDeviceDescription();

        // All define registers. See the definition of each register in the .cpp file
        REGISTER_REFERENCE(dummy1);
        REGISTER_REFERENCE(dummy2);
        REGISTER_REFERENCE(energy_active);
        REGISTER_REFERENCE(import_energy_active);
        REGISTER_REFERENCE(energy_active_nr);
        REGISTER_REFERENCE(import_energy_active_nr);
        REGISTER_REFERENCE(power_active);
        REGISTER_REFERENCE(l1_power_active);
        REGISTER_REFERENCE(l2_power_active);
        REGISTER_REFERENCE(l3_power_active);
        REGISTER_REFERENCE(voltage_ln);
        REGISTER_REFERENCE(l1n_voltage);
        REGISTER_REFERENCE(l2n_voltage);
        REGISTER_REFERENCE(l3n_voltage);
        REGISTER_REFERENCE(voltage_ll);
        REGISTER_REFERENCE(l12_voltage);
        REGISTER_REFERENCE(l23_voltage);
        REGISTER_REFERENCE(l31_voltage);
        REGISTER_REFERENCE(frequency);

        REGISTER_REFERENCE(l1_energy_active);
        REGISTER_REFERENCE(l2_energy_active);
        REGISTER_REFERENCE(l3_energy_active);
        REGISTER_REFERENCE(l1_import_energy_active);
        REGISTER_REFERENCE(l2_import_energy_active);
        REGISTER_REFERENCE(l3_import_energy_active);
        REGISTER_REFERENCE(export_energy_active);
        REGISTER_REFERENCE(export_energy_active_nr);
        REGISTER_REFERENCE(l1_export_energy_active);
        REGISTER_REFERENCE(l2_export_energy_active);
        REGISTER_REFERENCE(l3_export_energy_active);
        REGISTER_REFERENCE(energy_reactive);
        REGISTER_REFERENCE(l1_energy_reactive);
        REGISTER_REFERENCE(l2_energy_reactive);
        REGISTER_REFERENCE(l3_energy_reactive);
        REGISTER_REFERENCE(energy_apparent);
        REGISTER_REFERENCE(l1_energy_apparent);
        REGISTER_REFERENCE(l2_energy_apparent);
        REGISTER_REFERENCE(l3_energy_apparent);
        REGISTER_REFERENCE(power_factor);
        REGISTER_REFERENCE(l1_power_factor);
        REGISTER_REFERENCE(l2_power_factor);
        REGISTER_REFERENCE(l3_power_factor);
        REGISTER_REFERENCE(power_reactive);
        REGISTER_REFERENCE(l1_power_reactive);
        REGISTER_REFERENCE(l2_power_reactive);
        REGISTER_REFERENCE(l3_power_reactive);
        REGISTER_REFERENCE(power_apparent);
        REGISTER_REFERENCE(l1_power_apparent);
        REGISTER_REFERENCE(l2_power_apparent);
        REGISTER_REFERENCE(l3_power_apparent);
        REGISTER_REFERENCE(l1_current);
        REGISTER_REFERENCE(l2_current);
        REGISTER_REFERENCE(l3_current);
        REGISTER_REFERENCE(demand_power_active);
        REGISTER_REFERENCE(minimum_demand_power_active);
        REGISTER_REFERENCE(maximum_demand_power_active);
        REGISTER_REFERENCE(demand_power_apparent);
        REGISTER_REFERENCE(l1_demand_power_active);
        REGISTER_REFERENCE(l2_demand_power_active);
        REGISTER_REFERENCE(l3_demand_power_active);

        REGISTER_REFERENCE(passcode);
        REGISTER_REFERENCE(ct_current);
        REGISTER_REFERENCE(ct_current_l1);
        REGISTER_REFERENCE(ct_current_l2);
        REGISTER_REFERENCE(ct_current_l3);
        REGISTER_REFERENCE(ct_inverted);
        REGISTER_REFERENCE(measurement_averaging);
        REGISTER_REFERENCE(power_scale);
        REGISTER_REFERENCE(demand_period);
        REGISTER_REFERENCE(demand_subintervals);
        REGISTER_REFERENCE(l1_power_energy_adj);
        REGISTER_REFERENCE(l2_power_energy_adj);
        REGISTER_REFERENCE(l3_power_energy_adj);
        REGISTER_REFERENCE(l1_ct_phase_angle_adj);
        REGISTER_REFERENCE(l2_ct_phase_angle_adj);
        REGISTER_REFERENCE(l3_ct_phase_angle_adj);
        REGISTER_REFERENCE(minimum_power_reading);
        REGISTER_REFERENCE(phase_offset);
        REGISTER_REFERENCE(reset_energy);
        REGISTER_REFERENCE(reset_demand);
        REGISTER_REFERENCE(current_scale);
        REGISTER_REFERENCE(io_pin_mode);

        REGISTER_REFERENCE(apply_config);
        REGISTER_REFERENCE(modbus_address);
        REGISTER_REFERENCE(baud_rate);
        REGISTER_REFERENCE(parity_mode);
        REGISTER_REFERENCE(modbus_mode);
        REGISTER_REFERENCE(message_delay);

        REGISTER_REFERENCE(serial_number);
        REGISTER_REFERENCE(uptime);
        REGISTER_REFERENCE(total_uptime);
        REGISTER_REFERENCE(wattnode_model);
        REGISTER_REFERENCE(firmware_version);
        REGISTER_REFERENCE(options);
        REGISTER_REFERENCE(error_status);
        REGISTER_REFERENCE(power_fail_count);
        REGISTER_REFERENCE(crc_error_count);
        REGISTER_REFERENCE(frame_error_count);
        REGISTER_REFERENCE(packet_error_count);
        REGISTER_REFERENCE(overrun_count);
        REGISTER_REFERENCE(error_status_1);
        REGISTER_REFERENCE(error_status_2);
        REGISTER_REFERENCE(error_status_3);
        REGISTER_REFERENCE(error_status_4);
        REGISTER_REFERENCE(error_status_5);
        REGISTER_REFERENCE(error_status_6);
        REGISTER_REFERENCE(error_status_7);
        REGISTER_REFERENCE(error_status_8);

        REGISTER_REFERENCE(unknown1);

        REGISTER_REFERENCE(unknown2);
    };
}
