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
    Example of the registers is SolarEdge querying for.
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
const modbus::DeviceDescription &modbus::WattNode::getDeviceDescription()
{
    static modbus::DeviceDescription dd("wattnode",
        std::vector<modbus::RegisterBlock>{
            modbus::RegisterBlock::makeBlock("block0000",
                                  std::vector<Register>{
                                      modbus::Register("dummy1", 0, 1, modbus::Register::data_type_e::uint16, "Dummy 1 always returns 0", "", 1, modbus::Register::value_t::_uint16_t(0)),
                                      modbus::Register("dummy2", 1, 1, modbus::Register::data_type_e::uint16, "Dummy 2 always returns 0", "", 1, modbus::Register::value_t::_uint16_t(0))}),
            modbus::RegisterBlock::makeBlock("block1000",
                                  std::vector<Register>{
                                      modbus::Register("energy_active", 1000, 2, modbus::Register::data_type_e::float32, "Total Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("import_energy_active", 1002, 2, modbus::Register::data_type_e::float32, "Imported Total Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("energy_active_nr", 1004, 2, modbus::Register::data_type_e::float32, "Total Energy NR (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("import_energy_active_nr", 1006, 2, modbus::Register::data_type_e::float32, "Imported Total Energy NR (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("power_active", 1008, 2, modbus::Register::data_type_e::float32, "Total Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1_power_active", 1010, 2, modbus::Register::data_type_e::float32, "L1 Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2_power_active", 1012, 2, modbus::Register::data_type_e::float32, "L2 Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3_power_active", 1014, 2, modbus::Register::data_type_e::float32, "L3 Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("voltage_ln", 1016, 2, modbus::Register::data_type_e::float32, "Voltage L-N", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l1n_voltage", 1018, 2, modbus::Register::data_type_e::float32, "Voltage L1-N", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l2n_voltage", 1020, 2, modbus::Register::data_type_e::float32, "Voltage L2-N", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l3n_voltage", 1022, 2, modbus::Register::data_type_e::float32, "Voltage L3-N", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("voltage_ll", 1024, 2, modbus::Register::data_type_e::float32, "Voltage LL", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l12_voltage", 1026, 2, modbus::Register::data_type_e::float32, "Voltage L1-L2", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l23_voltage", 1028, 2, modbus::Register::data_type_e::float32, "Voltage L2-L3", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("l31_voltage", 1030, 2, modbus::Register::data_type_e::float32, "Voltage L3-L1", "V", 1, modbus::Register::value_t::_float32_t(0)),
                                      modbus::Register("frequency", 1032, 2, modbus::Register::data_type_e::float32, "Frequency", "", 1, modbus::Register::value_t::_float32_t(0))}),
            modbus::RegisterBlock::makeBlock("block1100",
                                  std::vector<Register>{
                                      modbus::Register("l1_energy_active", 1100, 2, modbus::Register::data_type_e::float32, "L1 Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                     // total active energy l1
                                      modbus::Register("l2_energy_active", 1102, 2, modbus::Register::data_type_e::float32, "L2 Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                     // total active energy l2
                                      modbus::Register("l3_energy_active", 1104, 2, modbus::Register::data_type_e::float32, "L3 Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                     // total active energy l3
                                      modbus::Register("l1_import_energy_active", 1106, 2, modbus::Register::data_type_e::float32, "L1 Imported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),     //  imported active energy l1
                                      modbus::Register("l2_import_energy_active", 1108, 2, modbus::Register::data_type_e::float32, "L2 Imported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),     //  imported active energy l2
                                      modbus::Register("l3_import_energy_active", 1110, 2, modbus::Register::data_type_e::float32, "L3 Imported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),     //  imported active energy l3
                                      modbus::Register("export_energy_active", 1112, 2, modbus::Register::data_type_e::float32, "Exported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),           //  total exported active energy
                                      modbus::Register("export_energy_active_nr", 1114, 2, modbus::Register::data_type_e::float32, "Exported Energy NR (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),        //  total exported active energy non-reset
                                      modbus::Register("l1_export_energy_active", 1116, 2, modbus::Register::data_type_e::float32, "L1 Exported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),     //  exported energy l1
                                      modbus::Register("l2_export_energy_active", 1118, 2, modbus::Register::data_type_e::float32, "L2 Exported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),     //  exported energy l2
                                      modbus::Register("l3_export_energy_active", 1120, 2, modbus::Register::data_type_e::float32, "L3 Exported Energy (Active)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),     //  exported energy l3
                                      modbus::Register("energy_reactive", 1122, 2, modbus::Register::data_type_e::float32, "Energy (Reactive)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                       // total reactive energy
                                      modbus::Register("l1_energy_reactive", 1124, 2, modbus::Register::data_type_e::float32, "L1 Energy (Reactive)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                 //  reactive energy l1
                                      modbus::Register("l2_energy_reactive", 1126, 2, modbus::Register::data_type_e::float32, "L2 Energy (Reactive)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                 //  reactive energy l2
                                      modbus::Register("l3_energy_reactive", 1128, 2, modbus::Register::data_type_e::float32, "L3 Energy (Reactive)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                 //  reactive energy l3
                                      modbus::Register("energy_apparent", 1130, 2, modbus::Register::data_type_e::float32, "Energy (Apparent)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                       //  total apparent energy
                                      modbus::Register("l1_energy_apparent", 1132, 2, modbus::Register::data_type_e::float32, "L1 Energy (Apparent)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                 //  apparent energy l1
                                      modbus::Register("l2_energy_apparent", 1134, 2, modbus::Register::data_type_e::float32, "L2 Energy (Apparent)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                 //  apparent energy l2
                                      modbus::Register("l3_energy_apparent", 1136, 2, modbus::Register::data_type_e::float32, "L3 Energy (Apparent)", "kWh", 1, modbus::Register::value_t::_float32_t(0)),                 //  apparent energy l3
                                      modbus::Register("power_factor", 1138, 2, modbus::Register::data_type_e::float32, "Power Factor", "", 1, modbus::Register::value_t::_float32_t(0)),                                  //  power factor
                                      modbus::Register("l1_power_factor", 1140, 2, modbus::Register::data_type_e::float32, "L1 Power Factor", "", 1, modbus::Register::value_t::_float32_t(0)),                            //  power factor l1
                                      modbus::Register("l2_power_factor", 1142, 2, modbus::Register::data_type_e::float32, "L1 Power Factor", "", 1, modbus::Register::value_t::_float32_t(0)),                            //  power factor l2
                                      modbus::Register("l3_power_factor", 1144, 2, modbus::Register::data_type_e::float32, "L1 Power Factor", "", 1, modbus::Register::value_t::_float32_t(0)),                            //  power factor l3
                                      modbus::Register("power_reactive", 1146, 2, modbus::Register::data_type_e::float32, "Power (Reactive)", "VAr", 1, modbus::Register::value_t::_float32_t(0)),                         //  total reactive power
                                      modbus::Register("l1_power_reactive", 1148, 2, modbus::Register::data_type_e::float32, "L1 Power (Reactive)", "VAr", 1, modbus::Register::value_t::_float32_t(0)),                   //  reactive power l1
                                      modbus::Register("l2_power_reactive", 1150, 2, modbus::Register::data_type_e::float32, "L2 Power (Reactive)", "VAr", 1, modbus::Register::value_t::_float32_t(0)),                   //  reactive power l2
                                      modbus::Register("l3_power_reactive", 1152, 2, modbus::Register::data_type_e::float32, "L3 Power (Reactive)", "VAr", 1, modbus::Register::value_t::_float32_t(0)),                   //  reactive power l3
                                      modbus::Register("power_apparent", 1154, 2, modbus::Register::data_type_e::float32, "Power (Apparent)", "VA", 1, modbus::Register::value_t::_float32_t(0)),                          //  total apparent power
                                      modbus::Register("l1_power_apparent", 1156, 2, modbus::Register::data_type_e::float32, "L1 Power (Apparent)", "VA", 1, modbus::Register::value_t::_float32_t(0)),                    //  apparent power l1
                                      modbus::Register("l2_power_apparent", 1158, 2, modbus::Register::data_type_e::float32, "L2 Power (Apparent)", "VA", 1, modbus::Register::value_t::_float32_t(0)),                    //  apparent power l2
                                      modbus::Register("l3_power_apparent", 1160, 2, modbus::Register::data_type_e::float32, "L3 Power (Apparent)", "VA", 1, modbus::Register::value_t::_float32_t(0)),                    //  apparent power l3
                                      modbus::Register("l1_current", 1162, 2, modbus::Register::data_type_e::float32, "L1 Current", "A", 1, modbus::Register::value_t::_float32_t(0)),                                     // current l1
                                      modbus::Register("l2_current", 1164, 2, modbus::Register::data_type_e::float32, "L2 Current", "A", 1, modbus::Register::value_t::_float32_t(0)),                                     //  current l2
                                      modbus::Register("l3_current", 1166, 2, modbus::Register::data_type_e::float32, "L3 Current", "A", 1, modbus::Register::value_t::_float32_t(0)),                                     //  current l3
                                      modbus::Register("demand_power_active", 1168, 2, modbus::Register::data_type_e::float32, "Demand Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),                 //  demand power
                                      modbus::Register("minimum_demand_power_active", 1170, 2, modbus::Register::data_type_e::float32, "Minimum Demand Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)), //  minimum demand power
                                      modbus::Register("maximum_demand_power_active", 1172, 2, modbus::Register::data_type_e::float32, "Maximum Demand Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)), //  maximum demand power
                                      modbus::Register("demand_power_apparent", 1174, 2, modbus::Register::data_type_e::float32, "Demand Power (Apparent)", "VA", 1, modbus::Register::value_t::_float32_t(0)),            //  apparent demand power
                                      modbus::Register("l1_demand_power_active", 1176, 2, modbus::Register::data_type_e::float32, "L1 Demand Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),           // demand power l1
                                      modbus::Register("l2_demand_power_active", 1178, 2, modbus::Register::data_type_e::float32, "L2 Demand Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0)),           //  demand power l2
                                      modbus::Register("l3_demand_power_active", 1180, 2, modbus::Register::data_type_e::float32, "L3 Demand Power (Active)", "W", 1, modbus::Register::value_t::_float32_t(0))            //  demand power l3
                                  }),
            modbus::RegisterBlock::makeBlock("block1600",
                                  std::vector<Register>{
                                      modbus::Register("passcode", 1600, 2, modbus::Register::data_type_e::uint32, "Passcode", "", 1, modbus::Register::value_t::_uint32_t(1234)),                             // 1234
                                      modbus::Register("ct_current", 1602, 1, modbus::Register::data_type_e::int16, "CT Current", "A", 1, modbus::Register::value_t::_int16_t(0)),                             // 0?
                                      modbus::Register("ct_current_l1", 1603, 1, modbus::Register::data_type_e::int16, "L1 CT Current", "A", 1, modbus::Register::value_t::_int16_t(0)),                       // 0?
                                      modbus::Register("ct_current_l2", 1604, 1, modbus::Register::data_type_e::int16, "L1 CT Current", "A", 1, modbus::Register::value_t::_int16_t(0)),                       // 0?
                                      modbus::Register("ct_current_l3", 1605, 1, modbus::Register::data_type_e::int16, "L1 CT Current", "A", 1, modbus::Register::value_t::_int16_t(0)),                       // 0?
                                      modbus::Register("ct_inverted", 1606, 1, modbus::Register::data_type_e::int16, "CT Inverted", "", 1, modbus::Register::value_t::_int16_t(0)),                            // 0
                                      modbus::Register("measurement_averaging", 1607, 1, modbus::Register::data_type_e::int16, "Measurement Averaging", "", 1, modbus::Register::value_t::_int16_t(0)),        // 0
                                      modbus::Register("power_scale", 1608, 1, modbus::Register::data_type_e::int16, "Power Scale", "", 1, modbus::Register::value_t::_int16_t(0)),                            // 0
                                      modbus::Register("demand_period", 1609, 1, modbus::Register::data_type_e::int16, "Demand Period", "Minute", 1, modbus::Register::value_t::_int16_t(15)),                 // 15
                                      modbus::Register("demand_subintervals", 1610, 1, modbus::Register::data_type_e::int16, "Demand Subintervals", "", 1, modbus::Register::value_t::_int16_t(0)),            // 1
                                      modbus::Register("l1_power_energy_adj", 1611, 1, modbus::Register::data_type_e::int16, "Power/Energy adjustment l1", "", 1, modbus::Register::value_t::_int16_t(10000)), // 10000
                                      modbus::Register("l2_power_energy_adj", 1612, 1, modbus::Register::data_type_e::int16, "Power/Energy adjustment l2", "", 1, modbus::Register::value_t::_int16_t(10000)), // 10000
                                      modbus::Register("l3_power_energy_adj", 1613, 1, modbus::Register::data_type_e::int16, "Power/Energy adjustment l3", "", 1, modbus::Register::value_t::_int16_t(10000)), // 10000
                                      modbus::Register("l1_ct_phase_angle_adj", 1614, 1, modbus::Register::data_type_e::int16, "L1 Voltage", "V", 1, modbus::Register::value_t::_int16_t(-1000)),              // -1000
                                      modbus::Register("l2_ct_phase_angle_adj", 1615, 1, modbus::Register::data_type_e::int16, "L1 Voltage", "V", 1, modbus::Register::value_t::_int16_t(-1000)),              // -1000
                                      modbus::Register("l3_ct_phase_angle_adj", 1616, 1, modbus::Register::data_type_e::int16, "L1 Voltage", "V", 1, modbus::Register::value_t::_int16_t(-1000)),              // -1000
                                      modbus::Register("minimum_power_reading", 1617, 1, modbus::Register::data_type_e::int16, "Minimum Power Reading", "", 1, modbus::Register::value_t::_int16_t(0)),        // 1500
                                      modbus::Register("phase_offset", 1618, 1, modbus::Register::data_type_e::int16, "Phase Offset", "", 1, modbus::Register::value_t::_int16_t(0)),                          // 0?
                                      modbus::Register("reset_energy", 1619, 1, modbus::Register::data_type_e::int16, "Reset Energy", "", 1, modbus::Register::value_t::_int16_t(0)),                          // 0
                                      modbus::Register("reset_demand", 1620, 1, modbus::Register::data_type_e::int16, "Reset Demand", "", 1, modbus::Register::value_t::_int16_t(0)),                          // 0
                                      modbus::Register("current_scale", 1621, 1, modbus::Register::data_type_e::int16, "Current Scale", "", 1, modbus::Register::value_t::_int16_t(20000)),                    // 20000
                                      modbus::Register("io_pin_mode", 1622, 1, modbus::Register::data_type_e::int16, "IO Pin Mode", "", 1, modbus::Register::value_t::_int16_t(0)),                            // 0
                                  }),
            modbus::RegisterBlock::makeBlock("block1650",
                                  std::vector<Register>{
                                      modbus::Register("apply_config", 1650, 1, modbus::Register::data_type_e::int16, "Apply Config", "", 1, modbus::Register::value_t::_int16_t(0)),      // 0
                                      modbus::Register("modbus_address", 1651, 1, modbus::Register::data_type_e::int16, "Modbus Address", "", 1, modbus::Register::value_t::_int16_t(0)),  // modbus address
                                      modbus::Register("baud_rate", 1652, 1, modbus::Register::data_type_e::int16, "Baud Rate", "", 1, modbus::Register::value_t::_int16_t(0)),            // 4
                                      modbus::Register("parity_mode", 1653, 1, modbus::Register::data_type_e::int16, "Parity Mode", "", 1, modbus::Register::value_t::_int16_t(0)),        // 0
                                      modbus::Register("modbus_mode", 1654, 1, modbus::Register::data_type_e::int16, "Modbus Mode", "", 1, modbus::Register::value_t::_int16_t(0)),        // 0
                                      modbus::Register("message_delay", 1655, 1, modbus::Register::data_type_e::int16, "Message Delay", "ms", 10, modbus::Register::value_t::_int16_t(0)), // 5
                                  }),
            modbus::RegisterBlock::makeBlock("block1700",
                                  std::vector<Register>{
                                      modbus::Register("serial_number", 1700, 2, modbus::Register::data_type_e::uint32, "Serial Number", "", 1, modbus::Register::value_t::_uint32_t(12345678)),  // serial number
                                      modbus::Register("uptime", 1702, 2, modbus::Register::data_type_e::uint32, "Uptime", "s", 1, modbus::Register::value_t::_uint32_t(0)),                      // 0
                                      modbus::Register("total_uptime", 1704, 2, modbus::Register::data_type_e::uint32, "Total Uptime", "2", 1, modbus::Register::value_t::_int16_t(0)),           // 0
                                      modbus::Register("wattnode_model", 1706, 1, modbus::Register::data_type_e::int16, "Wattnode Model", "", 1, modbus::Register::value_t::_int16_t(202)),       // 202
                                      modbus::Register("firmware_version", 1707, 1, modbus::Register::data_type_e::int16, "Firmware Version", "", 1, modbus::Register::value_t::_int16_t(31)),    // 31
                                      modbus::Register("options", 1708, 1, modbus::Register::data_type_e::int16, "Options", "", 1, modbus::Register::value_t::_int16_t(0)),                       // 0
                                      modbus::Register("error_status", 1709, 1, modbus::Register::data_type_e::int16, "Error Status", "", 1, modbus::Register::value_t::_int16_t(0)),             // 0
                                      modbus::Register("power_fail_count", 1710, 1, modbus::Register::data_type_e::int16, "Power Fail Count", "", 1, modbus::Register::value_t::_int16_t(0)),     // 0
                                      modbus::Register("crc_error_count", 1711, 1, modbus::Register::data_type_e::int16, "CRC Error Count", "", 1, modbus::Register::value_t::_int16_t(0)),       // 0
                                      modbus::Register("frame_error_count", 1712, 1, modbus::Register::data_type_e::int16, "Frame Error Count", "", 1, modbus::Register::value_t::_int16_t(0)),   // 0
                                      modbus::Register("packet_error_count", 1713, 1, modbus::Register::data_type_e::int16, "Packet Error Count", "", 1, modbus::Register::value_t::_int16_t(0)), // 0
                                      modbus::Register("overrun_count", 1714, 1, modbus::Register::data_type_e::int16, "Overrun Count", "", 1, modbus::Register::value_t::_int16_t(0)),           // 0
                                      modbus::Register("error_status_1", 1715, 1, modbus::Register::data_type_e::int16, "Error Status 1", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_2", 1716, 1, modbus::Register::data_type_e::int16, "Error Status 2", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_3", 1717, 1, modbus::Register::data_type_e::int16, "Error Status 3", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_4", 1718, 1, modbus::Register::data_type_e::int16, "Error Status 4", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_5", 1719, 1, modbus::Register::data_type_e::int16, "Error Status 5", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_6", 1720, 1, modbus::Register::data_type_e::int16, "Error Status 6", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_7", 1721, 1, modbus::Register::data_type_e::int16, "Error Status 7", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                      modbus::Register("error_status_8", 1722, 1, modbus::Register::data_type_e::int16, "Error Status 8", "", 1, modbus::Register::value_t::_int16_t(0)),         // 0
                                  }),
            // SolarEdge requests the value for the register 1736
            // Unclear what it is
            modbus::RegisterBlock::makeBlock("block1736",
                                  std::vector<Register>{
                                      modbus::Register("unknown1", 1736, 2, modbus::Register::data_type_e::uint32, "Unknown 1", "", 1, modbus::Register::value_t::_uint32_t(0)), // 0

                                  }),
            // SolarEdge requests the value for the register 2127
            // If you don't supply it, it will keep asking
            // if you supply it, it will only ask it once
            modbus::RegisterBlock::makeBlock("block2127",
                                  std::vector<Register>{
                                      modbus::Register("unknown2", 2127, 1, modbus::Register::data_type_e::uint16, "Unknown 2", "", 1, modbus::Register::value_t::_uint16_t(1)), // 0

                                  })});
    return dd;
}