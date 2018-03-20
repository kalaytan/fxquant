#if defined(_WIN32) // Win32-specific code
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include "debug.h"

namespace fx {

void debug_trace(const char* format, ...)
{
    const size_t max_length = (1024 - 1);
    char buf[max_length + 1];
    buf[max_length] = '\0';

    unsigned long pid = GetCurrentProcessId();
    unsigned long tid = GetCurrentThreadId();
    int n = _snprintf_s(buf, max_length, _TRUNCATE, "(%lu:%lu) ", pid, tid);

    if (n > 0)
    {
        va_list args;
        va_start(args, format);
        int m = _vsnprintf_s(buf + n, max_length - n, _TRUNCATE, format, args);
        va_end(args);

        size_t i = (m > 0) ? (m + n) : (max_length - 1);
        DEBUG_ASSERT(i < max_length);

        buf[i] = '\n';
        buf[i + 1] = '\0';

        OutputDebugStringA(buf);
    }
}
} // namespace fx

#endif

