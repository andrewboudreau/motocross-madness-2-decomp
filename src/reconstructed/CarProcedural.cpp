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
    return UnknownFunction460b50(squared);
}

// 0x0042f390
CarProcedural::CarProcedural(int flags) : GraphicsTest(flags)
{
    field_0x54 = 0;
    field_0x50 = 1;
    field_0x58 = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x68 = 0;
    field_0x6c = 0;
    field_0x170 = 120.0f;
    field_0x178 = 0.0f;
    field_0x17c = 0.0f;
    field_0x180 = 0.0f;
    field_0x1a4 = 0.1f;
    field_0x1a8 = 10.0f;
    field_0x1ac = 0.8f;
    field_0x1b0 = 0.5f;
    field_0x18c = 0.0f;
    field_0x190 = 0.0f;
    field_0x198 = 0.0f;
    field_0x19c = 0.0f;
    field_0x1a0 = 0.0f;
    field_0x4c = 0.0f;
    field_0x194 = 1e30f;
    field_0x1b4 = 0;
    field_0x1b8 = kVec3Zero;
    field_0x1c4 = kVec3Zero;
    field_0x1d0 = kVec3Zero;
    field_0x1dc = kVec3Zero;
    field_0x1f4 = kVec3Zero;
    field_0x1e8 = kVec3Zero;
    field_0x200 = kVec3YAxis;
    field_0x40 = kVec3Zero;
    field_0x20c = 0;
    field_0x34 = 0;
    field_0x210 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x5c = 1;
}

// 0x0042f590 (scalar deleting wrapper 0x0042f570)
CarProcedural::~CarProcedural()
{
    if (field_0x210)
        delete field_0x210;
    if (field_0x1b4)
        delete field_0x1b4;
}

// 0x00430e60
void CarProcedural::UnknownFunction430e60(float t, float* h0, float* h1, float* h2, float* h3)
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
    strncpy(field_0x70, name, length);
    field_0x70[length] = 0;

    field_0x34 = new (__FILE__, 99) CarProceduralModel(1);
    field_0x34->UnknownNodeVirtualSlot9(a0, name, a3, a4, 1);
    AppendChild(field_0x34, -1);
    field_0x1d0 = *a7;
    field_0x19c = a10;
    field_0x194 = 0.0f;
    field_0x1f4 = kVec3Zero;
    field_0x50 = a9;
    field_0x18c = 0.0f;
    CarProceduralNode* body = field_0x34->UnknownFunction4fdae0("Body");
    body->UnknownFunction4fc970(&field_0x1dc);

    field_0x60 = wheelCount;
    if (field_0x60 > 0) {
        field_0x210 = new (__FILE__, 126) CarProceduralNode*[field_0x60];
        if (field_0x60 > 0)
            field_0x210[0] = field_0x34->UnknownFunction4fdae0("TireFrontL");
        if (field_0x60 > 1)
            field_0x210[1] = field_0x34->UnknownFunction4fdae0("TireFrontR");
        if (field_0x60 > 2)
            field_0x210[2] = field_0x34->UnknownFunction4fdae0("TireBackL");
        if (field_0x60 > 3)
            field_0x210[3] = field_0x34->UnknownFunction4fdae0("TireBackR");
        if (field_0x60 > 4)
            field_0x210[4] = field_0x34->UnknownFunction4fdae0("TireMidL");
        if (field_0x60 > 5)
            field_0x210[5] = field_0x34->UnknownFunction4fdae0("TireMidR");
        Vector3 minimum;
        Vector3 maximum;
        field_0x210[0]->UnknownFunction4fe0a0(&minimum, &maximum);
        field_0x184 = 1.0f / maximum.y;
    }
    field_0x20c = a5;

    field_0x38 = new (__FILE__, 149) CarProceduralCollision(1);
    field_0x38->UnknownFunction4320f0(a0, 1, 1, 0);
    field_0x38->field_0x68 = 1;
    AppendChild(field_0x38, -1);
    field_0x188 = 5.0f;

    int tag = g_MemTagStack->Push("Collision");
    field_0x3c = new (__FILE__, 162) CarProceduralCollision(1);
    field_0x3c->UnknownFunction4320f0(a0, 1, 1, 1);
    field_0x3c->field_0x68 = 1;
    AppendChild(field_0x3c, -1);
    if (collisionFile && *collisionFile)
        field_0x3c->UnknownFunction432800(field_0x34, collisionFile);
    else
        field_0x3c->UnknownFunction432720(field_0x34, 0, 1, 0, 0);
    field_0x3c->field_0x54->field_0x00 = 1;
    field_0x3c->UnknownFunction435fe0();
    field_0x38->UnknownFunction439410(field_0x3c);
    g_MemTagStack->Pop(tag);

    UnknownFunction42fa00(pathFile, field_0x34->field_0x38);
    field_0x170 = speed;
    if (speed > 0.0)
        field_0x174 = 1.0f / speed;
    field_0x178 = a13;
    field_0x17c = a14;
    field_0x180 = a15;
    return this;
}

