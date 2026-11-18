// MorphBastardModifier.cpp -- see MorphBastardModifier.h for the evidence.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MorphBastardModifier.h"

#include "DebugAlloc.h"
#include "Parameterblocks.h"
#include "TextureMap.h"
#include "ResourceEntry.h"

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h): 0x006850f0, 0x00685100, 0x00685110 and
// 0x006850e0, initialised by 0x004a54f0..0x004a562b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// ControllerAngles helpers: the out-of-line helpers retail calls from its second and
// third axis blocks, declared with retail's pointer signatures.
// (Their first uses, in the x axis block, are expanded inline in retail;
// ControllerAngles writes that block out the same way.)  The defined providers are
// VectorCalls.cpp's Vec3DotCall, CrossProduct.cpp's CrossProductCall,
// MovingPart.cpp's GetMatrixRow, BoundingBoxTreeBuild.cpp's AxisAngleMatrix (a
// TreeMatrix4, the same 64 bytes) and CollisionObject.cpp's CollisionMatrixMultiply;
// config/game_build.json's link_aliases bind the names whose types differ (same cdecl
// ABI: a reference and a pointer are both one address, and the Vector3/Vec3 views are
// both three floats returned through a hidden pointer).
float FastInvSqrt(float x);                                          // 0x00460c00 (FastMath.cpp)
float Vec3Dot(const Vector3* a, const Vector3* b);                   // 0x0040ae30: a.b
Vector3 CrossProduct(const Vector3* a, const Vector3* b);            // 0x00515600: a x b
Vector3 GetMatrixRow(const Matrix4* m, int row);                     // 0x004a2350: row of m
void AxisAngleMatrix(Matrix4* out, float x, float y, float z, float angle); // 0x0042de90
void CollisionMatrixMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b); // 0x00436500

// 0x00404e60: the out-of-line Vector3 constructor.
struct MbVector3Call : Vector3 {
    MbVector3Call(float x, float y, float z);
};

// Normalise, or zero when the length is zero (inline in retail around calls to
// 0x0040ae30 and 0x00460c00).
static inline void MbNormalizeOrZero(Vector3& v)
{
    float squared = Vec3Dot(&v, &v);
    if (squared == 0.0f) {
        v = kVec3Zero;
    } else {
        float scale = FastInvSqrt(squared);
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }
}

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
    onFirstModifierList = 0;
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

// The channel's rigid inverse (0x004a33b0's second expansion): the same
// transpose and negated translation as MorphBastardInvertRigid, through a
// pointer and with the x term summed last. Retail's two expansions in
// 0x004a33b0 order their products differently, which suggests two source
// forms; this one gives retail's term order for the channel. The y term is
// (m31*m11 + m32*m21) + m30*m01 as at 0x004a3846..0x004a3859: summing the x
// product into the first pair instead rounds differently at 24-bit precision
// (tools/emu_cases/morph_load_parameter_file.py).
inline void MorphBastardInvertRigidInPlace(Matrix4* m)
{
    float t;
    t = m->m[0][1]; m->m[0][1] = m->m[1][0]; m->m[1][0] = t;
    t = m->m[0][2]; m->m[0][2] = m->m[2][0]; m->m[2][0] = t;
    t = m->m[1][2]; m->m[1][2] = m->m[2][1]; m->m[2][1] = t;
    Vector3 p(-((m->m[3][2] * m->m[2][0] + m->m[3][1] * m->m[1][0]) + m->m[3][0] * m->m[0][0]),
              -((m->m[3][1] * m->m[1][1] + m->m[3][2] * m->m[2][1]) + m->m[3][0] * m->m[0][1]),
              -(m->m[3][0] * m->m[0][2] + m->m[3][1] * m->m[1][2] + m->m[3][2] * m->m[2][2]));
    m->m[3][0] = p.x;
    m->m[3][1] = p.y;
    m->m[3][2] = p.z;
}

