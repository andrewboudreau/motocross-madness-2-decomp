// SoultreeQuadTreeRenderer.h -- SoultreeQuadTreeRenderer : GraphicsTest.  The minimal views of
// the scene nodes it walks are local to SoultreeQuadTreeRenderer.cpp.  Retail file D:\aardvark\VC\krusty2\SoultreeQuadTreeRenderer.cpp
// (__FILE__ at 0x005743f4, referenced by 0x5046e0 and 0x5047f0).
//
// Evidence (tier 1 unless noted):
//  * RTTI/vtable 0x00558168 (27 slots).  It differs from GraphicsTest's vtable in slots
//    0 (scalar deleting dtor 0x00504500, core 0x00504520), 8 (0x004452e0, the base-forwarding
//    body the linker shares with LightManager and D3DIMSoultreeModifier), 10 (0x00504690),
//    12 (0x00504620), 14 (0x00504570) and 23 (0x005048d0); analysis/vtable_overrides.json
//    lists exactly those.
//  * Ctor 0x00504490 (thiscall, ret 4): GraphicsTest(a) (0x0047bc70), vptr, then +0x34 = 0,
//    +0x38 = -1, +0x40 = 0 and +0x44 = FindTypeId("SoultreeObject") (0x00521ea0 on the
//    registry at 0x00575744). GraphicsTest ends at 0x34; the renderer's
//    node count starts at +0x34, followed by statsId at +0x38 (tier 2).
//  * Field names are tier 3: they come from the debug-overlay labels slot 14 prints
//    ("Total Objects in scene", "Objects Visible", "Objects in list").
#ifndef SOULTREE_QUADTREE_RENDERER_H
#define SOULTREE_QUADTREE_RENDERER_H

#include "core/GraphicsTest.h"

class SoultreeQuadTreeRenderer : public GraphicsTest {
public:
    explicit SoultreeQuadTreeRenderer(int flags);                    // 0x00504490
    virtual ~SoultreeQuadTreeRenderer();                              // slot 0: deleting 0x00504500, core 0x00504520
    virtual GameObject* SetRenderContext(void* context);              // 0x004452e0 (shared body)
    virtual int Tick(float dt);                                       // 0x00504690
    virtual int PrepareGeometry();                                    // 0x00504620
    virtual int Render3D();                            // 0x00504570
    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);                         // 0x005048d0

    void InsertNodesInQuadtree();    // owner: SoultreeQuadTreeRenderer.cpp (__FILE__ line 0x63)
    void RemoveNodesFromQuadtree();    // owner: SoultreeQuadTreeRenderer.cpp (__FILE__ line 0x7f)

    int sceneObjectCount;            // +0x34 count of nodes 0x5046e0 switched on ("Total Objects in scene")
    int statsId;                      // +0x38 -1 until slot 14 takes an id from the stats table
    int visibleCount;                 // +0x3c slot 12 counts the nodes it flagged ("Objects Visible")
    int field_0x40;                   // zeroed by the ctor
    char nodeTypeId;                  // +0x44 FindTypeId("SoultreeObject")
};

typedef char renderer_assert_offset[(sizeof(SoultreeQuadTreeRenderer) >= 0x45) ? 1 : -1];

#endif
