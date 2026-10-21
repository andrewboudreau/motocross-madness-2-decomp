#include "SurfaceMap.h"
#include "Game.h"
#include "PCRenderTarget.h"
#include "TrackGameGlobal.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Private ABI fixtures for external texture/render/device virtual calls.
// SurfaceMap and BaseObject themselves are compiled from production source.
TrackGame* g_TrackGame;
static int events[32], count, destroyed;
static int materialResult;
static SurfaceMaterial observed;
static TrackGame* switchGame;
static void Record(int event) { events[count++] = event; }

class TestTexture : public BaseObject {
public:
    explicit TestTexture(int value) : id(value) {}
    virtual ~TestTexture() { ++destroyed; }
    virtual int AddRef() { Record(10 + id); return BaseObject::AddRef(); }
    virtual int Release() { Record(20 + id); return BaseObject::Release(); }
#define UNUSED_TEXTURE_SLOT(n) virtual void Slot##n() { Record(-n); }
    UNUSED_TEXTURE_SLOT(4) UNUSED_TEXTURE_SLOT(5) UNUSED_TEXTURE_SLOT(6)
    UNUSED_TEXTURE_SLOT(7) UNUSED_TEXTURE_SLOT(8) UNUSED_TEXTURE_SLOT(9)
    UNUSED_TEXTURE_SLOT(10) UNUSED_TEXTURE_SLOT(11) UNUSED_TEXTURE_SLOT(12)
    UNUSED_TEXTURE_SLOT(13) UNUSED_TEXTURE_SLOT(14) UNUSED_TEXTURE_SLOT(15)
    UNUSED_TEXTURE_SLOT(16) UNUSED_TEXTURE_SLOT(17) UNUSED_TEXTURE_SLOT(18)
#undef UNUSED_TEXTURE_SLOT
    virtual void Bind() { Record(30 + id); }
    int id;
};

class TestDevice {
public:
#define UNUSED_DEVICE_SLOT(n) virtual long __stdcall Slot##n() { Record(-n); return -1; }
    UNUSED_DEVICE_SLOT(0) UNUSED_DEVICE_SLOT(1) UNUSED_DEVICE_SLOT(2)
    UNUSED_DEVICE_SLOT(3) UNUSED_DEVICE_SLOT(4) UNUSED_DEVICE_SLOT(5)
    UNUSED_DEVICE_SLOT(6) UNUSED_DEVICE_SLOT(7) UNUSED_DEVICE_SLOT(8)
    UNUSED_DEVICE_SLOT(9) UNUSED_DEVICE_SLOT(10) UNUSED_DEVICE_SLOT(11)
    UNUSED_DEVICE_SLOT(12) UNUSED_DEVICE_SLOT(13) UNUSED_DEVICE_SLOT(14)
    UNUSED_DEVICE_SLOT(15)
#undef UNUSED_DEVICE_SLOT
    virtual long __stdcall SetMaterial(const SurfaceMaterial* material) {
        Record(40);
        observed = *material;
        return materialResult;
    }
};

class TestTarget {
public:
#define UNUSED_TARGET_SLOT(n) virtual void Slot##n() { Record(-n); }
    UNUSED_TARGET_SLOT(0) UNUSED_TARGET_SLOT(1) UNUSED_TARGET_SLOT(2)
    UNUSED_TARGET_SLOT(3) UNUSED_TARGET_SLOT(4) UNUSED_TARGET_SLOT(5)
    UNUSED_TARGET_SLOT(6)
#undef UNUSED_TARGET_SLOT
    virtual long SetTextureStageState(int stage, int type, int value) {
        if (stage != 0 || value != 1) Record(-100);
        Record(id * 100 + type);
        if (switchGame) { g_TrackGame = switchGame; switchGame = 0; }
        return 0;
    }
    unsigned char field_0x04[0x50 - 4];
    TestDevice* device;
    int id;
};

static int Sequence(int a, int b = 0, int c = 0, int length = 1) {
    return count == length && events[0] == a &&
        (length < 2 || events[1] == b) && (length < 3 || events[2] == c);
}

