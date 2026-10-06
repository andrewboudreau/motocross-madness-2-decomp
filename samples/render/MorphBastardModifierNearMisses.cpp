// Near-miss MorphBastardModifier candidates, kept out of src/reconstructed
// until they match. See src/reconstructed/MorphBastardModifier.h for the
// class evidence.
//
// MorphBastardModifier::UnknownFunction4a33b0 (0x004a33b0, 1982 bytes): the
// parameter-file loader. Calls, strings, lines, field offsets and the frame
// size (0x350) match retail; the rigid inverse is an inline that builds a
// Vector3 (the plain-locals form leaves the frame 0x18 bytes short). The
// remaining differences are VC6 stack-slot and register assignment for the
// loop counters and flags (retail: controller +0x10, j +0x14, axis +0x1c,
// isNew +0x20, k +0x24, i +0x28, targetCount +0x30, channel axis +0x34,
// found +0x40).
//
// MorphBastardModifier::UnknownVirtualSlot27 (0x004a4c60, 1568 bytes): blends
// the target deltas of every channel into the copied mesh. The control flow,
// calls and the signed /32 vertex base match; register allocation of the
// object/channel loops differs (retail keeps the object offset in esi and
// reloads the index from +0x28, the channel in ebp).
//
// MorphBastardModifier::UnknownFunction4a3c80 (0x004a3c80, 2340 bytes,
// ret 0x44) is not reconstructed: it derives the controller's rotation about
// each axis from the controller and object matrices through 0x004a2350,
// 0x00515600, 0x0040ae30, 0x0042de90, 0x00436500, 0x00460c00 and 0x004a3be0
// and stores the three angles at channel +0x60..+0x68.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/MorphBastardModifier.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

