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
    DataAccess<Server<Optimizer>> optimizer(_optimizer);
    DataAccess<Client<SolarEdge>> solaredge(_solaredge);
    DataAccess<Client<Alfen>> alfen(_alfen);
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

    // Optimizer strategies:
    //   0 = normal: report the meter values unchanged
    //   1 = route PV surplus to the car, but never use battery power for the car
    //   2 = do not use PV for the car at all; all car power comes from the grid
    enum OptimizerStrategy
    {
        STRATEGY_NORMAL = 0,
        STRATEGY_PV_SURPLUS = 1,
        STRATEGY_GRID_ONLY = 2
    };

    int32_t strategy = optimizer.get<Optimizer::optimizer_strategy>();

    // In general, the car needs so much energy that it could absorb almost all solar energy.
    // The battery is then mainly used for peak shaving, which requires enough energy in the
    // battery to support peaks. The assumption is that the house peak is handled by the battery
    // with minor assistance from the grid, with a grid peak below 2.5 kW.
    // At night, when the house is mostly at rest, the car is charged with a constant 2.5 kW
    // from the grid.
    float wanted_from_grid = 0;
    if (strategy != STRATEGY_NORMAL)
    {
        // Special: optimize the value reported to the SolarEdge
        float power_alfen = alfen.get<Alfen::real_power_sum>();
        float power_net = engineering<EM24_E1::power_active>(meter);

        if (strategy == STRATEGY_PV_SURPLUS)
        {
            int16_t v_se_power_to_ac = solaredge.get<SolarEdge::ac_power>();
            int16_t v_se_power_to_ac_sf = solaredge.get<SolarEdge::ac_power_sf>();
            float power_battery = solaredge.get<SolarEdge::b1_instantaneous_power>();

            // Total power (PV + battery combined) delivered to AC by the inverter
            float power_to_ac_se = v_se_power_to_ac * pow(10, v_se_power_to_ac_sf);
            float power_pv = power_to_ac_se + power_battery;

            // Correct for negative values: PV production can never be below zero
            if (power_pv < 0)
            {
                power_battery = power_battery - power_pv;
                power_pv = 0;
            }

            // House consumption is always zero or larger
            float power_house = std::max(float(0), power_net + power_to_ac_se - power_alfen);

            // Whatever the house uses, take that from PV
            float pv_to_house = std::min(power_pv, power_house);

            // Whatever is left from PV, send that to the charging station
            float pv_to_alfen = std::max(float(0), std::min(power_alfen, power_pv - pv_to_house));

            // Whatever is left, send it to the battery (always 0 or larger)
            float pv_to_battery = power_pv - pv_to_house - pv_to_alfen;

            // Only the car power that is not covered by PV should be taken from the grid
            wanted_from_grid = power_alfen - pv_to_alfen;
        }
        else if (strategy == STRATEGY_GRID_ONLY)
        {
            // All car power should come from the grid
            wanted_from_grid = power_alfen;
        }

        // Fool the SolarEdge: report a reduced (net) power so it keeps charging/exporting
        wattnode.set<WattNode::power_active>( engineering<EM24_E1::power_active>(meter) - wanted_from_grid);
        wattnode.set<WattNode::l1_power_active>( engineering<EM24_E1::l1_power_active>(meter) - wanted_from_grid / 3);
        wattnode.set<WattNode::l2_power_active>( engineering<EM24_E1::l2_power_active>(meter) - wanted_from_grid / 3);
        wattnode.set<WattNode::l3_power_active>( engineering<EM24_E1::l3_power_active>(meter) - wanted_from_grid / 3);

        // Log message
        char buffer[400];
        sprintf(buffer, "s=%i, cn=%.1f wanted_from_grid=%.1f power_alfen=%.1f power_net=%.1f",
                strategy, power_net - wanted_from_grid, wanted_from_grid, power_alfen, power_net);
        optimizer.logMessage(buffer);
    }
}
