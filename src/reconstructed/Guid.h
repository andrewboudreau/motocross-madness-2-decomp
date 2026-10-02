#pragma once

// 16-byte GUID (Win32 GUID layout).
struct UnknownGuid {
    unsigned long data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char data4[8];
};
