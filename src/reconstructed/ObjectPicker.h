#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

// Reconstruction of part of ObjectPicker.cpp (code from 0x004b01c0, after
// Nulls.cpp; see src/krusty2/effects/README.md). Evidence and the
// per-function table are in docs/OBJECTPICKER.md.
//
// Confirmed (RTTI): ObjectPicker : GameObject, single non-virtual
// inheritance; vtable 0x0055550c (COL 0x0055dad0), overriding only slot 0
// (deleting destructor 0x004b01f0).

class GameCursor;
class TextureMapManager;
struct UnknownControlBinding;

// The camera fields the pick ray reads (RenderTarget+0x08): the
// camera-to-world rows of the matrix at +0xac, the position, the image
// plane distance and the viewport (x, y, width, height). Camera.h declares
// +0x198 as int and keeps these members protected.
struct UnknownPickCamera {
    unsigned char field_0x000[0xac];
    float field_0x0ac[3][4];
    unsigned char field_0x0dc[0x170 - 0xdc];
    Vector3 field_0x170;                  // position
    unsigned char field_0x17c[0x198 - 0x17c];
    float field_0x198;
    int field_0x19c;
    unsigned int field_0x1a0;             // viewport x
    unsigned int field_0x1a4;             // viewport y
    unsigned int field_0x1a8;             // viewport width
    unsigned int field_0x1ac;             // viewport height
    unsigned char field_0x1b0[0x1c4 - 0x1b0];
    int field_0x1c4;                      // cursor x range
    int field_0x1c8;                      // cursor y range
};

// The members of CollisionObject (krusty2/collision/CollisionObject.h) that
// ObjectPicker.cpp uses: the 0xb8-byte object built by its constructor
// 0x00431e70 and used as a one-segment probe. RTTI: CollisionObject :
// QuadTreeObject (+0), GraphicsTest (+0xc) : GameObject; GraphicsTest's own
// members are not declared here.
class CollisionObject;
typedef void (*UnknownPickCallback)(CollisionObject* self, CollisionObject* other);

class UnknownPickQuadTreeObject {
public:
    virtual void UnknownVirtualSlot0();
    int field_0x04;
    int field_0x08;
};

class CollisionObject : public UnknownPickQuadTreeObject, public GameObject {
public:
    explicit CollisionObject(int flags);  // 0x00431e70
    void UnknownFunction4320f0(void* a, int b, int c, int d); // 0x004320f0
    void UnknownFunction432ab0(int count, void* points);      // 0x00432ab0: sets the segment
    void UnknownFunction435fb0();         // 0x00435fb0
    int UnknownFunction438e70();          // 0x00438e70: 1 on a hit

    unsigned char field_0x38[0x68 - 0x38];
    int field_0x68;                       // ignore vegetation
    unsigned char field_0x6c[0x88 - 0x6c];
    UnknownPickCallback field_0x88;       // on-hit callback
    unsigned char field_0x8c[0xb8 - 0x8c];
};

class ObjectPicker : public GameObject {
public:
    explicit ObjectPicker(int flags);     // 0x004b01c0
    virtual ~ObjectPicker();              // 0x004b0730 (deleting wrapper 0x004b01f0)

    // 0x004b04c0: picks at the cursor's position (y counted from the bottom
    // of the render target).
    int UnknownFunction4b04c0();
    int UnknownFunction4b0500(int x, int y); // 0x004b0500: picks at (x, y)
    // 0x004b0210: builds the pick ray and the cursor (unless `cursor` is
    // given); deletes itself and returns 0 when there is no cursor.
    ObjectPicker* UnknownFunction4b0210(void* target, TextureMapManager* textures, UnknownPickCallback callback,
                                        GameCursor* cursor);

    int field_0x2c;
    CollisionObject* field_0x30;          // pick ray
    GameCursor* field_0x34;
    int field_0x38;                       // last picked x
    int field_0x3c;                       // last picked y
    UnknownControlBinding* field_0x40;    // cursor x binding
    UnknownControlBinding* field_0x44;    // cursor y binding
};
