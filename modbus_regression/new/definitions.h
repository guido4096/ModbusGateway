/**
 * @file      definitions.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Definitions to support formalization modbus register definitions
 */
// Compile-time schema migration: metadata is constexpr; device values remain runtime state.
#pragma once
#include <Arduino.h>
#include <array>
#include <initializer_list>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <tuple>
#include <type_traits>
#include <vector>
#include <time.h>

namespace modbus_gateway {
    // Lightweight, lock-free statistics counters used by both the Server::Process
    // method and the Client::readFromMeter method. Counters are plain integer
    // increments so the cost per request is a few CPU cycles. Rates (per second)
    // are only computed when the statistics are read.
    class Statistics
    {
    public:
        // Called for every invocation of the tracked method.
        void recordCall()
        {
            _calls++;
        }
        // Server side: a READ_HOLD_REGISTER request was served.
        void recordRead()
        {
            _reads++;
        }
        // Server side: a WRITE (FC06/FC16) request was served.
        void recordWrite()
        {
            _writes++;
        }
        // Server side: the request could not be processed (illegal data address).
        void recordFailure()
        {
            _failures++;
        }
        // Client side: the request could be queued onto the TCP master.
        void recordSuccess()
        {
            _success++;
        }
        // Client side: handleError or handleData detected a problem.
        void recordError()
        {
            _errors++;
        }
        // Client side: handleData received a well-formed response.
        void recordDataOk()
        {
            _dataOk++;
        }

        // Snapshot the counters and compute per-second rates over the elapsed
        // interval. Also stores the current counters as the new baseline.
        String allValuesAsString()
        {
            uint32_t now = millis();
            uint32_t elapsed = now - _lastSampleMs;
            if (elapsed == 0)
                elapsed = 1;
            _lastSampleMs = now;

            uint32_t calls = _calls - _lastCalls;
            uint32_t reads = _reads - _lastReads;
            uint32_t writes = _writes - _lastWrites;
            uint32_t failures = _failures - _lastFailures;
            uint32_t success = _success - _lastSuccess;
            uint32_t errors = _errors - _lastErrors;
            uint32_t dataOk = _dataOk - _lastDataOk;
            _lastCalls = _calls;
            _lastReads = _reads;
            _lastWrites = _writes;
            _lastFailures = _failures;
            _lastSuccess = _success;
            _lastErrors = _errors;
            _lastDataOk = _dataOk;

            float scale = 1000.0f / float(elapsed);

            String r;
            char buf[200];
            sprintf(buf, "interval=%lu ms\r\n", (unsigned long)elapsed);
            r += buf;
            sprintf(buf, "calls/s=%.2f\r\n", float(calls) * scale);
            r += buf;
            sprintf(buf, "read/s=%.2f\r\n", float(reads) * scale);
            r += buf;
            sprintf(buf, "write/s=%.2f\r\n", float(writes) * scale);
            r += buf;
            sprintf(buf, "failure/s=%.2f\r\n", float(failures) * scale);
            r += buf;
            sprintf(buf, "success/s=%.2f\r\n", float(success) * scale);
            r += buf;
            sprintf(buf, "error/s=%.2f\r\n", float(errors) * scale);
            r += buf;
            sprintf(buf, "dataok/s=%.2f\r\n", float(dataOk) * scale);
            r += buf;
            sprintf(buf, "totals: calls=%lu read=%lu write=%lu failure=%lu success=%lu error=%lu dataok=%lu\r\n",
                    (unsigned long)_calls, (unsigned long)_reads,
                    (unsigned long)_writes, (unsigned long)_failures,
                    (unsigned long)_success, (unsigned long)_errors,
                    (unsigned long)_dataOk);
            r += buf;
            return r;
        }

    private:
        volatile uint32_t _calls = 0;
        volatile uint32_t _reads = 0;
        volatile uint32_t _writes = 0;
        volatile uint32_t _failures = 0;
        volatile uint32_t _success = 0;
        volatile uint32_t _errors = 0;
        volatile uint32_t _dataOk = 0;
        volatile uint32_t _lastCalls = 0;
        volatile uint32_t _lastReads = 0;
        volatile uint32_t _lastWrites = 0;
        volatile uint32_t _lastFailures = 0;
        volatile uint32_t _lastSuccess = 0;
        volatile uint32_t _lastErrors = 0;
        volatile uint32_t _lastDataOk = 0;
        volatile uint32_t _lastSampleMs = 0;
    };

