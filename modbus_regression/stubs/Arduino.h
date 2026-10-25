#pragma once
#include <string>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <type_traits>
class String : public std::string {
public:
    using std::string::string;
    String(const std::string& value) : std::string(value) {}
    template<class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    explicit String(T value) : std::string(std::to_string(value)) {}
};
inline uint32_t native_now = 0;
inline uint32_t millis(){return native_now;}
inline bool getLocalTime(tm* t){ *t={}; return true;}
struct IPAddress { bool fromString(const char*) { return true; } };
struct SerialType { template<class... A> void begin(A...) {} template<class... A> void printf(const char*,A...){} void println(const char*){} template<class T> void print(const T&){} };
inline SerialType Serial;

inline SerialType Serial2;
constexpr int SERIAL_8N1 = 0;
