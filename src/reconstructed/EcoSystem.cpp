// EcoSystem.cpp -- D:\aardvark\VC\krusty2\EcoSystem.cpp, 0x00455da0..0x0045c830.
// See EcoSystem.h for the class evidence and docs/ECOSYSTEM.md for the
// match state. Line numbers in the allocation calls are the retail
// __LINE__ values.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "EcoSystem.h"

#include "MemTag.h"
#include "Parameterblocks.h"
#include "PeakHold.h"
#include "TypeRegistry.h"
#include "UnknownResourceManager.h"
#include "bmpfile.h"

// The four vector constants that open about 73 retail files
// (0x0059aea0, 0x0059aeb0, 0x0059aed8, 0x0059ae90; $E 0x00459830..0x00459b3b).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

EcoSystem* g_UnknownGlobal59aebc;             // 0x0059aebc
static int g_UnknownGlobal59aec0;             // 0x0059aec0: billboard draw time
static UnknownPeakHold g_UnknownGlobal59aef0(5000); // 0x0059aef0 ($E 0x00455db0, XCU 103)
static UnknownPeakHold g_UnknownGlobal59aec8(5000); // 0x0059aec8 ($E 0x00455dd0, XCU 104)
static int g_UnknownGlobal59aee4;             // 0x0059aee4: geometry draw time
static int g_UnknownGlobal59aee8;             // 0x0059aee8: classification time
static int g_UnknownGlobal59aefc;             // 0x0059aefc: textures were preloaded
static Matrix4* g_UnknownGlobal59af00;        // 0x0059af00: the view matrix
static int g_UnknownGlobal59af04;             // 0x0059af04: geometry objects drawn
static int g_UnknownGlobal59af08;             // 0x0059af08: billboards drawn
static int g_UnknownGlobal59af0c;             // 0x0059af0c: geometry blocks alive
static int g_UnknownGlobal59af10;             // 0x0059af10: textures preloaded
static UnknownEcoDetailBand* g_UnknownGlobal59af14; // 0x0059af14: the detail band table

// The detail bands, by level: 0x0056a600 for the 1555 (software) path,
// 0x0056a740 for 4444.
static UnknownEcoDetailBand g_UnknownGlobal56a600[10] = {
    {0, 0, 1555, 256, 0, 0, 0, 0},   {0, 0, 1555, 256, 0, 0, 0, 0},   {0, 0, 1555, 256, 0, 0, 0, 0},
    {10, 10, 1555, 384, 0, 0, 0, 0}, {20, 20, 1555, 384, 0, 1, 0, 0}, {20, 20, 1555, 448, 0, 1, 1, 0},
    {30, 30, 1555, 512, 0, 1, 1, 0}, {40, 40, 1555, 512, 0, 1, 1, 0}, {75, 75, 1555, 640, 1, 1, 1, 0},
    {150, 75, 1555, 768, 1, 1, 1, 1},
};
static UnknownEcoDetailBand g_UnknownGlobal56a740[10] = {
    {20, 0, 4444, 256, 0, 1, 1, 1},   {35, 5, 4444, 256, 0, 1, 1, 1},   {45, 10, 4444, 320, 0, 1, 1, 1},
    {60, 20, 4444, 384, 0, 1, 1, 1},  {75, 30, 4444, 448, 1, 1, 1, 1},  {90, 40, 4444, 512, 1, 1, 1, 1},
    {105, 50, 4444, 576, 1, 1, 1, 1}, {120, 60, 4444, 640, 1, 1, 1, 1}, {135, 67, 4444, 768, 1, 1, 1, 1},
    {150, 75, 4444, 768, 1, 1, 1, 1},
};

static int g_UnknownGlobal56a128 = -1;        // 0x0056a128
static int g_UnknownGlobal56a12c = 1;         // 0x0056a12c: drawing enabled (slot 23 toggles it)

// Inline helper (shape only; the retail header is not attested): the
// squared length, summed as z + (x + y) (the only order VC6 compiles to
// the retail load sequence).
static inline float UnknownSquareMagnitude(const Vector3* v) {
    return v->z * v->z + (v->x * v->x + v->y * v->y);
}

