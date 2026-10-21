// Native boundary checks for extracted, dual-strict production methods.
// Raw storage avoids constructing unrelated scene/rendering dependencies.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shadow/D3DIMSoultreeShadow.h"
#include "../../src/reconstructed/MatrixUtil.h"

typedef char ShadowSize[(sizeof(ProjectedShadow) == 0x134) ? 1 : -1];
typedef char LinkSize[(sizeof(D3DIMSoultreeShadow) == 0x48) ? 1 : -1];
typedef char VectorSize[(sizeof(Vector3) == sizeof(ShadowVec3) && sizeof(ShadowVec3) == 12) ? 1 : -1];
typedef char VectorX[(offsetof(Vector3, x) == offsetof(ShadowVec3, x)) ? 1 : -1];
typedef char VectorY[(offsetof(Vector3, y) == offsetof(ShadowVec3, y)) ? 1 : -1];
typedef char VectorZ[(offsetof(Vector3, z) == offsetof(ShadowVec3, z)) ? 1 : -1];
typedef char CasterOffset[(offsetof(D3DIMSoultreeShadow, caster) == 0x2c) ? 1 : -1];
typedef char ShadowOffset[(offsetof(D3DIMSoultreeShadow, shadow) == 0x30) ? 1 : -1];
typedef char ReceiverOffset[(offsetof(ProjectedShadow, receivers) == 0x12c) ? 1 : -1];

static int cases, allocations, reallocations, lookCalls, vertexCalls;
static int lastLine;
static unsigned int lastSize;
static const ShadowVec3* lookPosition;
static ShadowCameraProxy* lookReceiver;
static void Require(int ok, int line) {
    if (!ok) { printf("garage shadow failure at %d, case %d\n", line, cases); exit(1); }
}
#define CHECK(x) Require((x), __LINE__)

void* DebugMalloc(unsigned int size, const char* file, int line) {
    CHECK(file && *file); allocations++; lastSize = size; lastLine = line;
    void* p = malloc(size ? size : 1); CHECK(p != 0); return p;
}
void* DebugRealloc(void* old, unsigned int size, const char* file, int line) {
    CHECK(file && *file); reallocations++; lastSize = size; lastLine = line;
    void* p = realloc(old, size ? size : 1); CHECK(p != 0); return p;
}
int ShadowCaster::GetVertexCount() { vertexCalls++; return field_0x27c; }
void ShadowCameraProxy::SetLookAt(const ShadowVec3* position, int a, int b, int c, float* fov) {
    CHECK(a == 0 && b == 0 && c == 0 && fov == 0);
    lookCalls++; lookPosition = position; lookReceiver = this;
}
// All directions below are signed unit axes. Reaching the unexercised
// normalization helper is an error, not a replacement for real normalization.
float FastInvSqrt(float) { CHECK(0); return 0; }

extern "C" GameObject* ShadowPrimary(ProjectedShadow* value) { return value; }
extern "C" GameObject* ReceiverPrimary(D3DIMSoultreeShadow* value) { return value; }

