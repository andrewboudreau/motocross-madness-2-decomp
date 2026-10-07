// Near-miss candidates of D:\aardvark\VC\krusty2\EcoSystem.cpp, kept out of
// src/reconstructed/EcoSystem.cpp until they match. They compile against
// the reconstructed header; the exact functions they call live in the src
// unit (bound by address). What differs from retail (docs/ECOSYSTEM.md):
//   0x00456890  the camera pointer and the z store are scheduled before the
//               first fmul in retail (13 bytes); the block compiles the same
//               under every data-flow-equivalent spelling tried (locals
//               before/after the camera load, no camera local, the view as
//               a local, one declaration per statement, a position
//               reference, the products before the camera, `-=`, int
//               locals/casts, a Vector3 for the position, z first, a delta
//               vector), and every statement reordering only moves further
//               away; helper boundaries do not move it either (inline and
//               static accessors for the eye and the scaled coordinate,
//               pointer/reference/by-value helpers, a struct copy, the
//               difference as a Vector3: identical bytes or worse), and
//               /G6 scores lower (267)
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
//   0x0045c040  one byte: the spill slot of the probe stream's `new`
//               temporary (1618/1619)
//   0x00459ce0  the frame slots: same size, the locals and temporaries
//               are assigned in a different order (323/3300)
//   0x0045b060  frame 0xe0 for 0xf0 and the slot order; the lean of the
//               side normals is spilled differently (461/3919)
//   0x0045c7b0  register roles and byte update order

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "../../src/reconstructed/EcoSystem.h"

#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/PeakHold.h"
#include "../../src/reconstructed/D3DConstants.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/TypeRegistry.h"
#include "../../src/reconstructed/UnknownResourceManager.h"
#include "../../src/reconstructed/bmpfile.h"
#include "../../src/reconstructed/Tgafile.h"

// The unit's file statics these candidates read (same addresses as the src
// unit; see its definitions).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static int g_UnknownGlobal56a128 = -1;        // 0x0056a128: the overlay page
static int g_UnknownGlobal56a12c = 1;         // 0x0056a12c: drawing enabled
static int g_UnknownGlobal59aec0;             // 0x0059aec0: geometry draw time
static UnknownPeakHold g_UnknownGlobal59aef0(5000); // 0x0059aef0
static UnknownPeakHold g_UnknownGlobal59aec8(5000); // 0x0059aec8
static int g_UnknownGlobal59aee4;             // 0x0059aee4: draw time
static int g_UnknownGlobal59aee8;             // 0x0059aee8: classification time
static int g_UnknownGlobal59af04;             // 0x0059af04: geometry objects drawn
static int g_UnknownGlobal59af08;             // 0x0059af08: billboards drawn
static int g_UnknownGlobal59aefc;             // 0x0059aefc: textures were preloaded
static int g_UnknownGlobal59af0c;             // 0x0059af0c
static int g_UnknownGlobal59af10;             // 0x0059af10: textures preloaded
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

// By-value component read: it makes VC6 load the other operand first
// (0x004567e0, now exact in the src unit, needs the same).
static inline float UnknownEcoZ(const Vector3& v) {
    return v.z;
}

// The vector sum as 0x00458246 computes it (a value-returning inline in
// operator+ form). Retail sums z vertex-first (`fld [vertex.z]; fadd
// [start.z]`); only the by-value read of the vertex's z gives that order.
static inline Vector3 UnknownEcoOffset(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + UnknownEcoZ(b));
}

// 0x00456890: whether the object is beyond the detail band's 3D distance
// (then `fade` is 255), else the fade between the band's two distances.
void Vegetation::TestDistance(int* billboard, int* fade) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->definitionTable[definitionIndex];
    UnknownEcoCamera* camera = ((UnknownEcoRenderTarget*)g_UnknownGlobal59aebc->field_0x18)->field_0x08;
    float dx = camera->field_0x170.x - quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
    float dz = camera->field_0x170.z - quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
    *billboard = 0;
    float outer = (float)g_UnknownGlobal59af14[g_UnknownGlobal59aebc->detailLevel].geometryDistance;
    float inner = (float)g_UnknownGlobal59af14[g_UnknownGlobal59aebc->detailLevel].fadeStartDistance;
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
    if (definition->blendLods) {
        if (distance < inner)
            *fade = 0;
        else
            *fade = (int)((distance - inner) * 255.0f / (outer - inner));
    } else {
        *fade = 0;
    }
}

