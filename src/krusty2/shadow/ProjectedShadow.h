// ProjectedShadow.h -- ProjectedShadow (D:\aardvark\VC\krusty2\ProjectedShadow.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI .?AVProjectedShadow@@ COL 0x0055ea10, vtable 0x005575c8 (object_offset 0), one direct
//    base GameObject (mdisp 0, no virtual base), base chain GameObject -> BaseObject.
//  * Overrides (vtable_overrides.json): slot 0 (deleting dtor 0x004da6c0 -> core 0x004da6e0),
//    10 0x004dafc0, 12 0x004db000, 13 0x004dc410, 23 0x004dc4c0 (the shared "return 0" stub,
//    which is reconstructed in broadphase/Terrain.cpp).
//  * Ctor 0x004da570 (ret 4: one argument forwarded to the GameObject ctor 0x00468ca0).
//  * Object size >= 0x134 (last ctor store at +0x130).
//  * __FILE__ 0x00571fd8 "D:\aardvark\VC\krusty2\ProjectedShadow.cpp".
// The class renders a projected shadow into a small texture: the ctor/dtor, the caster list,
// the DriverInfo\<name>\DestColorShadow registry probe in Init and the per-frame update.
// Member names are tier 3 and each carries the evidence for its role.
#ifndef SHADOW_PROJECTEDSHADOW_H
#define SHADOW_PROJECTEDSHADOW_H

#include "core/GameObject.h"
#include "math/FastMath.h"
#include "core/DebugAlloc.h"

// Stand-in for the shared Vec3 (the real one lives in src/krusty2/math/Math3D.h, which
// is not included here: every includer would get its static const Vec3 objects and their
// $E initializers).  The zero vector read by the ctor (0x00689b48) is one of
// ProjectedShadow.cpp's own four vector constants, defined in that file.
struct ShadowMatrix {
    float m[4][4];   // row-major, translation in row 3 (caster +0xf8 +0x30 is added by 0x004db0d0)
};
// 0x004a1410: returns a 64-byte matrix by hidden pointer (copied to a local in 0x004db0d0, then
// diagonal / translation entries are overwritten) -- identity-like.  Tier 3 name.
ShadowMatrix ShadowMatrixIdentity();
// 0x004a1860: two 64-byte matrices by value, matrix returned by hidden pointer (tier 3 name).
ShadowMatrix ShadowMatrixMultiply(ShadowMatrix a, ShadowMatrix b);

struct ShadowVec3 {
    float x, y, z;
};

// DirectDraw-style surface reached through a texture's +0x70 member (tier 2: slot 37 is
// called with (this, 0) in Init, slot 38 with (this, 0) in the dtor, which fit IDirectDrawSurface7
// PageLock / PageUnlock).  The leading slots are unknown placeholders.
class ShadowSurface {
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
    virtual long __stdcall Slot16();
    virtual long __stdcall Slot17();
    virtual long __stdcall Slot18();
    virtual long __stdcall Slot19();
    virtual long __stdcall Slot20();
    virtual long __stdcall Slot21();
    virtual long __stdcall Slot22();
    virtual long __stdcall Slot23();
    virtual long __stdcall Slot24();
    virtual long __stdcall Slot25();
    virtual long __stdcall Slot26();
    virtual long __stdcall Slot27();
    virtual long __stdcall Slot28();
    virtual long __stdcall Slot29();
    virtual long __stdcall Slot30();
    virtual long __stdcall Slot31();
    virtual long __stdcall Slot32();
    virtual long __stdcall Slot33();
    virtual long __stdcall Slot34();
    virtual long __stdcall Slot35();
    virtual long __stdcall Slot36();
    virtual long __stdcall PageLock(unsigned long flags);    // slot 37 (tier 3)
    virtual long __stdcall PageUnlock(unsigned long flags);  // slot 38 (tier 3)
};

// The shadow texture object (allocated 0x80 bytes, ctor 0x004c5f00(arg, 1)).  PROVISIONAL
// stand-in; only the members ProjectedShadow touches are declared.
class ShadowTexture : public BaseObject {
public:
    ShadowTexture(void* owner, int flag);               // 0x004c5f00
    // vtable slot 4 (0x10): called with 17 stack words, returns nonzero on success (Init).
    virtual int Create(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j,
                       int k, int l, int m, int n, int o, int p, int q);
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual int UnknownVirtualSlot8(int a, int b, int c);          // 0x20: (1, 0, 1) in Init
    virtual int UnknownVirtualSlot9(int* rect, int count);          // 0x24: (rect, -1) in slot 13
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual short* LockPixels(int a, int b, int c);                // 0x34 (slot 13): (0,0,0) in 0x004db8c0, result stored at +0x64
    virtual void UnlockPixels(int a);                              // 0x38 (slot 14): (0) at the end of 0x004db8c0
    char pad_0x08[0x14 - 0x08];
    int size;                    // +0x14 texture edge (the clip rect is 0..size-1)
    int height;                  // +0x18 copied to the render target's +0x10 by Init
    char pad_0x1c[0x70 - 0x1c];
    ShadowSurface* surface;      // +0x70
    void* pixels;                // +0x74
    char pad_0x78[0x80 - 0x78];  // allocated with operator new(0x80) in Init
};