int main() {
    unsigned long shadowWords[0x134 / 4], expectedWords[0x134 / 4];
    unsigned long linkWords[0x48 / 4], linkBefore[0x48 / 4];
    unsigned long casterWords[3][sizeof(ShadowCaster) / 4];
    unsigned long cameraWords[sizeof(ShadowCameraProxy) / 4];
    ProjectedShadow* s = reinterpret_cast<ProjectedShadow*>(shadowWords);
    ProjectedShadow* expected = reinterpret_cast<ProjectedShadow*>(expectedWords);
    D3DIMSoultreeShadow* link = reinterpret_cast<D3DIMSoultreeShadow*>(linkWords);
    ShadowCaster* caster = reinterpret_cast<ShadowCaster*>(casterWords[0]);
    CHECK(ShadowPrimary(s) == reinterpret_cast<GameObject*>(s));
    CHECK(ReceiverPrimary(link) == reinterpret_cast<GameObject*>(link));
    int mode, value, seed;
    for (mode = 0; mode < 4; mode++) {
        for (value = -256; value < 512; value++) {
            memset(s, 0xa5, sizeof(*s));
            s->surfaceFormat = mode == 3 ? 0 : 0x115c;
            s->formatA = mode == 0 ? 9 : 0; s->formatB = mode == 1 ? 5 : 0;
            memcpy(expected, s, sizeof(*s));
            int alpha = ((value & 255) + 8) / 16;
            if (alpha > 15) alpha = 15;
            expected->shadowColor = 0xff404040;
            expected->shadowPixel = mode == 0 ? (short)(alpha * 273) :
                                    mode == 3 ? (short)0x8000 : (short)(alpha << 12);
            expected->colorKeyLow = mode == 0 ? 0xfff : mode == 1 ? 0xffff : mode == 2 ? 0 : 0x7fff;
            expected->colorKeyPair = mode == 0 ? 0x0fff0fff : mode == 1 ? 0xffffffff : mode == 2 ? 0 : 0x7fff7fff;
            s->SetShadowStrength(value);
            CHECK(memcmp(s, expected, sizeof(*s)) == 0);
            // Unsigned operations independently model the complete packed-word
            // result, including callers outside the usual 0..255 range.
            unsigned long grey = (unsigned long)value;
            expected->shadowColor = 0xff000000UL | (grey << 16) | (grey << 8) | grey;
            s->SetShadowGrey(value);
            CHECK(memcmp(s, expected, sizeof(*s)) == 0); cases++;
        }
    }
    for (seed = 0; seed < 64; seed++) {
        for (mode = 0; mode < 4; mode++) {
            memset(s, seed, sizeof(*s)); memset(link, 0x53 + seed, sizeof(*link));
            s->receivers = 0; s->receiverCount = 0; s->receiverCapacity = 0;
            memcpy(linkBefore, link, sizeof(*link)); memcpy(expected, s, sizeof(*s));
            allocations = reallocations = 0;
            // Null shadow must leave both receiver and owner storage untouched.
            CHECK(link->Attach(0x12340000 + seed, caster, 0) == 0);
            CHECK(memcmp(link, linkBefore, sizeof(*link)) == 0);
            CHECK(memcmp(s, expected, sizeof(*s)) == 0 && allocations == 0);
            int count = mode + 1;
            for (int n = 0; n < count; n++) {
                ShadowCaster* selected = n & 1 ? 0 : caster;
                memcpy(expected, s, sizeof(*s)); memcpy(linkBefore, link, sizeof(*link));
                CHECK(link->Attach(0x12340000 + seed, selected, s) == link);
                CHECK(linkWords[0x18 / 4] == (unsigned long)(0x12340000 + seed));
                CHECK(link->caster == selected && link->shadow == s);
                CHECK(s->receiverCount == n + 1 && s->receiverCapacity == n + 1);
                CHECK(lastSize == (unsigned int)((n + 1) * 4));
                CHECK(lastLine == (n ? 0x119 : 0x115));
                for (int j = 0; j <= n; j++) CHECK(s->receivers[j] == link);
                expected->receivers = s->receivers;
                expected->receiverCount = n + 1; expected->receiverCapacity = n + 1;
                linkBefore[0x18 / 4] = 0x12340000 + seed;
                linkBefore[0x2c / 4] = reinterpret_cast<unsigned long>(selected);
                linkBefore[0x30 / 4] = reinterpret_cast<unsigned long>(s);
                CHECK(memcmp(s, expected, sizeof(*s)) == 0);
                CHECK(memcmp(link, linkBefore, sizeof(*link)) == 0);
            }
            CHECK(allocations == 1 && reallocations == count - 1);
            // Existing spare capacity must not allocate.
            s->receiverCount = 0;
            memcpy(expected, s, sizeof(*s)); memcpy(linkBefore, link, sizeof(*link));
            CHECK(link->Attach(0, caster, s) == link);
            CHECK(s->receiverCapacity == count && s->receiverCount == 1);
            CHECK(allocations == 1 && reallocations == count - 1);
            expected->receiverCount = 1;
            linkBefore[0x18 / 4] = 0;
            linkBefore[0x2c / 4] = reinterpret_cast<unsigned long>(caster);
            CHECK(memcmp(s, expected, sizeof(*s)) == 0);
            CHECK(memcmp(link, linkBefore, sizeof(*link)) == 0);
            free(s->receivers); cases++;
        }
        memset(s, 0, sizeof(*s)); allocations = reallocations = vertexCalls = 0;
        int total = 0;
        for (int k = 0; k < 3; k++) {
            ShadowCaster* c = reinterpret_cast<ShadowCaster*>(casterWords[k]);
            c->field_0x27c = seed % 7 + k; total += c->field_0x27c;
            memcpy(expected, s, sizeof(*s));
            s->AddCaster(c);
            CHECK(s->casterCount == k + 1 && s->casterCapacity == k + 1);
            CHECK(s->casters[k] == c && s->vertexCapacity == total && s->vertexBuffer != 0);
            expected->casters = s->casters; expected->casterCount = k + 1; expected->casterCapacity = k + 1;
            expected->vertexCapacity = total; expected->vertexBuffer = s->vertexBuffer;
            CHECK(memcmp(s, expected, sizeof(*s)) == 0);
        }
        CHECK(vertexCalls == 6);
        void* old = s->vertexBuffer; int calls = reallocations;
        s->ResizeVertexBuffer(); CHECK(s->vertexBuffer == old && reallocations == calls);
        free(s->casters); free(s->vertexBuffer); cases++;
        for (mode = -1; mode <= 4; mode++) {
            memset(s, seed, sizeof(*s)); s->camera = reinterpret_cast<ShadowCameraProxy*>(cameraWords);
            memcpy(expected, s, sizeof(*s)); lookCalls = 0;
            ShadowVec3 position = { (float)seed, -2.0f, 5.0f };
            ShadowVec3 direction = { 0.0f, 0.0f, 0.0f };
            (&direction.x)[seed % 3] = seed & 1 ? -1.0f : 1.0f;
            int result = s->SetLight(&position, &direction, mode);
            CHECK(result == (mode == 1 || mode == 3));
            if (mode == 1 || mode == 3) { expected->lightDir = direction; expected->mode = mode; }
            if (mode == 1) {
                expected->lightVec2 = position;
                CHECK(lookCalls == 1 && lookPosition == &position && lookReceiver == s->camera);
            } else CHECK(lookCalls == 0);
            CHECK(memcmp(s, expected, sizeof(*s)) == 0); cases++;
        }
    }
    printf("Garage shadow: %d receiver, caster, color and light boundary cases passed\n", cases);
    return 0;
}
