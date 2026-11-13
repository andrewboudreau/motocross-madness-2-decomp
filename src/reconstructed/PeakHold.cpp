#include "PeakHold.h"

// cdecl 0x004bfa80: the current time stamp.
unsigned int ReadClock();

// 0x004cb670
PeakHold::PeakHold(unsigned int hold) {
    peak = 0;
    peakTime = ReadClock();
    holdTime = hold;
}

// 0x004cb690
int PeakHold::Peak() {
    if (ReadClock() - peakTime > holdTime)
        return 0;
    return peak;
}

// 0x004cb6b0
void PeakHold::Record(int value) {
    unsigned int now = ReadClock();
    if (value > peak || now - peakTime > holdTime) {
        peak = value;
        peakTime = now;
    }
}
