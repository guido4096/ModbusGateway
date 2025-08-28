/**
 * @file      em24.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Optimizer Modbus Definitions
 */
#include <Arduino.h>
#include <optimizer.h>
#include <vector>

// Protocol for EM24_E1 register list:
//   https://www.enika.eu/data/files/produkty/energy%20m/CP/em24%20ethernet%20cp.pdf
const modbus_gateway::DeviceDescription<modbus_gateway::Optimizer> &modbus_gateway::Optimizer::getDeviceDescription(uint16_t rtuServerId, uint32_t serialNumber)
{
    static DeviceDescription<Optimizer> dd =
        DeviceDescription<Optimizer>::makeDD("optimizer",
                                             {{"dynamic", 0x0000, {{requested_power_from_inverter, DataType::int32, "Requested Power From Inverter", "W", Scaling::none, Value::_int32_t(0), true}}}});
    return dd;
}
