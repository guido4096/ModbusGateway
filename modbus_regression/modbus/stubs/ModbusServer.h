#pragma once
#include "Transport.h"
class ModbusServer {
public:
    using Worker = std::function<ModbusMessage(ModbusMessage)>;
    std::map<std::pair<uint8_t, uint8_t>, Worker> workers;
    void registerWorker(uint8_t id, uint8_t fc, Worker worker) { workers[{id, fc}] = worker; }
    bool unregisterWorker(uint8_t id, uint8_t fc) { return workers.erase({id, fc}); }
    bool isServerFor(uint8_t id, uint8_t fc) { return workers.count({id, fc}); }
    ModbusMessage request(ModbusMessage request) {
        return workers.at({request.getServerID(), request.getFunctionCode()})(request);
    }
};
