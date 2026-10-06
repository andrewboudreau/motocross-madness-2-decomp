// Krusty3DObjects.cpp -- reconstruction of part of
// D:\aardvark\VC\krusty2\Krusty3DObjects.cpp. See Krusty3DObjects.h and
// docs/KRUSTY3DOBJECTS.md.

#include "Krusty3DObjects.h"

#include <stdio.h>
#include <string.h>

#include "ControlInterface.h"
#include "DebugAlloc.h"
#include "TextureMap.h"
#include "UnknownResourceManager.h"

// Per-TU vector constants: Krusty3DObjects.cpp's four dynamic initializers
// (.CRT$XCU 0x00566260..0x0056626c, code 0x0048d640..0x0048d77b) build
// (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into 0x0067c308, 0x0067c318,
// 0x0067c328 and 0x0067c2f8. Only this file's code reads them (the zero
// vector from 0x0048a986, 0x0048b837, 0x0048c97e, 0x0048d48e; the y axis
// from 0x0048b3d7 .. 0x0048cf9b).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

// Scales `v` to unit length; the zero vector stays zero.
static inline void Normalize(Vector3& v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 0.0f)
        v = kVec3Zero;
    else
        v *= UnknownFunction460c00(squared);
}

// ---------------------------------------------------------------- RunwayLights

// 0x0048a5b0
RunwayLights::RunwayLights(int flags) : GameObject(flags) {
    for (int i = 0; i < 5; i++)
        field_0x38[i] = 0;
    field_0x30 = 0;
    field_0x2c = 0;
}

// 0x0048a600
RunwayLights* RunwayLights::UnknownFunction48a600(void* value, int a, int b, UnknownRunwayTerrain* terrain) {
    char name[0x104];

    GameObject::UnknownVirtualSlot8(value);
    field_0x34 = terrain;
    sprintf(name, "%s\\%s", "Res", "pointer.slt");
    int i = 0;
    Vector3 position(0.0f, 0.0f, 0.0f);
    for (; i < 5; i++) {
        field_0x38[i] = (new (__FILE__, 0x40) ArcadeObject(1))
                            ->UnknownFunction401310(value, a, b, name, position, 0, 0, 2.0f, 0.5f, 0.5f, 0);
        if (!UnknownFunction469190(field_0x38[i], -1)) {
            Release();
            return 0;
        }
        field_0x38[i]->field_0x30 = 0;
    }
    return this;
}

// 0x0048a760
int RunwayLights::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (!field_0x30)
        return 1;
    for (int i = 0; i < 5; i++)
        field_0x38[i]->field_0x30 = 0;
    if (field_0x30->field_0x7a4)
        return 1;
    field_0x2c += frameTime;
    if (field_0x2c > 1.375f)
        field_0x2c = 0;
    if (field_0x30->field_0x78c)
        return 1;
    Vector3 direction;
    Vector3 position;
    Vector3 next;
    if (field_0x30->field_0x740->field_0x48) {
        field_0x30->field_0x740->field_0x48->UnknownFunction518080(field_0x30->field_0x744->field_0x38, &position);
        field_0x30->field_0x740->field_0x48->UnknownFunction518080(field_0x30->field_0x744->field_0x44, &next);
        direction = position - next;
    } else {
        position = field_0x30->field_0x10c;
        next = field_0x30->field_0x10c;
        direction = field_0x30->field_0x118;
    }
    if (direction.x == 0.0f && direction.z == 0.0f) {
        position = field_0x30->field_0x10c;
        next = field_0x30->field_0x10c;
        direction = field_0x30->field_0x118;
    } else {
        Normalize(direction);
    }
    if (field_0x2c > 1.125f)
        field_0x38[0]->field_0x30 = 1;
    else if (field_0x2c > 0.875f)
        field_0x38[1]->field_0x30 = 1;
    else if (field_0x2c > 0.625f)
        field_0x38[2]->field_0x30 = 1;
    else if (field_0x2c > 0.375f)
        field_0x38[3]->field_0x30 = 1;
    else if (field_0x2c > 0.125f)
        field_0x38[4]->field_0x30 = 1;
    Vector3 point;
    Vector3 normal;
    point = position + direction * 6.0f;
    field_0x34->UnknownFunction507c10(&point, &normal, 0, 0);
    point.y += 4.0f;
    field_0x38[0]->UnknownFunction4014f0(&point);
    field_0x38[0]->UnknownFunction401520(&direction, &normal, 1, 0);
    point = position + direction * 3.0f;
    field_0x34->UnknownFunction507c10(&point, &normal, 0, 0);
    point.y += 4.0f;
    field_0x38[1]->UnknownFunction4014f0(&point);
    field_0x38[1]->UnknownFunction401520(&direction, &normal, 1, 0);
    point = position;
    field_0x34->UnknownFunction507c10(&point, &normal, 0, 0);
    point.y += 4.0f;
    field_0x38[2]->UnknownFunction4014f0(&point);
    field_0x38[2]->UnknownFunction401520(&direction, &normal, 1, 0);
    point = position - direction * 3.0f;
    field_0x34->UnknownFunction507c10(&point, &normal, 0, 0);
    point.y += 4.0f;
    field_0x38[3]->UnknownFunction4014f0(&point);
    field_0x38[3]->UnknownFunction401520(&direction, &normal, 1, 0);
    point = position - direction * 6.0f;
    field_0x34->UnknownFunction507c10(&point, &normal, 0, 0);
    point.y += 4.0f;
    field_0x38[4]->UnknownFunction4014f0(&point);
    field_0x38[4]->UnknownFunction401520(&direction, &normal, 1, 0);
    return 1;
}

