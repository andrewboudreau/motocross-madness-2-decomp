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
// MorphBastardModifier::UnknownFunction4a3c80 (0x004a3c80..0x004a4ba5,
// 3878 bytes, ret 0x44): the controller angles. Controller matrix times
// the rest inverse (inline, through a copy), then per axis: rotate the
// identity's axis onto the controller's (cross product, 0x004a3be0 angle,
// normalise or fall back to the unit axis, axis-angle matrix, 0x00436500)
// and measure the angle between the next rows in degrees (57.2957764),
// signed by the cross product. Stores -x, -z, -y at +0x60/+0x64/+0x68.
// The first axis inlines identity, rows, cross product and the axis-angle
// matrix; the later ones call 0x004a2350, 0x00515600, 0x0042de90,
// 0x0040ae30 and the Vector3 constructor 0x00404e60 (likely VC6's inline
// budget). Normalised ratio 0.93 over the first half; 872 of 896
// instructions, retail frame 0x1fc (ours 0x1b4); the source here uses the
// inline Vector3 constructor where retail calls 0x00404e60, and VC6's
// term order differs in the products.
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

// ---------------------------------------------------------------------------
// 0x004a3c80..0x004a4ba5 (ret 0x44): the controller angles. Not matched; see
// the notes at the top of this file. The helpers retail calls out of line
// are declared here; the first uses of the same operations are inline in
// retail (identity, the row copies, the first cross products and the first
// axis-angle matrix).

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);

float FastInvSqrt(float x);                                          // 0x00460c00
float UnknownFunction40ae30(const Vector3* a, const Vector3* b);     // 0x0040ae30: a.b
Vector3 UnknownFunction515600(const Vector3* a, const Vector3* b);   // 0x00515600: a x b
Vector3 UnknownFunction4a2350(const Matrix4* m, int row);            // 0x004a2350: row of m
void UnknownFunction42de90(Matrix4* out, float x, float y, float z, float angle); // 0x0042de90: axis-angle
void UnknownFunction436500(Matrix4* out, const Matrix4* a, const Matrix4* b);    // 0x00436500: a * b

static inline void NormalizeOrZero(Vector3& v)
{
    float squared = UnknownFunction40ae30(&v, &v);
    if (squared == 0.0f) {
        v = kVec3Zero;
    } else {
        float scale = FastInvSqrt(squared);
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }
}

