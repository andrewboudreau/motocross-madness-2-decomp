// CarProcedural.cpp -- reconstruction of part of
// D:\aardvark\VC\krusty2\CarProcedural.cpp. See CarProcedural.h.

#include "CarProcedural.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "MemTag.h"
#include "TextureMap.h"
#include "UnknownResourceManager.h"

// Per-TU vector constants (the shape documented in docs/LZW.md): four
// dynamic initializers (0x00430eb0..0x00430feb, .CRT$XCU 0x005660d8..
// 0x005660e4) build (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into 0x00579700,
// 0x00579710, 0x00579720 and 0x005796f0.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Inline vector helpers (tier 2: the inlined shapes in 0x004308e0; the
// constructor form of operator- and the (x*x + y*y) grouping are what
// reproduce retail's instruction order).
static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
// |v|, exact for unit vectors.
static inline float Length(Vector3 v)
{
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

// 0x0042f390
CarProcedural::CarProcedural(int flags) : GraphicsTest(flags)
{
    started = 0;
    steersFrontWheels = 1;
    field_0x58 = 0;
    numWheels = 0;
    currentPathKey = 0;
    field_0x68 = 0;
    pathKeyCount = 0;
    pathSpeed = 120.0f;
    field_0x178 = 0.0f;
    field_0x17c = 0.0f;
    field_0x180 = 0.0f;
    lookAheadTime = 0.1f;
    velocityScale = 10.0f;
    backBlend = 0.8f;
    normalBlend = 0.5f;
    wheelSpinAngle = 0.0f;
    steeringAngle = 0.0f;
    frontWheelZ = 0.0f;
    trailDistance = 0.0f;
    trailTimeOffset = 0.0f;
    groundSpeed = 0.0f;
    pathTime = 1e30f;
    pathKeys = 0;
    field_0x1b8 = kVec3Zero;
    pathPosition = kVec3Zero;
    pathOffset = kVec3Zero;
    bodyOffset = kVec3Zero;
    smoothedBack = kVec3Zero;
    field_0x1e8 = kVec3Zero;
    smoothedNormal = kVec3YAxis;
    pathVelocity = kVec3Zero;
    groundProvider = 0;
    carModel = 0;
    wheelNodes = 0;
    sensorCollider = 0;
    bodyCollider = 0;
    field_0x5c = 1;
}

// 0x0042f590 (scalar deleting wrapper 0x0042f570)
CarProcedural::~CarProcedural()
{
    if (wheelNodes)
        delete wheelNodes;
    if (pathKeys)
        delete pathKeys;
}

// 0x00430e60
void CarProcedural::HermiteWeights(float t, float* h0, float* h1, float* h2, float* h3)
{
    float t2 = t * t;
    float t3 = t2 * t;
    *h0 = 1.0f - (3.0f * t2 - t3 - t3);
    *h1 = 3.0f * t2 - t3 - t3;
    *h2 = t3 - t2 - t2 + t;
    *h3 = t3 - t2;
}

// 0x0042f600: slot 27 sets the car up from its parameters: the model and
// its wheels, the two collision objects and the path.
CarProcedural* CarProcedural::UnknownVirtualSlot27(void* a0, const char* name, const char* collisionFile,
                                                   int a3, int a4, void* a5, const char* pathFile,
                                                   const Vector3* a7, float speed, int a9, float a10,
                                                   int a11, unsigned int wheelCount, float a13,
                                                   float a14, float a15)
{
    GameObject::UnknownVirtualSlot8(a0);
    int n = strlen(name);
    int length = n > 0xff ? 0xff : n;
    strncpy(carName, name, length);
    carName[length] = 0;

    carModel = new (__FILE__, 99) CarProceduralModel(1);
    carModel->UnknownNodeVirtualSlot9(a0, name, a3, a4, 1);
    AppendChild(carModel, -1);
    pathOffset = *a7;
    trailDistance = a10;
    pathTime = 0.0f;
    smoothedBack = kVec3Zero;
    steersFrontWheels = a9;
    wheelSpinAngle = 0.0f;
    CarProceduralNode* body = carModel->FindByName("Body");
    body->GetPosition(&bodyOffset);

    numWheels = wheelCount;
    if (numWheels > 0) {
        wheelNodes = new (__FILE__, 126) CarProceduralNode*[numWheels];
        if (numWheels > 0)
            wheelNodes[0] = carModel->FindByName("TireFrontL");
        if (numWheels > 1)
            wheelNodes[1] = carModel->FindByName("TireFrontR");
        if (numWheels > 2)
            wheelNodes[2] = carModel->FindByName("TireBackL");
        if (numWheels > 3)
            wheelNodes[3] = carModel->FindByName("TireBackR");
        if (numWheels > 4)
            wheelNodes[4] = carModel->FindByName("TireMidL");
        if (numWheels > 5)
            wheelNodes[5] = carModel->FindByName("TireMidR");
        Vector3 minimum;
        Vector3 maximum;
        wheelNodes[0]->GetLocalBounds(&minimum, &maximum);
        inverseWheelRadius = 1.0f / maximum.y;
    }
    groundProvider = a5;

    sensorCollider = new (__FILE__, 149) CarProceduralCollision(1);
    sensorCollider->Configure(a0, 1, 1, 0);
    sensorCollider->field_0x68 = 1;
    AppendChild(sensorCollider, -1);
    sensorHalfHeight = 5.0f;

    int tag = g_MemTagStack->Push("Collision");
    bodyCollider = new (__FILE__, 162) CarProceduralCollision(1);
    bodyCollider->Configure(a0, 1, 1, 1);
    bodyCollider->field_0x68 = 1;
    AppendChild(bodyCollider, -1);
    if (collisionFile && *collisionFile)
        bodyCollider->LoadShape(carModel, collisionFile);
    else
        bodyCollider->SetHullShape(carModel, 0, 1, 0, 0);
    bodyCollider->field_0x54->field_0x00 = 1;
    bodyCollider->UnknownFunction435fe0();
    sensorCollider->AddIgnoredOwner(bodyCollider);
    g_MemTagStack->Pop(tag);

    ReadPath(pathFile, carModel->field_0x38);
    pathSpeed = speed;
    if (speed > 0.0)
        inversePathSpeed = 1.0f / speed;
    field_0x178 = a13;
    field_0x17c = a14;
    field_0x180 = a15;
    return this;
}

// 0x0042fa00: reads the path from `pathFile`: one key per "FRAME <n>" line,
// whose position is the translation of the "transform" line naming the
// model (in quotes). The file's y and z are swapped.
void CarProcedural::ReadPath(const char* pathFile, const char* name)
{
    char quoted[0x100];
    char line[0x400];

    sprintf(quoted, "\"%s\"", name);
    UnknownTextureStream* stream = new (__FILE__, 217) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->Open(pathFile, "r", 0)) {
        pathKeyCount = 0;
        while (!stream->AtEnd()) {
            stream->ReadLine(line, 0x400);
            if (strncmp(line, "FRAME", strlen("FRAME")) == 0)
                pathKeyCount++;
        }
        stream->Seek(stream->sliceOffset, 0, 1);
        pathKeys = new (__FILE__, 241) CarProceduralKey[pathKeyCount];
        int i = -1;
        while (!stream->AtEnd()) {
            stream->ReadLine(line, 0x400);
            if (strncmp(line, "FRAME", strlen("FRAME")) == 0) {
                i++;
                char tag[0x400];
                unsigned long frame;
                sscanf(line, "%s %ld", tag, &frame);
                pathKeys[i].time = (float)frame;
            } else if (strncmp(line, "transform", strlen("transform")) == 0) {
                char key[0x100];
                char object[0x100];
                float m[9];
                Vector3 p;
                sscanf(line, "%s %s %f %f %f %f %f %f %f %f %f %f %f %f", key, object, &m[0], &m[1],
                       &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8], &p.x, &p.z, &p.y);
                if (strcmp(object, quoted) == 0)
                    pathKeys[i].position = p;
            }
        }
    }
    delete stream;
}

// 0x004308e0: searches, by bisection on the path parameter, for the offset
// before `time` at which the path is `distance` long (at most 200 steps).
float CarProcedural::FindOffsetAtDistance(float time, float distance)
{
    unsigned int count = 0;
    float result = 0.0f;
    float travelled = 0.0f;
    float step = 2.0f;
    int forward = 0;
    if (!(distance < 0.0))
        forward = 1;
    Vector3 previous = PositionAtTime(time);
    for (; count < 200; count++) {
        if (fabs(travelled - distance) <= 0.1f)
            break;
        Vector3 position;
        if (travelled < distance) {
            if (!forward) {
                step *= 0.5f;
                forward = 1;
            }
            result += step;
            position = PositionAtTime(time - result);
            travelled += Length(position - previous);
            previous = position;
        } else {
            if (forward) {
                step *= 0.5f;
                forward = 0;
            }
            result -= step;
            position = PositionAtTime(time - result);
            travelled -= Length(position - previous);
            previous = position;
        }
    }
    return result;
}
