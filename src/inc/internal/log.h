#pragma once
#include <c264/config.h>

void Log(const char* fmt, ...);

#if !defined(CONFIG_FORCE_LOG_POSITION)
    #define LOG(...) Log(__VA_ARGS__)
#else
    #define LOG LOGP
#endif
#define LOGP(fmt, ...) Log(fmt " at %s:%d", __VA_ARGS__, __FILE__, __LINE__)