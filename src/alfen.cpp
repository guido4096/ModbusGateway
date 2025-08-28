/**
 * @file      alfen.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Alfen Modbus Definitions
 */
#include <Arduino.h>
#include <alfen.h>
#include <vector>

// Protocol for Alfen register list:
// https://eu-assets.contentstack.com/v3/assets/blt08d332658a89f766/blt64e1bba919b76c5e/IP%20for%20Alfen%20NG9xx%20platform
const modbus_gateway::DeviceDescription<modbus_gateway::Alfen> &modbus_gateway::Alfen::getDeviceDescription()
{
    static DeviceDescription<Alfen> dd =
        DeviceDescription<Alfen>::makeDD("alfen",
                                         {{"dynamic", 320, {
                                                               {current_l1, DataType::float32, "Current Phase L1", "A", Scaling::none, Value::_float32_t(0), false},
                                                               {current_l2, DataType::float32, "Current Phase L2", "A", Scaling::none, Value::_float32_t(0), false},
                                                               {current_l3, DataType::float32, "Current Phase L3", "A", Scaling::none, Value::_float32_t(0), false},
                                                               {current_sum, DataType::float32, "Current Sum", "W", Scaling::none, Value::_float32_t(0), false},
                                                               {power_factor_l1, DataType::float32, "Power Factor L1", "", Scaling::none, Value::_float32_t(0), false},
                                                               {power_factor_l2, DataType::float32, "Power Factor L2", "", Scaling::none, Value::_float32_t(0), false},
                                                               {power_factor_l3, DataType::float32, "Power Factor L3", "", Scaling::none, Value::_float32_t(0), false},
                                                               {power_factor_sum, DataType::float32, "Power Factor Sum", "", Scaling::none, Value::_float32_t(0), false},
                                                               {frequency, DataType::float32, "Frequency", "Hz", Scaling::none, Value::_float32_t(0), false},
                                                               {real_power_l1, DataType::float32, "Real Power Phase L1", "W", Scaling::none, Value::_float32_t(0), false},
                                                               {real_power_l2, DataType::float32, "Real Power Phase L2", "W", Scaling::none, Value::_float32_t(0), false},
                                                               {real_power_l3, DataType::float32, "Real Power Phase L3", "W", Scaling::none, Value::_float32_t(0), false},
                                                               {real_power_sum, DataType::float32, "Real Power Sum", "W", Scaling::none, Value::_float32_t(0), false},
                                                           }}});
    return dd;
}
