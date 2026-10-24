#pragma once
#include <string>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
using String = std::string;
inline uint32_t millis(){return 0;}
inline bool getLocalTime(tm* t){ *t={}; return true;}
struct IPAddress {};
struct SerialType { template<class... A> void printf(const char*,A...){} void println(const char*){} void print(const char*){} };
inline SerialType Serial;
