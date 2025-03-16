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
        static const DeviceDescription<WattNode> &getDeviceDescription();

        // All defined registers. See the definition of each register in the .cpp file
        enum e_registers
        {
            dummy1,
            dummy2,
            energy_active,
            import_energy_active,
            energy_active_nr,
            import_energy_active_nr,
            power_active,
            l1_power_active,
            l2_power_active,
            l3_power_active,
            voltage_ln,
            l1n_voltage,
            l2n_voltage,
            l3n_voltage,
            voltage_ll,
            l12_voltage,
            l23_voltage,
            l31_voltage,
            frequency,

            l1_energy_active,
            l2_energy_active,
            l3_energy_active,
            l1_import_energy_active,
            l2_import_energy_active,
            l3_import_energy_active,
            export_energy_active,
            export_energy_active_nr,
            l1_export_energy_active,
            l2_export_energy_active,
            l3_export_energy_active,
            energy_reactive,
            l1_energy_reactive,
            l2_energy_reactive,
            l3_energy_reactive,
            energy_apparent,
            l1_energy_apparent,
            l2_energy_apparent,
            l3_energy_apparent,
            power_factor,
            l1_power_factor,
            l2_power_factor,
            l3_power_factor,
            power_reactive,
            l1_power_reactive,
            l2_power_reactive,
            l3_power_reactive,
            power_apparent,
            l1_power_apparent,
            l2_power_apparent,
            l3_power_apparent,
            l1_current,
            l2_current,
            l3_current,
            demand_power_active,
            minimum_demand_power_active,
            maximum_demand_power_active,
            demand_power_apparent,
            l1_demand_power_active,
            l2_demand_power_active,
            l3_demand_power_active,

            passcode,
            ct_current,
            ct_current_l1,
            ct_current_l2,
            ct_current_l3,
            ct_inverted,
            measurement_averaging,
            power_scale,
            demand_period,
            demand_subintervals,
            l1_power_energy_adj,
            l2_power_energy_adj,
            l3_power_energy_adj,
            l1_ct_phase_angle_adj,
            l2_ct_phase_angle_adj,
            l3_ct_phase_angle_adj,
            minimum_power_reading,
            phase_offset,
            reset_energy,
            reset_demand,
            current_scale,
            io_pin_mode,

            apply_config,
            modbus_address,
            baud_rate,
            parity_mode,
            modbus_mode,
            message_delay,

            serial_number,
            uptime,
            total_uptime,
            wattnode_model,
            firmware_version,
            options,
            error_status,
            power_fail_count,
            crc_error_count,
            frame_error_count,
            packet_error_count,
            overrun_count,
            error_status_1,
            error_status_2,
            error_status_3,
            error_status_4,
            error_status_5,
            error_status_6,
            error_status_7,
            error_status_8,

            unknown1,

            unknown2,
            last
        };
    };
}
