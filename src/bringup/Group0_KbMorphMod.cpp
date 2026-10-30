// Group0_KbMorphMod.cpp -- bring-up body for the rider morph loader.
//
// KrustyBike.cpp reaches MorphBastardModifier::LoadParameterFile (0x004a33b0,
// ret 0xc) through its KbMorphMod view (src/krusty2/vehicle/KrustyBikeTypes.h),
// so the symbol it needs is KbMorphMod::LoadParameterFile. The loader body is
// extracted from samples/render/MorphBastardModifierNearMisses.cpp (that
// sample reports 1942 of 2046 positions, with retail's calls, strings,
// lines and field offsets); only it and its channel-inverse inline are taken.
// __FILE__ is replaced by the retail string at 0x0056df3c.
//
// KbMorphMod is declared here locally (same `struct` keyword, same member
// signature) because KrustyBikeTypes.h cannot be included beside
// MorphBastardModifier.h (both bring their own Matrix4).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MorphBastardModifier.h"

#include "DebugAlloc.h"
#include "Parameterblocks.h"
#include "TextureMap.h"
#include "UnknownResourceManager.h"

static const char kMorphBastardModifierFile[] = "D:\\aardvark\\VC\\krusty2\\MorphBastardModifier.cpp";

// The channel's rigid inverse (0x004a33b0's second expansion): the same
// transpose and negated translation as MorphBastardInvertRigid, through a
// pointer and with the x term summed last. Retail's two expansions in
// 0x004a33b0 order their products differently, which suggests two source
// forms; this one gives retail's term order for the channel (not yet its
// operand order inside each product).
inline void MorphBastardInvertRigidInPlace(Matrix4* m)
{
    float t;
    t = m->m[0][1]; m->m[0][1] = m->m[1][0]; m->m[1][0] = t;
    t = m->m[0][2]; m->m[0][2] = m->m[2][0]; m->m[2][0] = t;
    t = m->m[1][2]; m->m[1][2] = m->m[2][1]; m->m[2][1] = t;
    Vector3 p(-((m->m[3][2] * m->m[2][0] + m->m[3][1] * m->m[1][0]) + m->m[3][0] * m->m[0][0]),
              -(m->m[3][0] * m->m[0][1] + m->m[3][1] * m->m[1][1] + m->m[3][2] * m->m[2][1]),
              -(m->m[3][0] * m->m[0][2] + m->m[3][1] * m->m[1][2] + m->m[3][2] * m->m[2][2]));
    m->m[3][0] = p.x;
    m->m[3][1] = p.y;
    m->m[3][2] = p.z;
}

