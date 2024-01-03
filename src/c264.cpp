#include <stdarg.h>
#include <stdio.h>

#include <c264/c264.h>
#include <c264/source.h>
#include <decoder.h>
#include <log.h>

extern "C" API int CreateDecoder(IDecoder** ppDecoder)
{
    if (ppDecoder == 0)
        return -1;
    *ppDecoder = new CDecoder;
    return 0;
}

extern "C" API int DestroryDecoder(IDecoder** ppDecoder)
{
    if (ppDecoder == 0)
        return -1;
    delete (CDecoder*)*ppDecoder;
    *ppDecoder = nullptr;
    return 0;
}

void Log(const char* fmt, ...)
{
    char buf[1024];
    va_list ls;
    va_start(ls, fmt);
    int msgLen = vsnprintf(buf, 1024, fmt, ls);
    buf[msgLen++] = '\n';
    buf[msgLen] = '\0';

    fprintf(stdout, buf);
}