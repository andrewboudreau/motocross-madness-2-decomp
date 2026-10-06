#pragma once

// A screen rectangle as Overlay keeps it (+0xe8, +0xf8) and as
// TextQueueOverlay keeps it (+0x30, +0x40).
struct UnknownOverlayRect {
    int left;
    int top;
    int right;
    int bottom;
};

inline UnknownOverlayRect UnknownMakeOverlayRect(int left, int top, int right, int bottom)
{
    UnknownOverlayRect rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}
