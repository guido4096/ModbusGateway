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
inline void formatRegisterValue(char* output, std::size_t outputSize,
                                const RegisterDescription& description, uint32_t bits) {
    const double scale = static_cast<double>(getScaling(description.scaling));
    const int decimals = static_cast<int>(fractionalDigits(description.scaling));
    switch (description.type) {
    case DataType::float32:
        snprintf(output, outputSize, "%.1f", static_cast<double>(fromBits(bits)) / scale);
        break;
    case DataType::int16: {
        const int16_t value = static_cast<int16_t>(static_cast<uint16_t>(bits));
        if (decimals) {
            snprintf(output, outputSize, "%.*f", decimals, static_cast<double>(value) / scale);
        } else {
            snprintf(output, outputSize, "%d", static_cast<int>(value));
        }
        break;
    }
    case DataType::uint16: {
        const uint16_t value = static_cast<uint16_t>(bits);
        if (decimals) {
            snprintf(output, outputSize, "%.*f", decimals, static_cast<double>(value) / scale);
        } else {
            snprintf(output, outputSize, "%u", static_cast<unsigned>(value));
        }
        break;
    }
    case DataType::int32: {
        int32_t value;
        std::memcpy(&value, &bits, sizeof(value));
        if (decimals) {
            snprintf(output, outputSize, "%.*f", decimals, static_cast<double>(value) / scale);
        } else {
            snprintf(output, outputSize, "%ld", static_cast<long>(value));
        }
        break;
    }
    case DataType::uint32:
        if (decimals) {
            snprintf(output, outputSize, "%.*f", decimals, static_cast<double>(bits) / scale);
        } else {
            snprintf(output, outputSize, "%lu", static_cast<unsigned long>(bits));
        }
        break;
    }
}

template<typename Metadata>
String formatDeviceValues(const Metadata& metadata, const uint16_t* words) {
    String result;
    char buffer[96];
    for (std::size_t blockIndex = 0; blockIndex < metadata.blocks.size(); ++blockIndex) {
        const auto& block = metadata.blocks[blockIndex];
        result += "Block ";
        result += block.name;
        result += "\r\n";
        for (const auto& registerDescription : metadata.registers) {
            if (registerDescription.block != blockIndex) {
                continue;
            }
            const uint32_t bits = loadBits(words + registerDescription.storage_offset,
                                           registerDescription.words, registerDescription.high_word_first);
            formatRegisterValue(buffer, sizeof(buffer), registerDescription, bits);
            result += "  ";
            result += registerDescription.name;
            result += "=";
            result += buffer;
            if (registerDescription.unit && *registerDescription.unit) {
                result += " ";
                result += registerDescription.unit;
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

    char buffer[128];
    for (std::size_t blockIndex = 0; blockIndex < metadata.blocks.size(); ++blockIndex) {
        const auto& block = metadata.blocks[blockIndex];
        snprintf(buffer, sizeof(buffer), "  Block: %s, offset=0x%04x (%06u), numreg=%u\r\n",
                 block.name, static_cast<unsigned>(block.address),
                 static_cast<unsigned>(block.address), static_cast<unsigned>(block.words));
        output.print(buffer);
        for (const auto& registerDescription : metadata.registers) {
            if (registerDescription.block != blockIndex) {
                continue;
            }
            snprintf(buffer, sizeof(buffer), "    Register 0x%04x (%06u): ",
                     static_cast<unsigned>(registerDescription.address), static_cast<unsigned>(registerDescription.address));
            output.print(buffer);
            output.print(registerDescription.name);
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
