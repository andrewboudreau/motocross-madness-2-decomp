// Wrecker.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\wrecker.cpp.
// See Wrecker.h and docs/WRECKER.md.

#include "Wrecker.h"

#include <stdlib.h>

#include "DebugAlloc.h"

// Per-TU vector constants (tier 2, the shape documented in docs/LZW.md and
// src/krusty2/math/Math3D.h): wrecker.cpp's four dynamic initializers
// (.CRT$XCU 0x00566574..0x00566580, code 0x00532f00..0x0053303b) build
// (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into 0x0068ad00, 0x0068ad10,
// 0x0068ad20 and 0x0068acf0. Slot 10 (0x00530739) reads the zero vector.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// A random value in [0, 1). A macro: as an inline function, VC6 moves the
// constant factor behind the caller's multiply (retail keeps rand * 1/32768
// first, then the scale).
#define RANDOM_UNIT() (rand() * (1.0f / 32768))

Wrecker::Wrecker(int flags) : GraphicsTest(flags) {
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x50 = 0;
    field_0x54 = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x44 = 0;
    field_0xac = 0;
    field_0xb4 = 0;
    field_0xd4 = 0;
    field_0xd8 = 0;
    field_0x104 = 0;
    field_0x108 = 0;
    field_0xdc = Vector3(0.0f, 0.0f, 0.0f);
    field_0x10c = Vector3(0.0f, 0.0f, 0.0f);
    field_0x118 = 5.0f;
    field_0x120 = 0;
    field_0x124 = 0x1d;
    field_0x11c = 0;
    for (int i = 0; i < 256; i++)
        field_0x128[i] = RANDOM_UNIT();
    field_0x528 = 0;
    field_0x52c = 0;
    field_0x530 = 0;
    field_0x534 = 0;
    field_0x538 = 0;
}

Wrecker::~Wrecker() {
    if (field_0x104) DebugFree(field_0x104, __FILE__, 0x3e);
    if (field_0x108) DebugFree(field_0x108, __FILE__, 0x40);
    if (field_0x58) DebugFree(field_0x58, __FILE__, 0x43);
    if (field_0x50) DebugFree(field_0x50, __FILE__, 0x45);
    if (field_0x48) DebugFree(field_0x48, __FILE__, 0x47);
}

Wrecker* Wrecker::UnknownFunction530190(void* owner, UnknownWreckerCharacter* character, const char* name) {
    GameObject::UnknownVirtualSlot8(owner);
    UnknownFunction5301c0(character, name);
    return this;
}

void Wrecker::UnknownFunction5301c0(UnknownWreckerCharacter* character, const char* name) {
    field_0x34 = character;
    character->field_0x1a0->UnknownFunction4fc690(0, Vector3(0.0f, 120.0f, 0.0f));
    field_0x3c = new (__FILE__, 0x5b) UnknownWreckerGravityModel(1);
    field_0x3c->UnknownVirtualSlot8(field_0x18);
    AppendChild(field_0x3c, -1);
    field_0x40 = new (__FILE__, 0x5f) UnknownWreckerRigidBody(1);
    field_0x40->UnknownVirtualSlot8(field_0x18);
    AppendChild(field_0x40, -1);
    field_0x40->UnknownVirtualSlot28(field_0x34->field_0x1a0);
    field_0x3c->UnknownVirtualSlot27(field_0x40);
    field_0x38 = new (__FILE__, 0x68) UnknownWreckerCollisionModel(1);
    field_0x38->UnknownVirtualSlot8(field_0x18);
    field_0x38->UnknownFunction43b9e0(field_0x40, 1, name);
    AppendChild(field_0x38, -1);
    field_0x38->field_0xec = 0;
    field_0x38->field_0x6c = 0;
    field_0x38->field_0x70 = 0;
    field_0x38->field_0x108 = 0;
    field_0x38->field_0xf0 = 0.99f;
    field_0x38->field_0xf4 = 0.95f;
    field_0x38->field_0xf8 = 2000.0f;
    g_UnknownGlobal68aba4->UnknownFunction4dcf20(field_0x38, field_0x38->field_0x84);
    field_0x40->UnknownVirtualSlot29(200.0f);
    field_0x68 = field_0x40->field_0x178 * (1.0f / 12);
    field_0x40->field_0x190 = ZeroMatrix();
    field_0x40->field_0x1d0 = ZeroMatrix();
    field_0x40->field_0x25_bit0 = 0;
    field_0x40->field_0x148 = Vector3(0.0f, 0.0f, 0.0f);
    field_0x40->field_0x13c = Vector3(0.0f, 0.0f, 0.0f);
    field_0x34->field_0x1a0->field_0x18c = 1;
    field_0x34->UnknownFunction4a8b10("Fall02");
    UnknownFunction532150();
    field_0x40->UnknownVirtualSlot34(&Vector3(0.0f, 0.0f, 0.0f));
    field_0x40->UnknownVirtualSlot33(&Vector3(0.0f, 0.0f, 0.0f));
    field_0x40->UnknownVirtualSlot44();
    field_0xb8 = field_0x34->field_0x1a0->field_0x140;
    field_0x38->field_0xe8 = 1;
    field_0x40->field_0x2a4 = 1;
    field_0x3c->field_0x38 = 1;
    field_0xe8 = Vector3(0.0f, 0.0f, 0.0f);
    field_0xf4 = Vector3(0.0f, 0.0f, 0.0f);
    field_0x100 = 0;
}