// PROVISIONAL stand-in for the ShadowCamera-like object at ProjectedShadow+0x58 (Init
// allocates 0x220 bytes and runs the ShadowCamera ctor 0x004da520 on it).
class ShadowCameraProxy : public GameObject {
public:
    ShadowCameraProxy(int flags);                         // 0x004da520
    void SetLensScale(float scale);                       // 0x0042e930
    void SetLookAt(const ShadowVec3* eye, int a, int b, int c, float* fov);  // 0x0042e9b0 (tier 3)
    virtual int UnknownVirtualSlot27();                   // vtable +0x6c (placeholder, not called here)
    virtual void UnknownVirtualSlot28();                  // vtable +0x70 called by 0x004db0d0 after slot 29
    virtual void UnknownVirtualSlot29(ShadowVec3 center); // vtable +0x74 called by 0x004db0d0 with the bounds centre
    char pad_0x2c[0xac - 0x2c];
    ShadowMatrix matrix_0xac;                             // +0xac pushed by value by 0x004db0d0 in mode 3
    ShadowMatrix matrix_0xec;                             // +0xec pushed by value by 0x004db0d0 in mode 1
    char pad_0x12c[0x170 - 0x12c];
    ShadowVec3 eyePosition;                               // +0x170 TerrainShadow slot 28 passes its address as the first SetLookAt argument
    char pad_0x17c[0x198 - 0x17c];
    float field_0x198;                                    // +0x198 TerrainShadow slot 28 feeds it to atan2 as x (tier 3)
    char pad_0x19c[0x220 - 0x19c];                        // allocated with operator new(0x220) in Init
};

// Settings object at *0x0056e26c; slot 22 (0x58) reads a named flag.  PROVISIONAL.
class ShadowSettings {
public:
    virtual void Slot0();
    virtual void Slot1();
    virtual void Slot2();
    virtual void Slot3();
    virtual void Slot4();
    virtual void Slot5();
    virtual void Slot6();
    virtual void Slot7();
    virtual void Slot8();
    virtual void Slot9();
    virtual void Slot10();
    virtual void Slot11();
    virtual void Slot12();
    virtual void Slot13();
    virtual void Slot14();
    virtual void Slot15();
    virtual void Slot16();
    virtual void Slot17();
    virtual void Slot18();
    virtual void Slot19();
    virtual void Slot20();
    virtual void Slot21();
    virtual int QueryFlag(const char* name, int defaultValue);   // slot 22 (0x58), ret 8 (tier 3)
};
extern ShadowSettings* g_pShadowSettings;   // 0x0056e26c

// Direct3D-style device at the render target's +0x50.  PROVISIONAL: only slot 13 (0x34).
class ShadowDevice {
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
    virtual long __stdcall SetTransformLike(void* arg);   // slot 13 (0x34) (tier 3)
};

struct ShadowDisplay {
    char pad_0x00[0x4bc];
    char driverName[1];           // +0x4bc used in the DriverInfo\%s\DestColorShadow key
};

struct ShadowOwner {
    char pad_0x00[0x1a0];
    char block_0x1a0[1];          // +0x1a0 address handed to the device slot 13
};

