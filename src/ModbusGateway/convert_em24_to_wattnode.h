/**
 * @file      convert_em24_to_wattnode.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Convert and copy em24 data to the wattnode slave
 */
#pragma once

#include "definitions.h"
#include "em24.h"
#include "wattnode.h"
#include "slave.h"
#include "master.h"
namespace modbus
{
    class ConvertEM24ToWattNode {
    public:

        ConvertEM24ToWattNode(modbus::Master<EM24>& meter, modbus::Slave<WattNode>& wattnode)
        : _meter(meter)
        , _wattnode(wattnode)
        {}

        void CopyDataFromMasterToSlave();

    private:       
        modbus::Master<EM24>&    _meter;
        modbus::Slave<WattNode>& _wattnode;
    };
}
