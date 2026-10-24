/**
 * @file      wattnode.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      WattNode Modbus Definitions used by SolarEdge
 */
#pragma once
#include "definitions.h"

namespace modbus_gateway {

struct WattNode : SchemaApi<WattNode> {
    // Device identity used by diagnostics; no runtime name argument is needed.
    static constexpr const char* name = "wattnode";
        // Logical register IDs, in index order. These are NOT wire addresses.
    // IDs index metadata.registers[] directly (O(1)); keep names stable.
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
    // ---- Compile-time schema dimensions ----
    using RegisterId = e_registers;
    static constexpr std::size_t register_count = static_cast<std::size_t>(last);
    static constexpr std::size_t block_count = 8;
    static constexpr std::size_t word_count = 173;
    using Metadata = StaticDescription<RegisterId, register_count, block_count, word_count>;
    // ---- Modbus register map ----
    // Each block starts at its wire address. Subsequent register addresses
    // are derived from preceding widths (1 word for 16-bit, 2 for 32-bit).
    // Field columns: ID, wire type, scale divisor, high-word-first,
    // description, engineering unit, default raw bits.
    // Keep the register order unchanged: it defines the wire layout.
    static constexpr Metadata makeMetadata() {
        return buildMetadata<RegisterId, register_count, block_count, word_count>({
            // Block block0000 (base 0): addresses within this block are contiguous.
            {"block0000", 0, {
                {dummy1, DataType::int16, Scaling::none, true, "Dummy 1 always returns 0", "", 0},
                {dummy2, DataType::int16, Scaling::none, true, "Dummy 2 always returns 0", "", 0},
            }},
            // Block block1000 (base 1000): addresses within this block are contiguous.
            {"block1000", 1000, {
                {energy_active, DataType::float32, Scaling::none, false, "Total Energy (Active)", "kWh", 0},
                {import_energy_active, DataType::float32, Scaling::none, false, "Imported Total Energy (Active)", "kWh", 0},
                {energy_active_nr, DataType::float32, Scaling::none, false, "Total Energy NR (Active)", "kWh", 0},
                {import_energy_active_nr, DataType::float32, Scaling::none, false, "Imported Total Energy NR (Active)", "kWh", 0},
                {power_active, DataType::float32, Scaling::none, false, "Total Power (Active)", "W", 0},
                {l1_power_active, DataType::float32, Scaling::none, false, "L1 Power (Active)", "W", 0},
                {l2_power_active, DataType::float32, Scaling::none, false, "L2 Power (Active)", "W", 0},
                {l3_power_active, DataType::float32, Scaling::none, false, "L3 Power (Active)", "W", 0},
                {voltage_ln, DataType::float32, Scaling::none, false, "Voltage L-N", "V", 0},
                {l1n_voltage, DataType::float32, Scaling::none, false, "Voltage L1-N", "V", 0},
                {l2n_voltage, DataType::float32, Scaling::none, false, "Voltage L2-N", "V", 0},
                {l3n_voltage, DataType::float32, Scaling::none, false, "Voltage L3-N", "V", 0},
                {voltage_ll, DataType::float32, Scaling::none, false, "Voltage LL", "V", 0},
                {l12_voltage, DataType::float32, Scaling::none, false, "Voltage L1-L2", "V", 0},
                {l23_voltage, DataType::float32, Scaling::none, false, "Voltage L2-L3", "V", 0},
                {l31_voltage, DataType::float32, Scaling::none, false, "Voltage L3-L1", "V", 0},
                {frequency, DataType::float32, Scaling::none, false, "Frequency", "", 0},
            }},
            // Block block1100 (base 1100): addresses within this block are contiguous.
            {"block1100", 1100, {
                {l1_energy_active, DataType::float32, Scaling::none, false, "L1 Energy (Active)", "kWh", 0},
                {l2_energy_active, DataType::float32, Scaling::none, false, "L2 Energy (Active)", "kWh", 0},
                {l3_energy_active, DataType::float32, Scaling::none, false, "L3 Energy (Active)", "kWh", 0},
                {l1_import_energy_active, DataType::float32, Scaling::none, false, "L1 Imported Energy (Active)", "kWh", 0},
                {l2_import_energy_active, DataType::float32, Scaling::none, false, "L2 Imported Energy (Active)", "kWh", 0},
                {l3_import_energy_active, DataType::float32, Scaling::none, false, "L3 Imported Energy (Active)", "kWh", 0},
                {export_energy_active, DataType::float32, Scaling::none, false, "Exported Energy (Active)", "kWh", 0},
                {export_energy_active_nr, DataType::float32, Scaling::none, false, "Exported Energy NR (Active)", "kWh", 0},
                {l1_export_energy_active, DataType::float32, Scaling::none, false, "L1 Exported Energy (Active)", "kWh", 0},
                {l2_export_energy_active, DataType::float32, Scaling::none, false, "L2 Exported Energy (Active)", "kWh", 0},
                {l3_export_energy_active, DataType::float32, Scaling::none, false, "L3 Exported Energy (Active)", "kWh", 0},
                {energy_reactive, DataType::float32, Scaling::none, false, "Energy (Reactive)", "kWh", 0},
                {l1_energy_reactive, DataType::float32, Scaling::none, false, "L1 Energy (Reactive)", "kWh", 0},
                {l2_energy_reactive, DataType::float32, Scaling::none, false, "L2 Energy (Reactive)", "kWh", 0},
                {l3_energy_reactive, DataType::float32, Scaling::none, false, "L3 Energy (Reactive)", "kWh", 0},
                {energy_apparent, DataType::float32, Scaling::none, false, "Energy (Apparent)", "kWh", 0},
                {l1_energy_apparent, DataType::float32, Scaling::none, false, "L1 Energy (Apparent)", "kWh", 0},
                {l2_energy_apparent, DataType::float32, Scaling::none, false, "L2 Energy (Apparent)", "kWh", 0},
                {l3_energy_apparent, DataType::float32, Scaling::none, false, "L3 Energy (Apparent)", "kWh", 0},
                {power_factor, DataType::float32, Scaling::none, false, "Power Factor", "", 0},
                {l1_power_factor, DataType::float32, Scaling::none, false, "L1 Power Factor", "", 0},
                {l2_power_factor, DataType::float32, Scaling::none, false, "L2 Power Factor", "", 0},
                {l3_power_factor, DataType::float32, Scaling::none, false, "L3 Power Factor", "", 0},
                {power_reactive, DataType::float32, Scaling::none, false, "Power (Reactive)", "VAr", 0},
                {l1_power_reactive, DataType::float32, Scaling::none, false, "L1 Power (Reactive)", "VAr", 0},
                {l2_power_reactive, DataType::float32, Scaling::none, false, "L2 Power (Reactive)", "VAr", 0},
                {l3_power_reactive, DataType::float32, Scaling::none, false, "L3 Power (Reactive)", "VAr", 0},
                {power_apparent, DataType::float32, Scaling::none, false, "Power (Apparent)", "VA", 0},
                {l1_power_apparent, DataType::float32, Scaling::none, false, "L1 Power (Apparent)", "VA", 0},
                {l2_power_apparent, DataType::float32, Scaling::none, false, "L2 Power (Apparent)", "VA", 0},
                {l3_power_apparent, DataType::float32, Scaling::none, false, "L3 Power (Apparent)", "VA", 0},
                {l1_current, DataType::float32, Scaling::none, false, "L1 Current", "A", 0},
                {l2_current, DataType::float32, Scaling::none, false, "L2 Current", "A", 0},
                {l3_current, DataType::float32, Scaling::none, false, "L3 Current", "A", 0},
                {demand_power_active, DataType::float32, Scaling::none, false, "Demand Power (Active)", "W", 0},
                {minimum_demand_power_active, DataType::float32, Scaling::none, false, "Minimum Demand Power (Active)", "W", 0},
                {maximum_demand_power_active, DataType::float32, Scaling::none, false, "Maximum Demand Power (Active)", "W", 0},
                {demand_power_apparent, DataType::float32, Scaling::none, false, "Demand Power (Apparent)", "VA", 0},
                {l1_demand_power_active, DataType::float32, Scaling::none, false, "L1 Demand Power (Active)", "W", 0},
                {l2_demand_power_active, DataType::float32, Scaling::none, false, "L2 Demand Power (Active)", "W", 0},
                {l3_demand_power_active, DataType::float32, Scaling::none, false, "L3 Demand Power (Active)", "W", 0},
            }},
            // Block block1600 (base 1600): addresses within this block are contiguous.
            {"block1600", 1600, {
                {passcode, DataType::uint32, Scaling::none, false, "Passcode", "", 1234},
                {ct_current, DataType::int16, Scaling::none, true, "CT Current", "A", 5},
                {ct_current_l1, DataType::int16, Scaling::none, true, "L1 CT Current", "A", 5},
                {ct_current_l2, DataType::int16, Scaling::none, true, "L2 CT Current", "A", 5},
                {ct_current_l3, DataType::int16, Scaling::none, true, "L3 CT Current", "A", 5},
                {ct_inverted, DataType::int16, Scaling::none, true, "CT Inverted", "", 0},
                {measurement_averaging, DataType::int16, Scaling::none, true, "Measurement Averaging", "", 0},
                {power_scale, DataType::int16, Scaling::none, true, "Power Scale", "", 0},
                {demand_period, DataType::int16, Scaling::none, true, "Demand Period", "Minute", 15},
                {demand_subintervals, DataType::int16, Scaling::none, true, "Demand Subintervals", "", 0},
                {l1_power_energy_adj, DataType::int16, Scaling::none, true, "L1 Power/Energy adjustment", "", 10000},
                {l2_power_energy_adj, DataType::int16, Scaling::none, true, "L2 Power/Energy adjustment", "", 10000},
                {l3_power_energy_adj, DataType::int16, Scaling::none, true, "L3 Power/Energy adjustment", "", 10000},
                {l1_ct_phase_angle_adj, DataType::int16, Scaling::none, true, "L1 CT Phase Angle adjustment", "", static_cast<uint32_t>(-1000)},
                {l2_ct_phase_angle_adj, DataType::int16, Scaling::none, true, "L2 CT Phase Angle adjustment", "", static_cast<uint32_t>(-1000)},
                {l3_ct_phase_angle_adj, DataType::int16, Scaling::none, true, "L3 CT Phase Angle adjustment", "", static_cast<uint32_t>(-1000)},
                {minimum_power_reading, DataType::int16, Scaling::none, true, "Minimum Power Reading", "", 0},
                {phase_offset, DataType::int16, Scaling::none, true, "Phase Offset", "", 120},
                {reset_energy, DataType::int16, Scaling::none, true, "Reset Energy", "", 0},
                {reset_demand, DataType::int16, Scaling::none, true, "Reset Demand", "", 0},
                {current_scale, DataType::int16, Scaling::none, true, "Current Scale", "", 20000},
                {io_pin_mode, DataType::int16, Scaling::none, true, "IO Pin Mode", "", 0},
            }},
            // Block block1650 (base 1650): addresses within this block are contiguous.
            {"block1650", 1650, {
                {apply_config, DataType::int16, Scaling::none, true, "Apply Config", "", 0},
                {modbus_address, DataType::int16, Scaling::none, false, "Modbus Address", "", 0},
                {baud_rate, DataType::int16, Scaling::none, true, "Baud Rate", "", 0},
                {parity_mode, DataType::int16, Scaling::none, true, "Parity Mode", "", 0},
                {modbus_mode, DataType::int16, Scaling::none, true, "Modbus Mode", "", 0},
                {message_delay, DataType::int16, Scaling::ten, true, "Message Delay", "ms", 0},
            }},
            // Block block1700 (base 1700): addresses within this block are contiguous.
            {"block1700", 1700, {
                {serial_number, DataType::uint32, Scaling::none, false, "Serial Number", "", 0},
                {uptime, DataType::uint32, Scaling::none, false, "Uptime", "s", 0},
                {total_uptime, DataType::uint32, Scaling::none, false, "Total Uptime", "s", 0},
                {wattnode_model, DataType::int16, Scaling::none, true, "Wattnode Model", "", 202},
                {firmware_version, DataType::int16, Scaling::none, true, "Firmware Version", "", 31},
                {options, DataType::int16, Scaling::none, true, "Options", "", 0},
                {error_status, DataType::int16, Scaling::none, true, "Error Status", "", 0},
                {power_fail_count, DataType::int16, Scaling::none, true, "Power Fail Count", "", 0},
                {crc_error_count, DataType::int16, Scaling::none, true, "CRC Error Count", "", 0},
                {frame_error_count, DataType::int16, Scaling::none, true, "Frame Error Count", "", 0},
                {packet_error_count, DataType::int16, Scaling::none, true, "Packet Error Count", "", 0},
                {overrun_count, DataType::int16, Scaling::none, true, "Overrun Count", "", 0},
                {error_status_1, DataType::int16, Scaling::none, true, "Error Status 1", "", 0},
                {error_status_2, DataType::int16, Scaling::none, true, "Error Status 2", "", 0},
                {error_status_3, DataType::int16, Scaling::none, true, "Error Status 3", "", 0},
                {error_status_4, DataType::int16, Scaling::none, true, "Error Status 4", "", 0},
                {error_status_5, DataType::int16, Scaling::none, true, "Error Status 5", "", 0},
                {error_status_6, DataType::int16, Scaling::none, true, "Error Status 6", "", 0},
                {error_status_7, DataType::int16, Scaling::none, true, "Error Status 7", "", 0},
                {error_status_8, DataType::int16, Scaling::none, true, "Error Status 8", "", 0},
            }},
            // Block block1736 (base 1736): addresses within this block are contiguous.
            {"block1736", 1736, {
                {unknown1, DataType::uint32, Scaling::none, false, "Unknown 1", "", 0},
            }},
            // Block block2127 (base 2127): addresses within this block are contiguous.
            {"block2127", 2127, {
                {unknown2, DataType::uint16, Scaling::none, true, "Unknown 2", "", 1},
            }},
        });
    }
    // Defined out of class because C++17 cannot initialize a constexpr static
    // member using an in-class constexpr member function before class completion.
    // ---- Device factory and indexed metadata access ----
    // SchemaApi provides DeviceType and O(1) description<R>().
    static const Metadata metadata;
    // This register mirrors the protocol server ID assigned by Server<Schema>.
    static constexpr RegisterId server_id_register = modbus_address;
    static DeviceType makeDevice(uint32_t serialNumber);
};

// Inline constexpr definition makes metadata available in every translation unit.
// C++17 requires this definition after the schema is complete.
inline constexpr WattNode::Metadata WattNode::metadata = WattNode::makeMetadata();
static_assert(WattNode::metadata.valid, "Invalid WattNode register schema");

static_assert(WattNode::description<WattNode::power_active>().address == 1008,
              "Unexpected WattNode power register address");

} // namespace modbus_gateway
