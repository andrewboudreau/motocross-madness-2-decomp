// Near-miss candidates of D:\aardvark\VC\krusty2\EcoSystem.cpp, kept out of
// src/reconstructed/EcoSystem.cpp until they match. They compile against
// the reconstructed header; the exact functions they call live in the src
// unit (bound by address). What differs from retail (docs/ECOSYSTEM.md):
//   0x004567e0  operand order of `position->x * scale` (retail loads x first)
//   0x00456890  the camera pointer is loaded before the first fmul in retail
//   0x00456a10  local slot layout (frame 0x50 in retail, 0x4c here)
//   0x004570a0  retail re-reads the definition after the position conversions
//   0x00456050  register roles of this/textures and the local layout
//   0x00457480  the probe stream also lives in esi; the aligned-frame EH
//               prologue is not recognised by the matcher
//   0x00457ed0  one scheduled load (the cylinder height) in the vertex loop
//   0x00458360  zero kept in ebp, the count tested twice
//   0x00458da0  four `[eax + esi]` operands come out as `[esi + eax]`
//   0x004598d0  loop counter in memory, definition byte cached in a register
//   0x00459b40  fidiv for the cell size, bitmap pointer in ebp
//   0x0045c6a0  pointer/count-down block loop
//   0x0045c7b0  register roles and byte update order

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "../../src/reconstructed/EcoSystem.h"

#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/TypeRegistry.h"
#include "../../src/reconstructed/UnknownResourceManager.h"
#include "../../src/reconstructed/bmpfile.h"

// The unit's file statics these candidates read (same addresses as the src
// unit; see its definitions).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static int g_UnknownGlobal59af0c;             // 0x0059af0c
static UnknownEcoDetailBand* g_UnknownGlobal59af14; // 0x0059af14

// The view (RenderTarget.h) the object was attached to, GameObject+0x18.

#define ECO_VIEW ((UnknownEcoRenderTarget*)field_0x18)

#define ECO_RGBA(r, g, b, a) ((unsigned int)(((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

// The identity matrix as the collision builders store it (inline shape
// only; the out-of-line IdentityMatrix 0x004a1410 is not what retail calls).
static inline void UnknownSetIdentity(Matrix4* m) {
    memset(m, 0, sizeof(Matrix4));
    (*m)(3, 3) = 1.0f;
    (*m)(2, 2) = 1.0f;
    (*m)(1, 1) = 1.0f;
    (*m)(0, 0) = 1.0f;
}

// The vector sum as 0x00458246 computes it (a value-returning inline; the
// operand order per component is what VC6 reproduces).
static inline Vector3 UnknownEcoOffset(const Vector3* a, const Vector3* b) {
    Vector3 r;
    r.x = b->x + a->x;
    r.y = a->y + b->y;
    r.z = a->z + b->z;
    return r;
}

// 0x004567e0
void Vegetation::UnknownFunction4567e0(TextureMapManager* textures, unsigned char definition,
                                       const Vector3* position, unsigned char heightParameter,
                                       unsigned char radiusParameter) {
    field_0x12 = definition;
    field_0x0c.x = (unsigned short)(int)(g_UnknownGlobal59aebc->field_0x5ac * position->x);
    field_0x0c.y = (unsigned short)(int)(g_UnknownGlobal59aebc->field_0x5ac * position->y);
    field_0x0c.z = (unsigned short)(int)(g_UnknownGlobal59aebc->field_0x5ac * position->z);
    field_0x15 = radiusParameter;
    field_0x14 = heightParameter;
    field_0x16_bit0 = 1;
}

// 0x00456890: whether the object is beyond the detail band's 3D distance
// (then `fade` is 255), else the fade between the band's two distances.
void Vegetation::UnknownFunction456890(int* billboard, int* fade) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->field_0x58[field_0x12];
    UnknownEcoCamera* camera = ((UnknownEcoRenderTarget*)g_UnknownGlobal59aebc->field_0x18)->field_0x08;
    float dx = camera->field_0x170.x - field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
    float dz = camera->field_0x170.z - field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8;
    *billboard = 0;
    float outer = (float)g_UnknownGlobal59af14[g_UnknownGlobal59aebc->field_0x5c0].field_0x00;
    float inner = (float)g_UnknownGlobal59af14[g_UnknownGlobal59aebc->field_0x5c0].field_0x04;
    if (dx > outer || dz > outer) {
        *billboard = 1;
        *fade = 0xff;
        return;
    }
    float distance = FastSqrt(dz * dz + dx * dx);
    if (distance > outer) {
        *billboard = 1;
        *fade = 0xff;
        return;
    }
    if (definition->field_0x1fc) {
        if (distance < inner)
            *fade = 0;
        else
            *fade = (int)((distance - inner) * 255.0f / (outer - inner));
    } else {
        *fade = 0;
    }
}

