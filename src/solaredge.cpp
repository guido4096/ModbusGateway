/**
 * @file      solaredge.cpp
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      SolarEdge Modbus Definitions
 */
#include <Arduino.h>
#include <solaredge.h>
#include <vector>

// Protocol for SolarEdge register list:
// https://knowledge-center.solaredge.com/sites/kc/files/sunspec-implementation-technical-note.pdf
const modbus_gateway::DeviceDescription<modbus_gateway::SolarEdge> &modbus_gateway::SolarEdge::getDeviceDescription()
{
    static DeviceDescription<SolarEdge> dd =
        DeviceDescription<SolarEdge>::makeDD("solaredge",
                                             {{"pv", 40083, {

                                                                {se_power, DataType::int16, "SE Power", "W", Scaling::none, Value::_int16_t(0), true},
                                                                {se_power_sf, DataType::int16, "SE Power SF", "", Scaling::none, Value::_int16_t(0), true},
                                                            }},
                                              {"battery", 57716, {

                                                                     {b1_instantaneous_power, DataType::float32, "Battery 1 Instantaneous Power", "W", Scaling::none, Value::_float32_t(0), true},
                                                                 }}}

        );
    return dd;
}
/*
~ 4Kw pv, which is used to charge the battery
TransactionID=0
Block pv
  SE Power=3899 W
  SE Power SF=-1 W
TransactionID=0
Block battery
  Battery 1 Instantaneous Power=3990.0 W
*/