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
#include "em24_e1.h"
#include "wattnode.h"
#include "optimizer.h"
#include "solaredge.h"
#include "alfen.h"
#include "server.h"
#include "client.h"
namespace modbus_gateway
{
    class ConvertEM24_E1ToWattNode
    {
    public:
        ConvertEM24_E1ToWattNode(modbus_gateway::Client<EM24_E1> &meter, modbus_gateway::Server<WattNode> &wattnode, modbus_gateway::Server<Optimizer> &optimizer, modbus_gateway::Client<SolarEdge> &solaredge, modbus_gateway::Client<Alfen> &alfen)
            : _meter(meter), _wattnode(wattnode), _optimizer(optimizer), _solaredge(solaredge), _alfen(alfen)
        {
        }

        void CopyDataFromMasterToSlave();

    private:
        modbus_gateway::Client<EM24_E1> &_meter;
        modbus_gateway::Server<WattNode> &_wattnode;
        modbus_gateway::Server<Optimizer> &_optimizer;
        modbus_gateway::Client<SolarEdge> &_solaredge;
        modbus_gateway::Client<Alfen> &_alfen;
    };
}
