#pragma once

// Value that holds its peak for a time (0x004cb670..0x004cb6d8; the TU is
// not established). Game.cpp constructs ten as globals with a 5000 hold for
// the slot 8 profile page. Names are provisional.
class UnknownPeakHold {
public:
    explicit UnknownPeakHold(unsigned int hold);  // 0x004cb670
    int Peak();                                   // 0x004cb690: the peak, 0 once stale
    void Record(int value);                       // 0x004cb6b0: raise, or restart when stale

    unsigned int peakTime;                        // +0x00: time stamp of the peak
    int peak;                                     // +0x04: the largest value since peakTime
    unsigned int holdTime;                        // +0x08: how long the peak is held
};
