/**
 * @file      em24.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      EM24 Modbus Definitions
 */
#pragma once

#include <Arduino.h>
#include <definitions.h>
#include <vector>
namespace modbus_gateway
{
    class Optimizer
    {
    public:
        static const DeviceDescription<Optimizer> &getDeviceDescription(uint16_t rtuServerId, uint32_t serialNumber);

        // All defined registers. See the definition of each register in the .cpp file
        enum e_registers
        {
            optimizer_strategy,
            last
        };
    };

}
