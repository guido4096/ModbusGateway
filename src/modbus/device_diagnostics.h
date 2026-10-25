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
#include "modbus/register_codec.h"

namespace modbus_gateway {
template<typename Metadata>
String formatDeviceValues(const Metadata& metadata, const uint16_t* words) {
    String result;
    char buf[96];
    for (std::size_t block_index = 0; block_index < metadata.blocks.size(); ++block_index) {
        const auto& block = metadata.blocks[block_index];
        result += "Block ";
        result += block.name;
        result += "\r\n";
        for (const auto& r : metadata.registers) {
            if (r.block != block_index) continue;
            const uint32_t bits = loadBits(words + r.storage_offset,
                                           r.words, r.high_word_first);
            const double scale = static_cast<double>(getScaling(r.scaling));
            const int decimals = static_cast<int>(fractionalDigits(r.scaling));
            switch (r.type) {
            case DataType::float32:
                snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(fromBits(bits)) / scale);
                break;
            case DataType::int16: {
                const int16_t v = static_cast<int16_t>(static_cast<uint16_t>(bits));
                if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v) / scale);
                else snprintf(buf, sizeof(buf), "%d", static_cast<int>(v));
                break;
            }
            case DataType::uint16: {
                const uint16_t v = static_cast<uint16_t>(bits);
                if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v) / scale);
                else snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(v));
                break;
            }
            case DataType::int32: {
                int32_t v;
                std::memcpy(&v, &bits, sizeof(v));
                if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v) / scale);
                else snprintf(buf, sizeof(buf), "%ld", static_cast<long>(v));
                break;
            }
            case DataType::uint32:
                if (decimals) snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(bits) / scale);
                else snprintf(buf, sizeof(buf), "%lu", static_cast<unsigned long>(bits));
                break;
            }
            result += "  ";
            result += r.name;
            result += "=";
            result += buf;
            if (r.unit && *r.unit) {
                result += " ";
                result += r.unit;
            }
            result += "\r\n";
        }
    }
    return result;
}

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
