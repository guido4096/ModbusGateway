/**
 * @file      convert_em24_to_wattnode.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Convert and copy em24 data to the wattnode slave
 */

#include "Arduino.h"
#include "convert_em24_to_wattnode.h"

void modbus_gateway::ConvertEM24ToWattNode::CopyDataFromMasterToSlave()
{   
    //Serial.printf("CopyDateFromEM24ToWattnode\n\r");
    // Block 1000
    _wattnode.setFloatValue(WattNode::energy_active, _meter.getFloatValue(EM24::import_energy_active)+_meter.getFloatValue(EM24::export_energy_active));// # total active energy
    _wattnode.setFloatValue(WattNode::import_energy_active, _meter.getFloatValue(EM24::import_energy_active));//  # imported active energy
    _wattnode.setFloatValue(WattNode::energy_active_nr, _meter.getFloatValue(EM24::import_energy_active)+_meter.getFloatValue(EM24::export_energy_active));//  # total active energy non-reset
    _wattnode.setFloatValue(WattNode::import_energy_active_nr, _meter.getFloatValue(EM24::import_energy_active));//  # imported active energy non-reset
    _wattnode.setFloatValue(WattNode::power_active, _meter.getFloatValue(EM24::power_active));//  # total power
    _wattnode.setFloatValue(WattNode::l1_power_active, _meter.getFloatValue(EM24::l1_power_active));
    _wattnode.setFloatValue(WattNode::l2_power_active, _meter.getFloatValue(EM24::l2_power_active));
    _wattnode.setFloatValue(WattNode::l3_power_active, _meter.getFloatValue(EM24::l3_power_active));
    _wattnode.setFloatValue(WattNode::voltage_ln, _meter.getFloatValue(EM24::voltage_ln));//  # l-n voltage
    _wattnode.setFloatValue(WattNode::l1n_voltage, _meter.getFloatValue(EM24::l1_voltage));//  # l1-n voltage
    _wattnode.setFloatValue(WattNode::l2n_voltage, _meter.getFloatValue(EM24::l2_voltage));//  # l2-n voltage
    _wattnode.setFloatValue(WattNode::l3n_voltage, _meter.getFloatValue(EM24::l3_voltage));//  # l3-n voltage
    _wattnode.setFloatValue(WattNode::voltage_ll, _meter.getFloatValue(EM24::voltage_ll));//  # l-l voltage
    _wattnode.setFloatValue(WattNode::l12_voltage, _meter.getFloatValue(EM24::l12_voltage));//  # l1-l2 voltage
    _wattnode.setFloatValue(WattNode::l23_voltage, _meter.getFloatValue(EM24::l23_voltage));//  # l2-l3 voltage
    _wattnode.setFloatValue(WattNode::l31_voltage, _meter.getFloatValue(EM24::l31_voltage));//  # l3-l1 voltage
    _wattnode.setFloatValue(WattNode::frequency, _meter.getFloatValue(EM24::frequency));//  # line frequency    
    
    // Block 1100
    _wattnode.setFloatValue(WattNode::l1_energy_active, _meter.getFloatValue(EM24::l1_import_energy_active)+_meter.getFloatValue(EM24::export_energy_active)/3); //  total active energy l1
    _wattnode.setFloatValue(WattNode::l2_energy_active, _meter.getFloatValue(EM24::l2_import_energy_active)+_meter.getFloatValue(EM24::export_energy_active)/3); //  total active energy l2
    _wattnode.setFloatValue(WattNode::l3_energy_active, _meter.getFloatValue(EM24::l3_import_energy_active)+_meter.getFloatValue(EM24::export_energy_active)/3); //  total active energy l3
    _wattnode.setFloatValue(WattNode::l1_import_energy_active, _meter.getFloatValue(EM24::l1_import_energy_active)); //  imported active energy l1
    _wattnode.setFloatValue(WattNode::l2_import_energy_active, _meter.getFloatValue(EM24::l2_import_energy_active)); //  imported active energy l2
    _wattnode.setFloatValue(WattNode::l3_import_energy_active, _meter.getFloatValue(EM24::l3_import_energy_active)); //  imported active energy l3
    _wattnode.setFloatValue(WattNode::export_energy_active, _meter.getFloatValue(EM24::export_energy_active)); //  total exported active energy
    _wattnode.setFloatValue(WattNode::export_energy_active_nr, _meter.getFloatValue(EM24::export_energy_active)); //  total exported active energy non-reset
    _wattnode.setFloatValue(WattNode::l1_export_energy_active, _meter.getFloatValue(EM24::export_energy_active)/3); //  exported energy l1
    _wattnode.setFloatValue(WattNode::l2_export_energy_active, _meter.getFloatValue(EM24::export_energy_active)/3); //  exported energy l2
    _wattnode.setFloatValue(WattNode::l3_export_energy_active, _meter.getFloatValue(EM24::export_energy_active)/3); //  exported energy l3
    _wattnode.setFloatValue(WattNode::energy_reactive, _meter.getFloatValue(EM24::import_energy_reactive) + _meter.getFloatValue(EM24::export_energy_reactive)); //  total reactive energy
    //_wattnode.setFloatValue(WattNode::l1_energy_reactive, _meter.getFloatValue(EM24::l1_energy_reactive)); //  reactive energy l1
    //_wattnode.setFloatValue(WattNode::l2_energy_reactive, _meter.getFloatValue(EM24::l2_energy_reactive)); //  reactive energy l2
    //_wattnode.setFloatValue(WattNode::l3_energy_reactive, _meter.getFloatValue(EM24::l3_energy_reactive)); //  reactive energy l3
    //_wattnode.setFloatValue(WattNode::energy_apparent, _meter.getFloatValue(EM24::energy_apparent)); //  total apparent energy
    //_wattnode.setFloatValue(WattNode::l1_energy_apparent, _meter.getFloatValue(EM24::l1_energy_apparent)); //  apparent energy l1
    //_wattnode.setFloatValue(WattNode::l2_energy_apparent, _meter.getFloatValue(EM24::l2_energy_apparent)); //  apparent energy l2
    //_wattnode.setFloatValue(WattNode::l3_energy_apparent, _meter.getFloatValue(EM24::l3_energy_apparent)); //  apparent energy l3
    _wattnode.setFloatValue(WattNode::power_factor, _meter.getFloatValue(EM24::total_pf)); //  power factor
    _wattnode.setFloatValue(WattNode::l1_power_factor, _meter.getFloatValue(EM24::l1_power_factor)); //  power factor l1
    _wattnode.setFloatValue(WattNode::l2_power_factor, _meter.getFloatValue(EM24::l2_power_factor)); //  power factor l2
    _wattnode.setFloatValue(WattNode::l3_power_factor, _meter.getFloatValue(EM24::l3_power_factor)); //  power factor l3
    _wattnode.setFloatValue(WattNode::power_reactive, _meter.getFloatValue(EM24::power_reactive)); //  total reactive power
    _wattnode.setFloatValue(WattNode::l1_power_reactive, _meter.getFloatValue(EM24::l1_power_reactive)); //  reactive power l1
    _wattnode.setFloatValue(WattNode::l2_power_reactive, _meter.getFloatValue(EM24::l2_power_reactive)); //  reactive power l2
    _wattnode.setFloatValue(WattNode::l3_power_reactive, _meter.getFloatValue(EM24::l3_power_reactive)); //  reactive power l3
    _wattnode.setFloatValue(WattNode::power_apparent, _meter.getFloatValue(EM24::power_apparent)); //  total apparent power
    _wattnode.setFloatValue(WattNode::l1_power_apparent, _meter.getFloatValue(EM24::l1_power_apparent)); //  apparent power l1
    _wattnode.setFloatValue(WattNode::l2_power_apparent, _meter.getFloatValue(EM24::l2_power_apparent)); //  apparent power l2
    _wattnode.setFloatValue(WattNode::l3_power_apparent, _meter.getFloatValue(EM24::l3_power_apparent)); //  apparent power l3
    _wattnode.setFloatValue(WattNode::l1_current, _meter.getFloatValue(EM24::l1_current)); //  current l1
    _wattnode.setFloatValue(WattNode::l2_current, _meter.getFloatValue(EM24::l2_current)); //  current l2
    _wattnode.setFloatValue(WattNode::l3_current, _meter.getFloatValue(EM24::l3_current)); //  current l3
    _wattnode.setFloatValue(WattNode::demand_power_active, _meter.getFloatValue(EM24::demand_power_active)); //  demand power
    //_wattnode.setFloatValue(WattNode::minimum_demand_power_active, _meter.getFloatValue(EM24::minimum_demand_power_active)); //  minimum demand power
    _wattnode.setFloatValue(WattNode::maximum_demand_power_active, _meter.getFloatValue(EM24::maximum_demand_power_active)); //  maximum demand power
    _wattnode.setFloatValue(WattNode::demand_power_apparent, _meter.getFloatValue(EM24::demand_power_apparent)); //  apparent demand power
    //_wattnode.setFloatValue(WattNode::l1_demand_power_active, _meter.getFloatValue(EM24::l1_demand_power_active)); //  demand power l1
    //_wattnode.setFloatValue(WattNode::l2_demand_power_active, _meter.getFloatValue(EM24::l2_demand_power_active)); //  demand power l2
    //_wattnode.setFloatValue(WattNode::l3_demand_power_active, _meter.getFloatValue(EM24::l3_demand_power_active)); //  demand power l3
}
