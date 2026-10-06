// SoultreeQuadTreeRenderer.h -- SoultreeQuadTreeRenderer : GraphicsTest and the minimal views
// of the scene nodes it walks.  Retail file D:\aardvark\VC\krusty2\SoultreeQuadTreeRenderer.cpp
// (__FILE__ at 0x005743f4, referenced by 0x5046e0 and 0x5047f0).
//
// Evidence (tier 1 unless noted):
//  * RTTI/vtable 0x00558168 (27 slots).  It differs from GraphicsTest's vtable only in slots
//    0 (scalar deleting dtor 0x00504500, core 0x00504520), 10 (0x00504690), 12 (0x00504620)
//    and 14 (0x00504570); analysis/vtable_overrides.json lists exactly those.
//  * Ctor 0x00504490 (thiscall, ret 4): GraphicsTest(a) (0x0047bc70), vptr, then +0x34 = 0,
//    +0x38 = -1, +0x40 = 0 and +0x44 = FindTypeId("SoultreeObject") (0x00521ea0 on the
//    registry at 0x00575744).  GraphicsTest ends at 0x38 in our model, so +0x34 is the base's
//    field_0x34 and the renderer's own members start at +0x38 (tier 2).
//  * Field names are tier 3: they come from the debug-overlay labels slot 14 prints
//    ("Total Objects in scene", "Objects Visible", "Objects in list").
#ifndef SOULTREE_QUADTREE_RENDERER_H
#define SOULTREE_QUADTREE_RENDERER_H

#include "core/GraphicsTest.h"
#ifdef MCM2_PHYSICS_COMMON_SOULTREEOBJECT_H
#error "SoultreeQuadTreeRenderer.h declares its own polymorphic SoultreeObject view"
#endif
#include "collision/CollisionObject.h"   // QuadTreeObject (objectTypeId at +8)

// SoultreeObject as the renderer sees it: RTTI .?AVSoultreeObject@@ has the direct bases
// QuadTreeObject (+0) and GameObject (+12).  Only the fields the renderer reads are named; the
// flat core/SoultreeObject.h lists the same offsets (tier 1).  The class is polymorphic and
// carries the RTTI names so that dynamic_cast compiles to the same __RTDynamicCast call as
// retail and references the type descriptors .?AVSoultreeObject@@ (0x00568950) and
// .?AVD3DIMSoultreeObject@@ (0x00568970).  This TU-local view and the flat physics layout in
// core/SoultreeObject.h are alternative declarations of one retail class: never include both.
class SoultreeObject : public QuadTreeObject, public GameObject {
public:
    char pad_0x38[0x14c - 0x38];
    int field_0x14c;                  // tested by slot 12
    char pad_0x150[0x18c - 0x150];
    int subtreeDirty;                 // +0x18c (common/SoultreeObject.h); set to 1 by 0x5046e0
    int field_0x190;                  // +0x190 cleared by 0x5046e0, set to 1 by 0x5047f0
    int field_0x194;                  // +0x194 set to 1 by 0x5046e0; tested by slot 10
    int field_0x198;                  // +0x198
    void Fn_4fecd0();                 // thiscall, plain ret
    void Fn_4fed70();                 // thiscall, plain ret: releases the +0x194 entry
};

// RTTI .?AVD3DIMSoultreeObject@@ : SoultreeObject (plain inheritance).
class D3DIMSoultreeObject : public SoultreeObject {
public:
    char pad_0x19c[0x28c - 0x19c];
    int field_0x28c;                  // nonzero gates the work in 0x5046e0
    char pad_0x290[0x2d4 - 0x290];
    int field_0x2d4;                  // set to 1 by slot 12
    void Fn_440d40(int a);            // thiscall, ret 4
    void Fn_443de0(int a, int b);     // thiscall, ret 8
};

// Debug allocator iterator over a GameObject tree: heap allocated, 0x94 bytes
// (operator new(0x94, __FILE__, line) at 0x504719 and 0x504829).
class GameObjectIterator {
public:
    GameObjectIterator(GameObject* root, int mode, const char* filter);   // 0x00469950 (ret 0xc)
    ~GameObjectIterator();                                                  // 0x00469a40
    GameObject* Next();                                                     // 0x00469a50
private:
    char field_0x00[0x94];
};

// 0x56e26c: the profiler object whose +0x38 is the on-screen text log (same pair as
// visibility/VisibilityQuadTree.h; tier 3 roles, from slot 14).  The two writers are cdecl
// free functions taking the log as their first argument.
struct SoultreeStatsLog {
    char pad_0x00[0x26c0];
    int nextLineIndex;                // +0x26c0 post-incremented to hand out a line index
};
struct SoultreeStats {
    char pad_0x00[0x38];
    SoultreeStatsLog* log;            // +0x38
};
void SoultreeSetLineName(SoultreeStatsLog* log, int line, const char* name);              // 0x00447fa0
void SoultreeLogLine(SoultreeStatsLog* log, int line, const char* format, ...);           // 0x00447f40

class SoultreeQuadTreeRenderer : public GraphicsTest {
public:
    explicit SoultreeQuadTreeRenderer(int flags);                    // 0x00504490
    virtual ~SoultreeQuadTreeRenderer();                              // slot 0: deleting 0x00504500, core 0x00504520
    virtual int GameObjectVirtualSlot10(float dt);                    // 0x00504690
    virtual int GameObjectVirtualSlot12();                            // 0x00504620
    virtual int GameObjectVirtualSlot14();                            // 0x00504570

    void Fn_5046e0();    // owner: SoultreeQuadTreeRenderer.cpp (__FILE__ line 0x63)
    void Fn_5047f0();    // owner: SoultreeQuadTreeRenderer.cpp (__FILE__ line 0x7f)

    // +0x34 (GraphicsTest::field_0x34) counts the nodes 0x5046e0 switched on.
    int statsId;                      // +0x38 -1 until slot 14 takes an id from the stats table
    int visibleCount;                 // +0x3c slot 12 counts the nodes it flagged ("Objects Visible")
    int field_0x40;                   // zeroed by the ctor
    char nodeTypeId;                  // +0x44 FindTypeId("SoultreeObject")
};

typedef char renderer_assert_offset[(sizeof(SoultreeQuadTreeRenderer) >= 0x45) ? 1 : -1];

#endif