// 0x0042fa00: reads the path from `pathFile`: one key per "FRAME <n>" line,
// whose position is the translation of the "transform" line naming the
// model (in quotes). The file's y and z are swapped.
void CarProcedural::UnknownFunction42fa00(const char* pathFile, const char* name)
{
    char quoted[0x100];
    char line[0x400];

    sprintf(quoted, "\"%s\"", name);
    UnknownTextureStream* stream = new (__FILE__, 217) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->UnknownFunction460f50(pathFile, "r", 0)) {
        field_0x6c = 0;
        while (!stream->UnknownFunction430ff0()) {
            stream->UnknownFunction461aa0(line, 0x400);
            if (strncmp(line, "FRAME", strlen("FRAME")) == 0)
                field_0x6c++;
        }
        stream->UnknownFunction461340(stream->field_0x130, 0, 1);
        field_0x1b4 = new (__FILE__, 241) CarProceduralKey[field_0x6c];
        int i = -1;
        while (!stream->UnknownFunction430ff0()) {
            stream->UnknownFunction461aa0(line, 0x400);
            if (strncmp(line, "FRAME", strlen("FRAME")) == 0) {
                i++;
                char tag[0x400];
                unsigned long frame;
                sscanf(line, "%s %ld", tag, &frame);
                field_0x1b4[i].time = (float)frame;
            } else if (strncmp(line, "transform", strlen("transform")) == 0) {
                char key[0x100];
                char object[0x100];
                float m[9];
                Vector3 p;
                sscanf(line, "%s %s %f %f %f %f %f %f %f %f %f %f %f %f", key, object, &m[0], &m[1],
                       &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8], &p.x, &p.z, &p.y);
                if (strcmp(object, quoted) == 0)
                    field_0x1b4[i].position = p;
            }
        }
    }
    delete stream;
}

// 0x004308e0: searches, by bisection on the path parameter, for the offset
// before `time` at which the path is `distance` long (at most 200 steps).
float CarProcedural::UnknownFunction4308e0(float time, float distance)
{
    unsigned int count = 0;
    float result = 0.0f;
    float travelled = 0.0f;
    float step = 2.0f;
    int forward = 0;
    if (!(distance < 0.0))
        forward = 1;
    Vector3 previous = UnknownFunction430b10(time);
    for (; count < 200; count++) {
        if (fabs(travelled - distance) <= 0.1)
            break;
        Vector3 position;
        if (travelled < distance) {
            if (!forward) {
                step *= 0.5f;
                forward = 1;
            }
            result += step;
            position = UnknownFunction430b10(time - result);
            travelled += Length(position - previous);
            previous = position;
        } else {
            if (forward) {
                step *= 0.5f;
                forward = 0;
            }
            result -= step;
            position = UnknownFunction430b10(time - result);
            travelled -= Length(position - previous);
            previous = position;
        }
    }
    return result;
}
