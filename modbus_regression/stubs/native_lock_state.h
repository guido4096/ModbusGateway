#pragma once
#include <cassert>
#include <mutex>

inline thread_local unsigned native_device_locks = 0;
class NativeDeviceMutex {
public:
    void lock() { mutex.lock(); ++native_device_locks; }
    void unlock() { assert(native_device_locks); --native_device_locks; mutex.unlock(); }
private:
    std::mutex mutex;
};
