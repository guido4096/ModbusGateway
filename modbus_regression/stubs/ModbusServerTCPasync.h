#pragma once
struct ModbusServerTCPasync {
    bool start(unsigned port, unsigned clients, unsigned timeout) {
        last_port = port; last_clients = clients; last_timeout = timeout;
        return true;
    }
    inline static unsigned last_port = 0, last_clients = 0, last_timeout = 0;
};
