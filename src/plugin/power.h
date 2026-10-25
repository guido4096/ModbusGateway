#pragma once

namespace modbus_gateway::plugin {
struct ActivePower {
    float total;
    float l1;
    float l2;
    float l3;
};

// Called with the meter and WattNode locks held. Only additional input-device
// locks may be acquired here; transport and persistent-storage I/O stay outside.
void adjustActivePower(ActivePower& power);
} // namespace modbus_gateway::plugin