// 0x004570a0
CollisionObject* Vegetation::GetCollisionObject(int index) {
    CollisionObject* object = g_UnknownGlobal59aebc->definitionTable[definitionIndex]->collisionObjects[index];
    float x = quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
    float y = quantizedPosition.y * g_UnknownGlobal59aebc->unitsPerCoordinate;
    float z = quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->definitionTable[definitionIndex];
    float radius = definition->RadiusForParameter(radiusParam) / definition->meanRadius;
    float height = definition->HeightForParameter(heightParam)
                   / g_UnknownGlobal59aebc->definitionTable[definitionIndex]->meanHeight;
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
int UnknownEcoDefinition::LoadModel(TextureMapManager* textures, int modelFlags) {
    char name[0x104];
    char section[0x80];
    if (billboardName[0]) {
        billboardTexture = UnknownFunction50a590(textures, billboardName, modelFlags, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
        if (!billboardTexture->UnknownVirtualSlot7())
            billboardTexture->UnknownVirtualSlot8(1, 0, 0);
    }
    if (name[0]) {
        int nameLength = strlen(name);
        int length = nameLength > 0x103 ? 0x103 : nameLength;
        strncpy(name, name, length);
        name[length] = 0;
        strcat(name, ".slt");
        UnknownTextureStream* stream = new(__FILE__, 0xc0) UnknownTextureStream(g_UnknownResourceManager572b44);
        if (stream->UnknownFunction460f50(name, "r", 0)) {
            int lod;
            UnknownParameterBlock* block = new(__FILE__, 0xc2) UnknownParameterBlock;
            block->UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
            block->UnknownFunction4b78f0("Material - 0");
            block->UnknownFunction4b7b30("TextureMap", name, -1);
            int format = 1555;
            if (!g_TrackGame->field_0x2d0 && (g_TrackGame->field_0x10->field_0x1c0 & 8)
                && g_TrackGame->GetRegistryFlag("KeyColorTrees", 0))
                format = g_TrackGame->field_0x10->field_0x28;
            modelTexture = UnknownFunction50a590(textures, name, format, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
            if (!modelTexture->UnknownVirtualSlot7()) {
                int loaded = modelTexture->field_0x20;
                if (loaded == 555 || loaded == 565 || loaded == 888 || loaded == 1555)
                    modelTexture->UnknownVirtualSlot18(keyColor);
                modelTexture->UnknownVirtualSlot8(1, 0, 0);
            }
            block->UnknownFunction4b78f0("LOD Information");
            block->UnknownFunction4b7f10("NumberOfLOD", 0, &lodCount);
            if (lodCount > kMaxLods)
                lodCount = kMaxLods;
            float maxY = -FLT_MAX;
            float minY = FLT_MAX;
            float maxX = -FLT_MAX;
            float minX = FLT_MAX;
            for (lod = 0; lod < lodCount; lod++) {
                int faces;
                int i;
                sprintf(section, "LOD %d - Surface 0", lod);
                block->UnknownFunction4b78f0(section);
                block->UnknownFunction4b7f10("NumberOfVertices", 0, &modelVertexCount[lod]);
                block->UnknownFunction4b7f10("NumberOfFaces", 0, &faces);
                modelIndexCount[lod] = faces * 3;
                modelVertices[lod] = (UnknownEcoModelVertex*)DebugMalloc(
                    modelVertexCount[lod] * sizeof(UnknownEcoModelVertex) + faces * 3 * sizeof(unsigned short), __FILE__, 0x106);
                modelIndices[lod] = (unsigned short*)(modelVertices[lod] + modelVertexCount[lod]);
                sprintf(section, "LOD %d - Surface 0 - Vertices", lod);
                block->UnknownFunction4b7f70(section);
                for (i = 0; i < modelVertexCount[lod]; i++) {
                    UnknownEcoModelVertex* vertex = &modelVertices[lod][i];
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
                unsigned short* index = modelIndices[lod];
                for (i = 0; i < modelIndexCount[lod] / 3; i++) {
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
            modelHeightScale = 1.0f / (maxY - minY);
            modelRadiusScale = 2.0f / (maxX - minX);
            field_0x1d4 = modelHeightScale * minY;
        }
        if (stream)
            delete stream;
    }
    return 1;
}

// 0x00457480
int EcoSystem::ReadEst(char* path, UnknownTextureStream* stream) {
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
            method = 1;
        else if (!_stricmp(value, "Auto"))
            method = 2;
        else
            return 0;
        totalObjects = GetPrivateProfileInt("EcoSystem", "TotalObjects", 40000, path);
        total = 0.0f;
        for (i = 1; i < 256; i++) {
            sprintf(section, "Vegetation_%d", i);
            GetPrivateProfileString(section, "Name", "NONE", value, 0x80, path);
            if (strcmp(value, "NONE")) {
                int red;
                int green;
                int blue;
                definitionTable[i] = new(__FILE__, 0x31f) UnknownEcoDefinition;
                strcpy(definitionTable[i]->name, value);
                GetPrivateProfileString(section, "BillboardName", "NONE", value, 0x80, path);
                strcpy(definitionTable[i]->billboardName, value);
                definitionTable[i]->meanHeight = UnknownFunction47b8a0(section, "MeanHeight", 10.0, path);
                definitionTable[i]->minHeight = UnknownFunction47b8a0(section, "MinHeight", 5.0, path);
                definitionTable[i]->maxHeight = UnknownFunction47b8a0(section, "MaxHeight", 15.0, path);
                definitionTable[i]->meanRadius = UnknownFunction47b8a0(section, "MeanRadius", 10.0, path);
                definitionTable[i]->minRadius = UnknownFunction47b8a0(section, "MinRadius", 5.0, path);
                definitionTable[i]->maxRadius = UnknownFunction47b8a0(section, "MaxRadius", 15.0, path);
                definitionTable[i]->uLeft = UnknownFunction47b8a0(section, "ULeft", 0.0, path) * (1.0f / 256.0f);
                definitionTable[i]->uRight = UnknownFunction47b8a0(section, "URight", 1.0, path) * (1.0f / 256.0f);
                definitionTable[i]->vBottom = UnknownFunction47b8a0(section, "VBottom", 0.0, path) * (1.0f / 256.0f);
                definitionTable[i]->uCenter = UnknownFunction47b8a0(section, "UCenter", 1.0, path) * (1.0f / 256.0f);
                definitionTable[i]->vTop = UnknownFunction47b8a0(section, "VTop", 1.0, path) * (1.0f / 256.0f);
                definitionTable[i]->usePlanarLighting = GetPrivateProfileInt(section, "UsePlanarLighting", 1, path) != 0;
                definitionTable[i]->blendLods = GetPrivateProfileInt(section, "BlendLODs", 1, path) != 0;
                definitionTable[i]->percentBias = UnknownFunction47b8a0(section, "PercentBias", 1.0, path);
                red = GetPrivateProfileInt(section, "KeyColorRed", 0x5a, path);
                green = GetPrivateProfileInt(section, "KeyColorGreen", 0x5b, path);
                blue = GetPrivateProfileInt(section, "KeyColorBlue", 0xb, path);
                definitionTable[i]->keyColor = (red << 16) | (green << 8) | blue;
                total = total + definitionTable[i]->percentBias;
                ReadCollisionObjects(path, i, definitionTable[i]);
            }
        }
        if (total != 100.0f) {
            for (i = 1; i < 256; i++) {
                if (definitionTable[i])
                    definitionTable[i]->percentBias = 100.0f / total * definitionTable[i]->percentBias;
            }
        }
        GetPrivateProfileString("EcoSystem", "PlacementBmp", "NONE", value, 0x80, path);
        if (!strcmp(value, "NONE")) {
            placementBmp[0] = 0;
        } else {
            int valueLength = strlen(value);
            int length = valueLength > 0x7f ? 0x7f : valueLength;
            strncpy(placementBmp, value, length);
            placementBmp[length] = 0;
        }
        if (method == 2) {
            for (i = 0; i < 256; i++) {
                if (definitionTable[i]) {
                    sprintf(section, "Vegetation_%d", i);
                    definitionTable[i]->meanSlope = UnknownFunction47b8a0(section, "MeanSlope", 45.0, path) * (1.0f / 90.0f);
                    definitionTable[i]->standardDeviationSlope =
                        UnknownFunction47b8a0(section, "StandardDeviationSlope", 45.0, path) * (1.0f / 90.0f);
                    definitionTable[i]->meanAspect =
                        (UnknownFunction47b8a0(section, "MeanAspect", 180.0, path) - 180.0f) * (1.0f / 90.0f);
                    definitionTable[i]->standardDeviationAspect =
                        (UnknownFunction47b8a0(section, "StandardDeviationAspect", 180.0, path) - 180.0f) * (1.0f / 90.0f);
                    definitionTable[i]->meanDrainage = UnknownFunction47b8a0(section, "MeanDrainage", 0.5, path);
                    definitionTable[i]->standardDeviationDrainage = UnknownFunction47b8a0(section, "StandardDeviationDrainage", 0.5, path);
                    definitionTable[i]->meanAltitude = UnknownFunction47b8a0(section, "MeanAltitude", 0.5, path);
                    definitionTable[i]->standardDeviationAltitude = UnknownFunction47b8a0(section, "StandardDeviationAltitude", 0.5, path);
                    GetPrivateProfileString(section, "ProbabilityTga", "NONE", value, 0x80, path);
                    if (!strcmp(value, "NONE"))
                        definitionTable[i]->probabilityTga[0] = 0;
                    else
                        strcpy(definitionTable[i]->probabilityTga, value);
                }
            }
            northAngle = UnknownFunction47b8a0("EcoSystem", "NorthAngle", 0.0, path);
            GetPrivateProfileString("EcoSystem", "ProbabilityTga", "NONE", value, 0x80, path);
            if (!strcmp(value, "NONE")) {
                probabilityTga[0] = 0;
            } else {
                int valueLength = strlen(value);
                int length = valueLength > 0x7f ? 0x7f : valueLength;
                strncpy(probabilityTga, value, length);
                probabilityTga[length] = 0;
            }
        }
        return 1;
    }
    if (strstr(path, ".esb")) {
        ReadEsb(path, stream);
        return 1;
    }
    return 0;
}

// 0x00457ed0: builds the definition's collision objects as children.
void EcoSystem::BuildCollisionObjects(UnknownEcoDefinition* definition) {
    Vector3 vertices[32];
    int indices[96];
    int i;
    int j;
    if (definition->collisionCount == 0) {
        definition->collisionObjects = 0;
        return;
    }
    int category = g_MemTagStack->Push("Collision");
    definition->collisionObjects = (CollisionObject**)DebugMalloc(definition->collisionCount * sizeof(CollisionObject*),
                                                             __FILE__, 0x3a5);
    for (i = 0; i < definition->collisionCount; i++) {
        UnknownEcoCollisionDefinition* shape = &definition->collisionDefinitions[i];
        definition->collisionObjects[i] = new(__FILE__, 0x3ad) CollisionObject(1);
        definition->collisionObjects[i]->UnknownFunction4320f0(g_TrackGame->field_0x10, 0, 0, 1);
        AppendChild(definition->collisionObjects[i], -1);
        definition->collisionObjects[i]->field_0x64 = 0x3e8;
        if (shape->type == 0) {
            definition->collisionObjects[i]->field_0x64 = 0x3e9;
            Vector3* hull = new(__FILE__, 0x3b8) Vector3[definition->modelVertexCount[0]];
            int* hullIndices = new(__FILE__, 0x3b9) int[definition->modelIndexCount[0]];
            for (j = 0; j < definition->modelIndexCount[0]; j++)
                hullIndices[j] = definition->modelIndices[0][j];
            for (j = 0; j < definition->modelVertexCount[0]; j++) {
                hull[j].x = definition->modelVertices[0][j].position.x;
                hull[j].y = definition->modelVertices[0][j].position.y;
                hull[j].z = definition->modelVertices[0][j].position.z;
            }
            definition->collisionObjects[i]->field_0x64 = 0x3e9;
            definition->collisionObjects[i]->UnknownFunction4328b0(hull, hullIndices, definition->modelIndexCount[0] / 3,
                                                               definition->modelVertexCount[0]);
            UnknownSetIdentity(&((UnknownEcoHullShape*)definition->collisionObjects[i]->field_0x54)->field_0xc8);
            delete hull;
            delete hullIndices;
        } else if (shape->type == 2) {
            definition->collisionObjects[i]->UnknownFunction4329a0(shape->start, shape->radius);
        } else if (shape->type == 3) {
            definition->collisionObjects[i]->UnknownFunction432a20(shape->start, shape->end, shape->radius);
        } else if (shape->type == 1) {
            definition->collisionObjects[i]->field_0x64 = 0x3e9;
            for (j = 0; j < 16; j++) {
                float angle = j * 0.39269909f;
                vertices[j].x = vertices[j + 16].x = (float)cos(angle) * shape->radius;
                vertices[j].z = vertices[j + 16].z = (float)sin(angle) * shape->radius;
                vertices[j].y = shape->height;
                vertices[j + 16].y = 0.0f;
            }
            for (j = 0; j < 32; j++)
                vertices[j] = UnknownEcoOffset(shape->start, vertices[j]);
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
            definition->collisionObjects[i]->UnknownFunction4328b0(vertices, indices, 32, 32);
            UnknownSetIdentity(&((UnknownEcoHullShape*)definition->collisionObjects[i]->field_0x54)->field_0xc8);
        }
    }
    g_MemTagStack->Pop(category);
}

// 0x00458360
void EcoSystem::ReadCollisionObjects(const char* path, int index, UnknownEcoDefinition* definition) {
    char key[0x80];
    char value[0x80];
    char section[0x100];
    char kind[0x80];
    int i;
    sprintf(section, "Vegetation_%d", index);
    int count = GetPrivateProfileInt(section, "NumCollisionObjects", 0, path);
    definition->collisionCount = count;
    if (count > 0) {
    definition->collisionDefinitions = (UnknownEcoCollisionDefinition*)DebugMalloc(count * sizeof(UnknownEcoCollisionDefinition),
                                                                          __FILE__, 0x40c);
    for (i = 1; i - 1 < definition->collisionCount; i++) {
        UnknownEcoCollisionDefinition* shape = &definition->collisionDefinitions[i - 1];
        sprintf(key, "CollisionObject%i", i);
        GetPrivateProfileString(section, key, "NONE", kind, 0x80, path);
        if (!_stricmp(kind, "GEOMETRY")) {
            shape->type = 0;
        } else if (!_stricmp(kind, "SPHERE")) {
            shape->type = 2;
            sprintf(key, "CollisionObject%iCenter", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->start.x = (float)atof(strtok(value, ","));
            shape->start.y = (float)atof(strtok(0, ","));
            shape->start.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iRadius", i);
            shape->radius = UnknownFunction47b8a0(section, key, 0.0, path);
        } else if (!_stricmp(kind, "RADIUSEDLINE")) {
            shape->type = 3;
            sprintf(key, "CollisionObject%iStart", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->start.x = (float)atof(strtok(value, ","));
            shape->start.y = (float)atof(strtok(0, ","));
            shape->start.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iEnd", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->end.x = (float)atof(strtok(value, ","));
            shape->end.y = (float)atof(strtok(0, ","));
            shape->end.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iRadius", i);
            shape->radius = UnknownFunction47b8a0(section, key, 0.0, path);
        } else if (!_stricmp(kind, "CYLINDER")) {
            shape->type = 1;
            sprintf(key, "CollisionObject%iBottom", i);
            GetPrivateProfileString(section, key, "NONE", value, 0x80, path);
            shape->start.x = (float)atof(strtok(value, ","));
            shape->start.y = (float)atof(strtok(0, ","));
            shape->start.z = (float)atof(strtok(0, "\n"));
            sprintf(key, "CollisionObject%iRadius", i);
            shape->radius = UnknownFunction47b8a0(section, key, 0.0, path);
            sprintf(key, "CollisionObject%iHeight", i);
            shape->height = UnknownFunction47b8a0(section, key, 0.0, path);
        }
    }
    } else {
        definition->collisionDefinitions = 0;
    }
}

// 0x00458da0: writes the placed objects as text next to the .est.
void EcoSystem::WriteListing(const char* path) {
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
    UnknownFunction461d40(file, "%i\n", placedCount);
    for (i = 0; i < placedCount; i++) {
        UnknownEcoDefinition* definition = definitionTable[vegetation[i].definitionIndex];
        float radius = definition->RadiusForParameter(vegetation[i].radiusParam);
        float height = definition->HeightForParameter(vegetation[i].heightParam);
        float z = vegetation[i].quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
        float y = vegetation[i].quantizedPosition.y * g_UnknownGlobal59aebc->unitsPerCoordinate;
        float x = vegetation[i].quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
        UnknownFunction461d40(file, "%i,%f,%f,%f,%f,%f\n", vegetation[i].definitionIndex, x, y, z,
                              definition->RadiusForParameter(vegetation[i].radiusParam),
                              definition->HeightForParameter(vegetation[i].heightParam));
    }
    fclose(file);
}

// 0x004598d0: places the objects the .esb lists.
int EcoSystem::PlaceStoredObjects() {
    UnknownTextureStream* stream = esbStream;
    int i;
    for (i = 0; i < placedCount; i++) {
        unsigned char definition;
        UnknownEcoCoordinates coordinates;
        unsigned char heightParameter;
        unsigned char radiusParameter;
        stream->UnknownFunction461640(&definition, 1, 1);
        stream->UnknownFunction461640(&coordinates, 6, 1);
        stream->UnknownFunction461640(&heightParameter, 1, 1);
        stream->UnknownFunction461640(&radiusParameter, 1, 1);
        vegetation[i].PlaceQuantized(textureManager, definition, &coordinates, heightParameter, radiusParameter);
        float radius = definitionTable[definition]->RadiusForParameter(radiusParameter);
        float height = definitionTable[definition]->HeightForParameter(heightParameter);
        float x = vegetation[i].quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
        float y = vegetation[i].quantizedPosition.y * g_UnknownGlobal59aebc->unitsPerCoordinate;
        float z = vegetation[i].quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
        unsigned int code = g_collisionQuadTree->ComputeCode(x - radius, z - radius, x + radius, z + radius);
        g_MemTagStack->Push("QuadTree");
        g_collisionQuadTree->Insert(&vegetation[i], code, y, y + height);
        g_MemTagStack->Push("EcoSystem");
    }
    if (!esbStreamInArchive) {
        if (esbStream)
            delete esbStream;
        esbStream = 0;
    }
    return 1;
}

// 0x00459b40: places the objects the PlacementBmp paints (one pixel per
// terrain cell, the palette index selecting the definition).
int EcoSystem::PlaceAuthoredObjects() {
    if (!placementBmp[0])
        return 0;
    UnknownBitmapFile* bitmap = UnknownFunction424140(placementBmp, 0);
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
            if (index && definitionTable[index]) {
                Vector3 position;
                if (placedCount == totalObjects)
                    goto done;
                position.x = (column + 0.5f) * cellX;
                position.y = 0.0f;
                position.z = (row + 0.5f) * cellZ;
                groundTerrain->QueryGround(&position, 0, 0, 0);
                unsigned char heightParameter = definitionTable[index]->RandomParameter();
                unsigned char radiusParameter = definitionTable[index]->ParameterForHeight(
                    definitionTable[index]->HeightForParameter(heightParameter));
                vegetation[placedCount].Place(textureManager, index, &position, heightParameter,
                                                             radiusParameter);
                placedCount++;
            }
        }
    }
done:
    UnknownFunction4245b0(bitmap);
    return 1;
}

// 0x0045c040 (cdecl): loads every texture the .esb names before the
// ecosystem itself is created (the loader then finds them in the manager).
// `path` is rewritten to the .esb name when one exists next to the .est.
int UnknownFunction45c040(TextureMapManager* textures, char* path, UnknownTextureStream* stream, int modelFlags) {
    char names[256][0x80];
    char billboardNames[256][0x80];
    unsigned int keyColors[256];
    char esbName[0x104];
    char sltName[0x104];
    unsigned char present;
    unsigned char length;
    int method;
    int collisionCount;
    int ownsStream = 0;
    int i;
    memset(billboardNames, 0, sizeof(billboardNames));
    memset(names, 0, sizeof(names));
    g_UnknownGlobal59af10 = 0;
    if (!stream && strstr(path, ".est")) {
        int pathLength = strlen(path);
        int count = pathLength > 0x103 ? 0x103 : pathLength;
        strncpy(esbName, path, count);
        esbName[count] = 0;
        strcpy(strstr(esbName, ".est"), ".esb");
        UnknownTextureStream* probe = new(__FILE__, 0xad7) UnknownTextureStream(g_UnknownResourceManager572b44);
        if (probe->UnknownFunction460f50(esbName, "rb", 0))
            strcpy(strstr(path, ".est"), ".esb");
        if (probe)
            delete probe;
    }
    if (!strstr(path, ".esb"))
        return 0;
    UnknownTextureStream* esb = stream;
    if (!esb) {
        UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(path, 1);
        if (!entry) {
            esb = new(__FILE__, 0xaf8) UnknownTextureStream(g_UnknownResourceManager572b44);
            if (!esb->UnknownFunction460f50(path, "rb", 0)) {
                if (esb)
                    delete esb;
                return 0;
            }
            ownsStream = 1;
        } else {
            esb = entry->field_0x14;
        }
    }
    esb->UnknownFunction461640(&method, 4, 1);
    for (i = 0; i < 256; i++) {
        esb->UnknownFunction461640(&present, 1, 1);
        if (present) {
            esb->UnknownFunction461640(&length, 1, 1);
            esb->UnknownFunction461640(names[i], length, 1);
            esb->UnknownFunction461640(&length, 1, 1);
            esb->UnknownFunction461640(billboardNames[i], length, 1);
            esb->UnknownFunction461340(0x34, 1, 1);
            esb->UnknownFunction461640(&keyColors[i], 4, 1);
            if (method == 2) {
                esb->UnknownFunction461340(0x24, 1, 1);
                esb->UnknownFunction461640(&present, 1, 1);
                if (present) {
                    esb->UnknownFunction461640(&length, 1, 1);
                    esb->UnknownFunction461340(length, 1, 1);
                }
            }
            esb->UnknownFunction461640(&collisionCount, 4, 1);
            esb->UnknownFunction461340(collisionCount * sizeof(UnknownEcoCollisionDefinition), 1, 1);
        }
    }
    if (ownsStream) {
        if (esb)
            delete esb;
    }
    for (i = 0; i < 256; i++) {
        if (billboardNames[i][0]) {
            UnknownEcoTexture* texture = UnknownFunction50a590(textures, billboardNames[i], modelFlags, 0, 2, 5, 6, 0, 0x80,
                                                               0xff00ff, 1, 1);
            if (!texture->UnknownVirtualSlot7()) {
                texture->UnknownVirtualSlot8(1, 0, 0);
                g_UnknownGlobal59af10++;
            }
        }
        if (names[i][0]) {
            strcpy(sltName, names[i]);
            strcat(sltName, ".slt");
            UnknownTextureStream* sltStream = new(__FILE__, 0xb50) UnknownTextureStream(g_UnknownResourceManager572b44);
            if (sltStream->UnknownFunction460f50(sltName, "r", 0)) {
                UnknownParameterBlock* block = new(__FILE__, 0xb52) UnknownParameterBlock;
                block->UnknownFunction4b77a0((UnknownParameterStream*)sltStream, 0, 1);
                block->UnknownFunction4b78f0("Material - 0");
                block->UnknownFunction4b7b30("TextureMap", sltName, -1);
                int format = 1555;
                if (!g_TrackGame->field_0x2d0 && (g_TrackGame->field_0x10->field_0x1c0 & 8)
                    && g_TrackGame->GetRegistryFlag("KeyColorTrees", 0))
                    format = g_TrackGame->field_0x10->field_0x28;
                UnknownEcoTexture* texture = UnknownFunction50a590(textures, sltName, format, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
                if (!texture->UnknownVirtualSlot7()) {
                    g_UnknownGlobal59af10++;
                    int loaded = texture->field_0x20;
                    if (loaded == 555 || loaded == 565 || loaded == 888 || loaded == 1555)
                        texture->UnknownVirtualSlot18(keyColors[i]);
                    texture->UnknownVirtualSlot8(1, 0, 0);
                }
                if (block)
                    delete block;
            }
            if (sltStream)
                delete sltStream;
        }
    }
    g_UnknownGlobal59aefc = 1;
    return 1;
}

// A random number in 0..1 (inline shape only: the helper boundary is what
// keeps VC6 from folding the scale into the generator's other constants).
static inline float UnknownEcoRandom() {
    return rand() * (1.0f / 32768.0f);
}

// The value a probability TGA holds at the terrain position (`x`, `z`):
// the green channel of a 32- or 24-bit image, the green field of a 16-bit
// one, scaled to 0..1. Inline shape only (the three branches are expanded
// at every sample site of the generator).
static inline float UnknownEcoSampleTga(UnknownTgaFile* tga, float x, float z) {
    float worldX = g_collisionQuadTree->field_0x50;
    int width = tga->width;
    int column = (int)(x / worldX * width);
    float worldZ = g_collisionQuadTree->field_0x54;
    int height = tga->height;
    int row = (int)((1.0f - z / worldZ) * height);
    if (column >= width)
        column = width - 1;
    if (row >= height)
        row = height - 1;
    int index = row * width + column;
    if (tga->bitsPerPixel == 32)
        return ((unsigned char*)tga->bits)[index * 4 + 1] * (1.0f / 255.0f);
    if (tga->bitsPerPixel == 24)
        return ((unsigned char*)tga->bits)[index * 3 + 1] * (1.0f / 255.0f);
    return ((((unsigned short*)tga->bits)[index] >> 5) & 0x1f) * (1.0f / 31.0f);
}

// Whether the probability TGA is nonzero at the terrain position.
static inline int UnknownEcoTgaIsSet(UnknownTgaFile* tga, float x, float z) {
    float worldX = g_collisionQuadTree->field_0x50;
    int width = tga->width;
    int column = (int)(x / worldX * width);
    float worldZ = g_collisionQuadTree->field_0x54;
    int height = tga->height;
    int row = (int)((1.0f - z / worldZ) * height);
    if (column >= width)
        column = width - 1;
    if (row >= height)
        row = height - 1;
    int index = row * width + column;
    if (tga->bitsPerPixel == 32)
        return ((unsigned char*)tga->bits)[index * 4 + 1] != 0;
    if (tga->bitsPerPixel == 24)
        return ((unsigned char*)tga->bits)[index * 3 + 1] != 0;
    return (((unsigned short*)tga->bits)[index] & 0x3e0) != 0;
}

// 0x00459ce0: the Auto method. Places the authored objects, then draws
// random positions (in 5x5 / 6x6 "EcoGen" rings when the registry asks
// for them) until TotalObjects are placed or 10000 draws in a row failed.
// A draw picks a definition from the PercentBias table and must pass the
// probability TGAs, the slope and altitude bell curves and, with
// `checkCodes`, the QuadTree code test.
int EcoSystem::GenerateObjects(int checkCodes) {
    UnknownTgaFile* tgas[256];
    float definitionIndex[256];
    float cumulative[256];
    Vector3 outerRing[20];
    Vector3 innerRing[12];
    Vector3 normal;
    float minHeight;
    float maxHeight;
    float spacing;
    float rangeX;
    float rangeZ;
    int i;
    int j;
    PlaceAuthoredObjects();
    UnknownTgaFile* tga = 0;
    memset(tgas, 0, sizeof(tgas));
    if (probabilityTga[0])
        tga = UnknownFunction5125c0(probabilityTga, 0, (int)g_UnknownResourceManager572b44);
    for (i = 0; i < 256; i++) {
        if (definitionTable[i] && definitionTable[i]->probabilityTga[0])
            tgas[i] = UnknownFunction5125c0(definitionTable[i]->probabilityTga, 0, (int)g_UnknownResourceManager572b44);
    }
    groundTerrain->GetHeightRange(&minHeight, &maxHeight);
    unsigned int attempt = 0;
    float total = 0.0f;
    int count = 0;
    int failures = 0;
    for (i = 0; i < 256; i++) {
        if (definitionTable[i]) {
            definitionIndex[count] = (float)i;
            total += definitionTable[i]->percentBias;
            cumulative[count] = total;
            count++;
        }
    }
    int mode = g_TrackGame->UnknownVirtualSlot20("EcoGen", 0);
    rangeX = g_collisionQuadTree->field_0x50;
    rangeZ = g_collisionQuadTree->field_0x54;
    if (mode == 5) {
        spacing = g_collisionQuadTree->field_0x50 * (1.0f / 5.0f);
        rangeX = rangeZ = spacing;
    } else if (mode == 6) {
        spacing = g_collisionQuadTree->field_0x50 * (1.0f / 6.0f);
        rangeX = rangeZ = spacing;
    }
    if (mode) {
        int k = 0;
        for (i = 0; i < mode; i++) {
            outerRing[k++] = Vector3(i * spacing, 0.0f, 0.0f);
            outerRing[k++] = Vector3(i * spacing, 0.0f, (mode - 1) * spacing);
        }
        for (i = 1; i < mode - 1; i++) {
            outerRing[k++] = Vector3(0.0f, 0.0f, i * spacing);
            outerRing[k++] = Vector3((mode - 1) * spacing, 0.0f, i * spacing);
        }
        k = 0;
        for (i = 1; i < mode - 1; i++) {
            innerRing[k++] = Vector3(i * spacing - spacing, 0.0f, 0.0f);
            innerRing[k++] = Vector3(i * spacing - spacing, 0.0f, (mode - 2) * spacing - spacing);
        }
        for (i = 2; i < mode - 2; i++) {
            innerRing[k++] = Vector3(0.0f, 0.0f, i * spacing - spacing);
            innerRing[k++] = Vector3((mode - 2) * spacing - spacing, 0.0f, i * spacing - spacing);
        }
    }
    while (placedCount < totalObjects && failures < 10000) {
        Vector3 position;
        Vector3 probe;
        failures++;
        attempt++;
        int selected = (int)definitionIndex[count - 1];
        float draw = rand() * (100.0f / 32767.0f);
        for (i = 0; i < count; i++) {
            if (draw <= cumulative[i]) {
                selected = (int)definitionIndex[i];
                break;
            }
        }
        position.x = UnknownEcoRandom() * rangeX;
        int clusterCount = 1;
        Vector3* cluster = 0;
        position.y = 0.0f;
        position.z = UnknownEcoRandom() * rangeZ;
        if (mode == 5) {
            if (attempt % 3 == 0) {
                clusterCount = 16;
                cluster = outerRing;
            } else if (attempt % 3 == 1) {
                clusterCount = 8;
                position.x += spacing;
                cluster = innerRing;
                position.z += spacing;
            } else {
                clusterCount = 1;
                cluster = 0;
                position.x = spacing * 2.0f + position.x;
                position.z = spacing * 2.0f + position.z;
            }
        } else if (mode == 6) {
            if (attempt % 6 == 0) {
                clusterCount = 20;
                cluster = outerRing;
            } else if (attempt % 6 == 1) {
                clusterCount = 12;
                position.x += spacing;
                cluster = innerRing;
                position.z += spacing;
            } else {
                clusterCount = 1;
                cluster = 0;
                position.x = (position.x + spacing) * 2.0f;
                position.z = (position.z + spacing) * 2.0f;
            }
        }
        if (placedCount + clusterCount > totalObjects)
            continue;
        unsigned char heightParameter = definitionTable[selected]->RandomParameter();
        unsigned char radiusParameter = definitionTable[selected]->ParameterForHeight(
            definitionTable[selected]->HeightForParameter(heightParameter));
        float radius = definitionTable[selected]->RadiusForParameter(radiusParameter);
        float height = definitionTable[selected]->HeightForParameter(heightParameter);
        probe = position;
        unsigned int code = g_collisionQuadTree->ComputeCode(position.x - radius, position.z - radius,
                                                             radius + position.x, radius + position.z);
        if (checkCodes && !g_collisionQuadTree->IsValidCode(code))
            continue;
        // The named local keeps VC6 from folding the two scales into one.
        float random = UnknownEcoRandom();
        float threshold = random * 3.0f;
        float probability;
        if (tga)
            probability = UnknownEcoSampleTga(tga, probe.x, probe.z);
        else
            probability = 1.0f;
        if (probability <= threshold)
            continue;
        if (tgas[selected])
            probability = UnknownEcoSampleTga(tgas[selected], probe.x, probe.z) * probability;
        if (probability <= threshold)
            continue;
        groundTerrain->QueryGround(&probe, &normal, 0, 0);
        float slope = (float)acos(normal.y);
        float altitude = probe.y;
        if (slope < 0.0f)
            slope = 0.0f;
        slope *= (float)(2.0 / 3.14159265358979);
        probability *= g_UnknownGlobal56ece0->Lookup(slope, definitionTable[selected]->meanSlope,
                                                      definitionTable[selected]->standardDeviationSlope);
        if (probability <= threshold)
            continue;
        altitude = (altitude - minHeight) / (maxHeight - minHeight);
        probability *= g_UnknownGlobal56ece0->Lookup(altitude, definitionTable[selected]->meanAltitude,
                                                      definitionTable[selected]->standardDeviationAltitude);
        if (probability <= threshold)
            continue;
        for (j = 1; j < clusterCount; j++) {
            probe = Vector3(position.x + cluster[j].x, cluster[j].y, position.z + cluster[j].z);
            if (tga && !UnknownEcoTgaIsSet(tga, probe.x, probe.z))
                break;
            if (tgas[selected] && !UnknownEcoTgaIsSet(tgas[selected], probe.x, probe.z))
                break;
        }
        if (j != clusterCount)
            continue;
        for (j = 0; j < clusterCount; j++) {
            if (clusterCount == 1)
                probe = position;
            else
                probe = Vector3(position.x + cluster[j].x, cluster[j].y, position.z + cluster[j].z);
            groundTerrain->QueryGround(&probe, 0, 0, 0);
            code = g_collisionQuadTree->ComputeCode(probe.x - radius, probe.z - radius, probe.x + radius,
                                                    probe.z + radius);
            vegetation[placedCount].Place(textureManager, selected, &probe, heightParameter, radiusParameter);
            g_MemTagStack->Push("QuadTree");
            g_collisionQuadTree->Insert(&vegetation[placedCount], code, probe.y, probe.y + height);
            g_MemTagStack->Push("EcoSystem");
            placedCount++;
        }
        failures = 0;
    }
    if (tga)
        UnknownFunction512dd0(tga);
    for (i = 0; i < 256; i++) {
        if (tgas[i])
            UnknownFunction512dd0(tgas[i]);
    }
    return 1;
}

// An xz direction normalised in place (the y component is cleared), zero
// when it has no length. Inline shape only.
static inline void UnknownEcoNormalizeXZ(Vector3* v) {
    float length = v->x * v->x + v->z * v->z;
    if (length == 0.0f) {
        *v = kVec3Zero;
    } else {
        length = FastInvSqrt(length);
        v->x = length * v->x;
        v->y = 0.0f;
        v->z = length * v->z;
    }
}

// The lit colour (no alpha) of a billboard face whose normal has the light
// intensity `intensity`. Inline shape only.
static inline float UnknownEcoMin1(float v) {
    return v < 1.0f ? v : 1.0f;
}
static inline unsigned int UnknownEcoLitColor(const EcoSystem* eco, float intensity) {
    float r = UnknownEcoMin1(intensity * eco->lightColor.x + eco->ambientLight.x);
    float g = UnknownEcoMin1(intensity * eco->lightColor.y + eco->ambientLight.y);
    float b = UnknownEcoMin1(intensity * eco->lightColor.z + eco->ambientLight.z);
    return ((int)(r * 255.0f) << 16) | ((int)(g * 255.0f) << 8) | (int)(b * 255.0f);
}

// The light falling on a face with normal `n` (the dot product is summed
// as z + (x + y), the order VC6 emits for retail; see UnknownSquareMagnitude).
static inline float UnknownEcoLightOn(const EcoSystem* eco, const Vector3* n) {
    return -(n->z * eco->lightDirection.z + (n->x * eco->lightDirection.x + n->y * eco->lightDirection.y));
}

// 0x0045b060: slot 14, the draw. The geometry list is drawn object by
// object; the billboards are built 120 at a time into the vertex buffer as
// view-aligned quads (or, when the band lights them, as three lit faces
// folded towards the camera) and drawn in one call. The timings and counts
// go to the "EcoSystem" debug overlay page.
int EcoSystem::UnknownVirtualSlot14() {
    Matrix4 identity;
    int savedPerspective;
    int savedShadeMode;
    Vector3 right;
    Vector3 left;
    Vector3 back;
    int i;
    if (!g_UnknownGlobal56a12c)
        return 1;
    unsigned int start = ReadClock();
    int category = g_MemTagStack->Push("EcoSystem");
    UnknownEcoCamera* camera = ECO_VIEW->field_0x08;
    int verticesBefore = ECO_VIEW->field_0x38;
    int trianglesBefore = ECO_VIEW->field_0x44;
    float projectionScale = camera->field_0x198;
    identity(0, 0) = 1.0f;
    identity(0, 1) = 0.0f;
    identity(0, 2) = 0.0f;
    identity(0, 3) = 0.0f;
    identity(1, 0) = 0.0f;
    identity(1, 1) = 1.0f;
    identity(1, 2) = 0.0f;
    identity(1, 3) = 0.0f;
    identity(2, 0) = 0.0f;
    identity(2, 1) = 0.0f;
    identity(2, 2) = 1.0f;
    identity(2, 3) = 0.0f;
    identity(3, 0) = 0.0f;
    identity(3, 1) = 0.0f;
    identity(3, 2) = 0.0f;
    identity(3, 3) = 1.0f;
    ECO_VIEW->field_0x08->UnknownVirtualSlot30(&identity);
    ECO_VIEW->UnknownVirtualSlot9(D3DRENDERSTATE_TEXTUREPERSPECTIVE, &savedPerspective);
    ECO_VIEW->UnknownVirtualSlot9(D3DRENDERSTATE_SHADEMODE, &savedShadeMode);
    // The three face normals: the quad's two sides and the face towards
    // the camera, the sides leaned back by a tenth of it.
    right.x = -camera->field_0x17c.z;
    right.z = camera->field_0x17c.x;
    left.x = camera->field_0x17c.z;
    left.z = -camera->field_0x17c.x;
    back.x = -camera->field_0x17c.x;
    back.z = -camera->field_0x17c.z;
    right.x = right.x - back.x * 0.1f;
    right.z = right.z - back.z * 0.1f;
    left.x = left.x - back.x * 0.1f;
    left.z = left.z - back.z * 0.1f;
    UnknownEcoNormalizeXZ(&right);
    UnknownEcoNormalizeXZ(&left);
    UnknownEcoNormalizeXZ(&back);
    // The three intensities (x right, y left, z back).
    Vector3 intensity;
    intensity.x = UnknownEcoLightOn(this, &right);
    if (intensity.x < 0.0f)
        intensity.x = 0.0f;
    intensity.y = UnknownEcoLightOn(this, &left);
    if (intensity.y < 0.0f)
        intensity.y = 0.0f;
    intensity.z = UnknownEcoLightOn(this, &back);
    if (intensity.z < 0.0f)
        intensity.z = 0.0f;
    unsigned int backColor = UnknownEcoLitColor(this, intensity.z);
    unsigned int rightColor = UnknownEcoLitColor(this, intensity.x);
    unsigned int leftColor = UnknownEcoLitColor(this, intensity.y);
    float inverse = UnknownFunction460c70(camera->field_0x17c.x * camera->field_0x17c.x
                                          + camera->field_0x17c.z * camera->field_0x17c.z);
    float sine = inverse * camera->field_0x17c.x;
    float cosine = inverse * camera->field_0x17c.z;
    g_UnknownGlobal59af08 = 0;
    g_UnknownGlobal59af04 = 0;
    if (g_TrackGame->field_0x2d0)
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, g_UnknownGlobal59af14[detailLevel].fog, 0);
    if (geometryCount) {
        SetRenderStates(definitionTable[vegetation[0].definitionIndex]->modelTexture->field_0x20);
        for (i = 0; i < geometryCount; i++) {
            if (g_UnknownGlobal56a12c)
                geometryList[i]->DrawGeometry(ECO_VIEW);
            g_UnknownGlobal59af04++;
        }
    }
    g_UnknownGlobal59aec0 = ReadClock() - start;
    if (g_TrackGame->field_0x2d0)
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    if (billboardCount) {
        definitionTable[vegetation[0].definitionIndex]->billboardTexture->UnknownVirtualSlot19();
        SetRenderStates(definitionTable[vegetation[0].definitionIndex]->billboardTexture->field_0x20);
        int next = 0;
        int remaining = billboardCount;
        float widthScale = (float)(15.0 / ECO_VIEW->field_0x0c);
        int stageLighting = g_UnknownGlobal59af14[detailLevel].stageLighting;
        float billboardRange = (float)g_UnknownGlobal59af14[detailLevel].billboardRange;
        int billboardLimit = g_UnknownGlobal59af14[detailLevel].billboardLimit;
        while (remaining) {
            int vertexCount = 0;
            int indexCount = 0;
            int offset0 = 0;
            int offset5 = 0xa0;
            int offset2 = 0x40;
            int offset1 = 0x20;
            int offset3 = 0x60;
            for (i = 0; i < remaining && i < 120; i++) {
                Vegetation* object = billboardList[next];
                UnknownEcoDefinition* definition = definitionTable[object->definitionIndex];
                float radius = definition->RadiusForParameter(object->radiusParam);
                int depthKey = (unsigned short)object->field_0x06;
                float depth = depthKey * depthUnit;
                if (depth < billboardRange && radius * projectionScale / depth > 2.0f) {
                    float height = definition->HeightForParameter(object->heightParam);
                    float uScale = (definition->uRight - definition->uLeft) / (radius + radius);
                    float uLeftWidth = (definition->vTop - definition->uLeft) * uScale;
                    float uRightWidth = (definition->uRight - definition->vTop) * uScale;
                    float x = object->quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
                    float y = object->quantizedPosition.y * g_UnknownGlobal59aebc->unitsPerCoordinate;
                    float z = object->quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
#define VERTEX(o) (*(UnknownEcoVertex*)((char*)billboardVertices + (o)))
                    VERTEX(offset0).position.x = uRightWidth * cosine + x;
                    VERTEX(offset3).position.x = VERTEX(offset0).position.x;
                    VERTEX(offset1).position.x = x - uLeftWidth * cosine;
                    VERTEX(offset2).position.x = VERTEX(offset1).position.x;
                    VERTEX(offset0).position.z = z - uRightWidth * sine;
                    VERTEX(offset0 + 0x60).position.z = VERTEX(offset0).position.z;
                    VERTEX(offset0 + 0x20).position.z = z + uLeftWidth * sine;
                    VERTEX(offset0 + 0x40).position.z = VERTEX(offset0 + 0x20).position.z;
                    VERTEX(offset0).position.y = height * definition->field_0x1d4 + y;
                    VERTEX(offset0 + 0x20).position.y = VERTEX(offset0).position.y;
                    VERTEX(offset0 + 0x40).position.y = height + VERTEX(offset0).position.y;
                    VERTEX(offset0 + 0x60).position.y = VERTEX(offset0 + 0x40).position.y;
                    unsigned int alpha;
                    if (object->isBillboard < definition->lodCount)
                        alpha = object->fadeLevel << 24;
                    else
                        alpha = 0xff000000;
                    int stride;
                    if (billboardLimit && ((stageLighting && depth < 20.0f) || depth * widthScale < radius)) {
                        VERTEX(offset5).position.x = x;
                        VERTEX(offset0 + 0x80).position.x = x;
                        VERTEX(offset0 + 0xa0).position.z = z;
                        VERTEX(offset0 + 0x80).position.z = z;
                        VERTEX(offset0 + 0x80).position.y = VERTEX(offset0).position.y;
                        VERTEX(offset0 + 0xa0).position.y = VERTEX(offset0 + 0x60).position.y;
                        VERTEX(offset0 + 0x40).tu = definition->uLeft;
                        VERTEX(offset0 + 0x20).tu = VERTEX(offset0 + 0x40).tu;
                        VERTEX(offset0 + 0xa0).tu = definition->vTop;
                        VERTEX(offset0 + 0x80).tu = VERTEX(offset0 + 0xa0).tu;
                        VERTEX(offset0 + 0x60).tu = definition->uRight;
                        VERTEX(offset0).tu = VERTEX(offset0 + 0x60).tu;
                        VERTEX(offset0 + 0xa0).tv = definition->uCenter;
                        VERTEX(offset0 + 0x60).tv = VERTEX(offset0 + 0xa0).tv;
                        VERTEX(offset0 + 0x40).tv = VERTEX(offset0 + 0x60).tv;
                        VERTEX(offset0 + 0x80).tv = definition->vBottom;
                        VERTEX(offset0 + 0x20).tv = VERTEX(offset0 + 0x80).tv;
                        VERTEX(offset0).tv = VERTEX(offset0 + 0x20).tv;
                        VERTEX(offset0 + 0x40).diffuse = alpha | rightColor;
                        VERTEX(offset0 + 0x20).diffuse = VERTEX(offset0 + 0x40).diffuse;
                        VERTEX(offset0 + 0x60).diffuse = alpha | leftColor;
                        VERTEX(offset0).diffuse = VERTEX(offset0 + 0x60).diffuse;
                        VERTEX(offset0 + 0xa0).diffuse = alpha | backColor;
                        VERTEX(offset0 + 0x80).diffuse = VERTEX(offset0 + 0xa0).diffuse;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 4;
                        billboardIndices[indexCount++] = vertexCount + 5;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 5;
                        billboardIndices[indexCount++] = vertexCount + 3;
                        billboardIndices[indexCount++] = vertexCount + 4;
                        billboardIndices[indexCount++] = vertexCount + 1;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount + 4;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount + 5;
                        vertexCount += 6;
                        stride = 0xc0;
                    } else {
                        VERTEX(offset0 + 0x60).tu = definition->uRight;
                        VERTEX(offset0).tu = VERTEX(offset0 + 0x60).tu;
                        VERTEX(offset0 + 0x40).tu = definition->uLeft;
                        VERTEX(offset0 + 0x20).tu = VERTEX(offset0 + 0x40).tu;
                        VERTEX(offset0 + 0x60).tv = definition->uCenter;
                        VERTEX(offset0 + 0x40).tv = VERTEX(offset0 + 0x60).tv;
                        VERTEX(offset0 + 0x20).tv = definition->vBottom;
                        VERTEX(offset0).tv = VERTEX(offset0 + 0x20).tv;
                        VERTEX(offset0 + 0x60).diffuse = alpha | backColor;
                        VERTEX(offset0).diffuse = VERTEX(offset0 + 0x60).diffuse;
                        VERTEX(offset0 + 0x40).diffuse = VERTEX(offset0).diffuse;
                        VERTEX(offset0 + 0x20).diffuse = VERTEX(offset0 + 0x40).diffuse;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 1;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount + 3;
                        vertexCount += 4;
                        stride = 0x80;
                    }
#undef VERTEX
                    offset3 += stride;
                    offset1 += stride;
                    offset2 += stride;
                    offset5 += stride;
                    offset0 += stride;
                    g_UnknownGlobal59af08++;
                }
                next++;
            }
            if (g_UnknownGlobal56a12c && vertexCount)
                ECO_VIEW->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, billboardVertices, vertexCount,
                                               billboardIndices, indexCount, 0);
            remaining -= i;
        }
    }
    g_UnknownGlobal59aee4 = ReadClock() - start;
    UnknownEcoDebugOverlay* overlay = g_TrackGame->field_0x38;
    if (overlay) {
        if (g_UnknownGlobal56a128 < 0)
            g_UnknownGlobal56a128 = overlay->field_0x26c0++;
        g_UnknownGlobal59aef0.UnknownFunction4cb6b0(g_UnknownGlobal59aee8);
        g_UnknownGlobal59aec8.UnknownFunction4cb6b0(g_UnknownGlobal59aee4);
        g_TrackGame->field_0x38->UnknownFunction447fa0(g_UnknownGlobal56a128, "Ecosystem:%d objects", totalObjects);
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "Visible  2D %d 3D %d", billboardCount,
                                                       geometryCount);
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "Rendered 2D %d 3D %d",
                                                       g_UnknownGlobal59af08, g_UnknownGlobal59af04);
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "Memory %d",
                                                       g_MemTagStack->UnknownFunction4a2d20("EcoSystem"));
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "PrepareGeometry %d %d",
                                                       g_UnknownGlobal59aee8, g_UnknownGlobal59aef0.UnknownFunction4cb690());
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "Render %d(3d:%d) %d",
                                                       g_UnknownGlobal59aee4, g_UnknownGlobal59aec0,
                                                       g_UnknownGlobal59aec8.UnknownFunction4cb690());
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "Total Transforms %d",
                                                       ECO_VIEW->field_0x38 - verticesBefore);
        g_TrackGame->field_0x38->UnknownFunction447f40(g_UnknownGlobal56a128, "Total Triangles  %d",
                                                       ECO_VIEW->field_0x44 - trianglesBefore);
    }
    g_MemTagStack->Pop(category);
    ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, savedShadeMode, 0);
    ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, savedPerspective, 0);
    ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, g_TrackGame->field_0x54c);
    ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, g_TrackGame->field_0x550);
    ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MIPFILTER, g_TrackGame->field_0x554);
    ECO_VIEW->UnknownVirtualSlot19();
    return GameObject::UnknownVirtualSlot14();
}
