// SurfaceMap.h -- layout and local stand-ins for the SurfaceMap class (0x005051f0..0x00505350).
//
// RTTI .?AVSurfaceMap@@ (COL 0x0055f2e0), single base BaseObject (mdisp 0), vtable 0x00558248 with
// BaseObject's 4 slots and an override only of slot 0 (0x005052a0, scalar deleting dtor).  The vptr is
// written at 0x00505220 (ctor) and 0x0050536d (dtor).
//
// Ownership is uncertain: the code sits between SteeringControl's __FILE__ xref (0x00504bd2) and
// Terrain.cpp's first (0x0050567c) but references no __FILE__ string, and it is not a method of
// SteeringControl, so it is kept in samples/ rather than promoted.
#ifndef SAMPLES_MOTION_SURFACEMAP_H
#define SAMPLES_MOTION_SURFACEMAP_H

#include "core/GameObject.h"

// 17 dwords copied at +0x0c..+0x4f.  Tier 2: the size and the 0x40-byte-offset SetMaterial call
// (device vtable slot 16, `[eax+0x40]`) are the layout of D3DMATERIAL7: diffuse, ambient, specular,
// emissive colours then a power float.  The defaults written by the ctor (eight 1.0f then zeros) are
// white diffuse and ambient, black specular and emissive and power 0.
struct SurfaceMaterial {
    float diffuse[4];
    float ambient[4];
    float specular[4];
    float emissive[4];
    float power;
};

// Texture-like object held at +0x08: reference counted (BaseObject slots 1 and 2 are called),
// and slot 19 (vtable +0x4c, no arguments) is tail-called by SurfaceMap::Select.  Tier 3 stand-in.
class SurfaceTexture : public BaseObject {
public:
    virtual void SurfaceTextureSlot4();
    virtual void SurfaceTextureSlot5();
    virtual void SurfaceTextureSlot6();
    virtual void SurfaceTextureSlot7();
    virtual void SurfaceTextureSlot8();
    virtual void SurfaceTextureSlot9();
    virtual void SurfaceTextureSlot10();
    virtual void SurfaceTextureSlot11();
    virtual void SurfaceTextureSlot12();
    virtual void SurfaceTextureSlot13();
    virtual void SurfaceTextureSlot14();
    virtual void SurfaceTextureSlot15();
    virtual void SurfaceTextureSlot16();
    virtual void SurfaceTextureSlot17();
    virtual void SurfaceTextureSlot18();
    virtual void Bind();   // slot 19
};

// COM-style device (IDirect3DDevice7 shaped: stdcall, `this` pushed).  Slot 16 is SetMaterial (+0x40).
class SurfaceDevice {
public:
    virtual long __stdcall Slot0();
    virtual long __stdcall Slot1();
    virtual long __stdcall Slot2();
    virtual long __stdcall Slot3();
    virtual long __stdcall Slot4();
    virtual long __stdcall Slot5();
    virtual long __stdcall Slot6();
    virtual long __stdcall Slot7();
    virtual long __stdcall Slot8();
    virtual long __stdcall Slot9();
    virtual long __stdcall Slot10();
    virtual long __stdcall Slot11();
    virtual long __stdcall Slot12();
    virtual long __stdcall Slot13();
    virtual long __stdcall Slot14();
    virtual long __stdcall Slot15();
    virtual long __stdcall SetMaterial(const SurfaceMaterial* material);
};

// The object at game +0x10 (reached from the global 0x0056e26c): holds the device at +0x50 and has
// a thiscall slot 7 (+0x1c) taking three ints that Select calls twice (0,1,1) and (0,4,1).
class SurfaceDisplay {
public:
    virtual void Slot0();
    virtual void Slot1();
    virtual void Slot2();
    virtual void Slot3();
    virtual void Slot4();
    virtual void Slot5();
    virtual void Slot6();
    virtual void Slot7(int a, int b, int c);
    char field_0x04[0x4c];
    SurfaceDevice* device;   // +0x50 target of the SetMaterial call
};

class SurfaceGame {
public:
    char field_0x00[0x10];
    SurfaceDisplay* display;   // +0x10
};

extern SurfaceGame* g_pGame;   // 0x0056e26c (g_TrackGame in src/reconstructed, a TrackGame*)

// Local copy of BaseObject's shape with the out-of-line constructor (0x00405120, called from
// SurfaceMap's ctor) declared.  core/GameObject.h's BaseObject has no declared constructor, so a
// class derived from it cannot emit that call; the layout and vtable shape are identical.
class SurfaceBaseObject {
public:
    SurfaceBaseObject();
    virtual ~SurfaceBaseObject();
    virtual void BaseObjectVirtualSlot1();
    virtual void BaseObjectVirtualSlot2();
    virtual int BaseObjectVirtualSlot3();
    int refCount;  // +0x04
};

class SurfaceMap : public SurfaceBaseObject {
public:
    SurfaceMap(SurfaceTexture* tex, const SurfaceMaterial* mat);
    virtual ~SurfaceMap();
    void SetTexture(SurfaceTexture* tex);
    int Apply();
    void Select();

    SurfaceTexture* texture;   // +0x08 add-ref'd by ctor/SetTexture, released by the dtor and SetTexture
    SurfaceMaterial material;  // +0x0c 17 dwords (D3DMATERIAL7 shaped), pushed to the device by Apply
};

#endif