// The view (RenderTarget.h) the object was attached to, GameObject+0x18.

#define ECO_VIEW ((UnknownEcoRenderTarget*)field_0x18)

#define ECO_RGBA(r, g, b, a) ((unsigned int)(((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

// 0x00455de0
UnknownEcoDefinition::UnknownEcoDefinition() {
    field_0x000[0] = 0;
    field_0x080[0] = 0;
    field_0x100[0] = 0;
    field_0x180 = 0.0f;
    field_0x184 = 0.0f;
    field_0x188 = 0.0f;
    field_0x18c = 0.0f;
    field_0x190 = 0.0f;
    field_0x194 = 0.0f;
    field_0x198 = 0.0f;
    field_0x19c = 0.0f;
    field_0x1a0 = 0.0f;
    field_0x1a4 = 0.0f;
    field_0x1a8 = 0.0f;
    field_0x1ac = 0.0f;
    field_0x1b0 = 0.0f;
    field_0x1b4 = 0.0f;
    field_0x1b8 = 0.0f;
    field_0x1bc = 1.0f;
    field_0x1c0 = 1.0f;
    field_0x1c4 = 0.0f;
    field_0x1c8 = 0.5f;
    field_0x204 = 0;
    field_0x20c = 0;
    field_0x208 = 0;
}

// 0x00455e80
UnknownEcoDefinition::~UnknownEcoDefinition() {
    int i;
    if (field_0x1e0) {
        field_0x1e0->UnknownVirtualSlot2();
        if (g_UnknownGlobal59aefc)
            field_0x1e0->UnknownVirtualSlot2();
    }
    if (field_0x1e4) {
        field_0x1e4->UnknownVirtualSlot2();
        if (g_UnknownGlobal59aefc)
            field_0x1e4->UnknownVirtualSlot2();
    }
    for (i = 0; i < field_0x1d8; i++) {
        if (field_0x1ec[i])
            DebugFree(field_0x1ec[i], __FILE__, 0x66);
    }
    if (field_0x208)
        DebugFree(field_0x208, __FILE__, 0x6a);
    if (field_0x20c)
        DebugFree(field_0x20c, __FILE__, 0x6e);
    g_UnknownGlobal59af10 = 0;
}

// 0x00455f50
int UnknownEcoDefinition::UnknownFunction455f50() {
    return rand() >> 8;
}

// 0x00455f60
int UnknownEcoDefinition::UnknownFunction455f60(float height) {
    return (int)(rand() * height / field_0x188) >> 8;
}

// 0x00455f90
float UnknownEcoDefinition::UnknownFunction455f90(unsigned char parameter) {
    if (parameter < 0x80)
        return field_0x180 - (field_0x180 - field_0x184) * parameter * (1.0f / 128.0f);
    return field_0x180 + (field_0x188 - field_0x180) * (255 - parameter) * (1.0f / 128.0f);
}

// 0x00455ff0
float UnknownEcoDefinition::UnknownFunction455ff0(unsigned char parameter) {
    if (parameter < 0x80)
        return field_0x18c - (field_0x18c - field_0x190) * parameter * (1.0f / 128.0f);
    return field_0x18c + (field_0x194 - field_0x18c) * (255 - parameter) * (1.0f / 128.0f);
}

// 0x00456650
Vegetation::Vegetation() {
    field_0x0c.x = 0;
    field_0x0c.y = 0;
    field_0x0c.z = 0;
    field_0x14 = 0;
    field_0x15 = 0;
    field_0x12 = 0;
    field_0x18 = 0;
    field_0x08 = g_UnknownGlobal59aebc->field_0x599;
}

