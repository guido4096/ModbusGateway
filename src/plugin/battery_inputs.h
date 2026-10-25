#pragma once
#include <cstdint>

namespace modbus_gateway::battery_optimizer {
struct Inputs {
    int32_t strategy;
    float charging_power;
    int16_t ac_power;
    int16_t ac_scale_factor;
    float battery_power;
};

// Snapshot and local logging are supplied by the plugin's transport adapter.
Inputs readInputs();
void logAdjustment(const char* message);
} // namespace modbus_gateway::battery_optimizer