// 0x00456a10
void Vegetation::UnknownFunction456a10(int billboard, int fade) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->field_0x58[field_0x12];
    UnknownEcoCamera* camera = ((UnknownEcoRenderTarget*)g_UnknownGlobal59aebc->field_0x18)->field_0x08;
    field_0x13 = (unsigned char)fade;
    if (billboard == field_0x16_bit0)
        return;
    if (field_0x16_bit0 < definition->field_0x1d8 && field_0x18)
        g_UnknownGlobal59af0c--;
    float maxX = -FLT_MAX;
    float minX = FLT_MAX;
    if (billboard < definition->field_0x1d8 && !field_0x18) {
        int vertexCount = definition->field_0x1f0[billboard];
        int indexCount = definition->field_0x1f4[billboard];
        int dwords = vertexCount * 8 + (indexCount + 1) / 2;
        field_0x18 = DebugMalloc(dwords * 4 + sizeof(AgeEntry), __FILE__, 0x1fb);
        AgeEntry* entry = (AgeEntry*)((int*)field_0x18 + dwords);
        g_UnknownGlobal59aebc->field_0x59c->UnknownFunction401050(entry, UnknownFunction456850, this, 0,
                                                                  dwords * 4 + sizeof(AgeEntry));
        g_UnknownGlobal59af0c++;
        memcpy((UnknownEcoVertex*)field_0x18 + vertexCount, definition->field_0x1e8[billboard], indexCount * 2);
        float radiusScale = definition->UnknownFunction455ff0(field_0x15) * definition->field_0x1d0;
        float heightScale = definition->UnknownFunction455f90(field_0x14) * definition->field_0x1cc;
        float c;
        float s;
        if (camera->field_0x17c.z != 0.0f) {
            float inverse = UnknownFunction460c70(camera->field_0x17c.x * camera->field_0x17c.x
                                                  + camera->field_0x17c.z * camera->field_0x17c.z) * radiusScale;
            c = inverse * camera->field_0x17c.z;
            s = -(inverse * camera->field_0x17c.x);
        } else {
            c = radiusScale;
            s = 0.0f;
        }
        Vector3 color;
        Vector3 ambient;
        color.x = g_UnknownGlobal59aebc->field_0x564.x;
        color.y = g_UnknownGlobal59aebc->field_0x564.y;
        color.z = g_UnknownGlobal59aebc->field_0x564.z;
        ambient.x = g_UnknownGlobal59aebc->field_0x570.x;
        ambient.y = g_UnknownGlobal59aebc->field_0x570.y;
        ambient.z = g_UnknownGlobal59aebc->field_0x570.z;
        unsigned int ambientColor = ECO_RGBA((int)(ambient.x * 255.0f), (int)(ambient.y * 255.0f),
                                             (int)(ambient.z * 255.0f), 255);
        UnknownEcoModelVertex* source = definition->field_0x1ec[billboard];
        UnknownEcoVertex* vertex = (UnknownEcoVertex*)field_0x18;
        int i;
        for (i = 0; i < vertexCount; i++) {
            int unlit = 0;
            Vector3 normal;
            vertex->position.x = c * source->position.x - s * source->position.z
                                 + field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
            vertex->position.y = field_0x0c.y * g_UnknownGlobal59aebc->field_0x5a8 + heightScale * source->position.y;
            vertex->position.z = field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8 + c * source->position.z
                                 + s * source->position.x;
            if (definition->field_0x1f8) {
                if (source->normal.y < 0.0f) {
                    unlit = 1;
                } else {
                    float length;
                    normal.x = c * source->normal.x - s * source->normal.z;
                    normal.z = c * source->normal.z + s * source->normal.x;
                    length = normal.x * normal.x + normal.z * normal.z;
                    if (length == 0.0f) {
                        normal = kVec3Zero;
                    } else {
                        length = FastInvSqrt(length);
                        normal.x = length * normal.x;
                        normal.y = 0.0f;
                        normal.z = length * normal.z;
                    }
                }
            } else {
                float length;
                normal.x = c * source->normal.x - s * source->normal.z;
                normal.y = source->normal.y;
                normal.z = s * source->normal.x + c * source->normal.z;
                length = normal.y * normal.y + normal.x * normal.x + normal.z * normal.z;
                if (length == 0.0f) {
                    normal = kVec3Zero;
                } else {
                    length = FastInvSqrt(length);
                    normal.x = length * normal.x;
                    normal.y = length * normal.y;
                    normal.z = length * normal.z;
                }
            }
            if (vertex->position.x > maxX)
                maxX = vertex->position.x;
            if (vertex->position.x < minX)
                minX = vertex->position.x;
            float intensity = -(normal.y * g_UnknownGlobal59aebc->field_0x57c.y
                                + normal.x * g_UnknownGlobal59aebc->field_0x57c.x
                                + normal.z * g_UnknownGlobal59aebc->field_0x57c.z);
            if (!unlit && source->normal.y > -0.95f && intensity > 0.0f) {
                float r = intensity * color.x + ambient.x;
                float g = intensity * color.y + ambient.y;
                float b = intensity * color.z + ambient.z;
                if (r > 1.0f)
                    r = 1.0f;
                if (g > 1.0f)
                    g = 1.0f;
                if (b > 1.0f)
                    b = 1.0f;
                vertex->diffuse = ECO_RGBA((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), 255);
            } else {
                vertex->diffuse = ambientColor;
            }
            vertex->reserved = 0;
            vertex->specular = 0;
            vertex->tu = source->tu;
            vertex->tv = source->tv;
            source++;
            vertex++;
        }
    }
    field_0x16_bit0 = billboard;
}

