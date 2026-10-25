/**
 * @file      wattnode.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      WattNode Modbus Definitions used by SolarEdge
 */
#include "devices/wattnode.h"

namespace modbus_gateway {
WattNode::DeviceType WattNode::makeDevice(uint32_t serialNumber) {
    DeviceType device;
    device.set<WattNode::serial_number>(serialNumber);
    return device;
}
// Call sites may use:
//   auto device = WattNode::makeDevice(serialNumber);
//   device.set<WattNode::power_active>(1234.0f);
//   float power = device.get<WattNode::power_active>();
//   constexpr auto address = WattNode::description<WattNode::power_active>().address;
// A mutex-owning application type can wrap the same DeviceType in DataAccess<Owner>.
} // namespace modbus_gateway