// The render target ProjectedShadow renders into (GameObject::field_0x18, set by slot 8 in Init;
// RTTI says PCRenderTarget, see src/reconstructed/PCRenderTarget.h).  PROVISIONAL stand-in.
class ShadowRenderTarget {
public:
    virtual void Slot0();
    virtual void Slot1();
    virtual void Slot2();
    virtual void Slot3();
    virtual void Slot4();
    virtual void Slot5();
    virtual void Slot6();
    virtual void Slot7();
    virtual void Slot8();
    virtual void Slot9();
    virtual void Slot10();
    virtual void Slot11();
    virtual void Slot12();
    virtual int IsFormatSupported(int format);   // slot 13 (0x34), ret 4 (tier 3)
    void SetOwner(ShadowOwner* owner);           // 0x004e8cf0
    ShadowDisplay* display;       // +0x04
    ShadowOwner* owner;           // +0x08
    int width;                    // +0x0c texture size is written here during Init
    int height;                   // +0x10
    char pad_0x14[0x48 - 0x14];
    void* surface;                // +0x48
    char pad_0x4c[4];
    ShadowDevice* device;         // +0x50
    char pad_0x54[0x1b0 - 0x54];
    unsigned int capsA;           // +0x1b0 capability bits tested in Init (0x1, 0x10, 0x100, 0x800)
    unsigned int capsB;           // +0x1b4 (0x1, 0x10, 0x20)
    char pad_0x1b8[4];
    unsigned int capsC;           // +0x1bc (0x2000)
};

// Caster entry (pointers stored at ProjectedShadow+0x2c).  PROVISIONAL.
class ShadowCaster {
public:
    int GetVertexCount();                        // 0x00444fe0 (tier 3 name)
    int PartCount(int which);                    // 0x00445030 (tier 3 name; called with -1 in 0x004dc2b0)
    // 0x004433f0: seven stack args, callee-cleaned (ret 0x1c in retail call shape).
    void GetPart(int index, void** vertices, int* vertexCount, unsigned short** indices, int* indexCount, int c, int d);
    // 0x004fe850: writes the local-space bounds centre and half extents (3 floats each).
    void GetBounds(ShadowVec3* center, ShadowVec3* halfExtents);
    char pad_0x00[0xf8];
    ShadowMatrix world;                          // +0xf8 row-vector transform applied to the bounds by 0x004db0d0
    char pad_0x138[0x1a4 - 0x138];
    void* mesh;                                  // +0x1a4 tested non-null by 0x004dc2b0 before the part loop
    char pad_0x1a8[0x27c - 0x1a8];
    int field_0x27c;                             // +0x27c compared with -1 by 0x004db8c0 before calling 0x004fdab0
    void ReleaseParts();                         // 0x004fdab0 (tier 3 name)
};

struct ShadowRect {
    int left, top, right, bottom;
    void Set(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
};

// Objects in the receiver list at ProjectedShadow+0x12c (RTTI .?AVShadowReceiver@@ COL via
// vtable 0x005515ec: direct base GameObject, 31 slots).  Slots 14/27/28/29/30 point at the
// shared stubs 0x00468c90 (return 1), 0x004aa190 (return 0), 0x0044d710 (ret); slot 0 is the
// deleting dtor 0x004477a0 over the ~ShadowReceiver body 0x004477c0 (set vptr, jmp 0x00468d60),
// slot 8 is 0x00447780 (calls GameObject slot 8 non-virtually, returns this).  All of that is
// inline code, so it is written here in the header (VC6 emits a COMDAT copy per using TU).
// Slot 28 returns int: the stub 0x004aa190 is `xor eax,eax; ret` and the D3DIM override
// returns 0 in eax.  Names tier 3.
class ShadowReceiver : public GameObject {
public:
    explicit ShadowReceiver(int flags) : GameObject(flags) {}
    virtual ~ShadowReceiver() {}
    virtual GameObject* GameObjectVirtualSlot8(int a)
    {
        GameObject::GameObjectVirtualSlot8(a);
        return this;
    }
    virtual int GameObjectVirtualSlot14() { return 1; }
    virtual int UnknownVirtualSlot27() { return 1; }
    virtual int UnknownVirtualSlot28() { return 0; }
    virtual int UnknownVirtualSlot29() { return 1; }
    virtual void UnknownVirtualSlot30() {}
};

class ProjectedShadow : public GameObject {
public:
    explicit ProjectedShadow(int flags);            // 0x004da570
    virtual ~ProjectedShadow();                     // deleting wrapper 0x004da6c0, core 0x004da6e0
    virtual int GameObjectVirtualSlot10(float dt);  // 0x004dafc0
    virtual int GameObjectVirtualSlot12();          // 0x004db000
    virtual int GameObjectVirtualSlot13();          // 0x004dc410

    ProjectedShadow* Init(ShadowRenderTarget* target, void* textureOwner);  // 0x004da7b0
    ShadowRenderTarget* Host() const { return (ShadowRenderTarget*)field_0x18; }  // GameObject::field_0x18 set by Init
    void AddCaster(ShadowCaster* caster);           // 0x004dab00
    void ClearCasters();                            // 0x004dab90
    void AddReceiver(ShadowReceiver* receiver);       // 0x004daba0
    void ResizeVertexBuffer();                      // 0x004dac50
    void SetShadowStrength(int strength);           // 0x004dace0
    void SetShadowGrey(int level);                  // 0x004dae30
    int SetLight(int newMode, const ShadowVec3* dir, const ShadowVec3* pos);  // 0x004dae50
    void ComputeBounds();                           // 0x004db0d0
    void RenderShadow();                            // 0x004db8c0
    void Present();                                 // 0x004dbd30
    void TintCasterVertices();                           // 0x004dc2b0