// 0x004570a0
CollisionObject* Vegetation::UnknownFunction4570a0(int index) {
    CollisionObject* object = g_UnknownGlobal59aebc->field_0x58[field_0x12]->field_0x20c[index];
    float x = field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
    float y = field_0x0c.y * g_UnknownGlobal59aebc->field_0x5a8;
    float z = field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8;
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->field_0x58[field_0x12];
    float radius = definition->UnknownFunction455ff0(field_0x15) / definition->field_0x18c;
    float height = definition->UnknownFunction455f90(field_0x14)
                   / g_UnknownGlobal59aebc->field_0x58[field_0x12]->field_0x180;
    if (object->field_0x50 == 4) {
        UnknownEcoSphereShape* shape = (UnknownEcoSphereShape*)object->field_0x54;
        shape->field_0x14 = radius;
        shape->field_0x18 = height;
        shape->field_0x1c(3, 0) = x;
        shape->field_0x1c(3, 1) = y;
        shape->field_0x1c(3, 2) = z;
    } else if (object->field_0x50 == 3) {
        UnknownEcoCapsuleShape* shape = (UnknownEcoCapsuleShape*)object->field_0x54;
        shape->field_0x20 = radius;
        shape->field_0x24 = height;
        shape->field_0x28(3, 0) = x;
        shape->field_0x28(3, 1) = y;
        shape->field_0x28(3, 2) = z;
    } else if (object->field_0x50 == 0) {
        UnknownEcoHullShape* shape = (UnknownEcoHullShape*)object->field_0x54;
        Matrix4 m = shape->field_0xc8;
        m(3, 0) = x;
        m(3, 1) = y;
        m(3, 2) = z;
        shape->field_0xc8(3, 1) = y;
        shape->field_0xc8(3, 2) = z;
        object->UnknownFunction435830(&m);
    }
    return object;
}

// 0x0045c6a0 (cdecl): fwrite through a running xor key, 1 KB at a time.
int UnknownFunction45c6a0(const unsigned char* data, int size, int count, FILE* file, unsigned char* key) {
    unsigned char buffer[0x400];
    int total = size * count;
    int blocks = total / 0x400;
    int block;
    int i;
    for (block = 0; block < blocks; block++) {
        for (i = 0; i < 0x400; i++) {
            buffer[i] = *data++ ^ *key;
            *key += buffer[i];
        }
        if (fwrite(buffer, 0x400, 1, file) != 1)
            return 0;
    }
    total %= 0x400;
    if (total == 0)
        return 1;
    for (i = 0; i < total; i++) {
        buffer[i] = *data++ ^ *key;
        *key += buffer[i];
    }
    return fwrite(buffer, total, 1, file) == 1;
}

// 0x0045c7b0 (cdecl): fread through the running xor key.
int UnknownFunction45c7b0(unsigned char* data, int size, int count, FILE* file, unsigned char* key) {
    int total = size * count;
    int read = fread(data, size, count, file);
    int i;
    if (read != count) {
        if (file->_flag & 0x10) {
            if (!read)
                return 0;
            total = read * size;
        }
        if (file->_flag & 0x20)
            return 0;
    }
    for (i = 0; i < total; i++) {
        unsigned char value = *data;
        *data = *key ^ value;
        *key += value;
        data++;
    }
    return 1;
}

