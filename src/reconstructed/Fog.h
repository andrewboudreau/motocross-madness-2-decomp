#pragma once

// Fog.h -- the fog objects of the quarry sky. Confirmed (tier 1): RTTI
// .?AVFog@@ (COL 0x0055c058, vtable 0x005526dc), .?AVFogOff@@ (COL
// 0x0055c0a8, vtable 0x0055274c) and .?AVFogOn@@ (COL 0x0055c0f8, vtable
// 0x005527bc), each : GameObject : BaseObject, single non-virtual at mdisp 0.
// Fog overrides slots 0 (deleting destructor 0x00462650), 10, 12, 14 and 22;
// FogOff slots 8 and 14; FogOn slot 14. FogOff's and FogOn's slot 0 is the
// folded compiler-generated deleting destructor 0x0048bce0.
//
// Code 0x00462620..0x00462ed5, after the vector set XCU 109-112 and before
// FollowCam.cpp (first xref 0x0046315a). No __FILE__ literal: the file name
// 'Fog.cpp' is ours (tier 3), as are the member names.
// QuarryStuntEvent.cpp's loader (lines 720, 729 and 750) creates one of each:
// the Fog draws a full-screen backdrop and sets the device fog for the
// scenery, FogOff restores the camera range and turns the fog off, and FogOn
// redraws through its Fog.

#include "GameObject.h"

struct UnknownControlEvent;
struct UnknownInputEntry;

class Fog : public GameObject {
public:
    explicit Fog(int flags);                  // 0x00462620 (ret 4)
    virtual ~Fog();                           // 0x00462670 (deleting wrapper 0x00462650)
    virtual int Tick(float frameTime);                 // 0x004627f0 (folded with BackgroundImage slot 27)
    virtual int PrepareGeometry();            // 0x00462800: camera range to the fog end
    virtual int UnknownVirtualSlot14();       // 0x00462850: backdrop and device fog
    // 0x00462c20: key 0x57 toggles "DriverInfo\<driver>\RenderFog", keys 0x21 / 0x22
    // move the fog in and out.
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);

    // 0x00462680 (ret 0x1c): slot 8 with `target`, the device fog mode, then the
    // colour, visibility and haziness. `farScale`, `nearScale` and `minimum` scale the
    // visibility into the fog range (QuarryStuntEvent passes 768, 256, 128).
    Fog* Create(void* target, unsigned int color, float visibility, float haziness,
                float farScale, float nearScale, float minimum);
    // 0x004627a0: sets the colour, visibility and haziness.
    void SetParameters(unsigned int color, float visibility, float haziness);
    void SetDetailLevel(int level);    // 0x00462db0: detail level

    // QuarryStuntEvent's debug page prints +0x2c, +0x38, +0x40 and +0x44 under the
    // labels "Fog R/G/B", "Visibility", "Haziness" and "(Table/Vertex/Range Fog)".
    unsigned int fogColor;                    // +0x2c 0x00RRGGBB; D3DRENDERSTATE_FOGCOLOR and the backdrop's vertex colour
    float fogStart;
    float fogEnd;
    float fogVisibility;                      // +0x38 SetParameters' visibility; "Visibility %f"
    float visibilityOffset;                   // keys 0x21 / 0x22 and the detail level move it
    float fogHaziness;                        // +0x40 SetParameters' haziness; "Haziness %f"
    int fogKind;                              // +0x44 fog kind: the D3DPRASTERCAPS_FOGVERTEX, FOGTABLE or FOGRANGE bit
    int renderFog;                            // "DriverInfo\<driver>\RenderFog" setting
    int drawnByFogOn;                         // set while FogOn draws it (skips the backdrop)
    float farScale;                           // scale the visibility into the fog range
    float nearScale;
    float minimumDistance;
};

class FogOff : public GameObject {
public:
    explicit FogOff(int flags);               // 0x00462e10 (ret 4)
    virtual GameObject* SetRenderContext(void* context);    // 0x00462e30 (folded)
    virtual int UnknownVirtualSlot14();       // 0x00462e50
};

class FogOn : public GameObject {
public:
    explicit FogOn(int flags);                // 0x00462e90 (ret 4)
    virtual int UnknownVirtualSlot14();       // 0x00462eb0
    GameObject* Create(void* target, Fog* fog); // 0x00486a10

    Fog* fog;                                 // the Fog it redraws
};
