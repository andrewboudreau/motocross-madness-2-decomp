#include "Camera.h"

// 0x004a1410: fills a local 4x4 matrix with 1.0 on the diagonal and 0.0
// elsewhere and returns it by value. No source string attributes this
// function; the nearest references are Lzw.cpp (before) and
// MorphBastardModifier.cpp (after), so the file name here is descriptive only.
CameraMatrix16 UnknownFunction4a1410() {
    CameraMatrix16 result;
    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++)
            result.m[row][column] = (row == column) ? 1.0f : 0.0f;
    }
    return result;
}
