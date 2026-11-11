// Bring-up definitions for two MorphBastardModifier methods (MorphBastardModifier.cpp in
// retail) that the reconstructed vtable/callers reference but src/ does not define yet.
//
// Status: best-guess-unreviewed.  Bodies are taken from the near-miss candidates in
// samples/render/MorphBastardModifierNearMisses.cpp (whose notes record the remaining
// codegen differences) and checked against the retail disassembly for control flow,
// field offsets and stores.  They are not byte matches; see that sample for the matching
// work.  The out-of-line helpers retail calls from ControllerAngles (0x0040ae30 dot,
// 0x00515600 cross, 0x004a2350 row, 0x0042de90 axis-angle, 0x00436500 multiply, 0x00404e60
// Vector3 ctor) and ModifyMesh's 0x00464e80 stub are declared below and bound to their
// existing src/ providers through link_aliases in config/game_build.json, so the linked
// bodies have retail's callee set (TU-local copies got inlined: linked 0.932 -> 0.942).
#include <math.h>
#include <string.h>

#include "../reconstructed/MorphBastardModifier.h"
#include "../reconstructed/DebugAlloc.h"

float FastInvSqrt(float x);  // 0x00460c00 (src/reconstructed/FastMath.cpp)

// 0x004a4c60 (ret 0xc): blends the target deltas of every channel into the copied mesh.
void MorphBastardModifier::ModifyMesh(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out)
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
        UnknownFunction464e80(i);
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
        int base = ((int)group->field_0x08 - (int)mesh->vertices) / 32;
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
            meshCopy->drawnVertices[base + v].x = offset.x + mesh->drawnVertices[base + v].x;
            meshCopy->drawnVertices[base + v].y = offset.y + mesh->drawnVertices[base + v].y;
            meshCopy->drawnVertices[base + v].z = offset.z + mesh->drawnVertices[base + v].z;
        }
    }
    *out = meshCopy;
}

// ---------------------------------------------------------------------------
// ControllerAngles helpers: the out-of-line helpers retail calls from its second and
// third axis blocks, declared with retail's pointer signatures (as the sample does).
// (Their first uses, in the x axis block, are expanded inline in retail;
// ControllerAngles writes that block out the same way.)  The defined providers are
// VectorCalls.cpp's Vec3DotCall, CrossProduct.cpp's CrossProductCall,
// MovingPart.cpp's GetMatrixRow, BoundingBoxTreeBuild.cpp's AxisAngleMatrix (a
// TreeMatrix4, the same 64 bytes) and CollisionObject.cpp's CollisionMatrixMultiply;
// config/game_build.json's link_aliases bind the names whose types differ (same cdecl
// ABI: a reference and a pointer are both one address, and the Vector3/Vec3 views are
// both three floats returned through a hidden pointer).
float Vec3Dot(const Vector3* a, const Vector3* b);                   // 0x0040ae30: a.b
Vector3 CrossProduct(const Vector3* a, const Vector3* b);            // 0x00515600: a x b
Vector3 GetMatrixRow(const Matrix4* m, int row);                     // 0x004a2350: row of m
void AxisAngleMatrix(Matrix4* out, float x, float y, float z, float angle); // 0x0042de90
void CollisionMatrixMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b); // 0x00436500

// 0x00404e60: the out-of-line Vector3 constructor.
struct MbVector3Call : Vector3 {
    MbVector3Call(float x, float y, float z);
};

// 0x006850f0: the shared zero vector the normalisation falls back to.
static const Vector3 kMbZero = Vector3(0.0f, 0.0f, 0.0f);

// Normalise, or zero when the length is zero (inline in retail around calls to
// 0x0040ae30 and 0x00460c00).
static inline void MbNormalizeOrZero(Vector3& v)
{
    float squared = Vec3Dot(&v, &v);
    if (squared == 0.0f) {
        v = kMbZero;
    } else {
        float scale = FastInvSqrt(squared);
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }
}

// 0x004a3c80 (ret 0x44): the rotation of the channel's controller relative to its rest
// pose (`inverse`), as three angles in degrees.  For each axis the identity's axis is
// rotated onto the controller's (cross product, 0x004a3be0 angle, normalise or fall back
// to the unit axis, axis-angle matrix, 0x00436500); the angle left between the next rows
// is the rotation about it, signed by the cross product.  Stores -x, -z, -y at
// +0x60/+0x64/+0x68.  Follows retail's shape (the near miss in
// samples/render/MorphBastardModifierNearMisses.cpp): the x axis block is written out
// inline, with its own axis-angle expansion (the length summed z, y, x as retail sums
// it), and the y and z blocks call the helpers above.  The z block's dot product of the
// two x rows is in retail too; its result is unused.
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
    // controller = world * inverse, written out element by element as retail expands it.
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
    Vector3 axis = Vector3(from.y * to.z - from.z * to.y, from.z * to.x - to.z * from.x,
                           to.y * from.x - from.y * to.x);
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
    Matrix4 rotation;
    rotation.m[0][0] = t * x * x + c;
    rotation.m[0][1] = t * x * y + s * z;
    rotation.m[0][2] = t * x * z - s * y;
    rotation.m[0][3] = 0.0f;
    rotation.m[1][0] = t * x * y - s * z;
    rotation.m[1][1] = t * y * y + c;
    rotation.m[1][2] = t * y * z + s * x;
    rotation.m[1][3] = 0.0f;
    rotation.m[2][0] = t * x * z + s * y;
    rotation.m[2][1] = t * y * z - s * x;
    rotation.m[2][2] = t * z * z + c;
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
    if (cross.y * to.y + cross.x * to.x + cross.z * to.z < 0.0f)
        angleX = -angleX;

    // The y axes.
    from = *(Vector3*)&identity.m[1][0];
    to = *(Vector3*)&controller.m[1][0];
    axis = CrossProduct(&from, &to);
    angle = AngleBetween(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = Vector3(0.0f, 1.0f, 0.0f);
    else
        MbNormalizeOrZero(axis);
    AxisAngleMatrix(&rotation, axis.x, axis.y, axis.z, angle);
    CollisionMatrixMultiply(&aligned, &identity, &rotation);
    cross = CrossProduct(&GetMatrixRow(&aligned, 2), &GetMatrixRow(&controller, 2));
    float angleY = AngleBetween(GetMatrixRow(&aligned, 2), GetMatrixRow(&controller, 2)) * 57.2957764f;
    if (Vec3Dot(&to, &cross) < 0.0f)
        angleY = -angleY;

    // The z axes.
    from = GetMatrixRow(&identity, 2);
    to = GetMatrixRow(&controller, 2);
    axis = CrossProduct(&from, &to);
    angle = AngleBetween(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = MbVector3Call(0.0f, 0.0f, 1.0f);
    else
        MbNormalizeOrZero(axis);
    AxisAngleMatrix(&rotation, axis.x, axis.y, axis.z, angle);
    CollisionMatrixMultiply(&aligned, &identity, &rotation);
    cross = CrossProduct(&GetMatrixRow(&aligned, 0), &GetMatrixRow(&controller, 0));
    Vec3Dot(&GetMatrixRow(&aligned, 0), &GetMatrixRow(&controller, 0));
    float angleZ = AngleBetween(GetMatrixRow(&aligned, 0), GetMatrixRow(&controller, 0)) * 57.2957764f;
    if (Vec3Dot(&to, &cross) < 0.0f)
        angleZ = -angleZ;

    channel->currentValue = -angleX;
    channel->field_0x64 = -angleZ;
    channel->field_0x68 = -angleY;
}
