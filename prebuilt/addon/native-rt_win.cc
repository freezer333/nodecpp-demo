// Platform specific (windows)
#include <Windows.h>

double native_now()
{
    FILETIME tm;
    ULONGLONG t;
#if defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8
    /* Windows 8, Windows Server 2012 and later. ---------------- */
    GetSystemTimePreciseAsFileTime(&tm);
#else
    /* Windows 2000 and later. ---------------------------------- */
    GetSystemTimeAsFileTime(&tm);
#endif
    t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
    double time_now = (double)t / 10000000.0;
    return time_now;
}