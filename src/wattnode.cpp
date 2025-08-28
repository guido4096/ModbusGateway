/**
 * @file      wattnode.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      WattNode Modbus Definitions used by SolarEdge
 */
#include <Arduino.h>
#include <wattnode.h>
#include <vector>

/*
    Example of the registers SolarEdge is querying for.
    Operation, Start register, Number of registers
    3 1010 6
    3 1600 23
    3 1010 6
    3 1700 23
    3 1010 6
    3 1736 2
    3 1010 6
    3 1600 23
    3 1010 6
    3 1650 6
    3 1010 6
    3 1010 6
    3 1010 6
    3 1700 23
    3 1010 6
    3 1000 34
    3 1010 6
    3 1 1
*/

// Protocol for WattNode register list:
//   https://ctlsys.com/wp-content/uploads/2016/10/WNC-Modbus-Register-List-V18.xls
//   https://ctlsys.com/wp-content/uploads/2016/10/WNC-Modbus-Manual-V18c.pdf
const modbus_gateway::DeviceDescription<modbus_gateway::WattNode> &modbus_gateway::WattNode::getDeviceDescription(uint16_t rtuServerId, uint32_t serialNumber)
{
    static DeviceDescription<WattNode> dd =
        DeviceDescription<WattNode>::makeDD("wattnode",
                                            {{"block0000", 0, {
                                                                  {dummy1, DataType::int16, "Dummy 1 always returns 0", "", Scaling::none, Value::_int16_t(0), true}, // 0
                                                                  {dummy2, DataType::int16, "Dummy 2 always returns 0", "", Scaling::none, Value::_int16_t(0), true}  // 0
                                                              }},
                                             {"block1000", 1000, {{energy_active, DataType::float32, "Total Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true}, // 0
                                                                  {import_energy_active, DataType::float32, "Imported Total Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},
                                                                  {energy_active_nr, DataType::float32, "Total Energy NR (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},
                                                                  {import_energy_active_nr, DataType::float32, "Imported Total Energy NR (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},
                                                                  {power_active, DataType::float32, "Total Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},
                                                                  {l1_power_active, DataType::float32, "L1 Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},
                                                                  {l2_power_active, DataType::float32, "L2 Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},
                                                                  {l3_power_active, DataType::float32, "L3 Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},
                                                                  {voltage_ln, DataType::float32, "Voltage L-N", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {l1n_voltage, DataType::float32, "Voltage L1-N", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {l2n_voltage, DataType::float32, "Voltage L2-N", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {l3n_voltage, DataType::float32, "Voltage L3-N", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {voltage_ll, DataType::float32, "Voltage LL", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {l12_voltage, DataType::float32, "Voltage L1-L2", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {l23_voltage, DataType::float32, "Voltage L2-L3", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {l31_voltage, DataType::float32, "Voltage L3-L1", "V", Scaling::none, Value::_float32_t(0), true},
                                                                  {frequency, DataType::float32, "Frequency", "", Scaling::none, Value::_float32_t(0), true}}},
                                             {"block1100", 1100, {
                                                                     {l1_energy_active, DataType::float32, "L1 Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},                     // total active energy l1
                                                                     {l2_energy_active, DataType::float32, "L2 Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},                     // total active energy l2
                                                                     {l3_energy_active, DataType::float32, "L3 Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},                     // total active energy l3
                                                                     {l1_import_energy_active, DataType::float32, "L1 Imported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  imported active energy l1
                                                                     {l2_import_energy_active, DataType::float32, "L2 Imported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  imported active energy l2
                                                                     {l3_import_energy_active, DataType::float32, "L3 Imported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  imported active energy l3
                                                                     {export_energy_active, DataType::float32, "Exported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},           //  total exported active energy
                                                                     {export_energy_active_nr, DataType::float32, "Exported Energy NR (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  total exported active energy non-reset
                                                                     {l1_export_energy_active, DataType::float32, "L1 Exported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  exported energy l1
                                                                     {l2_export_energy_active, DataType::float32, "L2 Exported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  exported energy l2
                                                                     {l3_export_energy_active, DataType::float32, "L3 Exported Energy (Active)", "kWh", Scaling::none, Value::_float32_t(0), true},     //  exported energy l3
                                                                     {energy_reactive, DataType::float32, "Energy (Reactive)", "kWh", Scaling::none, Value::_float32_t(0), true},                       // total reactive energy
                                                                     {l1_energy_reactive, DataType::float32, "L1 Energy (Reactive)", "kWh", Scaling::none, Value::_float32_t(0), true},                 //  reactive energy l1
                                                                     {l2_energy_reactive, DataType::float32, "L2 Energy (Reactive)", "kWh", Scaling::none, Value::_float32_t(0), true},                 //  reactive energy l2
                                                                     {l3_energy_reactive, DataType::float32, "L3 Energy (Reactive)", "kWh", Scaling::none, Value::_float32_t(0), true},                 //  reactive energy l3
                                                                     {energy_apparent, DataType::float32, "Energy (Apparent)", "kWh", Scaling::none, Value::_float32_t(0), true},                       //  total apparent energy
                                                                     {l1_energy_apparent, DataType::float32, "L1 Energy (Apparent)", "kWh", Scaling::none, Value::_float32_t(0), true},                 //  apparent energy l1
                                                                     {l2_energy_apparent, DataType::float32, "L2 Energy (Apparent)", "kWh", Scaling::none, Value::_float32_t(0), true},                 //  apparent energy l2
                                                                     {l3_energy_apparent, DataType::float32, "L3 Energy (Apparent)", "kWh", Scaling::none, Value::_float32_t(0), true},                 //  apparent energy l3
                                                                     {power_factor, DataType::float32, "Power Factor", "", Scaling::none, Value::_float32_t(0), true},                                  //  power factor
                                                                     {l1_power_factor, DataType::float32, "L1 Power Factor", "", Scaling::none, Value::_float32_t(0), true},                            //  power factor l1
                                                                     {l2_power_factor, DataType::float32, "L2 Power Factor", "", Scaling::none, Value::_float32_t(0), true},                            //  power factor l2
                                                                     {l3_power_factor, DataType::float32, "L3 Power Factor", "", Scaling::none, Value::_float32_t(0), true},                            //  power factor l3
                                                                     {power_reactive, DataType::float32, "Power (Reactive)", "VAr", Scaling::none, Value::_float32_t(0), true},                         //  total reactive power
                                                                     {l1_power_reactive, DataType::float32, "L1 Power (Reactive)", "VAr", Scaling::none, Value::_float32_t(0), true},                   //  reactive power l1
                                                                     {l2_power_reactive, DataType::float32, "L2 Power (Reactive)", "VAr", Scaling::none, Value::_float32_t(0), true},                   //  reactive power l2
                                                                     {l3_power_reactive, DataType::float32, "L3 Power (Reactive)", "VAr", Scaling::none, Value::_float32_t(0), true},                   //  reactive power l3
                                                                     {power_apparent, DataType::float32, "Power (Apparent)", "VA", Scaling::none, Value::_float32_t(0), true},                          //  total apparent power
                                                                     {l1_power_apparent, DataType::float32, "L1 Power (Apparent)", "VA", Scaling::none, Value::_float32_t(0), true},                    //  apparent power l1
                                                                     {l2_power_apparent, DataType::float32, "L2 Power (Apparent)", "VA", Scaling::none, Value::_float32_t(0), true},                    //  apparent power l2
                                                                     {l3_power_apparent, DataType::float32, "L3 Power (Apparent)", "VA", Scaling::none, Value::_float32_t(0), true},                    //  apparent power l3
                                                                     {l1_current, DataType::float32, "L1 Current", "A", Scaling::none, Value::_float32_t(0), true},                                     // current l1
                                                                     {l2_current, DataType::float32, "L2 Current", "A", Scaling::none, Value::_float32_t(0), true},                                     //  current l2
                                                                     {l3_current, DataType::float32, "L3 Current", "A", Scaling::none, Value::_float32_t(0), true},                                     //  current l3
                                                                     {demand_power_active, DataType::float32, "Demand Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},                 //  demand power
                                                                     {minimum_demand_power_active, DataType::float32, "Minimum Demand Power (Active)", "W", Scaling::none, Value::_float32_t(0), true}, //  minimum demand power
                                                                     {maximum_demand_power_active, DataType::float32, "Maximum Demand Power (Active)", "W", Scaling::none, Value::_float32_t(0), true}, //  maximum demand power
                                                                     {demand_power_apparent, DataType::float32, "Demand Power (Apparent)", "VA", Scaling::none, Value::_float32_t(0), true},            //  apparent demand power
                                                                     {l1_demand_power_active, DataType::float32, "L1 Demand Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},           // demand power l1
                                                                     {l2_demand_power_active, DataType::float32, "L2 Demand Power (Active)", "W", Scaling::none, Value::_float32_t(0), true},           //  demand power l2
                                                                     {l3_demand_power_active, DataType::float32, "L3 Demand Power (Active)", "W", Scaling::none, Value::_float32_t(0), true}            //  demand power l3
                                                                 }},
                                             {"block1600", 1600, {
                                                                     {passcode, DataType::uint32, "Passcode", "", Scaling::none, Value::_uint32_t(1234), true},                                 // 1234
                                                                     {ct_current, DataType::int16, "CT Current", "A", Scaling::none, Value::_int16_t(5), true},                                 // 5
                                                                     {ct_current_l1, DataType::int16, "L1 CT Current", "A", Scaling::none, Value::_int16_t(5), true},                           // 5
                                                                     {ct_current_l2, DataType::int16, "L2 CT Current", "A", Scaling::none, Value::_int16_t(5), true},                           // 5
                                                                     {ct_current_l3, DataType::int16, "L3 CT Current", "A", Scaling::none, Value::_int16_t(5), true},                           // 5
                                                                     {ct_inverted, DataType::int16, "CT Inverted", "", Scaling::none, Value::_int16_t(0), true},                                // 0
                                                                     {measurement_averaging, DataType::int16, "Measurement Averaging", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {power_scale, DataType::int16, "Power Scale", "", Scaling::none, Value::_int16_t(0), true},                                // 0
                                                                     {demand_period, DataType::int16, "Demand Period", "Minute", Scaling::none, Value::_int16_t(15), true},                     // 15
                                                                     {demand_subintervals, DataType::int16, "Demand Subintervals", "", Scaling::none, Value::_int16_t(0), true},                // 1
                                                                     {l1_power_energy_adj, DataType::int16, "L1 Power/Energy adjustment", "", Scaling::none, Value::_int16_t(10000), true},     // 10000
                                                                     {l2_power_energy_adj, DataType::int16, "L2 Power/Energy adjustment", "", Scaling::none, Value::_int16_t(10000), true},     // 10000
                                                                     {l3_power_energy_adj, DataType::int16, "L3 Power/Energy adjustment", "", Scaling::none, Value::_int16_t(10000), true},     // 10000
                                                                     {l1_ct_phase_angle_adj, DataType::int16, "L1 CT Phase Angle adjustment", "", Scaling::none, Value::_int16_t(-1000), true}, // -1000
                                                                     {l2_ct_phase_angle_adj, DataType::int16, "L2 CT Phase Angle adjustment", "", Scaling::none, Value::_int16_t(-1000), true}, // -1000
                                                                     {l3_ct_phase_angle_adj, DataType::int16, "L3 CT Phase Angle adjustment", "", Scaling::none, Value::_int16_t(-1000), true}, // -1000
                                                                     {minimum_power_reading, DataType::int16, "Minimum Power Reading", "", Scaling::none, Value::_int16_t(0), true},            // 1500
                                                                     {phase_offset, DataType::int16, "Phase Offset", "", Scaling::none, Value::_int16_t(120), true},                            // 120
                                                                     {reset_energy, DataType::int16, "Reset Energy", "", Scaling::none, Value::_int16_t(0), true},                              // 0
                                                                     {reset_demand, DataType::int16, "Reset Demand", "", Scaling::none, Value::_int16_t(0), true},                              // 0
                                                                     {current_scale, DataType::int16, "Current Scale", "", Scaling::none, Value::_int16_t(20000), true},                        // 20000
                                                                     {io_pin_mode, DataType::int16, "IO Pin Mode", "", Scaling::none, Value::_int16_t(0), true}                                 // 0
                                                                 }},
                                             {"block1650", 1650, {
                                                                     {apply_config, DataType::int16, "Apply Config", "", Scaling::none, Value::_int16_t(0), true},         // 0
                                                                     {modbus_address, DataType::int16, "Modbus Address", "", Scaling::none, Value::_int16_t(rtuServerId)}, // modbus address
                                                                     {baud_rate, DataType::int16, "Baud Rate", "", Scaling::none, Value::_int16_t(0), true},               // 4
                                                                     {parity_mode, DataType::int16, "Parity Mode", "", Scaling::none, Value::_int16_t(0), true},           // 0
                                                                     {modbus_mode, DataType::int16, "Modbus Mode", "", Scaling::none, Value::_int16_t(0), true},           // 0
                                                                     {message_delay, DataType::int16, "Message Delay", "ms", Scaling::ten, Value::_int16_t(0), true}       // 5
                                                                 }},
                                             {"block1700", 1700, {
                                                                     {serial_number, DataType::uint32, "Serial Number", "", Scaling::none, Value::_uint32_t(serialNumber), true}, // serial number
                                                                     {uptime, DataType::uint32, "Uptime", "s", Scaling::none, Value::_uint32_t(0), true},                         // 0
                                                                     {total_uptime, DataType::uint32, "Total Uptime", "s", Scaling::none, Value::_uint32_t(0), true},             // 0
                                                                     {wattnode_model, DataType::int16, "Wattnode Model", "", Scaling::none, Value::_int16_t(202), true},          // 202
                                                                     {firmware_version, DataType::int16, "Firmware Version", "", Scaling::none, Value::_int16_t(31), true},       // 31
                                                                     {options, DataType::int16, "Options", "", Scaling::none, Value::_int16_t(0), true},                          // 0
                                                                     {error_status, DataType::int16, "Error Status", "", Scaling::none, Value::_int16_t(0), true},                // 0
                                                                     {power_fail_count, DataType::int16, "Power Fail Count", "", Scaling::none, Value::_int16_t(0), true},        // 0
                                                                     {crc_error_count, DataType::int16, "CRC Error Count", "", Scaling::none, Value::_int16_t(0), true},          // 0
                                                                     {frame_error_count, DataType::int16, "Frame Error Count", "", Scaling::none, Value::_int16_t(0), true},      // 0
                                                                     {packet_error_count, DataType::int16, "Packet Error Count", "", Scaling::none, Value::_int16_t(0), true},    // 0
                                                                     {overrun_count, DataType::int16, "Overrun Count", "", Scaling::none, Value::_int16_t(0), true},              // 0
                                                                     {error_status_1, DataType::int16, "Error Status 1", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_2, DataType::int16, "Error Status 2", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_3, DataType::int16, "Error Status 3", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_4, DataType::int16, "Error Status 4", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_5, DataType::int16, "Error Status 5", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_6, DataType::int16, "Error Status 6", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_7, DataType::int16, "Error Status 7", "", Scaling::none, Value::_int16_t(0), true},            // 0
                                                                     {error_status_8, DataType::int16, "Error Status 8", "", Scaling::none, Value::_int16_t(0), true}             // 0
                                                                 }},
                                             // SolarEdge requests the value for the register 1736
                                             // Unclear what it is
                                             {"block1736", 1736, {
                                                                     {unknown1, DataType::uint32, "Unknown 1", "", Scaling::none, Value::_uint32_t(0), true} // 0
                                                                 }},
                                             // SolarEdge requests the value for the register 2127
                                             // If you don't supply it, it will keep asking
                                             // if you supply it, it will only ask it once
                                             {"block2127", 2127, {
                                                                     {unknown2, DataType::uint16, "Unknown 2", "", Scaling::none, Value::_uint16_t(1), true} // 0
                                                                 }}});
    return dd;
}