// 0x0048ad50
void RunwayLights::UnknownFunction48ad50(UnknownRunwayRacer* racer) {
    field_0x30 = racer;
}

// ------------------------------------------------------------------- VisualCue

// 0x0048ad60
VisualCue::VisualCue(int flags) : ArcadeObject(flags) {
    field_0xa8 = 0;
    field_0xbc = 0;
    field_0xc8 = 0;
    field_0xc0 = 0;
    field_0xc4 = -1;
    field_0xf8 = 0;
    field_0xac = 0;
    for (int i = 0; i < 11; i++)
        field_0xcc[i] = 0;
}

// 0x0048adf0
VisualCue* VisualCue::UnknownFunction48adf0(void* value, int a, int b, UnknownRunwayTerrain* terrain, Vector3 position,
                                            UnknownVisualCueView* view, int pixels, UnknownArcadeView* camera,
                                            float size, float c, float d) {
    char name[0x104];

    if (pixels && camera && c != 0.0f && d != 0.0f) {
        sprintf(name, "%s\\%s", "Res", "viscue.slt");
        if (ArcadeObject::UnknownFunction401310(value, a, b, name, position, pixels, camera, size, c, d, 0)) {
            field_0xa8 = view;
            field_0xb0 = 0;
            field_0xac = terrain;
            return this;
        }
    }
    Release();
    return 0;
}

// 0x0048af20
void VisualCue::UnknownFunction48af20(UnknownVisualCueView* view) {
    int iterator;
    UnknownVisualCueRacer* racer;

    field_0xf8 = 0;
    iterator = 0;
    field_0xa8 = view;
    while ((racer = field_0xa8->UnknownFunction4204e0(&iterator)) != 0) {
        field_0xcc[field_0xf8] = racer;
        field_0xf8++;
    }
    if (field_0xf8)
        field_0xc0++;
}

// 0x0048afa0
int VisualCue::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int active = 0;
    for (int i = 0; i < field_0xf8; i++)
        if (field_0xcc[i]->UnknownIsEnabled())
            active++;
    if (UnknownFunction43caa0(0x14, 0, event, 0x80000000) && !field_0xa8->field_0x190 &&
        field_0xa8->field_0x18a && active > 1) {
        field_0xbc = 1 - field_0xbc;
        return 1;
    }
    if (UnknownFunction43caa0(0xd2, 0, event, 0x80000000)) {
        if (++field_0xc0 >= active)
            field_0xc0 = 0;
        if (field_0xc0 == field_0xc8) {
            if (++field_0xc0 >= active)
                field_0xc0 = 0;
        }
        return 1;
    }
    if (UnknownFunction43caa0(0xd3, 0, event, 0x80000000)) {
        if (--field_0xc0 < 0)
            field_0xc0 = active - 1;
        if (field_0xc0 == field_0xc8) {
            if (--field_0xc0 < 0)
                field_0xc0 = active - 1;
        }
        return 1;
    }
    return 0;
}

// 0x0048bc10
int VisualCue::UnknownVirtualSlot14() {
    if (field_0x30)
        return ArcadeObject::UnknownVirtualSlot14();
    return 1;
}