    class Log
    {
    public:
        Log()
        {
            _log.resize(_max);
            _current = 0;
        }
        void addString(const String &s)
        {
            String v;
            struct tm timeinfo;
            getLocalTime(&timeinfo);
            char buffer[100] = "";
            sprintf(buffer, "%04u-%02u-%02u %u:%02u:%02u: ",
                    1900 + timeinfo.tm_year, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            v = buffer;
            v += s;
            _log[_current++] = v;
            if (_current >= _max)
                _current = 0;
        }

        String allValuesAsString()
        {
            String r;

            for (int i = 0; i < _max; i++)
            {
                int c = _current - i - 1;
                if (c < 0)
                    c += _max;
                r += _log[c];
                r += "\r\n";
            }
            return r;
        }

    private:
        const int _max = 200;
        int _current;
        std::vector<String> _log;
    };

enum class DataType : uint8_t { float32, int16, uint16, int32, uint32 };
enum class WordOrder : uint8_t { HighFirst, LowFirst };
enum class Scaling : uint16_t { none=1, ten=10, hundred=100, thousand=1000 };
constexpr uint16_t getScaling(Scaling s) { return static_cast<uint16_t>(s); }
constexpr uint8_t numberRegisters(DataType d) {
    return (d == DataType::int16 || d == DataType::uint16) ? 1 : 2;
}
constexpr unsigned fractionalDigits(Scaling s) {
    return s == Scaling::ten ? 1 : s == Scaling::hundred ? 2 : s == Scaling::thousand ? 3 : 0;
}

template<DataType T> struct Native;
template<> struct Native<DataType::float32> { using type = float; };
template<> struct Native<DataType::int16> { using type = int16_t; };
template<> struct Native<DataType::uint16> { using type = uint16_t; };
template<> struct Native<DataType::int32> { using type = int32_t; };
template<> struct Native<DataType::uint32> { using type = uint32_t; };

struct RegisterDescription {
    uint16_t address;
    uint16_t block;
    uint16_t word_offset;      // within the block
    uint16_t storage_offset;   // within the device's flat word storage
    uint8_t words;
    DataType type;
    Scaling scaling;
    bool high_word_first;
    const char* name;
    const char* unit;
    uint32_t default_bits;     // raw IEEE754/int bits; dynamic defaults set per instance
};
struct BlockDescription {
    const char* name;
    uint16_t address;
    uint16_t words;
    uint16_t storage_offset;
};

// Small metadata records live in flash/read-only storage; buffers live in the Device.
// No lookup loops are used when a register is identified by its enum value.
template<typename Id, std::size_t N, std::size_t B, std::size_t Words>
struct StaticDescription {
    static constexpr std::size_t word_count = Words;
    std::array<RegisterDescription, N> registers{};
    std::array<BlockDescription, B> blocks{};
    bool valid = true;
};


// C++17 value-and-type schema builder. Widths and array dimensions are
// derived from declarations, not repeated by each device.
// Every field explicitly specifies type, scaling, word order and raw default.
template<typename Id, DataType Type>
struct TypedField {
    using id_type = Id;
    static constexpr DataType data_type = Type;
    static constexpr std::size_t word_count = numberRegisters(Type);
    Id id;
    Scaling scaling;
    WordOrder order;
    const char* name;
    const char* unit;
    uint32_t default_bits;
};

template<DataType Type, typename Id>
constexpr auto field(Id id, Scaling scaling, WordOrder order,
                     const char* name, const char* unit, uint32_t default_bits) {
    return TypedField<Id, Type>{id, scaling, order, name, unit, default_bits};
}

template<typename... Fields>
struct TypedBlock {
    static constexpr std::size_t word_count = (Fields::word_count + ... + 0);
    const char* name;
    uint16_t address;
    std::tuple<Fields...> fields;
};

template<typename... Fields>
constexpr auto block(const char* name, uint16_t address, Fields... fields) {
    static_assert(sizeof...(Fields) > 0, "A Modbus block must contain fields");
    return TypedBlock<Fields...>{name, address, std::tuple<Fields...>{fields...}};
}

template<typename Id, typename... Blocks>
constexpr auto makeDeviceSchema(const Blocks&... blocks) {
    constexpr std::size_t N = static_cast<std::size_t>(Id::last);
    constexpr std::size_t B = sizeof...(Blocks);
    constexpr std::size_t Words = (Blocks::word_count + ... + 0);
    StaticDescription<Id, N, B, Words> m{};
    std::array<bool, N> assigned{};
    std::size_t blockNo = 0, storage = 0;

    auto appendBlock = [&](const auto& b) {
        std::size_t offset = 0;
        std::apply([&](const auto&... fs) {
            auto appendField = [&](const auto& f) {
                using F = std::decay_t<decltype(f)>;
                static_assert(std::is_same_v<typename F::id_type, Id>,
                              "Register ID type does not match schema");
                constexpr auto width = F::word_count;
                const auto index = static_cast<std::size_t>(f.id);
                if (index >= N || assigned[index] ||
                    uint32_t(b.address) + offset + width > 65536 ||
                    storage + offset + width > Words) {
                    m.valid = false;
                } else {
                    assigned[index] = true;
                    m.registers[index] = RegisterDescription{
                        static_cast<uint16_t>(uint32_t(b.address) + offset),
                        static_cast<uint16_t>(blockNo),
                        static_cast<uint16_t>(offset),
                        static_cast<uint16_t>(storage + offset),
                        static_cast<uint8_t>(width), F::data_type, f.scaling,
                        f.order == WordOrder::HighFirst,
                        f.name, f.unit, f.default_bits
                    };
                }
                offset += width;
            };
            (appendField(fs), ...);
        }, b.fields);
        m.blocks[blockNo] = BlockDescription{
            b.name, b.address, static_cast<uint16_t>(offset),
            static_cast<uint16_t>(storage)
        };
        storage += offset;
        ++blockNo;
    };
    (appendBlock(blocks), ...);
    if (storage != Words) m.valid = false;
    for (bool present : assigned) if (!present) m.valid = false;
    for (std::size_t i=0; i<B; ++i)
        for (std::size_t j=i+1; j<B; ++j) {
            const auto& a=m.blocks[i]; const auto& b=m.blocks[j];
            if (uint32_t(a.address)<uint32_t(b.address)+b.words &&
                uint32_t(b.address)<uint32_t(a.address)+a.words)
                m.valid=false;
        }
    return m;
}

// DeviceSpec is specialized by each concrete device. The constexpr construction
// in wattnode.h is the single source of truth for both metadata and indexing.
inline uint32_t toBits(float x) {
    static_assert(sizeof(float)==4, "IEEE754 float32 required");
    uint32_t bits; std::memcpy(&bits, &x, 4); return bits;
}
inline float fromBits(uint32_t bits) {
    float value; std::memcpy(&value, &bits, 4); return value;
}
inline uint32_t loadBits(const uint16_t* words, uint8_t count, bool highFirst) {
    if (count == 1) return words[0];
    return highFirst ? (uint32_t(words[0]) << 16) | words[1]
                     : (uint32_t(words[1]) << 16) | words[0];
}
inline void storeBits(uint16_t* words, uint8_t count, bool highFirst, uint32_t bits) {
    if (count == 1) { words[0] = uint16_t(bits); return; }
    uint16_t hi = uint16_t(bits >> 16), lo = uint16_t(bits);
    words[0] = highFirst ? hi : lo;
    words[1] = highFirst ? lo : hi;
}

template<typename T>
T decode(const uint16_t* data, const RegisterDescription& r) {
    uint32_t bits = loadBits(data, r.words, r.high_word_first);
    if constexpr (std::is_same_v<T,float>) {
        float raw = r.type == DataType::float32 ? fromBits(bits) :
                    r.type == DataType::int16 ? float(int16_t(uint16_t(bits))) :
                    r.type == DataType::uint16 ? float(uint16_t(bits)) :
                    r.type == DataType::int32 ? float(int32_t(bits)) : float(bits);
        return raw / getScaling(r.scaling);
    } else {
        // Exact integral access only; typed callers select the declared type.
        if constexpr (std::is_same_v<T,int16_t>) return int16_t(uint16_t(bits));
        else return static_cast<T>(bits);
    }
}

template<typename T>
void encode(uint16_t* data, const RegisterDescription& r, T value) {
    uint32_t bits = 0;
    if constexpr (std::is_same_v<T,float>) bits = toBits(value);
    else bits = static_cast<uint32_t>(value);
    storeBits(data, r.words, r.high_word_first, bits);
}

// Shared schema API: device headers provide only the register IDs and map.
// Inherited description<R>() is a direct array index, never a name search.
// The schema defines RegisterId and constexpr metadata; sizes are derived.
template<typename Schema> class Device;

template<typename Schema>
struct SchemaApi {
    using DeviceType = Device<Schema>;