// 0x00456690
unsigned short Vegetation::UnknownVirtualSlot0() {
    Matrix4* m = g_UnknownGlobal59af00;
    float depth = ((field_0x0c.z * (*m)(2, 2) + field_0x0c.x * (*m)(0, 2)) * g_UnknownGlobal59aebc->field_0x5a8
                   + (*m)(3, 2)) * g_UnknownGlobal59aebc->field_0x5b8;
    if (depth < 0.0)
        field_0x06 = 0;
    else if ((int)depth >= 0xffff)
        field_0x06 = (short)0xffff;
    else
        field_0x06 = (short)(int)depth;
    return field_0x06;
}

// 0x00456720
unsigned int Vegetation::UnknownVirtualSlot1(UnknownEcoQuadTree* tree) {
    float radius = g_UnknownGlobal59aebc->field_0x58[field_0x12]->UnknownFunction455ff0(field_0x15);
    float x = field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
    float z = field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8;
    return tree->ComputeCode(x - radius, z - radius, x + radius, z + radius);
}

// 0x004567a0
void Vegetation::UnknownFunction4567a0(TextureMapManager* textures, unsigned char definition,
                                       const UnknownEcoCoordinates* coordinates, unsigned char heightParameter,
                                       unsigned char radiusParameter) {
    field_0x12 = definition;
    field_0x0c = *coordinates;
    field_0x15 = radiusParameter;
    field_0x14 = heightParameter;
    field_0x16_bit0 = 1;
}

// 0x00456850
int UnknownFunction456850(void* owner, int context) {
    Vegetation* object = (Vegetation*)owner;
    DebugFree(object->field_0x18, __FILE__, 0x1a1);
    object->field_0x18 = 0;
    object->field_0x16_bit0 = 1;
    return 1;
}

// 0x00457000
void Vegetation::UnknownFunction457000(UnknownEcoRenderTarget* target) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->field_0x58[field_0x12];
    definition->field_0x1e4->UnknownVirtualSlot19();
    int vertexCount = definition->field_0x1f0[field_0x16_bit0];
    int indexCount = definition->field_0x1f4[field_0x16_bit0];
    target->UnknownVirtualSlot15(4, 0x1e2, field_0x18, vertexCount, (UnknownEcoVertex*)field_0x18 + vertexCount,
                                 indexCount, 0);
    g_UnknownGlobal59aebc->field_0x59c->UnknownFunction401250(
        (AgeEntry*)((int*)field_0x18 + vertexCount * 8 + (indexCount + 1) / 2));
}

// 0x00457080
int Vegetation::UnknownFunction457080() {
    return g_UnknownGlobal59aebc->field_0x58[field_0x12]->field_0x204;
}

// 0x00457230
float Vegetation::UnknownFunction457230() {
    return g_UnknownGlobal59aebc->field_0x58[field_0x12]->UnknownFunction455ff0(field_0x15);
}

// 0x00457250
EcoSystem::EcoSystem(int flags) : GameObject(flags) {
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    field_0x48 = 0;
    field_0x50 = 0;
    field_0x54 = 0;
    field_0x59c = 0;
    field_0x30 = 0;
    field_0x55c = 0;
    field_0x560 = 0x22b;
    memset(field_0x58, 0, sizeof(field_0x58));
    field_0x590 = 0;
    field_0x598 = 0xcd;
    g_UnknownGlobal59aebc = this;
    field_0x5a8 = 1.0f;
    field_0x5ac = 1.0f;
}

// 0x00457330
EcoSystem::~EcoSystem() {
    int i;
    for (i = 0; i < 256; i++) {
        if (field_0x58[i])
            delete field_0x58[i];
    }
    if (field_0x38) {
        for (i = 0; i < field_0x55c; i++) {
            if (field_0x38[i].field_0x18)
                DebugFree(field_0x38[i].field_0x18, __FILE__, 0x2cd);
        }
        delete field_0x38;
    }
    if (field_0x50)
        DebugFree(field_0x50, __FILE__, 0x2d2);
    if (field_0x54)
        DebugFree(field_0x54, __FILE__, 0x2d3);
    if (field_0x3c)
        DebugFree(field_0x3c, __FILE__, 0x2d4);
    if (field_0x40)
        DebugFree(field_0x40, __FILE__, 0x2d5);
    if (field_0x59c)
        delete field_0x59c;
    g_UnknownGlobal59aebc = 0;
    g_UnknownGlobal59aefc = 0;
}