// 0x0048bc30
int VisualCue::UnknownFunction48bc30() {
    if (field_0xc4 != field_0xc0) {
        field_0xc4 = field_0xc0;
        return field_0xc0;
    }
    if (field_0xc0 == field_0xc8) {
        if (++field_0xc0 >= field_0xf8)
            field_0xc0 = 0;
        field_0xc4 = field_0xc0;
        return field_0xc0;
    }
    return -1;
}

// 0x0048bc80
int VisualCue::UnknownFunction48bc80() {
    return field_0xbc && field_0xf8 > 1;
}

// --------------------------------------------------------- NumberObjectManager

// 0x0048bca0
NumberObjectManager::NumberObjectManager(int flags) : GameObject(flags) {
    field_0x2c = 0;
    field_0x30 = 1;
    field_0x34 = 0;
    for (int i = 0; i < 5; i++)
        field_0x38[i] = 0;
}

// 0x0048bd00
NumberObjectManager* NumberObjectManager::UnknownFunction48bd00(void* value, int b, UnknownNumberRacer* racer,
                                                                int pixels, UnknownNumberCamera* camera,
                                                                float size) {
    char name[0x104];

    GameObject::UnknownVirtualSlot8(value);
    field_0x2c = racer;
    char names[5][16] = {"one.slt", "two.slt", "three.slt", "four.slt", "five.slt"};
    if (!pixels || !camera) {
        Release();
        return 0;
    }
    field_0x34 = camera;
    UnknownTextureStream* stream = new (__FILE__, 0x23d) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    Vector3 position(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 5; i++) {
        sprintf(name, "%s\\%s", "Res", names[i]);
        field_0x38[i] = (new (__FILE__, 0x240) ArcadeObject(1))
                            ->UnknownFunction401310(value, 0, b, name, position, pixels,
                                                    (UnknownArcadeView*)camera, size, 0.5f, 0.1f, 0);
        if (!UnknownFunction469190(field_0x38[i], -1)) {
            Release();
            return 0;
        }
    }
    if (stream)
        delete stream;
    return this;
}

// The owner a NumberObjectManager compares its camera with (its +0x18).
struct UnknownNumberOwner {
    unsigned char field_0x00[0x08];
    UnknownNumberCamera* field_0x08;
};

// 0x0048bfc0
int NumberObjectManager::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    for (int i = 0; i < 5; i++)
        if (field_0x38[i])
            field_0x38[i]->field_0x30 = 0;
    if (((UnknownNumberOwner*)field_0x18)->field_0x08 == field_0x34) {
        int counting = field_0x2c->field_0x15c > 0.0f && field_0x2c->field_0x6c == 0;
        if (!(field_0x2c->field_0x38->field_0x76c > 1.0f) && !counting)
            return 1;
        field_0x30 = 1;
        Vector3 up;
        Vector3 forward;
        forward = field_0x34->field_0x17c;
        up = field_0x34->field_0x188;
        int index;
        if (counting) {
            index = (int)field_0x2c->field_0x15c;
            if (index >= 5 || index < 0 || !field_0x38[index])
                return 1;
            field_0x38[index]->field_0x30 = 1;
            field_0x38[index]->UnknownFunction401520(&forward, &up, 0, 1);
            Vector3 position = field_0x34->field_0x170;
            position += up * 2.5f;
            position += forward * 6.0f;
            field_0x38[index]->UnknownFunction4014f0(&position);
        } else {
            index = 5 - (int)field_0x2c->field_0x38->field_0x76c;
            if (index >= 5 || index < 0 || !field_0x38[index])
                return 1;
            field_0x38[index]->field_0x30 = 1;
            field_0x38[index]->UnknownFunction401520(&forward, &up, 0, 1);
            Vector3 position = field_0x34->field_0x170;
            position += up * 2.5f;
            position += forward * 6.0f;
            field_0x38[index]->UnknownFunction4014f0(&position);
        }
    }
    return 1;
}

// 0x0048c290
int NumberObjectManager::UnknownVirtualSlot14() {
    if (field_0x30)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}

// ---------------------------------------------------------- BonusObjectManager

