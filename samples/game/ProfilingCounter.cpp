#include <windows.h>
#include "ProfilingCounter.h"

__int64 ReadProfilingCounter() {
    LARGE_INTEGER stamp;
    QueryPerformanceCounter(&stamp);
    return stamp.QuadPart;
}