// 0x004587b0: writes the .esb next to the .est.
int EcoSystem::UnknownFunction4587b0(const char* path) {
    char name[0x104];
    unsigned char present;
    unsigned char length;
    int i;
    int pathLength = strlen(path);
    int count = pathLength > 0x103 ? 0x103 : pathLength;
    strncpy(name, path, count);
    name[count] = 0;
    strcpy(strrchr(name, '.'), ".esb");
    FILE* file = fopen(name, "wb");
    if (!file)
        return 0;
    fwrite(&field_0x30, 4, 1, file);
    for (i = 0; i < 256; i++) {
        present = field_0x58[i] != 0;
        fwrite(&present, 1, 1, file);
        if (present) {
            int j;
            length = strlen(field_0x58[i]->field_0x000) + 1;
            fwrite(&length, 1, 1, file);
            fwrite(field_0x58[i]->field_0x000, length, 1, file);
            length = strlen(field_0x58[i]->field_0x080) + 1;
            fwrite(&length, 1, 1, file);
            fwrite(field_0x58[i]->field_0x080, length, 1, file);
            fwrite(&field_0x58[i]->field_0x180, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x184, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x188, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x18c, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x190, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x194, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1b8, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1bc, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1c4, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1c0, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1c8, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1f8, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1fc, 4, 1, file);
            fwrite(&field_0x58[i]->field_0x1dc, 4, 1, file);
            if (field_0x30 == 2) {
                fwrite(&field_0x58[i]->field_0x200, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x198, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x19c, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x1a0, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x1a4, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x1a8, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x1ac, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x1b0, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x1b4, 4, 1, file);
                present = field_0x58[i]->field_0x100[0] != 0;
                fwrite(&present, 1, 1, file);
                if (present) {
                    length = strlen(field_0x58[i]->field_0x100) + 1;
                    fwrite(&length, 1, 1, file);
                    fwrite(field_0x58[i]->field_0x100, length, 1, file);
                }
            }
            fwrite(&field_0x58[i]->field_0x204, 4, 1, file);
            for (j = 0; j < field_0x58[i]->field_0x204; j++) {
                fwrite(&field_0x58[i]->field_0x208[j].field_0x00, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x208[j].field_0x04, 0xc, 1, file);
                fwrite(&field_0x58[i]->field_0x208[j].field_0x10, 0xc, 1, file);
                fwrite(&field_0x58[i]->field_0x208[j].field_0x1c, 4, 1, file);
                fwrite(&field_0x58[i]->field_0x208[j].field_0x20, 4, 1, file);
            }
        }
    }
    present = field_0x45c[0] != 0;
    fwrite(&present, 1, 1, file);
    if (present) {
        length = strlen(field_0x45c) + 1;
        fwrite(&length, 1, 1, file);
        fwrite(field_0x45c, length, 1, file);
    }
    if (field_0x30 == 2) {
        fwrite(&field_0x458, 4, 1, file);
        present = field_0x4dc[0] != 0;
        fwrite(&present, 1, 1, file);
        if (present) {
            length = strlen(field_0x4dc) + 1;
            fwrite(&length, 1, 1, file);
            fwrite(field_0x4dc, length, 1, file);
        }
    }
    fwrite(&field_0x55c, 4, 1, file);
    for (i = 0; i < field_0x55c; i++) {
        unsigned char definition = field_0x38[i].field_0x12;
        fwrite(&definition, 1, 1, file);
        fwrite(&field_0x38[i].field_0x0c, 6, 1, file);
        fwrite(&field_0x38[i].field_0x14, 1, 1, file);
        fwrite(&field_0x38[i].field_0x15, 1, 1, file);
    }
    fclose(file);
    UnknownFunction458da0(path);
    return 1;
}

