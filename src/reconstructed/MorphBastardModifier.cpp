// MorphBastardModifier.cpp -- see MorphBastardModifier.h for the evidence.

#include <math.h>
#include <string.h>

#include "MorphBastardModifier.h"

#include "DebugAlloc.h"

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h): 0x006850f0, 0x00685100, 0x00685110 and
// 0x006850e0, initialised by 0x004a54f0..0x004a562b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x004a3150
MorphBastardModifier::MorphBastardModifier(int flags)
    : D3DIMSoultreeModifier(flags)
{
    meshCopy = 0;
    objectCount = 0;
    morphObjects = 0;
    deltaSums = 0;
    deltaMinimums = 0;
    deltaMaximums = 0;
    field_0x3c = 0;
}

// 0x004a31a0
MorphBastardModifier::~MorphBastardModifier()
{
    if (morphObjects) {
        for (int i = 0; i < objectCount; i++) {
            for (int j = 0; j < morphObjects[i].channelCount; j++) {
                for (int k = 0; k < morphObjects[i].channels[j].targetCount; k++) {
                    DebugFree(morphObjects[i].channels[j].targets[k].vertexIndices, __FILE__, 28);
                    DebugFree(morphObjects[i].channels[j].targets[k].deltas, __FILE__, 29);
                    DebugFree(morphObjects[i].channels[j].targets[k].lastWeights, __FILE__, 30);
                }
                DebugFree(morphObjects[i].channels[j].targets, __FILE__, 32);
                DebugFree(morphObjects[i].channels[j].targetNumbers, __FILE__, 33);
            }
            DebugFree(morphObjects[i].channels, __FILE__, 35);
        }
        DebugFree(morphObjects, __FILE__, 37);
    }
    DebugFree(deltaSums, __FILE__, 40);
    DebugFree(deltaMinimums, __FILE__, 41);
    DebugFree(deltaMaximums, __FILE__, 42);
    if (meshCopy) {
        delete meshCopy->vertices;
        delete meshCopy->drawnVertices;
        delete meshCopy->normals;
        delete meshCopy->indices;
        delete meshCopy->uvs;
    }
}

// 0x004a3bb0
int CompareTargets(const void* a, const void* b)
{
    if (((const MorphBastardTarget*)a)->value > ((const MorphBastardTarget*)b)->value)
        return 1;
    if (((const MorphBastardTarget*)a)->value < ((const MorphBastardTarget*)b)->value)
        return -1;
    return 0;
}

// 0x004a3be0
float AngleBetween(Vector3 a, Vector3 b)
{
    float dot = a.z * b.z + a.x * b.x + a.y * b.y;
    if (dot + 0.001f > 1.0f)
        dot = 1.0f;
    if (dot - 0.001f < -1.0f)
        dot = -1.0f;
    if (dot == 1.0)
        return 0.0f;
    if (dot == 0.0)
        return 1.5707964f;
    return (float)acos(dot);
}

// 0x004a4bb0
void MorphBastardModifier::UpdateChannels(MorphBastardObject* object)
{
    Matrix4 m;
    object->node->GetLocalMatrix(&m);
    object->node->GetMatrixIn(0, &m);
    MorphBastardTransposeRotation(m);
    for (int i = 0; i < object->channelCount; i++)
        ControllerAngles(&object->channels[i], m);
}

// 0x004a5290
void MorphBastardModifier::CopyMesh(UnknownSoultreeMesh* mesh)
{
    meshCopy = new (__FILE__, 854) UnknownSoultreeMesh;
    meshCopy->groupCount = mesh->groupCount;
    meshCopy->vertexCount = mesh->vertexCount;
    meshCopy->faceCount = mesh->faceCount;
    meshCopy->vertices = new (__FILE__, 858) MorphBastardVertex[meshCopy->vertexCount];
    meshCopy->drawnVertices = new (__FILE__, 859) MorphBastardVertex[meshCopy->vertexCount];
    meshCopy->normals = new (__FILE__, 860) Vector3[meshCopy->vertexCount];
    meshCopy->indices = new (__FILE__, 861) MorphBastardFace[meshCopy->faceCount];
    meshCopy->field_0x20 = mesh->field_0x20;
    meshCopy->uvs = new (__FILE__, 863) int[meshCopy->vertexCount * 2];
    memcpy(meshCopy->vertices, mesh->vertices, meshCopy->vertexCount * sizeof(MorphBastardVertex));
    memcpy(meshCopy->drawnVertices, mesh->drawnVertices, meshCopy->vertexCount * sizeof(MorphBastardVertex));
    memcpy(meshCopy->normals, mesh->normals, meshCopy->vertexCount * sizeof(Vector3));
    memcpy(meshCopy->indices, mesh->indices, meshCopy->faceCount * sizeof(MorphBastardFace));
    memcpy(meshCopy->uvs, mesh->uvs, meshCopy->vertexCount * sizeof(int));
    meshCopy->groups = new (__FILE__, 871) MorphBastardMeshGroup[meshCopy->groupCount];
    int offset = 0;
    for (int i = 0; i < meshCopy->groupCount; i++) {
        meshCopy->groups[i].node = mesh->groups[i].node;
        meshCopy->groups[i].vertexCount = mesh->groups[i].vertexCount;
        meshCopy->groups[i].field_0x08 = meshCopy->vertices + offset;
        meshCopy->groups[i].field_0x0c = meshCopy->drawnVertices + offset;
        meshCopy->groups[i].field_0x10 = meshCopy->normals + offset;
        offset += meshCopy->groups[i].vertexCount;
    }
}
