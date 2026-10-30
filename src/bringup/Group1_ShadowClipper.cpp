// Group1_ShadowClipper.cpp -- bring-up body for D3DIMSoultreeShadow.cpp's local view of
// the visibility clipper.
//
// D3DIMShadowClipper (src/krusty2/shadow/D3DIMSoultreeShadow.cpp) is that file's view of
// the object at 0x00575a98; its TestBox is retail 0x0052f570 (thiscall, ret 0x1c), the
// function VisibilityClipper::TestBox already covers (bring-up body in
// src/bringup/Group4_Visibility.cpp).  The view forwards every argument unchanged; the
// camera view and VisibilityCamera are the same camera object (matrix at +0xec).
// The view classes are redeclared here with D3DIMSoultreeShadow.cpp's class keys so
// the mangled name stays
// ?TestBox@D3DIMShadowClipper@@QAEHPAVD3DIMShadowCameraView@@PBM11PAH2H@Z.

#include "visibility/VisibilityClipper.h"

class D3DIMShadowCameraView;

class D3DIMShadowClipper {
public:
    int TestBox(D3DIMShadowCameraView* camera, const float* matrix, const float* center,
                const float* extent, int* screenRect, int* cornersInside, int unused);
};

// 0x0052f570
int D3DIMShadowClipper::TestBox(D3DIMShadowCameraView* camera, const float* matrix,
                                const float* center, const float* extent, int* screenRect,
                                int* cornersInside, int unused)
{
    return reinterpret_cast<VisibilityClipper*>(this)->TestBox(
        reinterpret_cast<VisibilityCamera*>(camera), matrix, center, extent, screenRect,
        cornersInside, unused);
}