// 0x00458f70: reads the .esb (the stream's own, the archive's, or a new one).
int EcoSystem::UnknownFunction458f70(const char* path, UnknownTextureStream* stream) {
    unsigned char present;
    unsigned char length;
    int i;
    field_0x594 = 0;
    if (!stream) {
        UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(path, 1);
        if (!entry) {
            stream = new(__FILE__, 0x520) UnknownTextureStream(g_UnknownResourceManager572b44);
            if (!stream->UnknownFunction460f50(path, "rb", 0)) {
                if (stream)
                    delete stream;
                return 0;
            }
        } else {
            stream = entry->field_0x14;
            field_0x594 = 1;
        }
    } else {
        field_0x594 = 1;
    }
    field_0x590 = stream;
    stream->UnknownFunction461640(&field_0x30, 4, 1);
    for (i = 0; i < 256; i++) {
        stream->UnknownFunction461640(&present, 1, 1);
        if (present) {
            int j;
            stream->UnknownFunction461640(&length, 1, 1);
            field_0x58[i] = new(__FILE__, 0x543) UnknownEcoDefinition;
            stream->UnknownFunction461640(field_0x58[i]->field_0x000, length, 1);
            stream->UnknownFunction461640(&length, 1, 1);
            stream->UnknownFunction461640(field_0x58[i]->field_0x080, length, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x180, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x184, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x188, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x18c, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x190, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x194, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1b8, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1bc, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1c4, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1c0, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1c8, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1f8, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1fc, 4, 1);
            stream->UnknownFunction461640(&field_0x58[i]->field_0x1dc, 4, 1);
            if (field_0x30 == 2) {
                stream->UnknownFunction461640(&field_0x58[i]->field_0x200, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x198, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x19c, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x1a0, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x1a4, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x1a8, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x1ac, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x1b0, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x1b4, 4, 1);
                stream->UnknownFunction461640(&present, 1, 1);
                if (present) {
                    stream->UnknownFunction461640(&length, 1, 1);
                    stream->UnknownFunction461640(field_0x58[i]->field_0x100, length, 1);
                }
            }
            stream->UnknownFunction461640(&field_0x58[i]->field_0x204, 4, 1);
            field_0x58[i]->field_0x208 = (UnknownEcoCollisionDefinition*)DebugMalloc(
                field_0x58[i]->field_0x204 * sizeof(UnknownEcoCollisionDefinition), __FILE__, 0x56d);
            for (j = 0; j < field_0x58[i]->field_0x204; j++) {
                stream->UnknownFunction461640(&field_0x58[i]->field_0x208[j].field_0x00, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x208[j].field_0x04, 0xc, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x208[j].field_0x10, 0xc, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x208[j].field_0x1c, 4, 1);
                stream->UnknownFunction461640(&field_0x58[i]->field_0x208[j].field_0x20, 4, 1);
            }
        }
    }
    stream->UnknownFunction461640(&present, 1, 1);
    if (present) {
        stream->UnknownFunction461640(&length, 1, 1);
        stream->UnknownFunction461640(field_0x45c, length, 1);
    }
    if (field_0x30 == 2) {
        stream->UnknownFunction461640(&field_0x458, 4, 1);
        stream->UnknownFunction461640(&present, 1, 1);
        if (present) {
            stream->UnknownFunction461640(&length, 1, 1);
            stream->UnknownFunction461640(field_0x4dc, length, 1);
        }
    }
    stream->UnknownFunction461640(&field_0x55c, 4, 1);
    field_0x34 = field_0x55c;
    return 1;
}

// 0x004594c0
void EcoSystem::UnknownFunction4594c0(int level) {
    field_0x5c0 = level;
}

