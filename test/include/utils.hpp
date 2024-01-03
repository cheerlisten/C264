#pragma once
#include <stdarg.h>
#include <stdio.h>

inline void Log(const char* fmt, ...)
{
    char buf[1024];
    va_list ls;
    va_start(ls, fmt);
    int msgLen = vsnprintf(buf, 1024, fmt, ls);
    buf[msgLen++] = '\n';
    buf[msgLen] = '\0';

    fprintf(stdout, buf);
}

#define CALL(e) ASSERT_EQ(0, e)