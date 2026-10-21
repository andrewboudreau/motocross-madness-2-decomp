#include <windows.h>
#include <stdio.h>
#include "ProfilingCounter.h"

int main() {
    __int64 (*readCounter)() = &ReadProfilingCounter;
    __int64 previous = 0;
    int advances = 0;
    for (int i = 0; i < 4096; ++i) {
        LARGE_INTEGER before, after;
        if (!QueryPerformanceCounter(&before))
            return 1;
        __int64 value = readCounter();
        if (!QueryPerformanceCounter(&after))
            return 2;
        if (value < before.QuadPart || value > after.QuadPart)
            return 3;
        if (i && value < previous)
            return 4;
        if (i && value > previous)
            ++advances;
        previous = value;
    }
    if (!advances)
        return 5;
    printf("4096 real QPC brackets passed; %d counter advances\n", advances);
    return 0;
}