// 0x004a33b0
MorphBastardModifier* MorphBastardModifier::UnknownFunction4a33b0(void* value, const char* path,
                                                                 MorphBastardNode* model)
{
    char section[0x100];
    char objectName[0x100];
    char controllerName[0x100];
    int targetCount;
    float probe;

    D3DIMSoultreeModifier::UnknownVirtualSlot8(value);
    UnknownTextureStream* stream =
        new (__FILE__, 73) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    stream->UnknownFunction460f50(path, "r", 0);
    UnknownParameterBlock* block = new (__FILE__, 77) UnknownParameterBlock;
    block->UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
    sprintf(section, "MorphBastard Information");
    block->UnknownFunction4b78f0(section);
    block->UnknownFunction4b7f10("NumberOfMorphBastards", 0, &field_0x44);
    field_0x48 = new (__FILE__, 87) MorphBastardObject[field_0x44];

    for (int i = 0; i < field_0x44; i++) {
        sprintf(section, "MorphBastard %i", i);
        block->UnknownFunction4b78f0(section);
        block->UnknownFunction4b7b30("ObjectName", objectName, -1);
        MorphBastardNode* node = model->UnknownFunction4fdae0(objectName);
        field_0x48[i].field_0x08 = node;
        node->UnknownFunction4fca80(0, &field_0x48[i].field_0x0c);
        MorphBastardInvertRigid(field_0x48[i].field_0x0c);
        block->UnknownFunction4b7f10("NumberOfTargets", 0, &targetCount);
        field_0x48[i].field_0x00 = 0;
        field_0x48[i].field_0x04 = 0;

        for (int j = 0; j < targetCount; j++) {
            sprintf(section, "MorphBastard %i Target %i", i, j);
            block->UnknownFunction4b78f0(section);
            block->UnknownFunction4b7b30("ControllerName", controllerName, -1);
            MorphBastardNode* controller = model->UnknownFunction4fdae0(controllerName);
            int isNew = 1;
            for (int axis = 0; axis < 3; axis++) {
                int found;
                int channelAxis;
                if (axis == 0) {
                    channelAxis = axis;
                    found = block->UnknownFunction4b7e70("XAxis", &probe);
                } else if (axis == 1) {
                    channelAxis = axis;
                    found = block->UnknownFunction4b7e70("YAxis", &probe);
                } else if (axis == 2) {
                    channelAxis = axis;
                    found = block->UnknownFunction4b7e70("ZAxis", &probe);
                }
                if (!found)
                    continue;
                for (int k = 0; k < field_0x48[i].field_0x00; k++) {
                    MorphBastardChannel* channel = &field_0x48[i].field_0x04[k];
                    if (channel->field_0x44 == controller && channel->field_0x00 == channelAxis) {
                        channel->field_0x48 = (int*)DebugRealloc(
                            channel->field_0x48, channel->field_0x4c * 4 + 4, __FILE__, 143);
                        channel->field_0x48[channel->field_0x4c] = k;
                        channel->field_0x4c++;
                        isNew = 0;
                    }
                }
                if (isNew && controller) {
                    field_0x48[i].field_0x04 = (MorphBastardChannel*)DebugRealloc(
                        field_0x48[i].field_0x04,
                        (field_0x48[i].field_0x00 + 1) * sizeof(MorphBastardChannel), __FILE__, 152);
                    MorphBastardChannel* channel = &field_0x48[i].field_0x04[field_0x48[i].field_0x00];
                    channel->field_0x44 = controller;
                    channel->field_0x00 = channelAxis;
                    channel->field_0x54 = 0;
                    channel->field_0x58 = 0;
                    channel->field_0x4c = 2;
                    channel->field_0x50 = 0;
                    channel->field_0x48 = (int*)DebugMalloc(12, __FILE__, 161);
                    channel->field_0x48[0] = -1;
                    channel->field_0x48[1] = j;
                    channel->field_0x5c = -1;
                    controller->UnknownFunction4fca80(0, &channel->field_0x04);
                    MorphBastardInvertRigid(channel->field_0x04);
                    field_0x48[i].field_0x00++;
                }
            }
        }

        for (int k = 0; k < field_0x48[i].field_0x00; k++) {
            MorphBastardChannel* channel = &field_0x48[i].field_0x04[k];
            channel->field_0x50 = (MorphBastardTarget*)DebugMalloc(
                channel->field_0x4c * sizeof(MorphBastardTarget), __FILE__, 178);
            channel->field_0x50[0].field_0x00 = 0;
            channel->field_0x50[0].field_0x04 = 0;
            channel->field_0x50[0].field_0x0c = 0;
            channel->field_0x50[0].field_0x08 = 0;
            channel->field_0x50[0].field_0x10 = 0;
            float targetValue = 0;
            for (int t = 1; t < channel->field_0x4c; t++) {
                sprintf(section, "MorphBastard %i Target %i", i, channel->field_0x48[t]);
                block->UnknownFunction4b78f0(section);
                if (channel->field_0x00 == 0)
                    block->UnknownFunction4b7e70("XAxis", &targetValue);
                else if (channel->field_0x00 == 1)
                    block->UnknownFunction4b7e70("YAxis", &targetValue);
                else if (channel->field_0x00 == 2)
                    block->UnknownFunction4b7e70("ZAxis", &targetValue);
                channel->field_0x50[t].field_0x00 = targetValue;
                if (targetValue < channel->field_0x54)
                    channel->field_0x54 = targetValue;
                if (targetValue > channel->field_0x58)
                    channel->field_0x58 = targetValue;
                MorphBastardTarget* target = &channel->field_0x50[t];
                sprintf(section, "MorphBastard %i Target %i Deltas", i, channel->field_0x48[t]);
                target->field_0x04 = block->UnknownFunction4b7f70(section);
                target->field_0x08 = (int*)DebugMalloc(target->field_0x04 * 4, __FILE__, 217);
                target->field_0x0c = (Vector3*)DebugMalloc(target->field_0x04 * 12, __FILE__, 218);
                target->field_0x10 = (float*)DebugMalloc(target->field_0x04 * 4, __FILE__, 219);
                for (int r = 0; r < target->field_0x04; r++) {
                    block->UnknownFunction4b8010(0);
                    block->UnknownFunction4b8180(1, &target->field_0x08[r]);
                    block->UnknownFunction4b81c0(2, &target->field_0x0c[r].x);
                    block->UnknownFunction4b81c0(3, &target->field_0x0c[r].y);
                    block->UnknownFunction4b81c0(4, &target->field_0x0c[r].z);
                }
            }
            qsort(channel->field_0x50, channel->field_0x4c, sizeof(MorphBastardTarget),
                  UnknownFunction4a3bb0);
        }
    }

    if (block)
        delete block;
    if (stream)
        delete stream;
    return this;
}

