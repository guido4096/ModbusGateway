/**
 * @file      convert_em24_towattnode.cpp
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
    DataAccess <Server<WattNode>> wattnode(_wattnode);
    DataAccess <Client<EM24>> meter(_meter);
    //Serial.printf("CopyDateFromEM24ToWattnode\n\r");
    // Block 1000
    wattnode.setFloatValue(WattNode::energy_active, meter.getFloatValue(EM24::import_energy_active)+meter.getFloatValue(EM24::export_energy_active));// # total active energy
    wattnode.setFloatValue(WattNode::import_energy_active, meter.getFloatValue(EM24::import_energy_active));//  # imported active energy
    wattnode.setFloatValue(WattNode::energy_active_nr, meter.getFloatValue(EM24::import_energy_active)+meter.getFloatValue(EM24::export_energy_active));//  # total active energy non-reset
    wattnode.setFloatValue(WattNode::import_energy_active_nr, meter.getFloatValue(EM24::import_energy_active));//  # imported active energy non-reset
    wattnode.setFloatValue(WattNode::power_active, meter.getFloatValue(EM24::power_active));//  # total power
    wattnode.setFloatValue(WattNode::l1_power_active, meter.getFloatValue(EM24::l1_power_active));
    wattnode.setFloatValue(WattNode::l2_power_active, meter.getFloatValue(EM24::l2_power_active));
    wattnode.setFloatValue(WattNode::l3_power_active, meter.getFloatValue(EM24::l3_power_active));
    wattnode.setFloatValue(WattNode::voltage_ln, meter.getFloatValue(EM24::voltage_ln));//  # l-n voltage
    wattnode.setFloatValue(WattNode::l1n_voltage, meter.getFloatValue(EM24::l1_voltage));//  # l1-n voltage
    wattnode.setFloatValue(WattNode::l2n_voltage, meter.getFloatValue(EM24::l2_voltage));//  # l2-n voltage
    wattnode.setFloatValue(WattNode::l3n_voltage, meter.getFloatValue(EM24::l3_voltage));//  # l3-n voltage
    wattnode.setFloatValue(WattNode::voltage_ll, meter.getFloatValue(EM24::voltage_ll));//  # l-l voltage
    wattnode.setFloatValue(WattNode::l12_voltage, meter.getFloatValue(EM24::l12_voltage));//  # l1-l2 voltage
    wattnode.setFloatValue(WattNode::l23_voltage, meter.getFloatValue(EM24::l23_voltage));//  # l2-l3 voltage
    wattnode.setFloatValue(WattNode::l31_voltage, meter.getFloatValue(EM24::l31_voltage));//  # l3-l1 voltage
    wattnode.setFloatValue(WattNode::frequency, meter.getFloatValue(EM24::frequency));//  # line frequency    
    
    // Block 1100
    wattnode.setFloatValue(WattNode::l1_energy_active, meter.getFloatValue(EM24::l1_import_energy_active)+meter.getFloatValue(EM24::export_energy_active)/3); //  total active energy l1
    wattnode.setFloatValue(WattNode::l2_energy_active, meter.getFloatValue(EM24::l2_import_energy_active)+meter.getFloatValue(EM24::export_energy_active)/3); //  total active energy l2
    wattnode.setFloatValue(WattNode::l3_energy_active, meter.getFloatValue(EM24::l3_import_energy_active)+meter.getFloatValue(EM24::export_energy_active)/3); //  total active energy l3
    wattnode.setFloatValue(WattNode::l1_import_energy_active, meter.getFloatValue(EM24::l1_import_energy_active)); //  imported active energy l1
    wattnode.setFloatValue(WattNode::l2_import_energy_active, meter.getFloatValue(EM24::l2_import_energy_active)); //  imported active energy l2
    wattnode.setFloatValue(WattNode::l3_import_energy_active, meter.getFloatValue(EM24::l3_import_energy_active)); //  imported active energy l3
    wattnode.setFloatValue(WattNode::export_energy_active, meter.getFloatValue(EM24::export_energy_active)); //  total exported active energy
    wattnode.setFloatValue(WattNode::export_energy_active_nr, meter.getFloatValue(EM24::export_energy_active)); //  total exported active energy non-reset
    wattnode.setFloatValue(WattNode::l1_export_energy_active, meter.getFloatValue(EM24::export_energy_active)/3); //  exported energy l1
    wattnode.setFloatValue(WattNode::l2_export_energy_active, meter.getFloatValue(EM24::export_energy_active)/3); //  exported energy l2
    wattnode.setFloatValue(WattNode::l3_export_energy_active, meter.getFloatValue(EM24::export_energy_active)/3); //  exported energy l3
    wattnode.setFloatValue(WattNode::energy_reactive, meter.getFloatValue(EM24::import_energy_reactive) + meter.getFloatValue(EM24::export_energy_reactive)); //  total reactive energy
    //wattnode.setFloatValue(WattNode::l1_energy_reactive, meter.getFloatValue(EM24::l1_energy_reactive)); //  reactive energy l1
    //wattnode.setFloatValue(WattNode::l2_energy_reactive, meter.getFloatValue(EM24::l2_energy_reactive)); //  reactive energy l2
    //wattnode.setFloatValue(WattNode::l3_energy_reactive, meter.getFloatValue(EM24::l3_energy_reactive)); //  reactive energy l3
    //wattnode.setFloatValue(WattNode::energy_apparent, meter.getFloatValue(EM24::energy_apparent)); //  total apparent energy
    //wattnode.setFloatValue(WattNode::l1_energy_apparent, meter.getFloatValue(EM24::l1_energy_apparent)); //  apparent energy l1
    //wattnode.setFloatValue(WattNode::l2_energy_apparent, meter.getFloatValue(EM24::l2_energy_apparent)); //  apparent energy l2
    //wattnode.setFloatValue(WattNode::l3_energy_apparent, meter.getFloatValue(EM24::l3_energy_apparent)); //  apparent energy l3
    wattnode.setFloatValue(WattNode::power_factor, meter.getFloatValue(EM24::total_pf)); //  power factor
    wattnode.setFloatValue(WattNode::l1_power_factor, meter.getFloatValue(EM24::l1_power_factor)); //  power factor l1
    wattnode.setFloatValue(WattNode::l2_power_factor, meter.getFloatValue(EM24::l2_power_factor)); //  power factor l2
    wattnode.setFloatValue(WattNode::l3_power_factor, meter.getFloatValue(EM24::l3_power_factor)); //  power factor l3
    wattnode.setFloatValue(WattNode::power_reactive, meter.getFloatValue(EM24::power_reactive)); //  total reactive power
    wattnode.setFloatValue(WattNode::l1_power_reactive, meter.getFloatValue(EM24::l1_power_reactive)); //  reactive power l1
    wattnode.setFloatValue(WattNode::l2_power_reactive, meter.getFloatValue(EM24::l2_power_reactive)); //  reactive power l2
    wattnode.setFloatValue(WattNode::l3_power_reactive, meter.getFloatValue(EM24::l3_power_reactive)); //  reactive power l3
    wattnode.setFloatValue(WattNode::power_apparent, meter.getFloatValue(EM24::power_apparent)); //  total apparent power
    wattnode.setFloatValue(WattNode::l1_power_apparent, meter.getFloatValue(EM24::l1_power_apparent)); //  apparent power l1
    wattnode.setFloatValue(WattNode::l2_power_apparent, meter.getFloatValue(EM24::l2_power_apparent)); //  apparent power l2
    wattnode.setFloatValue(WattNode::l3_power_apparent, meter.getFloatValue(EM24::l3_power_apparent)); //  apparent power l3
    wattnode.setFloatValue(WattNode::l1_current, meter.getFloatValue(EM24::l1_current)); //  current l1
    wattnode.setFloatValue(WattNode::l2_current, meter.getFloatValue(EM24::l2_current)); //  current l2
    wattnode.setFloatValue(WattNode::l3_current, meter.getFloatValue(EM24::l3_current)); //  current l3
    wattnode.setFloatValue(WattNode::demand_power_active, meter.getFloatValue(EM24::demand_power_active)); //  demand power
    //wattnode.setFloatValue(WattNode::minimum_demand_power_active, meter.getFloatValue(EM24::minimum_demand_power_active)); //  minimum demand power
    wattnode.setFloatValue(WattNode::maximum_demand_power_active, meter.getFloatValue(EM24::maximum_demand_power_active)); //  maximum demand power
    wattnode.setFloatValue(WattNode::demand_power_apparent, meter.getFloatValue(EM24::demand_power_apparent)); //  apparent demand power
    //wattnode.setFloatValue(WattNode::l1_demand_power_active, meter.getFloatValue(EM24::l1_demand_power_active)); //  demand power l1
    //wattnode.setFloatValue(WattNode::l2_demand_power_active, meter.getFloatValue(EM24::l2_demand_power_active)); //  demand power l2
    //wattnode.setFloatValue(WattNode::l3_demand_power_active, meter.getFloatValue(EM24::l3_demand_power_active)); //  demand power l3
}
