#include <log.h>
#include <stdio.h>
#include <stdarg.h>

static LogLevel    g_ll;
static const char* g_llMapStr[] = { "Verbose", "Debug", "Info", "Warn", "Error", "Panic" };

void SetLogLevel(LogLevel ll)
{
    g_ll = ll;
    Log(LL_Info, "SetLogLevel %s", g_llMapStr[ll]);
}

void Log(LogLevel ll, const char* fmt, ...)
{
    if (ll >= g_ll)
    {
        char    buf[1024];
        va_list vl;

        va_start(vl, fmt);
        int bytes = vsprintf(buf, fmt, vl);
        va_end(vl);
        buf[bytes] = '\n';
        buf[bytes + 1] = '\0';

        puts(buf);
    }
}