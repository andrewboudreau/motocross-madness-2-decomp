#pragma once

// TrackOverlay.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\TrackOverlay.cpp",
// 0x00575290): the in-race overlays. Class names come from RTTI; every
// member and function name is provisional (docs/TRACKOVERLAY.md).

#include "Overlay.h"
#include "MatrixUtil.h"
#include "RaceView.h"

class TextureMapManager;

// 0x0050a590 (Texmap.cpp, cdecl): loads the texture file `name`; 0 on failure.
TextureMap* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                  int a4, int a5, int a6, int a7, int a8, int a9,
                                  unsigned int key, int a11, int a12);

// A rectangle built from its two horizontal and two vertical edges
// (0x00518d50, 0x00518d60). It has the same layout as UnknownOverlayRect.
struct UnknownTrackOverlayRect : public UnknownOverlayRect {
    UnknownTrackOverlayRect();                                    // 0x00518d50
    UnknownTrackOverlayRect(int left, int right, int top, int bottom); // 0x00518d60
};

// The object InstrumentOverlay+0x184 points to (its +0x3b4 is read the same
// way as KrustyBikeCamera+0x3b4); only that pointer is known.
struct UnknownInstrumentState {
    unsigned char field_0x000[0xbc];
    float field_0x0bc;                        // the gauge value before scaling
    unsigned char field_0x0c0[0x108 - 0xc0];
    bool field_0x108;
};

struct UnknownInstrumentSource {
    unsigned char field_0x000[0x3b4];
    UnknownInstrumentState* field_0x3b4;
};

// A dial needle (InstrumentOverlay+0x11c), 0x28 bytes.
struct UnknownGauge {
    int field_0x00;
    int field_0x04;
    int field_0x08;                           // centre x
    int field_0x0c;                           // centre y
    float field_0x10;                         // needle length
    float field_0x14;                         // value
    float field_0x18;                         // maximum value
    float field_0x1c;                         // value at the end of the first sweep
    float field_0x20;                         // start angle, degrees
    float field_0x24;                         // degrees per field_0x1c beyond the first sweep
};

// RTTI: InstrumentOverlay : Overlay (vtable 0x00558590), 0x188 bytes.
class InstrumentOverlay : public Overlay {
public:
    explicit InstrumentOverlay(int flags);    // 0x00518720
    virtual ~InstrumentOverlay();             // 0x00518c00 (deleting wrapper 0x00518750)
    virtual int UnknownVirtualSlot14();       // 0x00518940

    InstrumentOverlay* UnknownFunction518770(RenderTarget* target, TextureMapManager* manager, UnknownInstrumentSource* a3); // 0x00518770
    void UnknownFunction518c60(UnknownGauge* gauge, UnknownOverlayVertex* vertices); // 0x00518c60
    void UnknownFunction518cc0(UnknownGauge* gauge, UnknownOverlayVertex* vertex);   // 0x00518cc0

    UnknownGauge field_0x11c;
    UnknownOverlayVertex field_0x144[2];      // the needle, a line from tip to centre
    UnknownInstrumentSource* field_0x184;
};

// 0x00506e90 (thiscall, ret 0x18): casts the segment from->to against the
// terrain and returns nonzero on a hit (point in *out). Its class (krusty2
// Terrain) is not reconstructed under src/reconstructed.
class UnknownTerrain {
public:
    int UnknownFunction506e90(const Vector3* from, const Vector3* to, Vector3* out, int a4, int a5, int a6);
};

class Camera;

// The object at 0x00575a98. 0x0052f340 (thiscall) projects `point` with the
// camera and its matrix (+0xec); nonzero when it is on screen.
class UnknownProjector {
public:
    int UnknownFunction52f340(Camera* camera, const void* matrix, const Vector3* point,
                              Vector3* screen, float* depth);
};
extern UnknownProjector* g_UnknownGlobal575a98;

// NameOverlay+0x12c: only +0x4c (the terrain) is read.
struct UnknownNameOverlayWorld {
    unsigned char field_0x00[0x4c];
    UnknownTerrain* field_0x4c;
};