void Wrecker::UnknownFunction5305b0(int value) {
    field_0x58 = (int*)DebugRealloc(field_0x58, field_0x5c * 4 + 4, __FILE__, 0x9d);
    field_0x58[field_0x5c] = value;
    field_0x5c++;
}

void Wrecker::UnknownFunction5305f0(int value) {
    field_0x50 = (int*)DebugRealloc(field_0x50, field_0x54 * 4 + 4, __FILE__, 0xa4);
    field_0x50[field_0x54] = value;
    field_0x54++;
}

int Wrecker::UnknownFunction530630(int value) {
    field_0x48 = (int*)DebugRealloc(field_0x48, field_0x4c * 4 + 4, __FILE__, 0xac);
    field_0x48[field_0x4c] = value;
    field_0x4c++;
    return field_0x4c - 1;
}

void Wrecker::UnknownFunction530680(UnknownWreckerCollisionObject* object) {
    field_0x60 = (UnknownWreckerCollisionObject**)DebugRealloc(field_0x60, field_0x64 * 4 + 4, __FILE__, 0xb4);
    field_0x60[field_0x64] = object;
    field_0x64++;
    field_0x38->UnknownFunction439410(object);
    object->UnknownFunction439410(field_0x38);
}

void Wrecker::UnknownFunction532020(float frameTime) {
    for (int i = 0; i < field_0x38->field_0x100; i++) {
        if (field_0x38->field_0x104[i].field_0x10) {
            Vector3 point = field_0x38->field_0x104[i].field_0x0c->UnknownFunction4fd660(field_0x38->field_0x104[i].field_0x00);
            UnknownFunction531da0(field_0x104[i], point, frameTime);
            UnknownFunction531740(field_0x104[i], point, field_0x38->field_0x104[i].field_0x14, frameTime);
        }
    }
}

void Wrecker::UnknownFunction532150() {
    Vector3 size;
    field_0x34->field_0x1a0->UnknownFunction4fe850(&field_0x40->field_0x228, &size);
    field_0x40->field_0x190(0, 0) = (size.y * size.y + size.z * size.z) * field_0x68;
    field_0x40->field_0x190(1, 1) = (size.x * size.x + size.z * size.z) * field_0x68;
    field_0x40->field_0x190(2, 2) = (size.x * size.x + size.y * size.y) * field_0x68;
    field_0x40->field_0x1d0(0, 0) = 1.0f / field_0x40->field_0x190(0, 0);
    field_0x40->field_0x1d0(1, 1) = 1.0f / field_0x40->field_0x190(1, 1);
    field_0x40->field_0x1d0(2, 2) = 1.0f / field_0x40->field_0x190(2, 2);
}

void Wrecker::UnknownFunction532220(int index) {
    field_0x38->field_0xec = 1;
    field_0x38->UnknownFunction435fe0();
    field_0x38->UnknownVirtualSlot5();
    field_0x38->field_0x6c = 1;
    field_0x38->field_0x70 = 1;
    field_0x38->field_0x108 = 1;
    field_0x34->UnknownFunction4a8b40(field_0x48[index]);
    field_0x44 = 1;
    field_0x40->field_0x25_bit0 = 0;
    field_0x40->UnknownVirtualSlot44();
    field_0x40->field_0x16c = Vector3(0.0f, 0.0f, 0.0f);
    field_0x40->field_0x154 = Vector3(0.0f, 0.0f, 0.0f);
    field_0xac = 0;
}

