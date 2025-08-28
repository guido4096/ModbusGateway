/**
 * @file      solaredge.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      SolarEdge Modbus Definitions
 */
#pragma once

#include <Arduino.h>
#include <definitions.h>
#include <vector>
namespace modbus_gateway
{
    class SolarEdge
    {
    public:
        static const DeviceDescription<SolarEdge> &getDeviceDescription();

        // All defined registers. See the definition of each register in the .cpp file
        enum e_registers
        {
            se_power,
            se_power_sf,
            b1_instantaneous_power,
            last
        };
    };

}
