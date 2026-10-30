// Bring-up definitions for two MorphBastardModifier methods (MorphBastardModifier.cpp in
// retail) that the reconstructed vtable/callers reference but src/ does not define yet.
//
// Status: best-guess-unreviewed.  Bodies are taken from the near-miss candidates in
// samples/render/MorphBastardModifierNearMisses.cpp (whose notes record the remaining
// codegen differences) and checked against the retail disassembly for control flow,
// field offsets and stores.  They are not byte matches; see that sample for the matching
// work.  The out-of-line helpers retail calls from ControllerAngles (0x0040ae30 dot,
// 0x00515600 cross, 0x004a2350 row, 0x0042de90 axis-angle, 0x00436500 multiply) are
// written here as TU-local statics with the same arithmetic so this file adds no new
// external dependency.
#include <math.h>
#include <string.h>

#include "../reconstructed/MorphBastardModifier.h"
#include "../reconstructed/DebugAlloc.h"

float FastInvSqrt(float x);  // 0x00460c00 (src/reconstructed/FastMath.cpp)

// 0x004a4c60 (ret 0xc): blends the target deltas of every channel into the copied mesh.
void MorphBastardModifier::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
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
        // Retail calls the shared empty `ret 4` stub 0x00464e80 here with i
        // (MorphBastardModifier::UnknownFunction464e80); it has no effect and no
        // definition in src/, so the call is omitted.
        MorphBastardMeshGroup* group = 0;  // best guess: retail leaves it uninitialised if no group matches
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
// ControllerAngles helpers (TU-local copies of retail's out-of-line helpers).

// 0x0040ae30: a.b
static float MbDot(const Vector3& a, const Vector3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// 0x00515600: a x b
static Vector3 MbCross(const Vector3& a, const Vector3& b)
{
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// 0x004a2350: row `row` of m as a vector.
static Vector3 MbRow(const Matrix4& m, int row)
{
    return Vector3(m.m[row][0], m.m[row][1], m.m[row][2]);
}

// Normalise, or zero when the length is zero (0x0040ae30 + 0x00460c00 in retail).
static void MbNormalizeOrZero(Vector3& v)
{
    float squared = MbDot(v, v);
    if (squared == 0.0f) {
        v = Vector3(0.0f, 0.0f, 0.0f);
    } else {
        float scale = FastInvSqrt(squared);
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }
}

// 0x0042de90: rotation of `angle` radians about (x, y, z) (normalised here), row-vector
// convention, no translation.
static void MbAxisAngle(Matrix4* m, float x, float y, float z, float angle)
{
    m->m[0][3] = 0.0f;
    m->m[1][3] = 0.0f;
    m->m[2][3] = 0.0f;
    m->m[3][0] = 0.0f;
    m->m[3][1] = 0.0f;
    m->m[3][2] = 0.0f;
    float length = (float)sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float t = 1.0f - c;
    m->m[0][0] = t * x * x + c;
    m->m[0][1] = t * x * y + s * z;
    m->m[0][2] = t * x * z - s * y;
    m->m[1][0] = t * x * y - s * z;
    m->m[1][1] = t * y * y + c;
    m->m[1][2] = t * y * z + s * x;
    m->m[2][0] = t * x * z + s * y;
    m->m[2][1] = t * y * z - s * x;
    m->m[2][2] = t * z * z + c;
    m->m[3][3] = 1.0f;
}

// 0x00436500: out = b * a (row-vector convention): out[i][j] = sum_k b[i][k] * a[k][j].
static void MbMultiply(Matrix4* out, const Matrix4* a, const Matrix4* b)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            out->m[i][j] = b->m[i][0] * a->m[0][j] + b->m[i][1] * a->m[1][j] +
                           b->m[i][2] * a->m[2][j] + b->m[i][3] * a->m[3][j];
}

// Rotate `from` onto `to` (identity's axis onto the controller's) and return the
// rotation; `fallback` is used when the two are parallel.
static void MbAlign(Matrix4* aligned, const Matrix4& identity, const Vector3& from, const Vector3& to,
                    const Vector3& fallback)
{
    Vector3 axis = MbCross(from, to);
    float angle = AngleBetween(from, to);
    if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
        axis = fallback;
    else
        MbNormalizeOrZero(axis);
    Matrix4 rotation;
    MbAxisAngle(&rotation, axis.x, axis.y, axis.z, angle);
    MbMultiply(aligned, &identity, &rotation);
}

// 0x004a3c80 (ret 0x44): the rotation of the channel's controller relative to its rest
// pose (`inverse`), as three angles in degrees.  For each axis the identity's axis is
// rotated onto the controller's; the angle left between the next axes is the rotation
// about it, signed by the cross product.  Stores -x, -z, -y at +0x60/+0x64/+0x68.
void MorphBastardModifier::ControllerAngles(MorphBastardChannel* channel, Matrix4 inverse)
{
    Matrix4 identity;
    memset(&identity, 0, sizeof(identity));
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;
    identity.m[3][3] = 1.0f;

    Matrix4 world;
    channel->controller->GetMatrixIn(0, &world);
    Matrix4 controller;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            controller.m[r][c] = world.m[r][0] * inverse.m[0][c] + world.m[r][1] * inverse.m[1][c] +
                                 world.m[r][2] * inverse.m[2][c] + world.m[r][3] * inverse.m[3][c];

    Matrix4 aligned;

    // The x axes: angle between the aligned and controller y rows.
    Vector3 to = MbRow(controller, 0);
    MbAlign(&aligned, identity, MbRow(identity, 0), to, Vector3(1.0f, 0.0f, 0.0f));
    Vector3 cross = MbCross(MbRow(aligned, 1), MbRow(controller, 1));
    float angleX = AngleBetween(MbRow(aligned, 1), MbRow(controller, 1)) * 57.2957764f;
    if (MbDot(to, cross) < 0.0f)
        angleX = -angleX;

    // The y axes: angle between the z rows.
    to = MbRow(controller, 1);
    MbAlign(&aligned, identity, MbRow(identity, 1), to, Vector3(0.0f, 1.0f, 0.0f));
    cross = MbCross(MbRow(aligned, 2), MbRow(controller, 2));
    float angleY = AngleBetween(MbRow(aligned, 2), MbRow(controller, 2)) * 57.2957764f;
    if (MbDot(to, cross) < 0.0f)
        angleY = -angleY;

    // The z axes: angle between the x rows.
    to = MbRow(controller, 2);
    MbAlign(&aligned, identity, MbRow(identity, 2), to, Vector3(0.0f, 0.0f, 1.0f));
    cross = MbCross(MbRow(aligned, 0), MbRow(controller, 0));
    float angleZ = AngleBetween(MbRow(aligned, 0), MbRow(controller, 0)) * 57.2957764f;
    if (MbDot(to, cross) < 0.0f)
        angleZ = -angleZ;

    channel->currentValue = -angleX;
    channel->field_0x64 = -angleZ;
    channel->field_0x68 = -angleY;
}
