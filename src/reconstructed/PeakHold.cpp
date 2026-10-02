#include "PeakHold.h"

// cdecl 0x004bfa80: the current time stamp.
unsigned int UnknownFunction4bfa80();

// 0x004cb670
UnknownPeakHold::UnknownPeakHold(unsigned int hold) {
    field_0x04 = 0;
    field_0x00 = UnknownFunction4bfa80();
    field_0x08 = hold;
}

// 0x004cb690
int UnknownPeakHold::UnknownFunction4cb690() {
    if (UnknownFunction4bfa80() - field_0x00 > field_0x08)
        return 0;
    return field_0x04;
}

// 0x004cb6b0
void UnknownPeakHold::UnknownFunction4cb6b0(int value) {
    unsigned int now = UnknownFunction4bfa80();
    if (value > field_0x04 || now - field_0x00 > field_0x08) {
        field_0x04 = value;
        field_0x00 = now;
    }
}
