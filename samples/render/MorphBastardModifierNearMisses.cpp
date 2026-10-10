// Near-miss MorphBastardModifier candidates, kept out of src/reconstructed
// until they match. See src/reconstructed/MorphBastardModifier.h for the
// class evidence.
//
// MorphBastardModifier::UnknownFunction4a33b0 (0x004a33b0, 2046 bytes): the
// parameter-file loader, 1938 of 2046 positions. Calls, strings, lines,
// field offsets and the frame size (0x350) match retail; the object's rigid
// inverse is MorphBastardInvertRigid (an inline building a Vector3; the
// plain-locals form leaves the frame 0x18 bytes short). An existing channel
// records the target index j (retail reads it from j's slot), and its test
// is `channel->controller == controller && channelAxis == channel->axis`
// (retail's strength-reduced channel offset in ebx and its `cmp eax, ecx`).
// The new channel's inverse goes through MorphBastardInvertRigidInPlace,
// which gives retail's term order; the products' operand order inside
// t.x/t.y/t.z still differs (six term orders per sum, all 216 combinations,
// tried).
// Remaining: the stack homes of the spilled scalars (retail: controller
// +0x10, j +0x14, axis +0x1c, isNew +0x20, k +0x24, i +0x28, targetCount
// +0x30, found +0x40, probe +0x44; the two inverse temporaries 0x48/0x54 in
// the other order). Declaration scopes of targetCount, probe, found and i,
// constant channel axes, the isNew/controller test order and the
// targetValue initialisation do not move them.
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
    block->UnknownFunction4b7f10("NumberOfMorphBastards", 0, &objectCount);
    morphObjects = new (__FILE__, 87) MorphBastardObject[objectCount];

    for (int i = 0; i < objectCount; i++) {
        sprintf(section, "MorphBastard %i", i);
        block->UnknownFunction4b78f0(section);
        block->UnknownFunction4b7b30("ObjectName", objectName, -1);
        MorphBastardNode* node = model->UnknownFunction4fdae0(objectName);
        morphObjects[i].node = node;
        node->UnknownFunction4fca80(0, &morphObjects[i].inverseRestMatrix);
        MorphBastardInvertRigid(morphObjects[i].inverseRestMatrix);
        block->UnknownFunction4b7f10("NumberOfTargets", 0, &targetCount);
        morphObjects[i].channelCount = 0;
        morphObjects[i].channels = 0;

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
                for (int k = 0; k < morphObjects[i].channelCount; k++) {
                    MorphBastardChannel* channel = &morphObjects[i].channels[k];
                    if (channel->controller == controller && channelAxis == channel->axis) {
                        channel->targetNumbers = (int*)DebugRealloc(
                            channel->targetNumbers, channel->targetCount * 4 + 4, __FILE__, 143);
                        channel->targetNumbers[channel->targetCount] = j;
                        channel->targetCount++;
                        isNew = 0;
                    }
                }
                if (isNew && controller) {
                    morphObjects[i].channels = (MorphBastardChannel*)DebugRealloc(
                        morphObjects[i].channels,
                        (morphObjects[i].channelCount + 1) * sizeof(MorphBastardChannel), __FILE__, 152);
                    MorphBastardChannel* channel = &morphObjects[i].channels[morphObjects[i].channelCount];
                    channel->controller = controller;
                    channel->axis = channelAxis;
                    channel->minimumValue = 0;
                    channel->maximumValue = 0;
                    channel->targetCount = 2;
                    channel->targets = 0;
                    channel->targetNumbers = (int*)DebugMalloc(12, __FILE__, 161);
                    channel->targetNumbers[0] = -1;
                    channel->targetNumbers[1] = j;
                    channel->field_0x5c = -1;
                    controller->UnknownFunction4fca80(0, &channel->inverseRestMatrix);
                    MorphBastardInvertRigidInPlace(&channel->inverseRestMatrix);
                    morphObjects[i].channelCount++;
                }
            }
        }

        for (int k = 0; k < morphObjects[i].channelCount; k++) {
            MorphBastardChannel* channel = &morphObjects[i].channels[k];
            channel->targets = (MorphBastardTarget*)DebugMalloc(
                channel->targetCount * sizeof(MorphBastardTarget), __FILE__, 178);
            channel->targets[0].value = 0;
            channel->targets[0].deltaCount = 0;
            channel->targets[0].deltas = 0;
            channel->targets[0].vertexIndices = 0;
            channel->targets[0].lastWeights = 0;
            float targetValue = 0;
            for (int t = 1; t < channel->targetCount; t++) {
                sprintf(section, "MorphBastard %i Target %i", i, channel->targetNumbers[t]);
                block->UnknownFunction4b78f0(section);
                if (channel->axis == 0)
                    block->UnknownFunction4b7e70("XAxis", &targetValue);
                else if (channel->axis == 1)
                    block->UnknownFunction4b7e70("YAxis", &targetValue);
                else if (channel->axis == 2)
                    block->UnknownFunction4b7e70("ZAxis", &targetValue);
                channel->targets[t].value = targetValue;
                if (targetValue < channel->minimumValue)
                    channel->minimumValue = targetValue;
                if (targetValue > channel->maximumValue)
                    channel->maximumValue = targetValue;
                MorphBastardTarget* target = &channel->targets[t];
                sprintf(section, "MorphBastard %i Target %i Deltas", i, channel->targetNumbers[t]);
                target->deltaCount = block->UnknownFunction4b7f70(section);
                target->vertexIndices = (int*)DebugMalloc(target->deltaCount * 4, __FILE__, 217);
                target->deltas = (Vector3*)DebugMalloc(target->deltaCount * 12, __FILE__, 218);
                target->lastWeights = (float*)DebugMalloc(target->deltaCount * 4, __FILE__, 219);
                for (int r = 0; r < target->deltaCount; r++) {
                    block->UnknownFunction4b8010(0);
                    block->UnknownFunction4b8180(1, &target->vertexIndices[r]);
                    block->UnknownFunction4b81c0(2, &target->deltas[r].x);
                    block->UnknownFunction4b81c0(3, &target->deltas[r].y);
                    block->UnknownFunction4b81c0(4, &target->deltas[r].z);
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

// 0x004a4c60
void MorphBastardModifier::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                                UnknownSoultreeMesh** out)
{
    if (((MorphBastardSoultreeView*)object)->drawUnchanged) {
        *out = mesh;
        return;
    }
    if (!field_0x40)
        CopyMesh(mesh);
    memcpy(field_0x40->field_0x14, mesh->field_0x14, field_0x40->vertexCount * sizeof(MorphBastardVertex));

    float value;
    for (int i = 0; i < objectCount; i++) {
        UpdateChannels(&morphObjects[i]);
        UnknownFunction464e80(i);
        MorphBastardMeshGroup* group;
        for (int g = 0; g < mesh->groupCount; g++) {
            if (mesh->field_0x04[g].node == morphObjects[i].node)
                group = &mesh->field_0x04[g];
        }
        if (!deltaSums) {
            deltaSums = new (__FILE__, 541) Vector3[mesh->vertexCount];
            deltaMinimums = new (__FILE__, 542) Vector3[mesh->vertexCount];
            deltaMaximums = new (__FILE__, 543) Vector3[mesh->vertexCount];
        }
        int base = ((int)group->field_0x08 - (int)mesh->field_0x10) / 32;
        memset(deltaSums, 0, group->vertexCount * sizeof(Vector3));
        memset(deltaMinimums, 0, group->vertexCount * sizeof(Vector3));
        memset(deltaMaximums, 0, group->vertexCount * sizeof(Vector3));

        for (int j = 0; j < morphObjects[i].channelCount; j++) {
            MorphBastardChannel* channel = &morphObjects[i].channels[j];
            if (channel->axis == 0)
                value = channel->currentValue;
            else if (channel->axis == 1)
                value = channel->field_0x64;
            else if (channel->axis == 2)
                value = channel->field_0x68;

            float weight = 0.0f;
            float upper = 0.0f;
            int lo = -1;
            int hi = -1;
            if (value <= channel->minimumValue) {
                weight = 1.0f;
                lo = 0;
            } else if (value >= channel->maximumValue) {
                weight = 1.0f;
                lo = channel->targetCount - 1;
            } else {
                for (int k = 0; k < channel->targetCount - 1; k++) {
                    if (value > channel->targets[k].value &&
                        value <= channel->targets[k + 1].value) {
                        upper = (value - channel->targets[k].value) /
                                (channel->targets[k + 1].value - channel->targets[k].value);
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
                for (int d = 0; d < channel->targets[lo].deltaCount; d++) {
                    channel->targets[lo].lastWeights[d] = weight;
                    int index = channel->targets[lo].vertexIndices[d] - base;
                    Vector3 delta = weight * channel->targets[lo].deltas[d];
                    deltaSums[index] += delta;
                    if (delta.x < deltaMinimums[index].x)
                        deltaMinimums[index].x = delta.x;
                    else if (delta.x > deltaMaximums[index].x)
                        deltaMaximums[index].x = delta.x;
                    if (delta.y < deltaMinimums[index].y)
                        deltaMinimums[index].y = delta.y;
                    else if (delta.y > deltaMaximums[index].y)
                        deltaMaximums[index].y = delta.y;
                    if (delta.z < deltaMinimums[index].z)
                        deltaMinimums[index].z = delta.z;
                    else if (delta.z > deltaMaximums[index].z)
                        deltaMaximums[index].z = delta.z;
                }
            }
            if (upper != 0.0f) {
                for (int d = 0; d < channel->targets[hi].deltaCount; d++) {
                    channel->targets[hi].lastWeights[d] = upper;
                    int index = channel->targets[hi].vertexIndices[d] - base;
                    Vector3 delta = upper * channel->targets[hi].deltas[d];
                    deltaSums[index] += delta;
                    if (delta.x < deltaMinimums[index].x)
                        deltaMinimums[index].x = delta.x;
                    else if (delta.x > deltaMaximums[index].x)
                        deltaMaximums[index].x = delta.x;
                    if (delta.y < deltaMinimums[index].y)
                        deltaMinimums[index].y = delta.y;
                    else if (delta.y > deltaMaximums[index].y)
                        deltaMaximums[index].y = delta.y;
                    if (delta.z < deltaMinimums[index].z)
                        deltaMinimums[index].z = delta.z;
                    else if (delta.z > deltaMaximums[index].z)
                        deltaMaximums[index].z = delta.z;
                }
            }
        }

        for (int v = 0; v < group->vertexCount; v++) {
            Vector3 offset = deltaSums[v];
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
    channel->controller->UnknownFunction4fca80(0, &controller);
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
    float angle = AngleBetween(from, to);
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
    float angleX = AngleBetween(alignedY, controllerY) * 57.2957764f;
    if (cross.y * to.y + cross.x * to.x + cross.z * to.z < 0.0f)
        angleX = -angleX;

    // The y axes.
    from = *(Vector3*)&identity.m[1][0];
    to = *(Vector3*)&controller.m[1][0];
    axis = UnknownFunction515600(&from, &to);
    angle = AngleBetween(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(0.0f, 1.0f, 0.0f);
    else
        NormalizeOrZero(axis);
    UnknownFunction42de90(&rotation, axis.x, axis.y, axis.z, angle);
    UnknownFunction436500(&aligned, &identity, &rotation);
    cross = UnknownFunction515600(&UnknownFunction4a2350(&aligned, 2), &UnknownFunction4a2350(&controller, 2));
    float angleY = AngleBetween(UnknownFunction4a2350(&aligned, 2), UnknownFunction4a2350(&controller, 2)) *
                   57.2957764f;
    if (UnknownFunction40ae30(&to, &cross) < 0.0f)
        angleY = -angleY;

    // The z axes.
    from = UnknownFunction4a2350(&identity, 2);
    to = UnknownFunction4a2350(&controller, 2);
    axis = UnknownFunction515600(&from, &to);
    angle = AngleBetween(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(0.0f, 0.0f, 1.0f);
    else
        NormalizeOrZero(axis);
    UnknownFunction42de90(&rotation, axis.x, axis.y, axis.z, angle);
    UnknownFunction436500(&aligned, &identity, &rotation);
    cross = UnknownFunction515600(&UnknownFunction4a2350(&aligned, 0), &UnknownFunction4a2350(&controller, 0));
    UnknownFunction40ae30(&UnknownFunction4a2350(&aligned, 0), &UnknownFunction4a2350(&controller, 0));
    float angleZ = AngleBetween(UnknownFunction4a2350(&aligned, 0), UnknownFunction4a2350(&controller, 0)) *
                   57.2957764f;
    if (UnknownFunction40ae30(&to, &cross) < 0.0f)
        angleZ = -angleZ;

    channel->currentValue = -angleX;
    channel->field_0x64 = -angleZ;
    channel->field_0x68 = -angleY;
}
