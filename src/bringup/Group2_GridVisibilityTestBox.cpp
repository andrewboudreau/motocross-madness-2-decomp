// Group2_GridVisibilityTestBox.cpp -- link bring-up: Griddraw.h's GridVisibilityClipper
// view of the visibility clipper's box test, retail 0x0052f570 (thiscall, ret 0x1c).
//
// Retail has one box test and one corner array (0x0068abc8); VisibilityClipper::TestBox in
// src/bringup/Group4_Visibility.cpp is the bring-up body for it, checked against the
// retail disassembly (centre trivial accept, the eight corner outcodes, the shared-bit
// reject that zeroes the rect, the projection, the clamp to the viewport and the offset by
// its origin).  This file used to carry a second, equivalent copy of that body with its own
// corner array; it now forwards every argument unchanged, as Group1_ShadowClipper.cpp does
// for D3DIMSoultreeShadow.cpp's view.  The camera is the Camera (Camera.h; matrix at +0xec,
// viewport at +0x1a0..+0x1ac).  The seventh argument is an unused pointer.  Griddraw.h keeps its own declaration: Terrain.cpp's exact Render3D reaches
// Griddraw.h and is sensitive to its declaration count.
//
// The linked body rounds its six screen-rectangle values through FistpFloatToIntChecked
// compiled with /QIfist (Group4_Visibility.cpp; 516 against retail's 464 instructions,
// down from 836 with the former X87FloatToInt emulation).
#include "../reconstructed/Griddraw.h"
#include "visibility/VisibilityClipper.h"

// 0x0052f570
int GridVisibilityClipper::TestBox(Camera* camera, const float* matrix, const float* center,
                                   const float* extent, int* screenRect, int* cornersInside,
                                   int* unused)
{
    return reinterpret_cast<VisibilityClipper*>(this)->TestBox(
        camera, matrix, center, extent, screenRect,
        cornersInside, unused);
}