// 0x004a4c60
void MorphBastardModifier::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                                UnknownSoultreeMesh** out)
{
    if (((MorphBastardSoultreeView*)object)->field_0x27c) {
        *out = mesh;
        return;
    }
    if (!field_0x40)
        UnknownFunction4a5290(mesh);
    memcpy(field_0x40->field_0x14, mesh->field_0x14, field_0x40->field_0x08 * sizeof(MorphBastardVertex));

    float value;
    for (int i = 0; i < field_0x44; i++) {
        UnknownFunction4a4bb0(&field_0x48[i]);
        UnknownFunction464e80(i);
        MorphBastardMeshGroup* group;
        for (int g = 0; g < mesh->field_0x00; g++) {
            if (mesh->field_0x04[g].field_0x00 == field_0x48[i].field_0x08)
                group = &mesh->field_0x04[g];
        }
        if (!field_0x4c) {
            field_0x4c = new (__FILE__, 541) Vector3[mesh->field_0x08];
            field_0x50 = new (__FILE__, 542) Vector3[mesh->field_0x08];
            field_0x54 = new (__FILE__, 543) Vector3[mesh->field_0x08];
        }
        int base = ((int)group->field_0x08 - (int)mesh->field_0x10) / 32;
        memset(field_0x4c, 0, group->field_0x04 * sizeof(Vector3));
        memset(field_0x50, 0, group->field_0x04 * sizeof(Vector3));
        memset(field_0x54, 0, group->field_0x04 * sizeof(Vector3));

        for (int j = 0; j < field_0x48[i].field_0x00; j++) {
            MorphBastardChannel* channel = &field_0x48[i].field_0x04[j];
            if (channel->field_0x00 == 0)
                value = channel->field_0x60;
            else if (channel->field_0x00 == 1)
                value = channel->field_0x64;
            else if (channel->field_0x00 == 2)
                value = channel->field_0x68;

            float weight = 0.0f;
            float upper = 0.0f;
            int lo = -1;
            int hi = -1;
            if (value <= channel->field_0x54) {
                weight = 1.0f;
                lo = 0;
            } else if (value >= channel->field_0x58) {
                weight = 1.0f;
                lo = channel->field_0x4c - 1;
            } else {
                for (int k = 0; k < channel->field_0x4c - 1; k++) {
                    if (value > channel->field_0x50[k].field_0x00 &&
                        value <= channel->field_0x50[k + 1].field_0x00) {
                        upper = (value - channel->field_0x50[k].field_0x00) /
                                (channel->field_0x50[k + 1].field_0x00 - channel->field_0x50[k].field_0x00);
                        weight = 1.0f - upper;
                        lo = k;
                        hi = k + 1;
                    }
                }
            }
            if (lo == -1)
                continue;
            if (hi == -1)
                upper = 0.0f;

            if (weight != 0.0f) {
                for (int d = 0; d < channel->field_0x50[lo].field_0x04; d++) {
                    channel->field_0x50[lo].field_0x10[d] = weight;
                    int index = channel->field_0x50[lo].field_0x08[d] - base;
                    Vector3 delta = weight * channel->field_0x50[lo].field_0x0c[d];
                    field_0x4c[index] += delta;
                    if (delta.x < field_0x50[index].x)
                        field_0x50[index].x = delta.x;
                    else if (delta.x > field_0x54[index].x)
                        field_0x54[index].x = delta.x;
                    if (delta.y < field_0x50[index].y)
                        field_0x50[index].y = delta.y;
                    else if (delta.y > field_0x54[index].y)
                        field_0x54[index].y = delta.y;
                    if (delta.z < field_0x50[index].z)
                        field_0x50[index].z = delta.z;
                    else if (delta.z > field_0x54[index].z)
                        field_0x54[index].z = delta.z;
                }
            }
            if (upper != 0.0f) {
                for (int d = 0; d < channel->field_0x50[hi].field_0x04; d++) {
                    channel->field_0x50[hi].field_0x10[d] = upper;
                    int index = channel->field_0x50[hi].field_0x08[d] - base;
                    Vector3 delta = upper * channel->field_0x50[hi].field_0x0c[d];
                    field_0x4c[index] += delta;
                    if (delta.x < field_0x50[index].x)
                        field_0x50[index].x = delta.x;
                    else if (delta.x > field_0x54[index].x)
                        field_0x54[index].x = delta.x;
                    if (delta.y < field_0x50[index].y)
                        field_0x50[index].y = delta.y;
                    else if (delta.y > field_0x54[index].y)
                        field_0x54[index].y = delta.y;
                    if (delta.z < field_0x50[index].z)
                        field_0x50[index].z = delta.z;
                    else if (delta.z > field_0x54[index].z)
                        field_0x54[index].z = delta.z;
                }
            }
        }

        for (int v = 0; v < group->field_0x04; v++) {
            Vector3 offset = field_0x4c[v];
            field_0x40->field_0x14[base + v].x = offset.x + mesh->field_0x14[base + v].x;
            field_0x40->field_0x14[base + v].y = offset.y + mesh->field_0x14[base + v].y;
            field_0x40->field_0x14[base + v].z = offset.z + mesh->field_0x14[base + v].z;
        }
    }
    *out = field_0x40;
}