    ShadowCaster** casters;       // +0x2c pointer array (dtor frees at line 0x5f; AddCaster appends)
    int casterCount;              // +0x30 number of entries in casters
    int casterCapacity;           // +0x34 allocated entries of casters
    ShadowVec3 lightDir;          // +0x38 ctor zero vector; 0x004dae50 stores the normalised direction
    ShadowVec3 lightVec2;         // +0x44 ctor zero vector; 0x004dae50 copies its 3rd argument
    int field_0x50;               // +0x50 ctor 0
    ShadowTexture* texture;       // +0x54 created in Init, released by dtor
    ShadowCameraProxy* camera;    // +0x58 created in Init, released by dtor
    int field_0x5c;               // +0x5c Init stores its first argument
    void* field_0x60;             // +0x60 Init stores texture->pixels
    short* heightMap;             // +0x64 ctor 0
    ShadowRect clipRect;          // +0x68 left/top/right/bottom ints (ctor 0, Init 0,0,size-1,size-1)
    int prevMinX;                 // +0x78 ctor 0x7fffffff
    int prevMinY;                 // +0x7c ctor 0x7fffffff
    int prevMaxX;                 // +0x80 ctor 0x80000000
    int prevMaxY;                 // +0x84 ctor 0x80000000
    int sizeShift;                // +0x88 Init adds 6 (log2 of 0x40) to it
    int mode;                     // +0x8c ctor 1; 0x004dae50 stores its first argument
    float minDepth;               // +0x90 0x004db8c0 resets it to FLT_MAX, then keeps the smallest projected z
    short field_0x94;             // +0x94 quantised shadow alpha / colour key word (SetShadowStrength)
    char pad_0x96[2];
    int field_0x98;               // +0x98 ctor 1
    int formatA;                  // +0x9c Init selects 9/5/1/0xc from the device caps
    int formatB;                  // +0xa0
    int vertexCapacity;           // +0xa4 allocated vertex count of the buffer at +0xa8
    ShadowVec3* vertexBuffer;     // +0xa8 12 bytes per vertex (ResizeVertexBuffer)
    unsigned int shadowColor;     // +0xac ctor 0xff000000
    int surfaceFormat;            // +0xb0 Init picks 0x115c or 0x613
    unsigned short colorKeyLow;           // +0xb4
    char pad_0xb6[2];
    unsigned int colorKeyPair;           // +0xb8
    float lensScale;              // +0xbc ctor 1.0f
    ShadowVec3 halfExtents;       // +0xc0 ctor zero vector; 0x004db0d0 stores (max-min)*0.5 of the caster bounds
    ShadowVec3 center;            // +0xcc ctor zero vector; 0x004db0d0 stores (max+min)*0.5 of the caster bounds
    ShadowMatrix lightMatrix;     // +0xd8 0x004db0d0 stores ShadowMatrixMultiply(local, camera matrix) via rep movsd
    int frameCounter;             // +0x118 slot 10 increments it
    int updateThisFrame;          // +0x11c ctor 1; slot 10 sets 0, then 1 every updatePeriod frames
    int updatePeriod;             // +0x120 ctor 1
    int receiverCapacity;         // +0x124
    int receiverCount;            // +0x128
    ShadowReceiver** receivers;   // +0x12c
    int dirtyFlags;               // +0x130 ctor 0
};

// 0x004a1b00 / 0x004a1a50: transform `count` points from src (srcStride bytes apart) by the matrix
// into dst (dstStride bytes apart).  Tier 3 names and argument roles from the call at 0x004dba87.
void ShadowTransformPointsOrtho(ShadowVec3* dst, const void* src, const ShadowMatrix* m, int count, int dstStride, int srcStride);
void ShadowTransformPointsProjective(ShadowVec3* dst, const void* src, const ShadowMatrix* m, int count, int dstStride, int srcStride);
// 0x00461e60: rasterises a triangle of 2D integer points (3 x {x,y}) into a (1<<shift)-wide
// 16-bit buffer, value = height word.  Last argument is 1 at the call site.
void ShadowFillTriangle(int shift, short* buffer, short value, int* points, int flag);

#endif