// 0x004594d0
EcoSystem* EcoSystem::UnknownFunction4594d0(void* view, TextureMapManager* textures, UnknownEcoTerrain* terrain,
                                            LightManager* lights, char* path, UnknownTextureStream* stream,
                                            int textureFormat, int collisions, int level) {
    int i;
    GameObject::UnknownVirtualSlot8(view);
    g_UnknownGlobal59af14 = g_UnknownGlobal56e26c->field_0x2d0 ? g_UnknownGlobal56a600 : g_UnknownGlobal56a740;
    UnknownFunction4594c0(level);
    field_0x44 = terrain;
    field_0x48 = textures;
    field_0x4c = lights;
    field_0x560 = textureFormat;
    float size = g_collisionQuadTree->field_0x54;
    float width = g_collisionQuadTree->field_0x50;
    if (width > size)
        size = width;
    field_0x5ac = 65536.0f / size;
    field_0x5a8 = size * (1.0f / 65536.0f);
    field_0x59c = new(__FILE__, 0x5ca) AgeManager;
    UnknownFunction45a9a0();
    if (!UnknownFunction457480(path, stream)) {
        Release();
        return 0;
    }
    for (i = 0; i < 256; i++) {
        if (field_0x58[i]) {
            field_0x58[i]->UnknownFunction456050(textures, g_UnknownGlobal59af14[field_0x5c0].field_0x08);
            if (collisions)
                UnknownFunction457ed0(field_0x58[i]);
            else
                field_0x58[i]->field_0x204 = 0;
        }
    }
    field_0x599 = g_TypeRegistry->FindTypeId("Vegetation");
    field_0x38 = new(__FILE__, 0x5eb) Vegetation[field_0x34];
    field_0x3c = (Vegetation**)DebugMalloc(0x1f40, __FILE__, 0x5ec);
    field_0x40 = (Vegetation**)DebugMalloc(0x190, __FILE__, 0x5ed);
    field_0x5b0 = 2000;
    field_0x5b4 = 100;
    field_0x50 = (UnknownEcoVertex*)DebugMalloc(0x5a00, __FILE__, 0x5f1);
    field_0x54 = (unsigned short*)DebugMalloc(0xb40, __FILE__, 0x5f2);
    for (i = 0; i < 720; i++) {
        field_0x50[i].reserved = 0;
        field_0x50[i].specular = 0;
    }
    int index = 0;
    for (i = 0; i < 120; i++) {
        field_0x54[index++] = i * 6;
        field_0x54[index++] = i * 6 + 4;
        field_0x54[index++] = i * 6 + 5;
        field_0x54[index++] = i * 6;
        field_0x54[index++] = i * 6 + 5;
        field_0x54[index++] = i * 6 + 3;
        field_0x54[index++] = i * 6 + 4;
        field_0x54[index++] = i * 6 + 1;
        field_0x54[index++] = i * 6 + 2;
        field_0x54[index++] = i * 6 + 4;
        field_0x54[index++] = i * 6 + 2;
        field_0x54[index++] = i * 6 + 5;
    }
    if (strstr(path, ".esb")) {
        UnknownFunction4598d0();
    } else {
        if (field_0x30 == 1)
            UnknownFunction459b40();
        else
            UnknownFunction459ce0(UnknownFunction511ad0(textureFormat));
        UnknownFunction4587b0(path);
    }
    return this;
}

// 0x0045a9a0
void EcoSystem::UnknownFunction45a9a0() {
    UnknownEcoLight* light = field_0x4c->UnknownFunction4a0190(6);
    if (light) {
        field_0x570.x = light->field_0x54[0];
        field_0x570.y = light->field_0x54[1];
        field_0x570.z = light->field_0x54[2];
    } else {
        field_0x570.z = 0.0f;
        field_0x570.y = 0.0f;
        field_0x570.x = 0.0f;
    }
    light = field_0x4c->UnknownFunction4a0190(4);
    if (!light) {
        light = field_0x4c->UnknownFunction4a0190(2);
        if (!light) {
            field_0x564.z = 0.0f;
            field_0x564.y = 0.0f;
            field_0x564.x = 0.0f;
            return;
        }
    }
    field_0x564.x = light->field_0x54[0];
    field_0x564.y = light->field_0x54[1];
    field_0x564.z = light->field_0x54[2];
    field_0x57c.x = light->field_0x70.x;
    field_0x57c.y = 0.0f;
    field_0x57c.z = light->field_0x70.z;
    float length = UnknownSquareMagnitude(&field_0x57c);
    if (length == 0.0f) {
        field_0x57c = kVec3Zero;
        return;
    }
    length = FastInvSqrt(length);
    field_0x57c.x = length * field_0x57c.x;
    field_0x57c.y = length * field_0x57c.y;
    field_0x57c.z = length * field_0x57c.z;
}

