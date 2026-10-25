#include "modbus/diagnostics.h"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <thread>
#include <vector>
#include <iostream>

// Interpose only this test executable's wall clock to exercise NTP-unavailable startup.
static std::atomic<time_t> clock_time{0};
extern "C" time_t time(time_t* value)
#if !defined(__APPLE__)
noexcept
#endif
{
    const time_t now = clock_time.load();
    if (value) *value = now;
    return now;
}
int main() {
    using namespace modbus_gateway;
    native_now = 42;
    Log log;
    log.addString("startup");
    assert(log.allValuesAsString().find("uptime=42 ms: startup") != String::npos);
    clock_time = 1700000000;
    log.addString("wall clock");
    assert(log.allValuesAsString().find("2023-") != String::npos);
    for (int i=0; i<205; ++i) log.addString(("entry" + std::to_string(i)).c_str());
    const auto dump = log.allValuesAsString();
    assert(dump.find("entry204") < dump.find("entry203"));
    assert(dump.find("entry0\r\n") == String::npos);
    assert(std::count(dump.begin(), dump.end(), '\n') == 200);

    Statistics stats;
    native_now = 1000;
    std::atomic<bool> done{false};
    std::thread reader([&] { while (!done.load()) { stats.allValuesAsString(); log.allValuesAsString(); } });
    std::vector<std::thread> writers;
    for (int t=0; t<4; ++t) writers.emplace_back([&] {
        for (int n=0; n<10000; ++n) {
            stats.recordCall(); stats.recordRead(); stats.recordDataOk();
            if (n < 100) log.addString("parallel");
        }
    });
    for (auto& writer : writers) writer.join();
    done = true;
    reader.join();
    auto totals = stats.allValuesAsString();
    assert(totals.find("totals: calls=40000 read=40000") != String::npos);
    assert(totals.find("dataok=40000") != String::npos);
    native_now = 1100;
    auto rates = stats.allValuesAsString();
    assert(rates.find("calls/s=0.00") != String::npos);
    std::cout << "PASS: concurrent diagnostics, ring wrap and nonblocking fallback timestamps\n";
}
