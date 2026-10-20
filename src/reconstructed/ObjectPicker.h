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
    float viewMatrix[3][4];                   // +0x0ac
    unsigned char field_0x0dc[0x170 - 0xdc];
    Vector3 eyePosition;                      // +0x170 (Camera::eyePosition)
    unsigned char field_0x17c[0x198 - 0x17c];
    float focalLength;                        // +0x198
    int field_0x19c;
    unsigned int viewportX;                   // +0x1a0 viewport x
    unsigned int viewportY;                   // +0x1a4 viewport y
    unsigned int viewportWidth;               // +0x1a8 viewport width
    unsigned int viewportHeight;              // +0x1ac viewport height
    unsigned char field_0x1b0[0x1c4 - 0x1b0];
    int cursorRangeX;                         // +0x1c4 cursor x range
    int cursorRangeY;                         // +0x1c8 cursor y range
};

// Use the complete shared collision class: size 0xb8, QuadTreeObject at +0,
// GraphicsTest/GameObject at +0xc. The probe accesses +0x68 and +0x88.
#include "../krusty2/collision/CollisionObject.h"
typedef CollisionCallback UnknownPickCallback;

class ObjectPicker : public GameObject {
public:
    explicit ObjectPicker(int flags);     // 0x004b01c0
    virtual ~ObjectPicker();              // 0x004b0730 (deleting wrapper 0x004b01f0)

    // 0x004b04c0: picks at the cursor's position (y counted from the bottom
    // of the render target).
    int PickAtCursor();
    int PickAt(int x, int y); // 0x004b0500: picks at (x, y)
    // 0x004b0210: builds the pick ray and the cursor (unless `cursor` is
    // given); deletes itself and returns 0 when there is no cursor.
    ObjectPicker* Create(void* target, TextureMapManager* textures, UnknownPickCallback callback,
                                        GameCursor* cursor);

    int field_0x2c;
    CollisionObject* pickRay;                 // +0x30 pick ray
    GameCursor* pickCursor;                   // +0x34  the caller's cursor or the bound cursor.tga GameCursor; PickAtCursor reads its CursorPosition
    int lastPickX;                            // +0x38 last picked x
    int lastPickY;                            // +0x3c last picked y
    UnknownControlBinding* cursorBindingX;    // +0x40 cursor x binding
    UnknownControlBinding* cursorBindingY;    // +0x44 cursor y binding
};
