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

const modbus_gateway::DeviceDescription<modbus_gateway::Optimizer> &modbus_gateway::Optimizer::getDeviceDescription(uint16_t rtuServerId, uint32_t serialNumber)
{
    static DeviceDescription<Optimizer> dd =
        DeviceDescription<Optimizer>::makeDD("optimizer",
                                             {{"strategy", 0x0000, {{optimizer_strategy, DataType::int32, "Requested Power From Inverter", "0 1 2", Scaling::none, Value::_int32_t(0), true}}}});
    return dd;
}
