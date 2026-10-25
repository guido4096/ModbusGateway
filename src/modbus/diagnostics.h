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
#include <vector>
#include <time.h>

namespace modbus_gateway {
    // Plain counters are inexpensive on the request path, but `volatile` does not
    // make their increments atomic. Access from multiple tasks needs external
    // synchronization if exact counts matter.
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

} // namespace modbus_gateway