// 0x00456050: loads the billboard texture and the .slt model (vertices,
// faces and its texture); `modelFlags` is the detail band's model flag.
int UnknownEcoDefinition::UnknownFunction456050(TextureMapManager* textures, int modelFlags) {
    char name[0x104];
    char section[0x80];
    if (field_0x080[0]) {
        field_0x1e0 = UnknownFunction50a590(textures, field_0x080, modelFlags, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
        if (!field_0x1e0->UnknownVirtualSlot7())
            field_0x1e0->UnknownVirtualSlot8(1, 0, 0);
    }
    if (field_0x000[0]) {
        int nameLength = strlen(field_0x000);
        int length = nameLength > 0x103 ? 0x103 : nameLength;
        strncpy(name, field_0x000, length);
        name[length] = 0;
        strcat(name, ".slt");
        UnknownTextureStream* stream = new(__FILE__, 0xc0) UnknownTextureStream(g_UnknownResourceManager572b44);
        if (stream->UnknownFunction460f50(name, "r", 0)) {
            int lod;
            UnknownParameterBlock* block = new(__FILE__, 0xc2) UnknownParameterBlock;
            block->UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
            block->UnknownFunction4b78f0("Material - 0");
            block->UnknownFunction4b7b30("TextureMap", name, -1);
            int format = 0x613;
            if (!g_UnknownGlobal56e26c->field_0x2d0 && (g_UnknownGlobal56e26c->field_0x10->field_0x1c0 & 8)
                && g_UnknownGlobal56e26c->UnknownVirtualSlot22("KeyColorTrees", 0))
                format = g_UnknownGlobal56e26c->field_0x10->field_0x28;
            field_0x1e4 = UnknownFunction50a590(textures, name, format, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
            if (!field_0x1e4->UnknownVirtualSlot7()) {
                int loaded = field_0x1e4->field_0x20;
                if (loaded == 0x22b || loaded == 0x235 || loaded == 0x378 || loaded == 0x613)
                    field_0x1e4->UnknownVirtualSlot18(field_0x1dc);
                field_0x1e4->UnknownVirtualSlot8(1, 0, 0);
            }
            block->UnknownFunction4b78f0("LOD Information");
            block->UnknownFunction4b7f10("NumberOfLOD", 0, &field_0x1d8);
            if (field_0x1d8 > kMaxLods)
                field_0x1d8 = kMaxLods;
            float maxY = -FLT_MAX;
            float minY = FLT_MAX;
            float maxX = -FLT_MAX;
            float minX = FLT_MAX;
            for (lod = 0; lod < field_0x1d8; lod++) {
                int faces;
                int i;
                sprintf(section, "LOD %d - Surface 0", lod);
                block->UnknownFunction4b78f0(section);
                block->UnknownFunction4b7f10("NumberOfVertices", 0, &field_0x1f0[lod]);
                block->UnknownFunction4b7f10("NumberOfFaces", 0, &faces);
                field_0x1f4[lod] = faces * 3;
                field_0x1ec[lod] = (UnknownEcoModelVertex*)DebugMalloc(
                    field_0x1f0[lod] * sizeof(UnknownEcoModelVertex) + faces * 3 * sizeof(unsigned short), __FILE__, 0x106);
                field_0x1e8[lod] = (unsigned short*)(field_0x1ec[lod] + field_0x1f0[lod]);
                sprintf(section, "LOD %d - Surface 0 - Vertices", lod);
                block->UnknownFunction4b7f70(section);
                for (i = 0; i < field_0x1f0[lod]; i++) {
                    UnknownEcoModelVertex* vertex = &field_0x1ec[lod][i];
                    block->UnknownFunction4b8010(0);
                    block->UnknownFunction4b81c0(0, &vertex->position.x);
                    block->UnknownFunction4b81c0(1, &vertex->position.y);
                    block->UnknownFunction4b81c0(2, &vertex->position.z);
                    if (vertex->position.y > maxY)
                        maxY = vertex->position.y;
                    if (vertex->position.y < minY)
                        minY = vertex->position.y;
                    if (vertex->position.x > maxX)
                        maxX = vertex->position.x;
                    if (vertex->position.x < minX)
                        minX = vertex->position.x;
                    block->UnknownFunction4b81c0(3, &vertex->normal.x);
                    block->UnknownFunction4b81c0(4, &vertex->normal.y);
                    block->UnknownFunction4b81c0(5, &vertex->normal.z);
                    block->UnknownFunction4b81c0(6, &vertex->tu);
                    block->UnknownFunction4b81c0(7, &vertex->tv);
                    if (vertex->tv < 0.0f)
                        vertex->tv = vertex->tv + 1.0f;
                }
                sprintf(section, "LOD %i - Surface 0 - Faces", lod);
                block->UnknownFunction4b7f70(section);
                unsigned short* index = field_0x1e8[lod];
                for (i = 0; i < field_0x1f4[lod] / 3; i++) {
                    int a;
                    int b;
                    int c;
                    block->UnknownFunction4b8010(0);
                    block->UnknownFunction4b8180(0, &a);
                    block->UnknownFunction4b8180(1, &b);
                    block->UnknownFunction4b8180(2, &c);
                    *index++ = (unsigned short)a;
                    *index++ = (unsigned short)b;
                    *index++ = (unsigned short)c;
                }
            }
            if (block)
                delete block;
            field_0x1cc = 1.0f / (maxY - minY);
            field_0x1d0 = 2.0f / (maxX - minX);
            field_0x1d4 = field_0x1cc * minY;
        }
        if (stream)
            delete stream;
    }
    return 1;
}

// 0x00457480
int EcoSystem::UnknownFunction457480(char* path, UnknownTextureStream* stream) {
    char name[0x104];
    char value[0x80];
    char section[0x80];
    int i;
    if (!stream && strstr(path, ".est")) {
        int pathLength = strlen(path);
        int length = pathLength > 0x103 ? 0x103 : pathLength;
        strncpy(name, path, length);
        name[length] = 0;
        strcpy(strstr(name, ".est"), ".esb");
        UnknownTextureStream* probe = new(__FILE__, 0x2eb) UnknownTextureStream(g_UnknownResourceManager572b44);
        if (probe->UnknownFunction460f50(name, "rb", 0))
            strcpy(strstr(path, ".est"), ".esb");
        if (probe)
            delete probe;
    }
    if (strstr(path, ".est")) {
        float total;
        GetPrivateProfileString("EcoSystem", "Method", "NONE", value, 0x80, path);
        if (!strcmp(value, "NONE"))
            return 0;
        if (!_stricmp(value, "Authored"))
            field_0x30 = 1;
        else if (!_stricmp(value, "Auto"))
            field_0x30 = 2;
        else
            return 0;
        field_0x34 = GetPrivateProfileInt("EcoSystem", "TotalObjects", 40000, path);
        total = 0.0f;
        for (i = 1; i < 256; i++) {
            sprintf(section, "Vegetation_%d", i);
            GetPrivateProfileString(section, "Name", "NONE", value, 0x80, path);
            if (strcmp(value, "NONE")) {
                int red;
                int green;
                int blue;
                field_0x58[i] = new(__FILE__, 0x31f) UnknownEcoDefinition;
                strcpy(field_0x58[i]->field_0x000, value);
                GetPrivateProfileString(section, "BillboardName", "NONE", value, 0x80, path);
                strcpy(field_0x58[i]->field_0x080, value);
                field_0x58[i]->field_0x180 = UnknownFunction47b8a0(section, "MeanHeight", 10.0, path);
                field_0x58[i]->field_0x184 = UnknownFunction47b8a0(section, "MinHeight", 5.0, path);
                field_0x58[i]->field_0x188 = UnknownFunction47b8a0(section, "MaxHeight", 15.0, path);
                field_0x58[i]->field_0x18c = UnknownFunction47b8a0(section, "MeanRadius", 10.0, path);
                field_0x58[i]->field_0x190 = UnknownFunction47b8a0(section, "MinRadius", 5.0, path);
                field_0x58[i]->field_0x194 = UnknownFunction47b8a0(section, "MaxRadius", 15.0, path);
                field_0x58[i]->field_0x1b8 = UnknownFunction47b8a0(section, "ULeft", 0.0, path) * (1.0f / 256.0f);
                field_0x58[i]->field_0x1bc = UnknownFunction47b8a0(section, "URight", 1.0, path) * (1.0f / 256.0f);
                field_0x58[i]->field_0x1c4 = UnknownFunction47b8a0(section, "VBottom", 0.0, path) * (1.0f / 256.0f);
                field_0x58[i]->field_0x1c0 = UnknownFunction47b8a0(section, "UCenter", 1.0, path) * (1.0f / 256.0f);
                field_0x58[i]->field_0x1c8 = UnknownFunction47b8a0(section, "VTop", 1.0, path) * (1.0f / 256.0f);
                field_0x58[i]->field_0x1f8 = GetPrivateProfileInt(section, "UsePlanarLighting", 1, path) != 0;
                field_0x58[i]->field_0x1fc = GetPrivateProfileInt(section, "BlendLODs", 1, path) != 0;
                field_0x58[i]->field_0x200 = UnknownFunction47b8a0(section, "PercentBias", 1.0, path);
                red = GetPrivateProfileInt(section, "KeyColorRed", 0x5a, path);
                green = GetPrivateProfileInt(section, "KeyColorGreen", 0x5b, path);
                blue = GetPrivateProfileInt(section, "KeyColorBlue", 0xb, path);
                field_0x58[i]->field_0x1dc = (red << 16) | (green << 8) | blue;
                total = total + field_0x58[i]->field_0x200;
                UnknownFunction458360(path, i, field_0x58[i]);
            }
        }
        if (total != 100.0f) {
            for (i = 1; i < 256; i++) {
                if (field_0x58[i])
                    field_0x58[i]->field_0x200 = 100.0f / total * field_0x58[i]->field_0x200;
            }
        }
        GetPrivateProfileString("EcoSystem", "PlacementBmp", "NONE", value, 0x80, path);
        if (!strcmp(value, "NONE")) {
            field_0x45c[0] = 0;
        } else {
            int valueLength = strlen(value);
            int length = valueLength > 0x7f ? 0x7f : valueLength;
            strncpy(field_0x45c, value, length);
            field_0x45c[length] = 0;
        }
        if (field_0x30 == 2) {
            for (i = 0; i < 256; i++) {
                if (field_0x58[i]) {
                    sprintf(section, "Vegetation_%d", i);
                    field_0x58[i]->field_0x198 = UnknownFunction47b8a0(section, "MeanSlope", 45.0, path) * (1.0f / 90.0f);
                    field_0x58[i]->field_0x19c =
                        UnknownFunction47b8a0(section, "StandardDeviationSlope", 45.0, path) * (1.0f / 90.0f);
                    field_0x58[i]->field_0x1a0 =
                        (UnknownFunction47b8a0(section, "MeanAspect", 180.0, path) - 180.0f) * (1.0f / 90.0f);
                    field_0x58[i]->field_0x1a4 =
                        (UnknownFunction47b8a0(section, "StandardDeviationAspect", 180.0, path) - 180.0f) * (1.0f / 90.0f);
                    field_0x58[i]->field_0x1a8 = UnknownFunction47b8a0(section, "MeanDrainage", 0.5, path);
                    field_0x58[i]->field_0x1ac = UnknownFunction47b8a0(section, "StandardDeviationDrainage", 0.5, path);
                    field_0x58[i]->field_0x1b0 = UnknownFunction47b8a0(section, "MeanAltitude", 0.5, path);
                    field_0x58[i]->field_0x1b4 = UnknownFunction47b8a0(section, "StandardDeviationAltitude", 0.5, path);
                    GetPrivateProfileString(section, "ProbabilityTga", "NONE", value, 0x80, path);
                    if (!strcmp(value, "NONE"))
                        field_0x58[i]->field_0x100[0] = 0;
                    else
                        strcpy(field_0x58[i]->field_0x100, value);
                }
            }
            field_0x458 = UnknownFunction47b8a0("EcoSystem", "NorthAngle", 0.0, path);
            GetPrivateProfileString("EcoSystem", "ProbabilityTga", "NONE", value, 0x80, path);
            if (!strcmp(value, "NONE")) {
                field_0x4dc[0] = 0;
            } else {
                int valueLength = strlen(value);
                int length = valueLength > 0x7f ? 0x7f : valueLength;
                strncpy(field_0x4dc, value, length);
                field_0x4dc[length] = 0;
            }
        }
        return 1;
    }
    if (strstr(path, ".esb")) {
        UnknownFunction458f70(path, stream);
        return 1;
    }
    return 0;
}

// 0x00457ed0: builds the definition's collision objects as children.
void EcoSystem::UnknownFunction457ed0(UnknownEcoDefinition* definition) {
    Vector3 vertices[32];
    int indices[96];
    int i;
    int j;
    if (definition->field_0x204 == 0) {
        definition->field_0x20c = 0;
        return;
    }
    int category = g_MemTagStack->Push("Collision");
    definition->field_0x20c = (CollisionObject**)DebugMalloc(definition->field_0x204 * sizeof(CollisionObject*),
                                                             __FILE__, 0x3a5);
    for (i = 0; i < definition->field_0x204; i++) {
        UnknownEcoCollisionDefinition* shape = &definition->field_0x208[i];
        definition->field_0x20c[i] = new(__FILE__, 0x3ad) CollisionObject(1);
        definition->field_0x20c[i]->UnknownFunction4320f0(g_UnknownGlobal56e26c->field_0x10, 0, 0, 1);
        UnknownFunction469190(definition->field_0x20c[i], -1);
        definition->field_0x20c[i]->field_0x64 = 0x3e8;
        if (shape->field_0x00 == 0) {
            definition->field_0x20c[i]->field_0x64 = 0x3e9;
            Vector3* hull = new(__FILE__, 0x3b8) Vector3[definition->field_0x1f0[0]];
            int* hullIndices = new(__FILE__, 0x3b9) int[definition->field_0x1f4[0]];
            for (j = 0; j < definition->field_0x1f4[0]; j++)
                hullIndices[j] = definition->field_0x1e8[0][j];
            for (j = 0; j < definition->field_0x1f0[0]; j++) {
                hull[j].x = definition->field_0x1ec[0][j].position.x;
                hull[j].y = definition->field_0x1ec[0][j].position.y;
                hull[j].z = definition->field_0x1ec[0][j].position.z;
            }
            definition->field_0x20c[i]->field_0x64 = 0x3e9;
            definition->field_0x20c[i]->UnknownFunction4328b0(hull, hullIndices, definition->field_0x1f4[0] / 3,
                                                               definition->field_0x1f0[0]);
            UnknownSetIdentity(&((UnknownEcoHullShape*)definition->field_0x20c[i]->field_0x54)->field_0xc8);
            delete hull;
            delete hullIndices;
        } else if (shape->field_0x00 == 2) {
            definition->field_0x20c[i]->UnknownFunction4329a0(shape->field_0x04, shape->field_0x1c);
        } else if (shape->field_0x00 == 3) {
            definition->field_0x20c[i]->UnknownFunction432a20(shape->field_0x04, shape->field_0x10, shape->field_0x1c);
        } else if (shape->field_0x00 == 1) {
            definition->field_0x20c[i]->field_0x64 = 0x3e9;
            for (j = 0; j < 16; j++) {
                float angle = j * 0.39269909f;
                vertices[j].x = vertices[j + 16].x = (float)cos(angle) * shape->field_0x1c;
                vertices[j].z = vertices[j + 16].z = (float)sin(angle) * shape->field_0x1c;
                vertices[j].y = shape->field_0x20;
                vertices[j + 16].y = 0.0f;
            }
            for (j = 0; j < 32; j++)
                vertices[j] = UnknownEcoOffset(&vertices[j], &shape->field_0x04);
            for (j = 0; j < 16; j++) {
                int next = j + 1;
                if (next >= 16)
                    next = 0;
                indices[j * 6 + 0] = j;
                indices[j * 6 + 1] = next;
                indices[j * 6 + 2] = next + 16;
                indices[j * 6 + 3] = j;
                indices[j * 6 + 4] = next + 16;
                indices[j * 6 + 5] = j + 16;
            }
            definition->field_0x20c[i]->UnknownFunction4328b0(vertices, indices, 32, 32);
            UnknownSetIdentity(&((UnknownEcoHullShape*)definition->field_0x20c[i]->field_0x54)->field_0xc8);
        }
    }
    g_MemTagStack->Pop(category);
}

// 0x00458360
void EcoSystem::UnknownFunction458360(const char* path, int index, UnknownEcoDefinition* definition) {
    char key[0x80];
    char value[0x80];
    char section[0x100];
    char kind[0x80];
    int i;
    sprintf(section, "Vegetation_%d", index);
    int count = GetPrivateProfileInt(section, "NumCollisionObjects", 0, path);
    definition->field_0x204 = count;
    if (count > 0) {
    definition->field_0x208 = (UnknownEcoCollisionDefinition*)DebugMalloc(count * sizeof(UnknownEcoCollisionDefinition),
                                                                          __FILE__, 0x40c);
    for (i = 1; i - 1 < definition->field_0x204; i++) {
        UnknownEcoCollisionDefinition* shape = &definition->field_0x208[i - 1];
        sprintf(key, "CollisionObject%i", i);
        GetPrivateProfileString(section, key, "NONE", kind, 0x80, path);
        if (!_stricmp(kind, "GEOMETRY")) {
            shape->field_0x00 = 0;
        } else if (!_stricmp(kind, "SPHERE")) {
            shape->field_0x00 = 2;
            sprintf(key, "CollisionObject%iCenter", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->field_0x04.x = (float)atof(strtok(value, ","));
            shape->field_0x04.y = (float)atof(strtok(0, ","));
            shape->field_0x04.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iRadius", i);
            shape->field_0x1c = UnknownFunction47b8a0(section, key, 0.0, path);
        } else if (!_stricmp(kind, "RADIUSEDLINE")) {
            shape->field_0x00 = 3;
            sprintf(key, "CollisionObject%iStart", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->field_0x04.x = (float)atof(strtok(value, ","));
            shape->field_0x04.y = (float)atof(strtok(0, ","));
            shape->field_0x04.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iEnd", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->field_0x10.x = (float)atof(strtok(value, ","));
            shape->field_0x10.y = (float)atof(strtok(0, ","));
            shape->field_0x10.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iRadius", i);
            shape->field_0x1c = UnknownFunction47b8a0(section, key, 0.0, path);
        } else if (!_stricmp(kind, "CYLINDER")) {
            shape->field_0x00 = 1;
            sprintf(key, "CollisionObject%iBottom", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->field_0x04.x = (float)atof(strtok(value, ","));
            shape->field_0x04.y = (float)atof(strtok(0, ","));
            shape->field_0x04.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iRadius", i);
            shape->field_0x1c = UnknownFunction47b8a0(section, key, 0.0, path);
            sprintf(key, "CollisionObject%iHeight", i);
            shape->field_0x20 = UnknownFunction47b8a0(section, key, 0.0, path);
        }
    }
    } else {
        definition->field_0x208 = 0;
    }
}

// 0x00458da0: writes the placed objects as text next to the .est.
void EcoSystem::UnknownFunction458da0(const char* path) {
    char name[0x104];
    int i;
    int pathLength = strlen(path);
    int length = pathLength > 0x103 ? 0x103 : pathLength;
    strncpy(name, path, length);
    name[length] = 0;
    strcpy(strrchr(name, '.'), ".txt");
    FILE* file = fopen(name, "w");
    if (!file)
        return;
    UnknownFunction461d40(file, "%i\n", field_0x55c);
    for (i = 0; i < field_0x55c; i++) {
        UnknownEcoDefinition* definition = field_0x58[field_0x38[i].field_0x12];
        float radius = definition->UnknownFunction455ff0(field_0x38[i].field_0x15);
        float height = definition->UnknownFunction455f90(field_0x38[i].field_0x14);
        float z = field_0x38[i].field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8;
        float y = field_0x38[i].field_0x0c.y * g_UnknownGlobal59aebc->field_0x5a8;
        float x = field_0x38[i].field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
        UnknownFunction461d40(file, "%i,%f,%f,%f,%f,%f\n", field_0x38[i].field_0x12, x, y, z,
                              definition->UnknownFunction455ff0(field_0x38[i].field_0x15),
                              definition->UnknownFunction455f90(field_0x38[i].field_0x14));
    }
    fclose(file);
}

// 0x004598d0: places the objects the .esb lists.
int EcoSystem::UnknownFunction4598d0() {
    UnknownTextureStream* stream = field_0x590;
    int i;
    for (i = 0; i < field_0x55c; i++) {
        unsigned char definition;
        UnknownEcoCoordinates coordinates;
        unsigned char heightParameter;
        unsigned char radiusParameter;
        stream->UnknownFunction461640(&definition, 1, 1);
        stream->UnknownFunction461640(&coordinates, 6, 1);
        stream->UnknownFunction461640(&heightParameter, 1, 1);
        stream->UnknownFunction461640(&radiusParameter, 1, 1);
        field_0x38[i].UnknownFunction4567a0(field_0x48, definition, &coordinates, heightParameter, radiusParameter);
        float radius = field_0x58[definition]->UnknownFunction455ff0(radiusParameter);
        float height = field_0x58[definition]->UnknownFunction455f90(heightParameter);
        float x = field_0x38[i].field_0x0c.x * g_UnknownGlobal59aebc->field_0x5a8;
        float y = field_0x38[i].field_0x0c.y * g_UnknownGlobal59aebc->field_0x5a8;
        float z = field_0x38[i].field_0x0c.z * g_UnknownGlobal59aebc->field_0x5a8;
        unsigned int code = g_collisionQuadTree->ComputeCode(x - radius, z - radius, x + radius, z + radius);
        g_MemTagStack->Push("QuadTree");
        g_collisionQuadTree->Insert(&field_0x38[i], code, y, y + height);
        g_MemTagStack->Push("EcoSystem");
    }
    if (!field_0x594) {
        if (field_0x590)
            delete field_0x590;
        field_0x590 = 0;
    }
    return 1;
}

// 0x00459b40: places the objects the PlacementBmp paints (one pixel per
// terrain cell, the palette index selecting the definition).
int EcoSystem::UnknownFunction459b40() {
    if (!field_0x45c[0])
        return 0;
    UnknownBitmapFile* bitmap = UnknownFunction424140(field_0x45c, 0);
    if (!bitmap)
        return 0;
    int width = bitmap->infoHeader.width;
    int height = bitmap->infoHeader.height;
    float cellX = g_collisionQuadTree->field_0x50 / width;
    float cellZ = g_collisionQuadTree->field_0x54 / height;
    unsigned char* pixel = (unsigned char*)bitmap->bits;
    int row;
    int column;
    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column++) {
            int index = *pixel++;
            if (index && field_0x58[index]) {
                Vector3 position;
                if (field_0x55c == field_0x34)
                    goto done;
                position.x = (column + 0.5f) * cellX;
                position.y = 0.0f;
                position.z = (row + 0.5f) * cellZ;
                field_0x44->QueryGround(&position, 0, 0, 0);
                unsigned char heightParameter = field_0x58[index]->UnknownFunction455f50();
                unsigned char radiusParameter = field_0x58[index]->UnknownFunction455f60(
                    field_0x58[index]->UnknownFunction455f90(heightParameter));
                field_0x38[field_0x55c].UnknownFunction4567e0(field_0x48, index, &position, heightParameter,
                                                             radiusParameter);
                field_0x55c++;
            }
        }
    }
done:
    UnknownFunction4245b0(bitmap);
    return 1;
}