// 0x004a33b0: reads the .mbf parameter file (KrustyBike.cpp loads "RiderMorph.mbf").
// Not exact (1950 of 2046 bytes in place: the spilled scalars' stack homes and
// the operand order of the channel inverse's products; docs/MORPHBASTARDMODIFIER.md).
MorphBastardModifier* MorphBastardModifier::LoadParameterFile(void* renderContext, const char* path,
                                                                 MorphBastardNode* model)
{
    float targetValue;
    char section[0x100];
    char objectName[0x100];
    char controllerName[0x100];
    int targetCount;

    D3DIMSoultreeModifier::SetRenderContext(renderContext);
    FileStream* stream =
        new (__FILE__, 73) FileStream(g_ResourceManager);
    stream->Open(path, "r", 0);
    ParameterBlock* block = new (__FILE__, 77) ParameterBlock;
    block->Attach((FileStream*)stream, 0, 1);
    sprintf(section, "MorphBastard Information");
    block->SelectSection(section);
    block->GetInt("NumberOfMorphBastards", 0, &objectCount);
    morphObjects = new (__FILE__, 87) MorphBastardObject[objectCount];

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
                    controller->GetMatrixIn(0, &channel->inverseRestMatrix);
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
            targetValue = 0;
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
                target->vertexIndices = (int*)DebugMalloc(target->deltaCount * 4, __FILE__, 217);
                target->deltas = (Vector3*)DebugMalloc(target->deltaCount * 12, __FILE__, 218);
                target->lastWeights = (float*)DebugMalloc(target->deltaCount * 4, __FILE__, 219);
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

// 0x004a3c80 (ret 0x44): the rotation of the channel's controller relative to its rest
// pose (`inverse`), as three angles in degrees.  For each axis the identity's axis is
// rotated onto the controller's (cross product, 0x004a3be0 angle, normalise or fall back
// to the unit axis, axis-angle matrix, 0x00436500); the angle left between the next rows
// is the rotation about it, signed by the cross product.  Stores -x, -z, -y at
// +0x60/+0x64/+0x68.  The x axis block is written out inline, with its own axis-angle
// expansion (the length summed z, y, x as retail sums it), and the y and z blocks call
// the helpers above.  Each block has its own from/to/cross/axis set, as retail's frame
// does.  The z block's dot product of the two x rows is in retail too; its result is
// unused.
// Not exact (570 of 3878 bytes in place, 3722 compiled: frame 0x1f0 against retail's
// 0x1fc, and retail keeps a copy of the x block's `to`; docs/MORPHBASTARDMODIFIER.md).
void MorphBastardModifier::ControllerAngles(MorphBastardChannel* channel, Matrix4 inverse)
{
    Matrix4 identity;
    memset(&identity, 0, sizeof(identity));
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;
    identity.m[3][3] = 1.0f;
    Matrix4 controller;
    channel->controller->GetMatrixIn(0, &controller);
    Matrix4 world = controller;
    // controller = world * inverse, written out element by element as retail expands it:
    // each sum is grouped from the last column (0x004a3cde adds w03*i30 + w02*i20 first; a
    // flat four-term sum adds w00*i00 + w01*i10 first, which rounds differently).
    controller(0, 0) = world(0, 0) * inverse(0, 0) +
                       (world(0, 1) * inverse(1, 0) + (world(0, 2) * inverse(2, 0) + world(0, 3) * inverse(3, 0)));
    controller(0, 1) = world(0, 0) * inverse(0, 1) +
                       (world(0, 1) * inverse(1, 1) + (world(0, 2) * inverse(2, 1) + world(0, 3) * inverse(3, 1)));
    controller(0, 2) = world(0, 0) * inverse(0, 2) +
                       (world(0, 1) * inverse(1, 2) + (world(0, 2) * inverse(2, 2) + world(0, 3) * inverse(3, 2)));
    controller(0, 3) = world(0, 0) * inverse(0, 3) +
                       (world(0, 1) * inverse(1, 3) + (world(0, 2) * inverse(2, 3) + world(0, 3) * inverse(3, 3)));
    controller(1, 0) = world(1, 0) * inverse(0, 0) +
                       (world(1, 1) * inverse(1, 0) + (world(1, 2) * inverse(2, 0) + world(1, 3) * inverse(3, 0)));
    controller(1, 1) = world(1, 0) * inverse(0, 1) +
                       (world(1, 1) * inverse(1, 1) + (world(1, 2) * inverse(2, 1) + world(1, 3) * inverse(3, 1)));
    controller(1, 2) = world(1, 0) * inverse(0, 2) +
                       (world(1, 1) * inverse(1, 2) + (world(1, 2) * inverse(2, 2) + world(1, 3) * inverse(3, 2)));
    controller(1, 3) = world(1, 0) * inverse(0, 3) +
                       (world(1, 1) * inverse(1, 3) + (world(1, 2) * inverse(2, 3) + world(1, 3) * inverse(3, 3)));
    controller(2, 0) = world(2, 0) * inverse(0, 0) +
                       (world(2, 1) * inverse(1, 0) + (world(2, 2) * inverse(2, 0) + world(2, 3) * inverse(3, 0)));
    controller(2, 1) = world(2, 0) * inverse(0, 1) +
                       (world(2, 1) * inverse(1, 1) + (world(2, 2) * inverse(2, 1) + world(2, 3) * inverse(3, 1)));
    controller(2, 2) = world(2, 0) * inverse(0, 2) +
                       (world(2, 1) * inverse(1, 2) + (world(2, 2) * inverse(2, 2) + world(2, 3) * inverse(3, 2)));
    controller(2, 3) = world(2, 0) * inverse(0, 3) +
                       (world(2, 1) * inverse(1, 3) + (world(2, 2) * inverse(2, 3) + world(2, 3) * inverse(3, 3)));
    controller(3, 0) = world(3, 0) * inverse(0, 0) +
                       (world(3, 1) * inverse(1, 0) + (world(3, 2) * inverse(2, 0) + world(3, 3) * inverse(3, 0)));
    // Retail sums the next two in another order: ((w32*i21 + w31*i11) + w30*i01) + w33*i31
    // (0x004a408c..0x004a40c8) and ((w30*i02 + w33*i32) + w32*i22) + w31*i12
    // (0x004a40d1..0x004a410d).
    controller(3, 1) = ((world(3, 2) * inverse(2, 1) + world(3, 1) * inverse(1, 1)) + world(3, 0) * inverse(0, 1)) +
                       world(3, 3) * inverse(3, 1);
    controller(3, 2) = ((world(3, 0) * inverse(0, 2) + world(3, 3) * inverse(3, 2)) + world(3, 2) * inverse(2, 2)) +
                       world(3, 1) * inverse(1, 2);
    controller(3, 3) = world(3, 0) * inverse(0, 3) +
                       (world(3, 1) * inverse(1, 3) + (world(3, 2) * inverse(2, 3) + world(3, 3) * inverse(3, 3)));

    // The x axes.
    Vector3 from = *(Vector3*)&identity.m[0][0];
    Vector3 to = *(Vector3*)&controller.m[0][0];
    Vector3 axis = Vector3(from.y * to.z - from.z * to.y, from.z * to.x - to.z * from.x, to.y * from.x - from.y * to.x);
    float angle = AngleBetween(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(1.0f, 0.0f, 0.0f);
    else
        MbNormalizeOrZero(axis);
    float length = (float)sqrt(axis.z * axis.z + axis.y * axis.y + axis.x * axis.x);
    float x = axis.x / length;
    float y = axis.y / length;
    float z = axis.z / length;
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float t = 1.0f - c;
    // Unlike AxisAngleMatrix 0x0042de90 (which shares t*x), retail's x block stores t*y and
    // t*z as binary32 and builds the rows from them (0x004a4395, 0x004a43c0).  VC6 still
    // multiplies the first use of each by the unrounded product; emu_diff runs agree.
    Matrix4 rotation;
    rotation.m[0][0] = t * x * x + c;
    float ty = t * y;
    rotation.m[0][1] = x * ty + s * z;
    float tz = t * z;
    rotation.m[0][2] = x * tz - s * y;
    rotation.m[0][3] = 0.0f;
    rotation.m[1][0] = x * ty - s * z;
    rotation.m[1][1] = y * ty + c;
    rotation.m[1][2] = z * ty + s * x;
    rotation.m[1][3] = 0.0f;
    rotation.m[2][0] = x * tz + s * y;
    rotation.m[2][1] = z * ty - s * x;
    rotation.m[2][2] = z * tz + c;
    rotation.m[2][3] = 0.0f;
    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;
    Matrix4 aligned;
    CollisionMatrixMultiply(&aligned, &identity, &rotation);
    Vector3 alignedY = *(Vector3*)&aligned.m[1][0];
    Vector3 controllerY = *(Vector3*)&controller.m[1][0];
    Vector3 cross(alignedY.y * controllerY.z - alignedY.z * controllerY.y,
                  alignedY.z * controllerY.x - alignedY.x * controllerY.z,
                  alignedY.x * controllerY.y - alignedY.y * controllerY.x);
    float angleX = AngleBetween(alignedY, controllerY) * 57.2957764f;
    // Retail adds the z product to y + x last ((cy*ty + cx*tx) + cz*tz, 0x004a4573..0x004a4596);
    // the flat sum compiled to (cz*tz + cx*tx) + cy*ty, so the sign test could differ at
    // 24-bit precision.  The named partial sum also matches retail's matrix-product operands.
    float crossDotXY = cross.y * to.y + cross.x * to.x;
    if (crossDotXY + cross.z * to.z < 0.0f)
        angleX = -angleX;

    // The y axes.
    Vector3 fromY = *(Vector3*)&identity.m[1][0];
    Vector3 toY = *(Vector3*)&controller.m[1][0];
    Vector3 axisY = CrossProduct(&fromY, &toY);
    angle = AngleBetween(fromY, toY);
    if (axisY.x == 0.0f && axisY.y == 0.0f && axisY.z == 0.0f)
        axisY = Vector3(0.0f, 1.0f, 0.0f);
    else
        MbNormalizeOrZero(axisY);
    AxisAngleMatrix(&rotation, axisY.x, axisY.y, axisY.z, angle);
    CollisionMatrixMultiply(&aligned, &identity, &rotation);
    Vector3 crossY = CrossProduct(&GetMatrixRow(&aligned, 2), &GetMatrixRow(&controller, 2));
    float angleY = AngleBetween(GetMatrixRow(&aligned, 2), GetMatrixRow(&controller, 2)) *
                   57.2957764f;
    if (Vec3Dot(&toY, &crossY) < 0.0f)
        angleY = -angleY;

    // The z axes.
    Vector3 fromZ = GetMatrixRow(&identity, 2);
    Vector3 toZ = GetMatrixRow(&controller, 2);
    Vector3 axisZ = CrossProduct(&fromZ, &toZ);
    angle = AngleBetween(fromZ, toZ);
    if (axisZ.x == 0.0f && axisZ.y == 0.0f && axisZ.z == 0.0f)
        axisZ = MbVector3Call(0.0f, 0.0f, 1.0f);
    else
        MbNormalizeOrZero(axisZ);
    AxisAngleMatrix(&rotation, axisZ.x, axisZ.y, axisZ.z, angle);
    CollisionMatrixMultiply(&aligned, &identity, &rotation);
    Vector3 crossZ = CrossProduct(&GetMatrixRow(&aligned, 0), &GetMatrixRow(&controller, 0));
    Vec3Dot(&GetMatrixRow(&aligned, 0), &GetMatrixRow(&controller, 0));
    float angleZ = AngleBetween(GetMatrixRow(&aligned, 0), GetMatrixRow(&controller, 0)) *
                   57.2957764f;
    if (Vec3Dot(&toZ, &crossZ) < 0.0f)
        angleZ = -angleZ;

    channel->currentValue = -angleX;
    channel->currentValue1 = -angleZ;
    channel->currentValue2 = -angleY;
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

// 0x004a4c60 (slot 27, ret 0xc): blends the target deltas of every channel into the
// copied mesh.
// Not exact (185 of 1571 bytes in place: the object and channel loops' registers;
// docs/MORPHBASTARDMODIFIER.md).
void MorphBastardModifier::ModifyMesh(D3DIMSoultreeObject* object, ModifierSoultreeSurfaceView* mesh,
                                      ModifierSoultreeSurfaceView** out)
{
    if (((MorphBastardSoultreeView*)object)->drawUnchanged) {
        *out = mesh;
        return;
    }
    if (!meshCopy)
        CopyMesh(mesh);
    memcpy(meshCopy->drawnVertices, mesh->drawnVertices, meshCopy->vertexCount * sizeof(MorphBastardVertex));

    float value;
    for (int i = 0; i < objectCount; i++) {
        UpdateChannels(&morphObjects[i]);
        // The shared empty `ret 4` stub 0x00464e80, called with i (retail 0x004a4ce9);
        // link_aliases bind it to the defined copy of that stub.
        OnChannelsUpdated(i);
        // As in retail, group is not initialised: a mesh without this object's node
        // leaves it undefined (retail then reads whatever its stack slot holds).
        MorphBastardMeshGroup* group;
        for (int g = 0; g < mesh->groupCount; g++) {
            if (mesh->groups[g].node == morphObjects[i].node)
                group = &mesh->groups[g];
        }
        if (!deltaSums) {
            deltaSums = new (__FILE__, 541) Vector3[mesh->vertexCount];
            deltaMinimums = new (__FILE__, 542) Vector3[mesh->vertexCount];
            deltaMaximums = new (__FILE__, 543) Vector3[mesh->vertexCount];
        }
        int base = ((int)group->sourceVertices - (int)mesh->vertices) / 32;
        memset(deltaSums, 0, group->vertexCount * sizeof(Vector3));
        memset(deltaMinimums, 0, group->vertexCount * sizeof(Vector3));
        memset(deltaMaximums, 0, group->vertexCount * sizeof(Vector3));

        for (int j = 0; j < morphObjects[i].channelCount; j++) {
            MorphBastardChannel* channel = &morphObjects[i].channels[j];
            if (channel->axis == 0)
                value = channel->currentValue;
            else if (channel->axis == 1)
                value = channel->currentValue1;
            else if (channel->axis == 2)
                value = channel->currentValue2;

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
            meshCopy->drawnVertices[base + v].x = offset.x + mesh->drawnVertices[base + v].x;
            meshCopy->drawnVertices[base + v].y = offset.y + mesh->drawnVertices[base + v].y;
            meshCopy->drawnVertices[base + v].z = offset.z + mesh->drawnVertices[base + v].z;
        }
    }
    *out = meshCopy;
}

// 0x004a5290
void MorphBastardModifier::CopyMesh(ModifierSoultreeSurfaceView* mesh)
{
    meshCopy = new (__FILE__, 854) ModifierSoultreeSurfaceView;
    meshCopy->groupCount = mesh->groupCount;
    meshCopy->vertexCount = mesh->vertexCount;
    meshCopy->faceCount = mesh->faceCount;
    meshCopy->vertices = new (__FILE__, 858) MorphBastardVertex[meshCopy->vertexCount];
    meshCopy->drawnVertices = new (__FILE__, 859) MorphBastardVertex[meshCopy->vertexCount];
    meshCopy->normals = new (__FILE__, 860) Vector3[meshCopy->vertexCount];
    meshCopy->indices = new (__FILE__, 861) MorphBastardFace[meshCopy->faceCount];
    meshCopy->materials = mesh->materials;
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
        meshCopy->groups[i].sourceVertices = meshCopy->vertices + offset;
        meshCopy->groups[i].transformedVertices = meshCopy->drawnVertices + offset;
        meshCopy->groups[i].normals = meshCopy->normals + offset;
        offset += meshCopy->groups[i].vertexCount;
    }
}