// 0x004a33b0
MorphBastardModifier* MorphBastardModifier::LoadParameterFile(void* value, const char* path,
                                                                 MorphBastardNode* model)
{
    char section[0x100];
    char objectName[0x100];
    char controllerName[0x100];
    int targetCount;

    D3DIMSoultreeModifier::SetRenderContext(value);
    UnknownTextureStream* stream =
        new (kMorphBastardModifierFile, 73) UnknownTextureStream(g_ResourceManager);
    stream->Open(path, "r", 0);
    UnknownParameterBlock* block = new (kMorphBastardModifierFile, 77) UnknownParameterBlock;
    block->Attach((UnknownParameterStream*)stream, 0, 1);
    sprintf(section, "MorphBastard Information");
    block->SelectSection(section);
    block->GetInt("NumberOfMorphBastards", 0, &objectCount);
    morphObjects = new (kMorphBastardModifierFile, 87) MorphBastardObject[objectCount];

    for (int i = 0; i < objectCount; i++) {
        sprintf(section, "MorphBastard %i", i);
        block->SelectSection(section);
        block->ReadString("ObjectName", objectName, -1);
        MorphBastardNode* node = model->FindByName(objectName);
        morphObjects[i].node = node;
        node->GetMatrixIn(0, &morphObjects[i].inverseRestMatrix);
        MorphBastardInvertRigid(morphObjects[i].inverseRestMatrix);
        block->GetInt("NumberOfTargets", 0, &targetCount);
        morphObjects[i].channelCount = 0;
        morphObjects[i].channels = 0;

        for (int j = 0; j < targetCount; j++) {
            sprintf(section, "MorphBastard %i Target %i", i, j);
            block->SelectSection(section);
            block->ReadString("ControllerName", controllerName, -1);
            MorphBastardNode* controller = model->FindByName(controllerName);
            int isNew = 1;
            for (int axis = 0; axis < 3; axis++) {
                int found;
                float probe;
                int channelAxis;
                if (axis == 0) {
                    channelAxis = axis;
                    found = block->ReadFloat("XAxis", &probe);
                } else if (axis == 1) {
                    channelAxis = axis;
                    found = block->ReadFloat("YAxis", &probe);
                } else if (axis == 2) {
                    channelAxis = axis;
                    found = block->ReadFloat("ZAxis", &probe);
                }
                if (!found)
                    continue;
                for (int k = 0; k < morphObjects[i].channelCount; k++) {
                    MorphBastardChannel* channel = &morphObjects[i].channels[k];
                    if (channel->controller == controller && channelAxis == channel->axis) {
                        channel->targetNumbers = (int*)DebugRealloc(
                            channel->targetNumbers, channel->targetCount * 4 + 4, kMorphBastardModifierFile, 143);
                        channel->targetNumbers[channel->targetCount] = j;
                        channel->targetCount++;
                        isNew = 0;
                    }
                }
                if (isNew && controller) {
                    morphObjects[i].channels = (MorphBastardChannel*)DebugRealloc(
                        morphObjects[i].channels,
                        (morphObjects[i].channelCount + 1) * sizeof(MorphBastardChannel), kMorphBastardModifierFile, 152);
                    MorphBastardChannel* channel = &morphObjects[i].channels[morphObjects[i].channelCount];
                    channel->controller = controller;
                    channel->axis = channelAxis;
                    channel->minimumValue = 0;
                    channel->maximumValue = 0;
                    channel->targetCount = 2;
                    channel->targets = 0;
                    channel->targetNumbers = (int*)DebugMalloc(12, kMorphBastardModifierFile, 161);
                    channel->targetNumbers[0] = -1;
                    channel->targetNumbers[1] = j;
                    channel->field_0x5c = -1;
                    controller->GetMatrixIn(0, &channel->inverseRestMatrix);
                    MorphBastardInvertRigidInPlace(&channel->inverseRestMatrix);
                    morphObjects[i].channelCount++;
                }
            }
        }

        for (int k = 0; k < morphObjects[i].channelCount; k++) {
            MorphBastardChannel* channel = &morphObjects[i].channels[k];
            channel->targets = (MorphBastardTarget*)DebugMalloc(
                channel->targetCount * sizeof(MorphBastardTarget), kMorphBastardModifierFile, 178);
            channel->targets[0].value = 0;
            channel->targets[0].deltaCount = 0;
            channel->targets[0].deltas = 0;
            channel->targets[0].vertexIndices = 0;
            channel->targets[0].lastWeights = 0;
            float targetValue = 0;
            for (int t = 1; t < channel->targetCount; t++) {
                sprintf(section, "MorphBastard %i Target %i", i, channel->targetNumbers[t]);
                block->SelectSection(section);
                if (channel->axis == 0)
                    block->ReadFloat("XAxis", &targetValue);
                else if (channel->axis == 1)
                    block->ReadFloat("YAxis", &targetValue);
                else if (channel->axis == 2)
                    block->ReadFloat("ZAxis", &targetValue);
                channel->targets[t].value = targetValue;
                if (targetValue < channel->minimumValue)
                    channel->minimumValue = targetValue;
                if (targetValue > channel->maximumValue)
                    channel->maximumValue = targetValue;
                MorphBastardTarget* target = &channel->targets[t];
                sprintf(section, "MorphBastard %i Target %i Deltas", i, channel->targetNumbers[t]);
                target->deltaCount = block->BeginRows(section);
                target->vertexIndices = (int*)DebugMalloc(target->deltaCount * 4, kMorphBastardModifierFile, 217);
                target->deltas = (Vector3*)DebugMalloc(target->deltaCount * 12, kMorphBastardModifierFile, 218);
                target->lastWeights = (float*)DebugMalloc(target->deltaCount * 4, kMorphBastardModifierFile, 219);
                for (int r = 0; r < target->deltaCount; r++) {
                    block->ReadRow(0);
                    block->GetRowInt(1, &target->vertexIndices[r]);
                    block->GetRowFloat(2, &target->deltas[r].x);
                    block->GetRowFloat(3, &target->deltas[r].y);
                    block->GetRowFloat(4, &target->deltas[r].z);
                }
            }
            qsort(channel->targets, channel->targetCount, sizeof(MorphBastardTarget),
                  CompareTargets);
        }
    }

    if (block)
        delete block;
    if (stream)
        delete stream;
    return this;
}

// The view KrustyBike.cpp calls through (0x58-byte MorphBastardModifier).
struct KbMorphMod {
    KbMorphMod* LoadParameterFile(void* a, const char* path, SoultreeObject* node);
};

KbMorphMod* KbMorphMod::LoadParameterFile(void* a, const char* path, SoultreeObject* node)
{
    return (KbMorphMod*)((MorphBastardModifier*)this)->LoadParameterFile(a, path, node);
}