void Wrecker::UnknownFunction532310() {
    field_0x38->field_0xec = 1;
    field_0x38->UnknownFunction435fe0();
    field_0x38->UnknownVirtualSlot5();
    field_0x38->field_0x6c = 1;
    field_0x38->field_0x70 = 1;
    field_0x38->field_0x108 = 1;
    field_0x44 = 2;
    field_0x34->UnknownFunction4a8b80(field_0x50[(int)(RANDOM_UNIT() * field_0x54)], 0.5f);
    field_0x530 = 1;
    field_0x534 = 1;
    field_0x40->field_0x25_bit0 = 1;
    field_0x40->UnknownVirtualSlot44();
    field_0x40->field_0x154 = field_0xbc;
    field_0x40->field_0x160 = field_0xbc;
    field_0x40->field_0x16c = field_0x40->field_0x234->UnknownFunction4fd710(field_0xc8);
    for (int i = 0; i < field_0x38->field_0x100; i++)
        field_0x104[i] = Vector3(0.0f, 0.0f, 0.0f);
    field_0xac = 0;
    field_0x538 = 1;
}

void Wrecker::UnknownFunction532490() {
    field_0x44 = 2;
    field_0xd4 = field_0x50[(int)(RANDOM_UNIT() * field_0x54)];
    if (!field_0x34->field_0x34) {
        field_0x34->UnknownFunction4a8b80(field_0xd4, 0.5f);
        field_0xd4 = 0;
        if (field_0x530) {
            field_0xb4 = 1;
            field_0x538 = 0;
            field_0x530 = 0;
        } else {
            field_0x530 = 1;
        }
    }
    field_0x40->field_0x25_bit0 = 1;
    for (int i = 0; i < field_0x38->field_0x100; i++)
        field_0x104[i] = Vector3(0.0f, 0.0f, 0.0f);
    field_0x534 = 1;
}

void Wrecker::UnknownFunction5327c0() {
    if (field_0x44) {
        for (int i = 0; i < field_0x64; i++) {
            field_0x38->UnknownFunction439410(field_0x60[i]);
            field_0x60[i]->UnknownFunction439410(field_0x38);
        }
        field_0x44 = 0;
        field_0x40->field_0x25_bit0 = 0;
        field_0x38->field_0xec = 0;
        field_0x38->field_0x6c = 0;
        field_0x38->field_0x70 = 0;
        field_0x38->field_0x108 = 0;
        field_0x38->field_0x58 = 0;
        g_UnknownGlobal68aba4->UnknownFunction4dcf20(field_0x38, field_0x38->field_0x84);
        field_0xd4 = 0;
        field_0x52c = 1;
        field_0x34->field_0x1a0->UnknownFunction4fcc70();
        field_0x34->UnknownFunction4a8ac0(&field_0x34->field_0x190, 0);
        UnknownFunction532150();
    }
}

int Wrecker::UnknownVirtualSlot14() {
    return GameObject::UnknownVirtualSlot14();
}

int Wrecker::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return GameObject::UnknownVirtualSlot23(event, entry);
}

void Wrecker::UnknownFunction5328b0(UnknownWreckerSkeleton* frame, Vector3 center, Vector3 halfExtents) {
    field_0xe8 = center;
    field_0xf4 = halfExtents;
    field_0x100 = frame;
}

void Wrecker::UnknownFunction532900(Vector3 offset, int bone) {
    field_0x38->UnknownFunction43c7f0(offset, bone);
    field_0x104 = (Vector3*)DebugRealloc(field_0x104, field_0x38->field_0x100 * sizeof(Vector3), __FILE__, 0x308);
    field_0x104[field_0x38->field_0x100 - 1] = Vector3(0.0f, 0.0f, 0.0f);
    field_0x108 = (int*)DebugRealloc(field_0x108, field_0x38->field_0x100 * 4, __FILE__, 0x30b);
    field_0x108[field_0x38->field_0x100 - 1] = 0;
}
