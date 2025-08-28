/**
 * @file      convert_em24_towattnode.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Convert and copy em24 data to the wattnode slave
 */

#include "Arduino.h"
#include "convert_em24_e1_to_wattnode.h"
#include "algorithm"

void modbus_gateway::ConvertEM24_E1ToWattNode::CopyDataFromMasterToSlave()
{
    DataAccess<Server<WattNode>> wattnode(_wattnode);
    DataAccess<Client<EM24_E1>> meter(_meter);
    DataAccess<Server<Optimizer>> optimizer(_optimizer);
    DataAccess<Client<SolarEdge>> solaredge(_solaredge);
    DataAccess<Client<Alfen>> alfen(_alfen);
    // Serial.printf("CopyDateFromEM24ToWattnode\n\r");
    //  Block 1000
    wattnode.setFloatValue(WattNode::energy_active, meter.getFloatValue(EM24_E1::import_energy_active) + meter.getFloatValue(EM24_E1::export_energy_active));    // # total active energy
    wattnode.setFloatValue(WattNode::import_energy_active, meter.getFloatValue(EM24_E1::import_energy_active));                                                  //  # imported active energy
    wattnode.setFloatValue(WattNode::energy_active_nr, meter.getFloatValue(EM24_E1::import_energy_active) + meter.getFloatValue(EM24_E1::export_energy_active)); //  # total active energy non-reset
    wattnode.setFloatValue(WattNode::import_energy_active_nr, meter.getFloatValue(EM24_E1::import_energy_active));                                               //  # imported active energy non-reset
    wattnode.setFloatValue(WattNode::power_active, meter.getFloatValue(EM24_E1::power_active));                                                                  //  # total power
    // wattnode.setFloatValue(WattNode::power_active, optimizer.getFloatValue(Optimizer::requested_power_from_inverter)); //  # total power
    wattnode.setFloatValue(WattNode::l1_power_active, meter.getFloatValue(EM24_E1::l1_power_active));
    wattnode.setFloatValue(WattNode::l2_power_active, meter.getFloatValue(EM24_E1::l2_power_active));
    wattnode.setFloatValue(WattNode::l3_power_active, meter.getFloatValue(EM24_E1::l3_power_active));
    wattnode.setFloatValue(WattNode::voltage_ln, meter.getFloatValue(EM24_E1::voltage_ln));   //  # l-n voltage
    wattnode.setFloatValue(WattNode::l1n_voltage, meter.getFloatValue(EM24_E1::l1_voltage));  //  # l1-n voltage
    wattnode.setFloatValue(WattNode::l2n_voltage, meter.getFloatValue(EM24_E1::l2_voltage));  //  # l2-n voltage
    wattnode.setFloatValue(WattNode::l3n_voltage, meter.getFloatValue(EM24_E1::l3_voltage));  //  # l3-n voltage
    wattnode.setFloatValue(WattNode::voltage_ll, meter.getFloatValue(EM24_E1::voltage_ll));   //  # l-l voltage
    wattnode.setFloatValue(WattNode::l12_voltage, meter.getFloatValue(EM24_E1::l12_voltage)); //  # l1-l2 voltage
    wattnode.setFloatValue(WattNode::l23_voltage, meter.getFloatValue(EM24_E1::l23_voltage)); //  # l2-l3 voltage
    wattnode.setFloatValue(WattNode::l31_voltage, meter.getFloatValue(EM24_E1::l31_voltage)); //  # l3-l1 voltage
    wattnode.setFloatValue(WattNode::frequency, meter.getFloatValue(EM24_E1::frequency));     //  # line frequency

    // Block 1100
    wattnode.setFloatValue(WattNode::l1_energy_active, meter.getFloatValue(EM24_E1::l1_import_energy_active) + meter.getFloatValue(EM24_E1::export_energy_active) / 3); //  total active energy l1
    wattnode.setFloatValue(WattNode::l2_energy_active, meter.getFloatValue(EM24_E1::l2_import_energy_active) + meter.getFloatValue(EM24_E1::export_energy_active) / 3); //  total active energy l2
    wattnode.setFloatValue(WattNode::l3_energy_active, meter.getFloatValue(EM24_E1::l3_import_energy_active) + meter.getFloatValue(EM24_E1::export_energy_active) / 3); //  total active energy l3
    wattnode.setFloatValue(WattNode::l1_import_energy_active, meter.getFloatValue(EM24_E1::l1_import_energy_active));                                                   //  imported active energy l1
    wattnode.setFloatValue(WattNode::l2_import_energy_active, meter.getFloatValue(EM24_E1::l2_import_energy_active));                                                   //  imported active energy l2
    wattnode.setFloatValue(WattNode::l3_import_energy_active, meter.getFloatValue(EM24_E1::l3_import_energy_active));                                                   //  imported active energy l3
    wattnode.setFloatValue(WattNode::export_energy_active, meter.getFloatValue(EM24_E1::export_energy_active));                                                         //  total exported active energy
    wattnode.setFloatValue(WattNode::export_energy_active_nr, meter.getFloatValue(EM24_E1::export_energy_active));                                                      //  total exported active energy non-reset
    wattnode.setFloatValue(WattNode::l1_export_energy_active, meter.getFloatValue(EM24_E1::export_energy_active) / 3);                                                  //  exported energy l1
    wattnode.setFloatValue(WattNode::l2_export_energy_active, meter.getFloatValue(EM24_E1::export_energy_active) / 3);                                                  //  exported energy l2
    wattnode.setFloatValue(WattNode::l3_export_energy_active, meter.getFloatValue(EM24_E1::export_energy_active) / 3);                                                  //  exported energy l3
    wattnode.setFloatValue(WattNode::energy_reactive, meter.getFloatValue(EM24_E1::import_energy_reactive) + meter.getFloatValue(EM24_E1::export_energy_reactive));     //  total reactive energy
    // wattnode.setFloatValue(WattNode::l1_energy_reactive, meter.getFloatValue(EM24_E1::l1_energy_reactive)); //  reactive energy l1
    // wattnode.setFloatValue(WattNode::l2_energy_reactive, meter.getFloatValue(EM24_E1::l2_energy_reactive)); //  reactive energy l2
    // wattnode.setFloatValue(WattNode::l3_energy_reactive, meter.getFloatValue(EM24_E1::l3_energy_reactive)); //  reactive energy l3
    // wattnode.setFloatValue(WattNode::energy_apparent, meter.getFloatValue(EM24_E1::energy_apparent)); //  total apparent energy
    // wattnode.setFloatValue(WattNode::l1_energy_apparent, meter.getFloatValue(EM24_E1::l1_energy_apparent)); //  apparent energy l1
    // wattnode.setFloatValue(WattNode::l2_energy_apparent, meter.getFloatValue(EM24_E1::l2_energy_apparent)); //  apparent energy l2
    // wattnode.setFloatValue(WattNode::l3_energy_apparent, meter.getFloatValue(EM24_E1::l3_energy_apparent)); //  apparent energy l3
    wattnode.setFloatValue(WattNode::power_factor, meter.getFloatValue(EM24_E1::total_pf));                   //  power factor
    wattnode.setFloatValue(WattNode::l1_power_factor, meter.getFloatValue(EM24_E1::l1_power_factor));         //  power factor l1
    wattnode.setFloatValue(WattNode::l2_power_factor, meter.getFloatValue(EM24_E1::l2_power_factor));         //  power factor l2
    wattnode.setFloatValue(WattNode::l3_power_factor, meter.getFloatValue(EM24_E1::l3_power_factor));         //  power factor l3
    wattnode.setFloatValue(WattNode::power_reactive, meter.getFloatValue(EM24_E1::power_reactive));           //  total reactive power
    wattnode.setFloatValue(WattNode::l1_power_reactive, meter.getFloatValue(EM24_E1::l1_power_reactive));     //  reactive power l1
    wattnode.setFloatValue(WattNode::l2_power_reactive, meter.getFloatValue(EM24_E1::l2_power_reactive));     //  reactive power l2
    wattnode.setFloatValue(WattNode::l3_power_reactive, meter.getFloatValue(EM24_E1::l3_power_reactive));     //  reactive power l3
    wattnode.setFloatValue(WattNode::power_apparent, meter.getFloatValue(EM24_E1::power_apparent));           //  total apparent power
    wattnode.setFloatValue(WattNode::l1_power_apparent, meter.getFloatValue(EM24_E1::l1_power_apparent));     //  apparent power l1
    wattnode.setFloatValue(WattNode::l2_power_apparent, meter.getFloatValue(EM24_E1::l2_power_apparent));     //  apparent power l2
    wattnode.setFloatValue(WattNode::l3_power_apparent, meter.getFloatValue(EM24_E1::l3_power_apparent));     //  apparent power l3
    wattnode.setFloatValue(WattNode::l1_current, meter.getFloatValue(EM24_E1::l1_current));                   //  current l1
    wattnode.setFloatValue(WattNode::l2_current, meter.getFloatValue(EM24_E1::l2_current));                   //  current l2
    wattnode.setFloatValue(WattNode::l3_current, meter.getFloatValue(EM24_E1::l3_current));                   //  current l3
    wattnode.setFloatValue(WattNode::demand_power_active, meter.getFloatValue(EM24_E1::demand_power_active)); //  demand power
    // wattnode.setFloatValue(WattNode::minimum_demand_power_active, meter.getFloatValue(EM24_E1::minimum_demand_power_active)); //  minimum demand power
    wattnode.setFloatValue(WattNode::maximum_demand_power_active, meter.getFloatValue(EM24_E1::maximum_demand_power_active)); //  maximum demand power
    wattnode.setFloatValue(WattNode::demand_power_apparent, meter.getFloatValue(EM24_E1::demand_power_apparent));             //  apparent demand power
    // wattnode.setFloatValue(WattNode::l1_demand_power_active, meter.getFloatValue(EM24_E1::l1_demand_power_active)); //  demand power l1
    // wattnode.setFloatValue(WattNode::l2_demand_power_active, meter.getFloatValue(EM24_E1::l2_demand_power_active)); //  demand power l2
    // wattnode.setFloatValue(WattNode::l3_demand_power_active, meter.getFloatValue(EM24_E1::l3_demand_power_active)); //  demand power l3

    // alfen=0.0 solar=1724.6 battery=1313.0 net=2559.7
    // solar positive = solar production
    // battery positive = charging battery
    // net positive = using from grid

    // Special: optimize value
    int16_t v_se_power_to_ac = solaredge.getInt16Value(SolarEdge::se_power);
    int16_t v_se_power_to_ac_sf = solaredge.getInt16Value(SolarEdge::se_power_sf);
    float power_to_ac_se = v_se_power_to_ac * pow(10, v_se_power_to_ac_sf); // total power, pv+battery combined, to AC
    float power_alfen = alfen.getFloatValue(Alfen::real_power_sum);
    float power_battery = solaredge.getFloatValue(SolarEdge::b1_instantaneous_power);
    float power_net = meter.getFloatValue(EM24_E1::power_active);
    float power_pv = power_to_ac_se + power_battery;
    // correct for negative values
    if (power_pv < 0)
    {
        power_battery = power_battery - power_pv;
        power_pv = 0;
    }

    float power_house = std::max(float(0), power_net + power_to_ac_se - power_alfen); // house is always 0 or larger

    // Whatever the house uses, take that from PV
    float pv_to_house = std::min(power_pv, power_house);

    // Whatever is left from pv, send that to charging station
    float pv_to_alfen = std::max(float(0), std::min(power_alfen, power_pv - pv_to_house));

    // Whatever is left, send it to the battery
    float pv_to_battery = power_pv - pv_to_house - pv_to_alfen; // Always 0 or larger

    int16_t strategy = 1; // 0: normal . 1: surplus to alfen and not to battery. 2: do not use pv for alfen
    float wanted_from_grid = 0;
    if (strategy != 0)
    {
        if (strategy == 1)
        {
            // How much should we consume from the grid? We only want the power from the alfen that is not covered by pv to be used from the net
            wanted_from_grid = power_alfen - pv_to_alfen;
        }
        else if (strategy == 2)
        {
            // How much should we consume from the grid? All power from alfen should come grid
            wanted_from_grid = power_alfen;
        }

        // Fool the SolarEdge
        wattnode.setFloatValue(WattNode::power_active, meter.getFloatValue(EM24_E1::power_active) - wanted_from_grid); //  # total power
        wattnode.setFloatValue(WattNode::l1_power_active, meter.getFloatValue(EM24_E1::l1_power_active) - wanted_from_grid / 3);
        wattnode.setFloatValue(WattNode::l2_power_active, meter.getFloatValue(EM24_E1::l2_power_active) - wanted_from_grid / 3);
        wattnode.setFloatValue(WattNode::l3_power_active, meter.getFloatValue(EM24_E1::l3_power_active) - wanted_from_grid / 3);
    }
    // Log message
    char buffer[400];
    sprintf(buffer, "cn=%.1f wanted_from_grid=%.1f  power_pv=%.1f power_alfen=%.1f power_to_ac_se=%.1f power_battery=%.1f power_net=%.1f power_house=%.1f pv_to_house=%.1f pv_to_alfen=%.1f pv_to_battery=%.1f",
            power_net - wanted_from_grid, wanted_from_grid, power_pv, power_alfen, power_to_ac_se, power_battery, power_net, power_house, pv_to_house, pv_to_alfen, pv_to_battery);
    optimizer.logMessage(buffer);
}