// 0x0045aad0
int EcoSystem::UnknownVirtualSlot12() {
    if (!g_UnknownGlobal56a12c)
        return 1;
    int category = g_MemTagStack->Push("EcoSystem");
    unsigned int start = ReadClock();
    if (!field_0x34)
        return 1;
    if (field_0x59c) {
        int stale;
        int total = field_0x59c->UnknownFunction401130(&stale);
        if (stale > 0x40000)
            field_0x59c->UnknownFunction4011b0(total - stale);
        field_0x59c->UnknownFunction401040();
    }
    g_UnknownGlobal59af00 = &ECO_VIEW->field_0x08->field_0xac;
    field_0x5b8 = 65535.0f / ECO_VIEW->field_0x08->field_0x1c0;
    field_0x5bc = ECO_VIEW->field_0x08->field_0x1c0 * (1.0f / 65535.0f);
    field_0x5a0 = 0;
    field_0x5a4 = 0;
    g_collisionQuadTree->RestartQuery();
    Vegetation* object = (Vegetation*)g_collisionQuadTree->NextObjectSorted();
    while (object) {
        if (object->field_0x08 == field_0x599) {
            int billboard = object->field_0x16_bit0;
            int fade = object->field_0x13;
            object->UnknownFunction456890(&billboard, &fade);
            UnknownEcoDefinition* definition = field_0x58[object->field_0x12];
            int visible = 1;
            if (!billboard) {
                float radius = definition->UnknownFunction455ff0(object->field_0x15);
                float height = definition->UnknownFunction455f90(object->field_0x14) * 0.5;
                Vector3 center;
                if (radius <= height)
                    radius = height;
                center.x = object->field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
                center.y = object->field_0x0c.y * g_UnknownGlobal59aebc->field_0x5a8 + height;
                center.z = object->field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8;
                visible = g_visibilityClipper->SphereInFrustum(ECO_VIEW->field_0x08, &ECO_VIEW->field_0x08->field_0xec,
                                                               &center, radius, 0);
            }
            if (visible) {
                object->UnknownFunction456a10(billboard, fade);
                if (object->field_0x13) {
                    if (field_0x5a0 == field_0x5b0) {
                        Vegetation** old = field_0x3c;
                        field_0x3c = (Vegetation**)DebugRealloc(old, (field_0x5b0 + 100) * sizeof(Vegetation*),
                                                                __FILE__, 0x8dd);
                        if (field_0x3c != old)
                            field_0x5b0 += 100;
                    }
                    field_0x3c[field_0x5a0] = object;
                    field_0x5a0++;
                }
                if (object->field_0x16_bit0 < definition->field_0x1d8) {
                    if (field_0x5a4 == field_0x5b4) {
                        Vegetation** old = field_0x40;
                        field_0x40 = (Vegetation**)DebugRealloc(old, (field_0x5b4 + 20) * sizeof(Vegetation*),
                                                                __FILE__, 0x8ea);
                        if (field_0x40 != old)
                            field_0x5b4 += 20;
                    }
                    field_0x40[field_0x5a4] = object;
                    field_0x5a4++;
                }
            }
        }
        object = (Vegetation*)g_collisionQuadTree->NextObjectSorted();
    }
    g_UnknownGlobal59aee8 = ReadClock() - start;
    g_MemTagStack->Pop(category);
    return 1;
}