// 0x0056c820: the bonus animation keys (times are frames / 15).
static const UnknownBonusKey g_UnknownBonusKeys[7] = {
    {0, 0.0f, {0.0275f, 0.0275f, 0.0275f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {6, 0.4f, {2.78f, 2.78f, 2.78f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {8, 0.5333f, {2.09f, 2.09f, 2.09f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {25, 1.6667f, {2.09f, 2.09f, 2.09f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {33, 2.2f, {1.99f, 1.99f, 1.99f}, {0.0f, 18.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 3.1415923f},
    {40, 2.6667f, {1.18f, 1.18f, 1.18f}, {0.0f, 29.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 4.607669f},
    {90, 6.0f, {0.05f, 0.05f, 0.05f}, {0.0f, 117.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 15.079643f},
};

// A key's vector built from its three floats.
#define KEY_VECTOR(i, member) \
    Vector3(g_UnknownBonusKeys[i].member[0], g_UnknownBonusKeys[i].member[1], g_UnknownBonusKeys[i].member[2])

// 0x0048c2b0
BonusObjectManager::BonusObjectManager(int flags) : GameObject(flags) {
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x1dc = 0;
    field_0x1ec = 0;
    field_0x1f0 = 0;
    field_0x30 = -1;
    field_0x1e0 = 60.0f;
    field_0x1e8 = 1.0f;
}

// 0x0048c320
BonusObjectManager::~BonusObjectManager() {
}

// 0x0048c330
int BonusObjectManager::UnknownFunction48c330(void* value, int a, int b) {
    char name[0x104];
    Vector3 position;
    int digit;
    int place;

    position = Vector3(0.74f, 0.0f, 0.0f);
    sprintf(name, "%s\\%1d.slt", "Res", 0);
    field_0x40[0][0] = new (__FILE__, 0x2d3) D3DIMSoultreeObject(field_0x25_bit0);
    field_0x40[0][0]->UnknownVirtualSlot9(value, name, a, b, 1);
    field_0x40[0][0]->UnknownFunction4fc660(&position);
    field_0x3c->UnknownFunction469190(field_0x40[0][0], -1);
    field_0x40[0][0]->UnknownFunction4444e0();
    position.x = 0.37f;
    field_0x40[0][1] = new (__FILE__, 0x2da) D3DIMSoultreeObject(field_0x25_bit0);
    field_0x40[0][1]->UnknownVirtualSlot9(value, name, a, b, 1);
    field_0x40[0][1]->UnknownFunction4fc660(&position);
    field_0x3c->UnknownFunction469190(field_0x40[0][1], -1);
    field_0x40[0][1]->UnknownFunction4444e0();
    for (digit = 1; digit < 10; digit++) {
        field_0x40[digit][0] = 0;
        field_0x40[digit][1] = 0;
    }
    for (digit = 0; digit < 10; digit++) {
        sprintf(name, "%s\\%1d.slt", "Res", digit);
        position.x = 0.0f;
        for (place = 2; place < 5; place++) {
            field_0x40[digit][place] = new (__FILE__, 0x2ec) D3DIMSoultreeObject(field_0x25_bit0);
            field_0x40[digit][place]->UnknownVirtualSlot9(value, name, a, b, 1);
            field_0x40[digit][place]->UnknownFunction4fc660(&position);
            field_0x3c->UnknownFunction469190(field_0x40[digit][place], -1);
            field_0x40[digit][place]->UnknownFunction4444e0();
            position.x -= 0.37f;
        }
    }
    for (place = 0; place < 5; place++)
        field_0x108[place] = -1;
    position.x = 1.18f;
    sprintf(name, "%s\\x.slt", "Res", 0);
    field_0x11c = new (__FILE__, 0x2fe) D3DIMSoultreeObject(field_0x25_bit0);
    field_0x11c->UnknownVirtualSlot9(value, name, a, b, 1);
    field_0x11c->UnknownFunction4fc660(&position);
    field_0x3c->UnknownFunction469190(field_0x11c, -1);
    field_0x11c->UnknownFunction4444e0();
    position.x = 1.67f;
    sprintf(name, "%s\\DecimalPoint.slt", "Res", 0);
    field_0x120 = new (__FILE__, 0x307) D3DIMSoultreeObject(field_0x25_bit0);
    field_0x120->UnknownVirtualSlot9(value, name, a, b, 1);
    field_0x120->UnknownFunction4fc660(&position);
    field_0x3c->UnknownFunction469190(field_0x120, -1);
    field_0x120->UnknownFunction4444e0();
    for (digit = 0; digit < 10; digit++) {
        sprintf(name, "%s\\%1d.slt", "Res", digit);
        position.x = 1.87f;
        for (place = 0; place < 2; place++) {
            field_0x124[digit][place] = new (__FILE__, 0x312) D3DIMSoultreeObject(field_0x25_bit0);
            field_0x124[digit][place]->UnknownVirtualSlot9(value, name, a, b, 1);
            field_0x124[digit][place]->UnknownFunction4fc660(&position);
            field_0x3c->UnknownFunction469190(field_0x124[digit][place], -1);
            field_0x124[digit][place]->UnknownFunction4444e0();
            position.x -= 0.41f;
        }
    }
    for (place = 0; place < 2; place++)
        field_0x174[place] = -1;
    return 1;
}

// 0x0048c8a0
int BonusObjectManager::UnknownFunction48c8a0(void* value, int a, int b) {
    field_0x3c = new (__FILE__, 0x326) D3DIMSoultreeObject(1);
    if (!UnknownFunction469190(field_0x3c->UnknownVirtualSlot9(value, "", a, b, 1), -1))
        return 0;
    strcpy(field_0x3c->field_0x038, "Base Bonus Frame");
    field_0x3c->field_0x174 = kVec3Zero;
    field_0x3c->field_0x180 = Vector3(0.9f, 0.22f, 0.09f);
    field_0x3c->field_0x170 = 1;
    field_0x3c->field_0x158 = kVec3Zero;
    field_0x3c->field_0x164 = Vector3(0.9f, 0.22f, 0.09f);
    field_0x3c->field_0x154 = 1;
    field_0x1e4 = field_0x3c->field_0x164.x + field_0x3c->field_0x164.x;
    return 1;
}

// 0x0048ca60
BonusObjectManager* BonusObjectManager::UnknownFunction48ca60(void* value, int a, int b, UnknownBonusRacer* racer,
                                                              UnknownBonusCamera* camera) {
    GameObject::UnknownVirtualSlot8(value);
    field_0x2c = racer;
    field_0x38 = camera;
    if (!UnknownFunction48c8a0(value, a, b)) {
        Release();
        return 0;
    }
    if (!UnknownFunction48c330(value, a, b)) {
        Release();
        return 0;
    }
    return this;
}

// 0x0048cad0
void BonusObjectManager::UnknownFunction48cad0() {
    int i = ++field_0x30;
    field_0x17c = g_UnknownBonusKeys[i + 1].field_0x04 - g_UnknownBonusKeys[i].field_0x04;
    field_0x180 = g_UnknownBonusKeys[i].field_0x2c;
    field_0x184 = g_UnknownBonusKeys[i + 1].field_0x2c - field_0x180;
    Vector3 next = KEY_VECTOR(i + 1, field_0x14);
    field_0x194 = KEY_VECTOR(i, field_0x14);
    field_0x1a0 = next - field_0x194;
    next = KEY_VECTOR(i + 1, field_0x08);
    field_0x1ac = KEY_VECTOR(i, field_0x08);
    field_0x1b8 = next - field_0x1ac;
    next = KEY_VECTOR(i + 1, field_0x20);
    field_0x1c4 = KEY_VECTOR(i, field_0x20);
    field_0x1d0 = next - field_0x1c4;
}

// 0x0048d540
void BonusObjectManager::UnknownFunction48d540() {
    if (field_0x30 < 0)
        return;
    for (int i = 0; i < 5; i++) {
        if (field_0x108[i] >= 0) {
            field_0x3c->UnknownFunction4fd990(field_0x40[field_0x108[i]][i]);
            field_0x40[field_0x108[i]][i]->UnknownFunction4444e0();
            field_0x108[i] = -1;
        }
    }
    if (field_0x174[0] >= 0) {
        for (int i = 0; i < 2; i++) {
            field_0x3c->UnknownFunction4fd990(field_0x124[field_0x174[i]][i]);
            field_0x124[field_0x174[i]][i]->UnknownFunction4444e0();
            field_0x174[i] = -1;
        }
        field_0x3c->UnknownFunction4fd990(field_0x11c);
        field_0x11c->UnknownFunction4444e0();
        field_0x3c->UnknownFunction4fd990(field_0x120);
        field_0x120->UnknownFunction4444e0();
    }
    field_0x34 = 0;
    field_0x30 = -1;
}

// 0x0048d620
int BonusObjectManager::UnknownVirtualSlot14() {
    if (field_0x3c->field_0x2d4)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}
