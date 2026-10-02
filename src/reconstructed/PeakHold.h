#pragma once

// Value that holds its peak for a time (0x004cb670..0x004cb6d8; the TU is
// not established). Game.cpp constructs ten as globals with a 5000 hold for
// the slot 8 profile page. Names are provisional.
class UnknownPeakHold {
public:
    explicit UnknownPeakHold(unsigned int hold);  // 0x004cb670
    int UnknownFunction4cb690();                  // 0x004cb690: the peak, 0 once stale
    void UnknownFunction4cb6b0(int value);        // 0x004cb6b0: raise, or restart when stale

    unsigned int field_0x00;                      // time of the peak
    int field_0x04;                               // peak
    unsigned int field_0x08;                      // hold time
};
