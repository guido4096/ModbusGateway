#pragma once
#include <Arduino.h>
#include "native_lock_state.h"
#include <functional>
#include <map>

class WebServer {
public:
    void on(const char* route, std::function<void()> handler) { routes[route] = handler; }
    void send(int status, const char* type, const char* body) {
        assert(native_device_locks == 0);
        code = status;
        content_type = type;
        response = body;
    }
    std::map<String, std::function<void()>> routes;
    int code = 0;
    String content_type;
    String response;
};
