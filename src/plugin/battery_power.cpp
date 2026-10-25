/**
 * @file      battery_power.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @note      Battery optimizer power adjustment
 */
#include "plugin/power.h"
#include "plugin/battery_inputs.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace modbus_gateway::plugin {
void adjustActivePower(ActivePower& power) {
    const auto input = battery_optimizer::readInputs();
    if (input.strategy == 0) return;

    // 1 reserves battery power for the house; 2 supplies all car power from grid.
    float wanted_from_grid = 0;
    if (input.strategy == 1) {
        const float power_to_ac = input.ac_power * std::pow(10, input.ac_scale_factor);
        // Inverter AC combines PV and battery power.
        float power_pv = power_to_ac + input.battery_power;
        if (power_pv < 0) power_pv = 0;
        const float power_house = std::max(0.0f, power.total + power_to_ac - input.charging_power);
        const float pv_to_house = std::min(power_pv, power_house);
        const float pv_to_car = std::max(0.0f, std::min(input.charging_power, power_pv - pv_to_house));
        wanted_from_grid = input.charging_power - pv_to_car;
    } else if (input.strategy == 2) {
        wanted_from_grid = input.charging_power;
    }

    const float power_net = power.total;
    power.total -= wanted_from_grid;
    power.l1 -= wanted_from_grid / 3;
    power.l2 -= wanted_from_grid / 3;
    power.l3 -= wanted_from_grid / 3;

    char buffer[400];
    snprintf(buffer, sizeof(buffer), "s=%i, cn=%.1f wanted_from_grid=%.1f power_alfen=%.1f power_net=%.1f",
             static_cast<int>(input.strategy), power.total, wanted_from_grid,
             input.charging_power, power_net);
    battery_optimizer::logAdjustment(buffer);
}
} // namespace modbus_gateway::plugin