// 0x004a3c80: the rotation of the channel's controller relative to its rest
// pose (`inverse`), as three angles in degrees. For each axis the identity's
// axis is rotated onto the controller's; the angle left between the next
// axes is the rotation about it.
void MorphBastardModifier::UnknownFunction4a3c80(MorphBastardChannel* channel, Matrix4 inverse)
{
    Matrix4 identity;
    memset(&identity, 0, sizeof(identity));
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;
    identity.m[3][3] = 1.0f;
    Matrix4 controller;
    channel->field_0x44->UnknownFunction4fca80(0, &controller);
    Matrix4 world = controller;
    controller(0, 0) = world(0, 0) * inverse(0, 0) + world(0, 1) * inverse(1, 0) + world(0, 2) * inverse(2, 0) +
                       world(0, 3) * inverse(3, 0);
    controller(0, 1) = world(0, 0) * inverse(0, 1) + world(0, 1) * inverse(1, 1) + world(0, 2) * inverse(2, 1) +
                       world(0, 3) * inverse(3, 1);
    controller(0, 2) = world(0, 0) * inverse(0, 2) + world(0, 1) * inverse(1, 2) + world(0, 2) * inverse(2, 2) +
                       world(0, 3) * inverse(3, 2);
    controller(0, 3) = world(0, 0) * inverse(0, 3) + world(0, 1) * inverse(1, 3) + world(0, 2) * inverse(2, 3) +
                       world(0, 3) * inverse(3, 3);
    controller(1, 0) = world(1, 0) * inverse(0, 0) + world(1, 1) * inverse(1, 0) + world(1, 2) * inverse(2, 0) +
                       world(1, 3) * inverse(3, 0);
    controller(1, 1) = world(1, 0) * inverse(0, 1) + world(1, 1) * inverse(1, 1) + world(1, 2) * inverse(2, 1) +
                       world(1, 3) * inverse(3, 1);
    controller(1, 2) = world(1, 0) * inverse(0, 2) + world(1, 1) * inverse(1, 2) + world(1, 2) * inverse(2, 2) +
                       world(1, 3) * inverse(3, 2);
    controller(1, 3) = world(1, 0) * inverse(0, 3) + world(1, 1) * inverse(1, 3) + world(1, 2) * inverse(2, 3) +
                       world(1, 3) * inverse(3, 3);
    controller(2, 0) = world(2, 0) * inverse(0, 0) + world(2, 1) * inverse(1, 0) + world(2, 2) * inverse(2, 0) +
                       world(2, 3) * inverse(3, 0);
    controller(2, 1) = world(2, 0) * inverse(0, 1) + world(2, 1) * inverse(1, 1) + world(2, 2) * inverse(2, 1) +
                       world(2, 3) * inverse(3, 1);
    controller(2, 2) = world(2, 0) * inverse(0, 2) + world(2, 1) * inverse(1, 2) + world(2, 2) * inverse(2, 2) +
                       world(2, 3) * inverse(3, 2);
    controller(2, 3) = world(2, 0) * inverse(0, 3) + world(2, 1) * inverse(1, 3) + world(2, 2) * inverse(2, 3) +
                       world(2, 3) * inverse(3, 3);
    controller(3, 0) = world(3, 0) * inverse(0, 0) + world(3, 1) * inverse(1, 0) + world(3, 2) * inverse(2, 0) +
                       world(3, 3) * inverse(3, 0);
    controller(3, 1) = world(3, 0) * inverse(0, 1) + world(3, 1) * inverse(1, 1) + world(3, 2) * inverse(2, 1) +
                       world(3, 3) * inverse(3, 1);
    controller(3, 2) = world(3, 0) * inverse(0, 2) + world(3, 1) * inverse(1, 2) + world(3, 2) * inverse(2, 2) +
                       world(3, 3) * inverse(3, 2);
    controller(3, 3) = world(3, 0) * inverse(0, 3) + world(3, 1) * inverse(1, 3) + world(3, 2) * inverse(2, 3) +
                       world(3, 3) * inverse(3, 3);

    // The x axes.
    Vector3 from = *(Vector3*)&identity.m[0][0];
    Vector3 to = *(Vector3*)&controller.m[0][0];
    Vector3 axis = Vector3(from.y * to.z - from.z * to.y, from.z * to.x - to.z * from.x, to.y * from.x - from.y * to.x);
    float angle = UnknownFunction4a3be0(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(1.0f, 0.0f, 0.0f);
    else
        NormalizeOrZero(axis);
    float length = (float)sqrt(axis.z * axis.z + axis.y * axis.y + axis.x * axis.x);
    float x = axis.x / length;
    float y = axis.y / length;
    float z = axis.z / length;
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float t = 1.0f - c;
    Matrix4 rotation;
    rotation(0, 0) = t * x * x + c;
    rotation(0, 1) = t * x * y + s * z;
    rotation(0, 2) = t * x * z - s * y;
    rotation(0, 3) = 0.0f;
    rotation(1, 0) = t * x * y - s * z;
    rotation(1, 1) = t * y * y + c;
    rotation(1, 2) = t * y * z + s * x;
    rotation(1, 3) = 0.0f;
    rotation(2, 0) = t * x * z + s * y;
    rotation(2, 1) = t * y * z - s * x;
    rotation(2, 2) = t * z * z + c;
    rotation(2, 3) = 0.0f;
    rotation(3, 0) = 0.0f;
    rotation(3, 1) = 0.0f;
    rotation(3, 2) = 0.0f;
    rotation(3, 3) = 1.0f;
    Matrix4 aligned;
    UnknownFunction436500(&aligned, &identity, &rotation);
    Vector3 alignedY = *(Vector3*)&aligned.m[1][0];
    Vector3 controllerY = *(Vector3*)&controller.m[1][0];
    Vector3 cross(alignedY.y * controllerY.z - alignedY.z * controllerY.y,
                  alignedY.z * controllerY.x - alignedY.x * controllerY.z,
                  alignedY.x * controllerY.y - alignedY.y * controllerY.x);
    float angleX = UnknownFunction4a3be0(alignedY, controllerY) * 57.2957764f;
    if (cross.y * to.y + cross.x * to.x + cross.z * to.z < 0.0f)
        angleX = -angleX;

    // The y axes.
    from = *(Vector3*)&identity.m[1][0];
    to = *(Vector3*)&controller.m[1][0];
    axis = UnknownFunction515600(&from, &to);
    angle = UnknownFunction4a3be0(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(0.0f, 1.0f, 0.0f);
    else
        NormalizeOrZero(axis);
    UnknownFunction42de90(&rotation, axis.x, axis.y, axis.z, angle);
    UnknownFunction436500(&aligned, &identity, &rotation);
    cross = UnknownFunction515600(&UnknownFunction4a2350(&aligned, 2), &UnknownFunction4a2350(&controller, 2));
    float angleY = UnknownFunction4a3be0(UnknownFunction4a2350(&aligned, 2), UnknownFunction4a2350(&controller, 2)) *
                   57.2957764f;
    if (UnknownFunction40ae30(&to, &cross) < 0.0f)
        angleY = -angleY;

    // The z axes.
    from = UnknownFunction4a2350(&identity, 2);
    to = UnknownFunction4a2350(&controller, 2);
    axis = UnknownFunction515600(&from, &to);
    angle = UnknownFunction4a3be0(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(0.0f, 0.0f, 1.0f);
    else
        NormalizeOrZero(axis);
    UnknownFunction42de90(&rotation, axis.x, axis.y, axis.z, angle);
    UnknownFunction436500(&aligned, &identity, &rotation);
    cross = UnknownFunction515600(&UnknownFunction4a2350(&aligned, 0), &UnknownFunction4a2350(&controller, 0));
    UnknownFunction40ae30(&UnknownFunction4a2350(&aligned, 0), &UnknownFunction4a2350(&controller, 0));
    float angleZ = UnknownFunction4a3be0(UnknownFunction4a2350(&aligned, 0), UnknownFunction4a2350(&controller, 0)) *
                   57.2957764f;
    if (UnknownFunction40ae30(&to, &cross) < 0.0f)
        angleZ = -angleZ;

    channel->field_0x60 = -angleX;
    channel->field_0x64 = -angleZ;
    channel->field_0x68 = -angleY;
}