void* operator new(unsigned int, void* storage) { return storage; }

int main() {
    if (sizeof(SurfaceMap) != 0x70 || sizeof(SurfaceMaterial) != 0x44 ||
        offsetof(SurfaceMap, texture) != 8 || offsetof(SurfaceMap, material) != 12 ||
        offsetof(TestTarget, device) != offsetof(PCRenderTarget, device)) return 1;
    TestDevice device;
    TestTarget first, second;
    first.device = second.device = &device;
    first.id = 1; second.id = 2;
    union GameStorage { double alignment; unsigned char bytes[sizeof(Game)]; } game1, game2;
    memset(&game1, 0, sizeof(game1)); memset(&game2, 0, sizeof(game2));
    reinterpret_cast<Game*>(game1.bytes)->renderTarget = reinterpret_cast<RenderTarget*>(&first);
    reinterpret_cast<Game*>(game2.bytes)->renderTarget = reinterpret_cast<RenderTarget*>(&second);
    g_TrackGame = reinterpret_cast<TrackGame*>(game1.bytes);
    TestTexture* tex1 = new TestTexture(1);
    TestTexture* tex2 = new TestTexture(2);
    union { double alignment; unsigned char bytes[sizeof(SurfaceMap)]; } storage;
    memset(storage.bytes, 0xa5, sizeof(storage.bytes));
    SurfaceMap* map = new(storage.bytes) SurfaceMap(reinterpret_cast<TextureMap*>(tex1), 0);
    if (!Sequence(11, 40, 0, 2) || tex1->GetRefCount() != 2 || map->GetRefCount() != 1) return 2;
    for (int i = 0; i < 17; ++i)
        if (reinterpret_cast<float*>(&observed)[i] != (i < 8 ? 1.0f : 0.0f)) return 3;
    for (i = 0x50; i < 0x70; ++i) if (storage.bytes[i] != 0xa5) return 4;
    count = 0; map->SetTexture(reinterpret_cast<TextureMap*>(tex1));
    if (!Sequence(21, 11, 40, 3) || tex1->GetRefCount() != 2) return 5;
    count = 0; map->SetTexture(reinterpret_cast<TextureMap*>(tex2));
    if (!Sequence(21, 12, 40, 3) || tex1->GetRefCount() != 1 || tex2->GetRefCount() != 2) return 6;
    count = 0; map->Select();
    if (!Sequence(40, 32, 0, 2)) return 7;
    count = 0; map->SetTexture(0);
    if (!Sequence(22, 40, 0, 2) || tex2->GetRefCount() != 1) return 8;
    count = 0; switchGame = reinterpret_cast<TrackGame*>(game2.bytes); map->Select();
    if (!Sequence(40, 101, 204, 3)) return 9;
    materialResult = 1; count = 0;
    if (map->Apply() != 0 || !Sequence(40)) return 10;
    materialResult = -1; count = 0;
    if (map->Apply() != 0 || !Sequence(40)) return 11;
    materialResult = 0; count = 0;
    if (map->Apply() != 1 || !Sequence(40)) return 12;
    count = 0; map->~SurfaceMap();
    if (count) return 13;
    SurfaceMaterial custom;
    for (i = 0; i < 17; ++i) reinterpret_cast<float*>(&custom)[i] = (float)i - 8.5f;
    count = 0; map = new(storage.bytes) SurfaceMap(reinterpret_cast<TextureMap*>(tex1), &custom);
    if (!Sequence(11, 40, 0, 2) || memcmp(&observed, &custom, sizeof(custom))) return 14;
    count = 0; map->~SurfaceMap();
    if (!Sequence(21) || tex1->GetRefCount() != 1) return 15;
    tex1->Release(); tex2->Release();
    if (destroyed != 2) return 16;
    puts("SurfaceMap: real BaseObject ownership, material data, virtual ABI and shared-pointer reload pass");
    return 0;
}
