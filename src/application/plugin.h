#pragma once
#include <Arduino.h>

class WebServer;

namespace modbus_gateway::plugin {
// Exactly one implementation is linked from src/plugin/ on every branch.
// setup runs after networking is ready and before the HTTP server starts.
void setup(WebServer& server);
bool loop(unsigned long now); // true when work was scheduled; the host yields
void appendRootLinks(String& output);
} // namespace modbus_gateway::plugin