    template<auto R>
    static constexpr const RegisterDescription& description() {
        static_assert(std::is_same_v<decltype(R), typename Schema::RegisterId>,
                      "Register ID belongs to another schema");
        constexpr auto index = static_cast<std::size_t>(R);
        static_assert(index < Schema::metadata.registers.size(), "Register index out of range");
        return Schema::metadata.registers[index];
    }
};

template<typename Schema>
class Device {
public:
    using RegisterType = typename Schema::RegisterId;
    static constexpr auto& metadata = Schema::metadata;
    static constexpr std::size_t word_count = decltype(Schema::metadata)::word_count;

    Device() {
        for (std::size_t i=0; i<metadata.registers.size(); ++i) {
            const auto& d = metadata.registers[i];
            storeBits(words_.data()+d.storage_offset, d.words,
                      d.high_word_first, d.default_bits);
        }
    }

    // The protocol server ID is instance state, independent of register layout.
    void setServerId(uint8_t id) { server_id_ = id; }
    uint8_t serverId() const { return server_id_; }

    template<RegisterType R>
    using value_type = typename Native<metadata.registers[static_cast<std::size_t>(R)].type>::type;

    template<RegisterType R>
    static constexpr const RegisterDescription& description() {
        constexpr auto i = static_cast<std::size_t>(R);
        static_assert(i < Schema::metadata.registers.size(), "Register ID out of range");
        return metadata.registers[i];
    }

