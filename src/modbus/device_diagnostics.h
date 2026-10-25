/**
 * @file      device_diagnostics.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Modbus device description formatting
 */
#pragma once
#include <Arduino.h>
#include <cstdio>
#include "modbus/register_schema.h"

namespace modbus_gateway {
// The device name comes from Schema::name. Metadata is constexpr;
// formatting is performed only when diagnostics are requested.
// A Stream-like sink needs only print(const char*), as Arduino Print does.
template<typename Schema, typename Output>
void printDeviceDescription(Output& output) {
    const auto& metadata = Schema::metadata;
    output.print("Device: ");
    output.print(Schema::name);
    output.print("\r\n");
    output.print(metadata.valid
        ? "  No problems detected in the device description\r\n"
        : "  INVALID device description\r\n");

    char buf[128];
    for (std::size_t blockIndex = 0; blockIndex < metadata.blocks.size(); ++blockIndex) {
        const auto& block = metadata.blocks[blockIndex];
        snprintf(buf, sizeof(buf), "  Block: %s, offset=0x%04x (%06u), numreg=%u\r\n",
                 block.name, static_cast<unsigned>(block.address),
                 static_cast<unsigned>(block.address), static_cast<unsigned>(block.words));
        output.print(buf);
        for (const auto& reg : metadata.registers) {
            if (reg.block != blockIndex) continue;
            snprintf(buf, sizeof(buf), "    Register 0x%04x (%06u): ",
                     static_cast<unsigned>(reg.address), static_cast<unsigned>(reg.address));
            output.print(buf);
            output.print(reg.name);
            output.print("\r\n");
        }
        output.print("\r\n");
    }
}

// Collect the diagnostic output in an Arduino String.
template<typename Schema>
String getDeviceDescription() {
    struct StringOutput {
        String& value;
        void print(const char* text) { value += text; }
    };
    String result;
    StringOutput output{result};
    printDeviceDescription<Schema>(output);
    return result;
}

} // namespace modbus_gateway
