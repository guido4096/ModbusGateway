/**
 * @file      diagnostics.h
 * @author    Guido Jansen (guido@ngjansen.be)
 * @license   MIT
 * @copyright Copyright (c) 2025 Guido Jansen
 * @date      04-Mar-2025
 * @note      Runtime statistics and logging
 */
#pragma once
#include <Arduino.h>
#include <cstdio>
#include <array>
#include <atomic>
#include <mutex>
#include <time.h>

namespace modbus_gateway {
// Counters are atomic; sampling serializes its own interval/baseline state.
class Statistics {
public:
    // Called for every invocation of the tracked method.
    void recordCall() {
        _calls.fetch_add(1, std::memory_order_relaxed);
    }
    // Server side: a READ_HOLD_REGISTER request was served.
    void recordRead() {
        _reads.fetch_add(1, std::memory_order_relaxed);
    }
    // Server side: a WRITE (FC06/FC16) request was served.
    void recordWrite() {
        _writes.fetch_add(1, std::memory_order_relaxed);
    }
    // A server request was rejected, or a client request could not be queued.
    void recordFailure() {
        _failures.fetch_add(1, std::memory_order_relaxed);
    }
    // Client side: the request could be queued onto the TCP master.
    void recordSuccess() {
        _success.fetch_add(1, std::memory_order_relaxed);
    }
    // Client side: handleError or handleData detected a problem.
    void recordError() {
        _errors.fetch_add(1, std::memory_order_relaxed);
    }
    // Client side: handleData received a well-formed response.
    void recordDataOk() {
        _dataOk.fetch_add(1, std::memory_order_relaxed);
    }

    // Snapshot the counters and compute per-second rates over the elapsed
    // interval. Also stores the current counters as the new baseline.
    String allValuesAsString() {
        std::lock_guard<std::mutex> lock(_sampleMutex);
        const uint32_t callsTotal = _calls.load(std::memory_order_relaxed);
        const uint32_t readsTotal = _reads.load(std::memory_order_relaxed);
        const uint32_t writesTotal = _writes.load(std::memory_order_relaxed);
        const uint32_t failuresTotal = _failures.load(std::memory_order_relaxed);
        const uint32_t successTotal = _success.load(std::memory_order_relaxed);
        const uint32_t errorsTotal = _errors.load(std::memory_order_relaxed);
        const uint32_t dataOkTotal = _dataOk.load(std::memory_order_relaxed);
        uint32_t now = millis();
        uint32_t elapsedMs = now - _lastSampleMs;
        if (elapsedMs == 0) {
            elapsedMs = 1;
        }
        _lastSampleMs = now;

        const uint32_t callsInInterval = callsTotal - _lastCalls;
        const uint32_t readsInInterval = readsTotal - _lastReads;
        const uint32_t writesInInterval = writesTotal - _lastWrites;
        const uint32_t failuresInInterval = failuresTotal - _lastFailures;
        const uint32_t successInInterval = successTotal - _lastSuccess;
        const uint32_t errorsInInterval = errorsTotal - _lastErrors;
        const uint32_t dataOkInInterval = dataOkTotal - _lastDataOk;
        _lastCalls = callsTotal;
        _lastReads = readsTotal;
        _lastWrites = writesTotal;
        _lastFailures = failuresTotal;
        _lastSuccess = successTotal;
        _lastErrors = errorsTotal;
        _lastDataOk = dataOkTotal;

        float scale = 1000.0f / float(elapsedMs);

        String r;
        char buf[200];
        snprintf(buf, sizeof(buf), "interval=%lu ms\r\n", (unsigned long)elapsedMs);
        r += buf;
        snprintf(buf, sizeof(buf), "calls/s=%.2f\r\n", float(callsInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "read/s=%.2f\r\n", float(readsInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "write/s=%.2f\r\n", float(writesInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "failure/s=%.2f\r\n", float(failuresInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "success/s=%.2f\r\n", float(successInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "error/s=%.2f\r\n", float(errorsInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "dataok/s=%.2f\r\n", float(dataOkInInterval) * scale);
        r += buf;
        snprintf(buf, sizeof(buf), "totals: calls=%lu read=%lu write=%lu failure=%lu success=%lu error=%lu dataok=%lu\r\n",
                (unsigned long)callsTotal, (unsigned long)readsTotal,
                (unsigned long)writesTotal, (unsigned long)failuresTotal,
                (unsigned long)successTotal, (unsigned long)errorsTotal,
                (unsigned long)dataOkTotal);
        r += buf;
        return r;
    }

private:
    std::mutex _sampleMutex;
    std::atomic<uint32_t> _calls{0};
    std::atomic<uint32_t> _reads{0};
    std::atomic<uint32_t> _writes{0};
    std::atomic<uint32_t> _failures{0};
    std::atomic<uint32_t> _success{0};
    std::atomic<uint32_t> _errors{0};
    std::atomic<uint32_t> _dataOk{0};
    uint32_t _lastCalls = 0;
    uint32_t _lastReads = 0;
    uint32_t _lastWrites = 0;
    uint32_t _lastFailures = 0;
    uint32_t _lastSuccess = 0;
    uint32_t _lastErrors = 0;
    uint32_t _lastDataOk = 0;
    uint32_t _lastSampleMs = 0;
};

class Log {
public:
    void addString(const String& message) {
        // time() reads the local clock without waiting for NTP synchronization.
        const time_t now = time(nullptr);
        tm timeinfo{};
        char buffer[100];
        if (now >= minimum_wall_time && localtime_r(&now, &timeinfo)) {
            snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u %u:%02u:%02u: ",
                     unsigned(1900 + timeinfo.tm_year), unsigned(timeinfo.tm_mon + 1),
                     unsigned(timeinfo.tm_mday), unsigned(timeinfo.tm_hour),
                     unsigned(timeinfo.tm_min), unsigned(timeinfo.tm_sec));
        } else {
            snprintf(buffer, sizeof(buffer), "uptime=%lu ms: ", (unsigned long)millis());
        }
        String entry = buffer;
        entry += message;
        std::lock_guard<std::mutex> lock(_mutex);
        _entries[_nextEntry] = entry;
        _nextEntry = (_nextEntry + 1) % entry_capacity;
    }

    String allValuesAsString() {
        std::lock_guard<std::mutex> lock(_mutex);
        String result;
        for (std::size_t entriesAgo = 0; entriesAgo < entry_capacity; ++entriesAgo) {
            const auto entryIndex = (_nextEntry + entry_capacity - entriesAgo - 1) % entry_capacity;
            result += _entries[entryIndex];
            result += "\r\n";
        }
        return result;
    }

private:
    static constexpr time_t minimum_wall_time = 1609459200; // 2021-01-01 UTC
    static constexpr std::size_t entry_capacity = 200;
    std::size_t _nextEntry = 0;
    std::array<String, entry_capacity> _entries{};
    std::mutex _mutex;
};


} // namespace modbus_gateway