    template<RegisterType R>
    value_type<R> get() const {
        constexpr auto& d = description<R>();
        return decode<value_type<R>>(words_.data()+d.storage_offset, d);
    }

    template<RegisterType R>
    void set(value_type<R> v) {
        constexpr auto& d = description<R>();
        encode(words_.data()+d.storage_offset, d, v);
    }

    // Runtime ID lookup also O(1), with bounds checked once.
    static const RegisterDescription* describe(RegisterType id) {
        auto i = static_cast<std::size_t>(id);
        return i < metadata.registers.size() ? &metadata.registers[i] : nullptr;
    }

    // Address-based protocol requests traverse blocks (B is typically small).
    // A request crossing a hole fails, rather than fabricating zeros.
    bool readHoldingRegisters(uint16_t address, uint16_t count, uint16_t* output) const {
        if (!output || count == 0 || uint32_t(address)+count > 65536) return false;
        for (uint32_t n=0; n<count; ++n) {
            auto pos = findWord(uint32_t(address)+n);
            if (pos == invalid_position) return false;
        }
        for (uint32_t n=0; n<count; ++n)
            output[n] = words_[findWord(uint32_t(address)+n)];
        return true;
    }
    bool writeHoldingRegisters(uint16_t address, uint16_t count, const uint16_t* input) {
        if (!input || count == 0 || uint32_t(address)+count > 65536) return false;
        for (uint32_t n=0; n<count; ++n)
            if (findWord(uint32_t(address)+n) == invalid_position) return false;
        for (uint32_t n=0; n<count; ++n)
            words_[findWord(uint32_t(address)+n)] = input[n];
        return true;
    }
    // Human-readable diagnostic dump, preserving the original HTTP interface.
    String allValuesAsString() const {
        String result;
        char buf[96];
        for (std::size_t block_index = 0; block_index < metadata.blocks.size(); ++block_index) {
            const auto& block = metadata.blocks[block_index];
            result += "Block ";
            result += block.name;
            result += "\r\n";
            for (const auto& r : metadata.registers) {
                if (r.block != block_index) continue;
                const uint32_t bits = loadBits(words_.data() + r.storage_offset,
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

    const uint16_t* rawWords() const { return words_.data(); }
    static constexpr const auto& blocks() { return metadata.blocks; }

private:
    static constexpr std::size_t invalid_position = static_cast<std::size_t>(-1);
    static std::size_t findWord(uint32_t address) {
        for (const auto& b : metadata.blocks)
            if (address >= b.address && address < uint32_t(b.address)+b.words)
                return b.storage_offset + address - b.address;
        return invalid_position;
    }
    std::array<uint16_t, word_count> words_{};
    uint8_t server_id_ = 0;
};

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

// Compatibility with HTTP handlers that need a String.
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

// Optional lock-scoped access. The owner must expose _Mutex and _device.
template<typename Owner>
class DataAccess {
public:
    using Schema = typename Owner::Schema;
    using RegisterType = typename Schema::RegisterId;
    explicit DataAccess(Owner& owner) : owner_(owner) { owner_._Mutex.lock(); }
    ~DataAccess() { owner_._Mutex.unlock(); }
    DataAccess(const DataAccess&) = delete;
    DataAccess& operator=(const DataAccess&) = delete;

    template<RegisterType R>
    auto get() const { return owner_._device.template get<R>(); }
    template<RegisterType R>
    void set(typename Device<Schema>::template value_type<R> v) {
        owner_._device.template set<R>(v);
    }
    String allValuesAsString() const { return owner_._device.allValuesAsString(); }
    String getStatistics() { return owner_._statistics.allValuesAsString(); }
    String getLog() { return owner_._log.allValuesAsString(); }
    void logMessage(const String& message) { owner_._log.addString(message); }
    bool readHoldingRegisters(uint16_t addr, uint16_t count, uint16_t* dst) const {
        return owner_._device.readHoldingRegisters(addr,count,dst);
    }
    bool writeHoldingRegisters(uint16_t addr, uint16_t count, const uint16_t* src) {
        return owner_._device.writeHoldingRegisters(addr,count,src);
    }
private:
    Owner& owner_;
};
} // namespace modbus_gateway
