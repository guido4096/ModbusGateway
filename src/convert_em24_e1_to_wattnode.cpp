/**
 * @file      convert_em24_e1_to_wattnode.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Convert and copy em24 data to the wattnode slave
 */
#include "Arduino.h"
#include "convert_em24_e1_to_wattnode.h"
#include "algorithm"
#include <cmath>


// Return a measurement in engineering units. Integer Modbus fields contain
// scaled raw values; the typed get<R>() intentionally returns raw integers.
namespace {
template<modbus_gateway::EM24_E1::RegisterId R, typename Access>
float engineering(const Access& access) {
    constexpr auto& description = modbus_gateway::EM24_E1::description<R>();
    return static_cast<float>(access.template get<R>()) /
           modbus_gateway::getScaling(description.scaling);
}
}

void modbus_gateway::ConvertEM24_E1ToWattNode::CopyDataFromMasterToSlave()
{
    DataAccess<Server<WattNode>> wattnode(_wattnode);
    DataAccess<Client<EM24_E1>> meter(_meter);

    // Serial.printf("CopyDateFromEM24ToWattnode\n\r");
    //  Block 1000
    wattnode.set<WattNode::energy_active>( engineering<EM24_E1::import_energy_active>(meter) + engineering<EM24_E1::export_energy_active>(meter));    // # total active energy
    wattnode.set<WattNode::import_energy_active>( engineering<EM24_E1::import_energy_active>(meter));                                                  //  # imported active energy
    wattnode.set<WattNode::energy_active_nr>( engineering<EM24_E1::import_energy_active>(meter) + engineering<EM24_E1::export_energy_active>(meter)); //  # total active energy non-reset
    wattnode.set<WattNode::import_energy_active_nr>( engineering<EM24_E1::import_energy_active>(meter));                                               //  # imported active energy non-reset
    wattnode.set<WattNode::power_active>( engineering<EM24_E1::power_active>(meter));                                                                  //  # total power
    wattnode.set<WattNode::l1_power_active>( engineering<EM24_E1::l1_power_active>(meter));
    wattnode.set<WattNode::l2_power_active>( engineering<EM24_E1::l2_power_active>(meter));
    wattnode.set<WattNode::l3_power_active>( engineering<EM24_E1::l3_power_active>(meter));
    wattnode.set<WattNode::voltage_ln>( engineering<EM24_E1::voltage_ln>(meter));   //  # l-n voltage
    wattnode.set<WattNode::l1n_voltage>( engineering<EM24_E1::l1_voltage>(meter));  //  # l1-n voltage
    wattnode.set<WattNode::l2n_voltage>( engineering<EM24_E1::l2_voltage>(meter));  //  # l2-n voltage
    wattnode.set<WattNode::l3n_voltage>( engineering<EM24_E1::l3_voltage>(meter));  //  # l3-n voltage
    wattnode.set<WattNode::voltage_ll>( engineering<EM24_E1::voltage_ll>(meter));   //  # l-l voltage
    wattnode.set<WattNode::l12_voltage>( engineering<EM24_E1::l12_voltage>(meter)); //  # l1-l2 voltage
    wattnode.set<WattNode::l23_voltage>( engineering<EM24_E1::l23_voltage>(meter)); //  # l2-l3 voltage
    wattnode.set<WattNode::l31_voltage>( engineering<EM24_E1::l31_voltage>(meter)); //  # l3-l1 voltage
    wattnode.set<WattNode::frequency>( engineering<EM24_E1::frequency>(meter));     //  # line frequency

    // Block 1100
    wattnode.set<WattNode::l1_energy_active>( engineering<EM24_E1::l1_import_energy_active>(meter) + engineering<EM24_E1::export_energy_active>(meter) / 3); //  total active energy l1
    wattnode.set<WattNode::l2_energy_active>( engineering<EM24_E1::l2_import_energy_active>(meter) + engineering<EM24_E1::export_energy_active>(meter) / 3); //  total active energy l2
    wattnode.set<WattNode::l3_energy_active>( engineering<EM24_E1::l3_import_energy_active>(meter) + engineering<EM24_E1::export_energy_active>(meter) / 3); //  total active energy l3
    wattnode.set<WattNode::l1_import_energy_active>( engineering<EM24_E1::l1_import_energy_active>(meter));                                                   //  imported active energy l1
    wattnode.set<WattNode::l2_import_energy_active>( engineering<EM24_E1::l2_import_energy_active>(meter));                                                   //  imported active energy l2
    wattnode.set<WattNode::l3_import_energy_active>( engineering<EM24_E1::l3_import_energy_active>(meter));                                                   //  imported active energy l3
    wattnode.set<WattNode::export_energy_active>( engineering<EM24_E1::export_energy_active>(meter));                                                         //  total exported active energy
    wattnode.set<WattNode::export_energy_active_nr>( engineering<EM24_E1::export_energy_active>(meter));                                                      //  total exported active energy non-reset
    wattnode.set<WattNode::l1_export_energy_active>( engineering<EM24_E1::export_energy_active>(meter) / 3);                                                  //  exported energy l1
    wattnode.set<WattNode::l2_export_energy_active>( engineering<EM24_E1::export_energy_active>(meter) / 3);                                                  //  exported energy l2
    wattnode.set<WattNode::l3_export_energy_active>( engineering<EM24_E1::export_energy_active>(meter) / 3);                                                  //  exported energy l3
    wattnode.set<WattNode::energy_reactive>( engineering<EM24_E1::import_energy_reactive>(meter) + engineering<EM24_E1::export_energy_reactive>(meter));     //  total reactive energy
    // wattnode.set<WattNode::l1_energy_reactive>( engineering<EM24_E1::l1_energy_reactive>(meter)); //  reactive energy l1
    // wattnode.set<WattNode::l2_energy_reactive>( engineering<EM24_E1::l2_energy_reactive>(meter)); //  reactive energy l2
    // wattnode.set<WattNode::l3_energy_reactive>( engineering<EM24_E1::l3_energy_reactive>(meter)); //  reactive energy l3
    // wattnode.set<WattNode::energy_apparent>( engineering<EM24_E1::energy_apparent>(meter)); //  total apparent energy
    // wattnode.set<WattNode::l1_energy_apparent>( engineering<EM24_E1::l1_energy_apparent>(meter)); //  apparent energy l1
    // wattnode.set<WattNode::l2_energy_apparent>( engineering<EM24_E1::l2_energy_apparent>(meter)); //  apparent energy l2
    // wattnode.set<WattNode::l3_energy_apparent>( engineering<EM24_E1::l3_energy_apparent>(meter)); //  apparent energy l3
    wattnode.set<WattNode::power_factor>( engineering<EM24_E1::total_pf>(meter));                   //  power factor
    wattnode.set<WattNode::l1_power_factor>( engineering<EM24_E1::l1_power_factor>(meter));         //  power factor l1
    wattnode.set<WattNode::l2_power_factor>( engineering<EM24_E1::l2_power_factor>(meter));         //  power factor l2
    wattnode.set<WattNode::l3_power_factor>( engineering<EM24_E1::l3_power_factor>(meter));         //  power factor l3
    wattnode.set<WattNode::power_reactive>( engineering<EM24_E1::power_reactive>(meter));           //  total reactive power
    wattnode.set<WattNode::l1_power_reactive>( engineering<EM24_E1::l1_power_reactive>(meter));     //  reactive power l1
    wattnode.set<WattNode::l2_power_reactive>( engineering<EM24_E1::l2_power_reactive>(meter));     //  reactive power l2
    wattnode.set<WattNode::l3_power_reactive>( engineering<EM24_E1::l3_power_reactive>(meter));     //  reactive power l3
    wattnode.set<WattNode::power_apparent>( engineering<EM24_E1::power_apparent>(meter));           //  total apparent power
    wattnode.set<WattNode::l1_power_apparent>( engineering<EM24_E1::l1_power_apparent>(meter));     //  apparent power l1
    wattnode.set<WattNode::l2_power_apparent>( engineering<EM24_E1::l2_power_apparent>(meter));     //  apparent power l2
    wattnode.set<WattNode::l3_power_apparent>( engineering<EM24_E1::l3_power_apparent>(meter));     //  apparent power l3
    wattnode.set<WattNode::l1_current>( engineering<EM24_E1::l1_current>(meter));                   //  current l1
    wattnode.set<WattNode::l2_current>( engineering<EM24_E1::l2_current>(meter));                   //  current l2
    wattnode.set<WattNode::l3_current>( engineering<EM24_E1::l3_current>(meter));                   //  current l3
    wattnode.set<WattNode::demand_power_active>( engineering<EM24_E1::demand_power_active>(meter)); //  demand power
    // wattnode.set<WattNode::minimum_demand_power_active>( engineering<EM24_E1::minimum_demand_power_active>(meter)); //  minimum demand power
    wattnode.set<WattNode::maximum_demand_power_active>( engineering<EM24_E1::maximum_demand_power_active>(meter)); //  maximum demand power
    wattnode.set<WattNode::demand_power_apparent>( engineering<EM24_E1::demand_power_apparent>(meter));             //  apparent demand power
    // wattnode.set<WattNode::l1_demand_power_active>( engineering<EM24_E1::l1_demand_power_active>(meter)); //  demand power l1
    // wattnode.set<WattNode::l2_demand_power_active>( engineering<EM24_E1::l2_demand_power_active>(meter)); //  demand power l2
    // wattnode.set<WattNode::l3_demand_power_active>( engineering<EM24_E1::l3_demand_power_active>(meter)); //  demand power l3
}
