// Group2_GridVisibilityTestBox.cpp -- link bring-up: Griddraw.h's GridVisibilityClipper
// view of the visibility clipper's box test, retail 0x0052f570 (thiscall, ret 0x1c).
//
// Retail has one box test and one corner array (0x0068abc8); VisibilityClipper::TestBox in
// src/bringup/Group4_Visibility.cpp is the bring-up body for it, checked against the
// retail disassembly (centre trivial accept, the eight corner outcodes, the shared-bit
// reject that zeroes the rect, the projection, the clamp to the viewport and the offset by
// its origin).  This file used to carry a second, equivalent copy of that body with its own
// corner array; it now forwards every argument unchanged, as Group1_ShadowClipper.cpp does
// for D3DIMSoultreeShadow.cpp's view.  GridCamera and VisibilityCamera are the same camera
// object (matrix at +0xec, viewport at +0x1a0..+0x1ac).  The seventh argument is unused
// by the body; Griddraw.h declares it as a pointer, VisibilityClipper.h as an int.
//
// The linked body's size against retail (836 against 464 instructions) is the rounding
// shape only: retail's inline fistp helper is four instructions a site, X87FloatToInt
// inlines to about 62, and TestBox rounds six times.
#include "../reconstructed/Griddraw.h"
#include "visibility/VisibilityClipper.h"

// 0x0052f570
int GridVisibilityClipper::TestBox(GridCamera* camera, const float* matrix, const float* center,
                                   const float* extent, int* screenRect, int* cornersInside,
                                   int* a6)
{
    return reinterpret_cast<VisibilityClipper*>(this)->TestBox(
        reinterpret_cast<VisibilityCamera*>(camera), matrix, center, extent, screenRect,
        cornersInside, reinterpret_cast<int>(a6));
}
