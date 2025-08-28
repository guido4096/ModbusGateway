/**
 * @file      alfen.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Alfen Modbus Definitions
 */
#pragma once

#include <Arduino.h>
#include <definitions.h>
#include <vector>
namespace modbus_gateway
{
    class Alfen
    {
    public:
        static const DeviceDescription<Alfen> &getDeviceDescription();

        // All defined registers. See the definition of each register in the .cpp file
        enum e_registers
        {
            current_l1,
            current_l2,
            current_l3,
            current_sum,
            power_factor_l1,
            power_factor_l2,
            power_factor_l3,
            power_factor_sum,
            frequency,
            real_power_l1,
            real_power_l2,
            real_power_l3,
            real_power_sum,
            last
        };
    };

}