// RTTI: NameOverlay : Overlay (vtable 0x0055860c), 0x174 bytes: a rider's
// name tag that follows a point on screen and fades when it is hidden.
class NameOverlay : public Overlay {
public:
    explicit NameOverlay(int flags);          // 0x00518d80
    virtual ~NameOverlay();                   // 0x00518e20 (deleting wrapper 0x00518e00)
    virtual int UnknownVirtualSlot14();       // 0x005190e0

    NameOverlay* UnknownFunction518e30(RenderTarget* target, TextureMap* texture,
                                       const UnknownOverlayRect* rect, int a4,
                                       const UnknownOverlayRect* source, UnknownEventRacer* a6,
                                       UnknownNameOverlayWorld* a7, int a8); // 0x00518e30
    int UnknownFunction519000(const Vector3* point);                    // 0x00519000
    void UnknownFunction519080(Vector3 point);                // 0x00519080
    void UnknownFunction5190a0(Vector3 point, int a4, int a5); // 0x005190a0

    UnknownEventRacer* field_0x11c;
    int field_0x120;                          // source width
    int field_0x124;
    int field_0x128;                          // source height
    UnknownNameOverlayWorld* field_0x12c;
    int field_0x130;                          // phase of the every-fourth-frame test
    int field_0x134;                          // frame counter, 0..3
    int field_0x138;                          // last visibility result
    int field_0x13c;
    int field_0x140;
    float field_0x144;                        // fade time, 0.25
    float field_0x148;
    int field_0x14c;
    Vector3 field_0x150;
    Vector3 field_0x15c;
    int field_0x168;
    int field_0x16c;
    int field_0x170;
};

// RTTI: StatsOverlay : Overlay (vtable 0x00558680), 0x9b8 bytes: the race
// statistics panel, redrawn about once a second for the view mode
// (TrackGame +0x2d74).
class StatsOverlay : public Overlay {
public:
    explicit StatsOverlay(int flags);         // 0x00519370
    virtual ~StatsOverlay();                  // 0x00519420 (deleting wrapper 0x00519400)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00519900
    virtual int UnknownVirtualSlot13();       // 0x00519940
    virtual int UnknownVirtualSlot14();       // 0x00519980

    int UnknownFunction519880(int value);     // 0x00519880
    int UnknownFunction5198a0();              // 0x005198a0
    int UnknownFunction519a20();              // 0x00519a20
    void UnknownFunction519e10();             // 0x00519e10
    int UnknownFunction519ef0();              // 0x00519ef0
    void UnknownFunction51a480();             // 0x0051a480
    int UnknownFunction51a560();              // 0x0051a560
    int UnknownFunction51aa40();              // 0x0051aa40

    void* field_0x11c;                        // GDI object (DeleteObject)
    void* field_0x120;                        // GDI object (DeleteObject)
    int field_0x124;
    int field_0x128;
    float field_0x12c;                        // seconds since the last redraw
    UnknownTrackOverlayRect field_0x130[8];
    char field_0x1b0[0x400];
    char field_0x5b0[0x400];
    int field_0x9b0;
    int field_0x9b4;
};

// 0x005199f0 (cdecl): qsort comparator for UnknownEventScore records
// (EventManager.h), larger value first.
int UnknownFunction5199f0(const void* a, const void* b);

// RTTI: DropTextOverlay : GameObject (vtable 0x005586f0), 0xc8 bytes: one
// line of text drawn with GDI and a one-pixel black drop shadow.
class DropTextOverlay : public GameObject {
public:
    explicit DropTextOverlay(int flags);      // 0x0051ae80
    virtual ~DropTextOverlay();               // 0x0051aee0 (deleting wrapper 0x0051aec0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0051b070 (not reconstructed)
    virtual int UnknownVirtualSlot15();       // 0x0051b0f0

    void UnknownFunction51b1f0();             // 0x0051b1f0: shows the text again

    void* field_0x2c;                         // font (DeleteObject)
    float field_0x30;                         // 1.0
    int field_0x34;                           // shown
    int field_0x38;
    int field_0x3c;                           // text x
    int field_0x40;
    int field_0x44;                           // text y
    char field_0x48[0x80];                    // text
};
