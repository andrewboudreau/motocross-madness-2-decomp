// Input-device storage read by DeviceSetup.cpp. Retail values and extents are
// confirmed; these names and the original defining translation unit are not.

// 0x00568df8: TranslatePc98ScanCode accepts indices 0..255; -1 means
// retain the original code. Used for keyboard subtype 0x62 outside NT.
int g_Pc98ScanCodes[256] = {
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 40, 28, 116, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, -1, -1, 112, -1, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, -1, 69, 115, 52, 113, 98, 99, 100, 101, 102,
    103, 104, 105, 106, 107, -1, -1, 66, 67, 68, 64, 70, 71, 72, 73, 74,
    75, 76, 78, 80, -1, -1, -1, 82, 83, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 84, 85, 86, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    114, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 81, -1, 13, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 77, -1, -1,
    12, 26, 39, 51, 53, 96, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 79, -1, 65, -1, 97, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 62, 58, 55, -1, 59, -1, 60, -1, 63,
    61, 54, 56, 57, -1, -1, -1, -1, -1, -1, -1, 119, 120, 121, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

// 0x005691f8: lazy-detection sentinel, replaced by DetectKeyboardLayout.
int g_KeyboardSubtype = -1;

// 0x0059ada8: PE zero-fill, set by the same keyboard/platform detection.
int g_IsWindowsNT;