// 0x0045ade0: the render and texture-stage states for `format`.
void EcoSystem::UnknownFunction45ade0(int format) {
    float alphaReference;
    if (format == 0x613) {
        alphaReference = (float)(g_UnknownGlobal56e26c->field_0x2d0 ? 0 : 0xc0);
        ECO_VIEW->UnknownVirtualSlot8(0x1b, 0, 0);
        ECO_VIEW->UnknownVirtualSlot8(0x29, 0, 0);
    } else if (format == 0x115c || format == 0x22b8) {
        alphaReference = 0.0f;
        ECO_VIEW->UnknownVirtualSlot8(0x1b, 1, 0);
        ECO_VIEW->UnknownVirtualSlot8(0x29, 0, 0);
    } else {
        alphaReference = 0.0f;
        ECO_VIEW->UnknownVirtualSlot8(0x1b, 0, 0);
        ECO_VIEW->UnknownVirtualSlot8(0x29, 1, 0);
    }
    ECO_VIEW->UnknownVirtualSlot18((int)alphaReference);
    if (g_UnknownGlobal59af14[field_0x5c0].field_0x14) {
        ECO_VIEW->UnknownVirtualSlot7(0, 1, 4);
        ECO_VIEW->UnknownVirtualSlot7(0, 2, 2);
        ECO_VIEW->UnknownVirtualSlot7(0, 3, 0);
        if (g_UnknownGlobal59af14[field_0x5c0].field_0x18)
            ECO_VIEW->UnknownVirtualSlot8(9, 2, 0);
        else
            ECO_VIEW->UnknownVirtualSlot8(9, 1, 0);
        if (format == 0x115c || format == 0x22b8) {
            ECO_VIEW->UnknownVirtualSlot7(0, 4, 4);
            ECO_VIEW->UnknownVirtualSlot7(0, 5, 2);
            ECO_VIEW->UnknownVirtualSlot7(0, 6, 0);
        } else {
            ECO_VIEW->UnknownVirtualSlot7(0, 4, 2);
            ECO_VIEW->UnknownVirtualSlot7(0, 5, 2);
        }
    } else {
        ECO_VIEW->UnknownVirtualSlot7(0, 1, 2);
        ECO_VIEW->UnknownVirtualSlot7(0, 2, 2);
        ECO_VIEW->UnknownVirtualSlot8(9, 1, 0);
        // Both arms are the same in retail (the second is reached by jump
        // threading from the format test below).
        if (format == 0x115c) {
            ECO_VIEW->UnknownVirtualSlot7(0, 4, 2);
            ECO_VIEW->UnknownVirtualSlot7(0, 5, 2);
        } else {
            ECO_VIEW->UnknownVirtualSlot7(0, 4, 2);
            ECO_VIEW->UnknownVirtualSlot7(0, 5, 2);
        }
    }
    if (format == 0x613) {
        ECO_VIEW->UnknownVirtualSlot7(0, 0x12, 1);
        if (g_UnknownGlobal56e26c->field_0x2d0) {
            ECO_VIEW->UnknownVirtualSlot7(0, 0x10, 1);
            ECO_VIEW->UnknownVirtualSlot7(0, 0x11, 1);
        } else {
            ECO_VIEW->UnknownVirtualSlot7(0, 0x10, 2);
            ECO_VIEW->UnknownVirtualSlot7(0, 0x11, 2);
        }
    } else if (format == 0x115c) {
        ECO_VIEW->UnknownVirtualSlot7(0, 0x12, 1);
        ECO_VIEW->UnknownVirtualSlot7(0, 0x10, 2);
        ECO_VIEW->UnknownVirtualSlot7(0, 0x11, 2);
    } else {
        ECO_VIEW->UnknownVirtualSlot7(0, 0x12, 1);
        ECO_VIEW->UnknownVirtualSlot7(0, 0x10, 2);
        ECO_VIEW->UnknownVirtualSlot7(0, 0x11, 2);
    }
}

// 0x0045bfe0: the key toggling the drawing.
int EcoSystem::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (GameObject::UnknownVirtualSlot23(event, entry))
        return 1;
    if (UnknownFunction43caa0(4, 0, event, 0x80)) {
        g_UnknownGlobal56a12c = 1 - g_UnknownGlobal56a12c;
        return 1;
    }
    return 0;
}
