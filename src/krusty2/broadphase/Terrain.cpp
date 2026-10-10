// Terrain.cpp -- reconstruction of D:\aardvark\VC\krusty2\Terrain.cpp.
#include "Terrain.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

// d3dtypes.h colour macros (Terrain::Load builds two D3DCOLORs).
#define RGBA_MAKE(r, g, b, a) ((unsigned int)(((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define RGBA_GETRED(rgb) (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb) (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb) ((rgb) & 0xff)

Terrain::Terrain(int a)
    : GameObject(a)
{
    gridCellSize = 1.0f;
    stream = 0;
    field_0x84 = 0;
    field_0x88 = 0;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0x34 = 0;
    field_0xb4 = 0;
    ownedObjectCount = 0;
    field_0x53c = 0;
    heightField = 0;
    drawDistance = 1000;
    drawDistanceDirty = 0;
    field_0x30 = 0;
    field_0xbf8 = 0;
    field_0x8c = 0;
    field_0xbfc = 0;
    field_0xc00 = 0;
    field_0xc04 = 0;
    field_0xc08 = 0;
    field_0xc0c = 0;
    appliedDrawDistance = 0;
    field_0x70 = 0;
    field_0x78 = 0;
    field_0x74 = 0;
    field_0x7c = 0;
    field_0x80 = 0;
    blockCount = 0;
    blocks = 0;
    invGridCellSize = 1.0f;
    field_0xc34 = 0;
    field_0xc38 = 0;
    field_0xc3c = 0;
    TerrainMatrix tmp;
    transform = *GetIdentityMatrix(&tmp);
    ownedObjectArray = 0;
    ownedObjectArrayCount = 0;
    field_0xcc0 = 0;
    field_0xcac = 0;
    lowestQualityOverride = 0;
    field_0xcb4 = 0;
}

Terrain::~Terrain()
{
    int scope = g_MemTagStack->Push("Terrain");
    if (ownedObjectArray) {
        for (int i = 0; i < ownedObjectArrayCount; i++)
            ownedObjectArray[i]->BaseObjectVirtualSlot2();
        DebugFree(ownedObjectArray, __FILE__, 0x4b3);
    }
    if (field_0xc3c)
        field_0xc3c->BaseObjectVirtualSlot2();
    if (field_0x30)
        field_0x30->BaseObjectVirtualSlot2();
    if (ownedObjectCount) {
        for (int i = 0; i < ownedObjectCount; i++) {
            if (ownedObjects[i])
                ownedObjects[i]->BaseObjectVirtualSlot2();
        }
    }
    if (heightField) {
        heightField->Shutdown(1);
        if (heightField)
            delete heightField;
    }
    if (field_0x34) {
        field_0x34->Release();
        field_0x34 = 0;
    }
    if (blocks) {
        for (int i = 0; i < blockCount; i++) {
            if (blocks[i])
                delete blocks[i];
        }
        DebugFree(blocks, __FILE__, 0x4d0);
    }
    if (field_0xc84)
        delete field_0xc84;
    if (field_0xc88)
        delete field_0xc88;
    g_MemTagStack->Pop(scope);
}

// 0x005057d0.  Moves `object` out of the live prefix of field_0xcb8[] (swap with the last live
// element, shrink the live count), keeping the pointer just behind the new end.
void Terrain::RetireOwnedObject(BaseObject* object)
{
    if (ownedObjectArrayCount) {
        for (int i = 0; i < ownedObjectArrayCount; i++) {
            if (ownedObjectArray[i] == object) {
                ownedObjectArrayCount--;
                ownedObjectArray[i] = ownedObjectArray[ownedObjectArrayCount];
                ownedObjectArray[ownedObjectArrayCount] = object;
                return;
            }
        }
    }
}

// Boundary declarations for Terrain::Load (tier 1 addresses, tier 3 names).
class TerrainDebugOverlay;
struct TerrainGameDisplay {                                     // Display (src/reconstructed/Display.h)
    char pad_0x000[0x4bc];
    char driverName[1];                                         // +0x4bc the DriverInfo registry key component
};
struct TerrainGameSettings {                                    // TrackGame (src/reconstructed/Game.h)
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18(); virtual void Slot19();
    virtual int GetRegistryInt(const char* name, int defaultValue);   // slot 20 (+0x50)
    virtual void Slot21(); virtual void Slot22(); virtual void Slot23(); virtual void Slot24();
    virtual int SetRegistryInt(const char* name, int value);          // slot 25 (+0x64)
    char pad_0x004[0x0c - 0x04];
    TerrainGameDisplay* display;                                // +0x00c
    char pad_0x010[0x38 - 0x10];
    TerrainDebugOverlay* overlay;                               // +0x038 profile page printer
    char pad_0x03c[0x2d0 - 0x3c];
    int softwareRendering;                                      // +0x2d0
    unsigned char debugFlags;                                   // +0x2d4 bit 2: print the terrain profile page
    char pad_0x2d5[0x54c - 0x2d5];
    int magFilter;                                              // +0x54c texture stage state 0x10 value
    int minFilter;                                              // +0x550 state 0x11
    int mipFilter;                                              // +0x554 state 0x12
};
extern TerrainGameSettings* g_terrainGame;                      // 0x0056e26c
extern TerrainQualityEntry g_terrainQualityHardware[10];        // 0x00574920 (Terrain.cpp .data)
extern TerrainQualityEntry g_terrainQualitySoftware[10];        // 0x005749c0
extern int g_terrainTextureSize;                                // 0x0067a684 width of the last surface record
extern int g_terrainTextureShift;                               // 0x0056c200 bit length of that width
// 0x004a2fc0: calloc(count, size, __FILE__, __LINE__) (BikeRace.h's DebugCalloc).
void* DebugCalloc(unsigned int count, unsigned int size, const char* file, int line);
// 0x0050a590 (TextureMap.h): shared texture `name` through the manager.
TerrainSurfaceBase* TerrainLoadTexture(TextureMapManager* manager, const char* name, int format,
                                       void* palette, int flags, int addressU, int addressV,
                                       void* choice, int alphaThreshold, unsigned int key,
                                       int addRef, int fromArchive);
// ColorMapper (0x18b10 bytes; ctor 0x004ddf40, 0x004de270 returns its 256 RGB triplets).
class TerrainColorMapper : public BaseObject {
public:
    explicit TerrainColorMapper(TerrainStream* stream);
    unsigned char* GetColors();
    char pad_0x08[0x18b10 - 0x08];
};
// GridBaseBlock (src/reconstructed/Gridbase.h; 0xd50 bytes, ctor 0x0047dc20).
class TerrainGridBlock {
public:
    explicit TerrainGridBlock(TerrainStream* stream);
    char pad_0x000[0xd50];
};

// In-place scale (an inlined Vec3 operator*=: the factor stays on the x87 stack).
static inline void ScaleTerrainVec3(TerrainVec3& v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
}

// 0x005059d0.
Terrain* Terrain::Load(int host, TerrainStream* stream, int, TerrainSurfaceDesc* desc,
                       int quality, int field0xc40, const char* textureName, int halfFormat)
{
    GameObject::GameObjectVirtualSlot8(host);
    field_0xc18 = *desc;
    int surfaceFlags = halfFormat ? 8 : 0;
    g_pTerrainQualityTable = g_terrainQualityHardware;
    if (!g_terrainGame->softwareRendering)
        g_pTerrainQualityTable = g_terrainQualitySoftware;
    SelectQuality(quality);
    if (field0xc40)
        field_0xc34 = 1;
    if (!stream) {
        BaseObjectVirtualSlot2();
        return 0;
    }
    this->stream = stream;
    if (stream->inner)
        stream->Seek(stream->innerStart, 0, 0);
    field_0xc38 = this->stream->Tell();
    field_0x38 = field_0xc18.registry;
    field_0xc40 = field0xc40;
    field_0xc84 = new(__FILE__, 0x11c) TerrainOwned;
    field_0xc88 = new(__FILE__, 0x122) TerrainOwned;
    ownedObjectArray = (BaseObject**)DebugCalloc(200, 4, __FILE__, 0x127);

    char tag[4];
    int version;
    this->stream->Read(tag, 4, 1);
    this->stream->Read(&version, 4, 1);
    if (strcmp(tag, "TRN") != 0 || version != 5)
        goto fail;
    field_0xc94 = 5;
    field_0xc98 = 6;
    if (!textureName)
        textureName = "testnoise.tga";
    field_0xc3c = TerrainLoadTexture(field_0xc18.manager, textureName, 0x115c, 0, 2, 5, 6, 0,
                                     0x80, 0xff00ff, 1, 1);
    if (field_0xc3c) {
        field_0xc3c->CreateTextureSurface(1, 0, 1);
        colorKey = field_0xc3c->field_0x3c;
    } else {
        colorKey = 0xffffffff;
    }

    {
        unsigned int key = colorKey;
        unsigned int alpha = 0xff - (key >> 24);
        unsigned int keep = 0xff - alpha;
        unsigned int red = RGBA_GETRED(key) * keep / 0xff;
        field_0xc9c = RGBA_MAKE(alpha, alpha, alpha, 0xff);
        field_0xca0 = RGBA_MAKE(red, RGBA_GETGREEN(key) * keep / 0xff, RGBA_GETBLUE(key) * keep / 0xff, 0xff);
    }
    qualityParamA = 0x100;

    int hasPalette;
    this->stream->Read(&hasPalette, 4, 1);
    if (hasPalette) {
        TerrainColorMapper* mapper = new(__FILE__, 0x167) TerrainColorMapper(this->stream);
        field_0x30 = mapper;
        unsigned char* colors = mapper->GetColors();
        TerrainPaletteEntry entries[256];
        for (int i = 0; i < 256; i++) {
            entries[i].red = colors[i * 3];
            entries[i].green = colors[i * 3 + 1];
            entries[i].blue = colors[i * 3 + 2];
            entries[i].flags = 0;
        }
        TerrainDirectDraw* directDraw = ((TerrainHost*)field_0x18)->display->directDraw;
        if (directDraw->CreatePalette(0x44, entries, &field_0x34, 0) != 0)
            goto fail;
    } else {
        field_0x34 = 0;
    }

    int gridSize;
    int unusedHeader;
    int blockRecords;
    int surfaceRecords;
    this->stream->Read(&field_0xbe8, 4, 1);
    this->stream->Read(&gridCellSize, 4, 1);
    this->stream->Read(&gridSize, 4, 1);
    this->stream->Read(&unusedHeader, 4, 1);
    this->stream->Read(&blockCount, 4, 1);
    this->stream->Read(&blockRecords, 4, 1);
    blocks = (void**)DebugMalloc(blockCount * 4, __FILE__, 0x197);
    this->stream->Seek(blockCount * 4, 1, 1);
    this->stream->Read(&ownedObjectCount, 4, 1);
    this->stream->Read(&surfaceRecords, 4, 1);
    this->stream->Seek(ownedObjectCount * 4, 1, 1);
    int i;
    for (i = 0; i < blockRecords; i++)
        blocks[i] = new(__FILE__, 0x1ab) TerrainGridBlock(this->stream);

    if (surfaceRecords) {
        int width;
        for (i = 0; i < surfaceRecords; i++) {
            int height;
            int format;
            this->stream->Read(&width, 4, 1);
            this->stream->Read(&height, 4, 1);
            this->stream->Read(&format, 4, 1);
            TerrainSurfaceBase* surface;
            if (field_0x38 && width == 0x100) {
                TerrainSurface* managed = new(__FILE__, 0x1bb) TerrainSurface(field_0xc18.manager);
                surface = managed;
                surface->LoadSurface(this->stream, width, height, 1, format, 0, 0, field_0x30,
                                     surfaceFlags | 2, field_0x34, 2, 1, &field_0xc18, 0x80, 0xff00ff);
                field_0x38->Register(managed);
            } else {
                surface = new(__FILE__, 0x1c8) TerrainSurfaceBase(field_0xc18.manager, 1);
                surface->LoadSurface(this->stream, width, height, 1, format, 0, 0, field_0x30,
                                     surfaceFlags | 2, field_0x34, 2, 1, &field_0xc18, 0x80, 0xff00ff);
                if (statusFlags & 1)
                    surface->CreateTextureSurface(1, 0, 0);
            }
            ownedObjects[i] = surface;
        }
        g_terrainTextureSize = width;
        g_terrainTextureShift = 0;
        for (int w = width; w > 0; w >>= 1)
            g_terrainTextureShift++;
    }

    boundsMin = TerrainVec3(FLT_MAX, FLT_MAX, FLT_MAX);
    boundsMax = TerrainVec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    heightField = new(__FILE__, 0x1e6) TerrainShutdownObject(this->stream, host, 0, this, gridSize,
                                                             gridSize * 4, 0, 0, 0, &boundsMin,
                                                             &boundsMax);
    boundsMax.x += 16.0f;
    boundsMax.z += 16.0f;
    ScaleTerrainVec3(boundsMax, gridCellSize);
    ScaleTerrainVec3(boundsMin, gridCellSize);
    if (heightField) {
        int levels = heightField->field_0x28;
        int shift = heightField->gridShift;
        if (levels > 0)
            shift -= levels * 4;
        invGridCellSize = 1.0f / ((float)(1 << shift) * gridCellSize);
    }
    return this;

fail:
    heightField = 0;
    BaseObjectVirtualSlot2();
    return 0;
}

// Boundary declarations for slot 14 (tier 1 addresses and offsets, tier 3 names).
// The IDirect3DDevice7-shaped device at the render target's +0x50 (slot 35 SetTexture,
// slot 38 ValidateDevice).
class TerrainDevice {
public:
    virtual long __stdcall Slot0(); virtual long __stdcall Slot1(); virtual long __stdcall Slot2();
    virtual long __stdcall Slot3(); virtual long __stdcall Slot4(); virtual long __stdcall Slot5();
    virtual long __stdcall Slot6(); virtual long __stdcall Slot7(); virtual long __stdcall Slot8();
    virtual long __stdcall Slot9(); virtual long __stdcall Slot10(); virtual long __stdcall Slot11();
    virtual long __stdcall Slot12(); virtual long __stdcall Slot13(); virtual long __stdcall Slot14();
    virtual long __stdcall Slot15(); virtual long __stdcall Slot16(); virtual long __stdcall Slot17();
    virtual long __stdcall Slot18(); virtual long __stdcall Slot19(); virtual long __stdcall Slot20();
    virtual long __stdcall Slot21(); virtual long __stdcall Slot22(); virtual long __stdcall Slot23();
    virtual long __stdcall Slot24(); virtual long __stdcall Slot25(); virtual long __stdcall Slot26();
    virtual long __stdcall Slot27(); virtual long __stdcall Slot28(); virtual long __stdcall Slot29();
    virtual long __stdcall Slot30(); virtual long __stdcall Slot31(); virtual long __stdcall Slot32();
    virtual long __stdcall Slot33(); virtual long __stdcall Slot34();
    virtual long __stdcall SetTexture(unsigned long stage, void* surface);           // slot 35 (+0x8c)
    virtual long __stdcall Slot36(); virtual long __stdcall Slot37();
    virtual long __stdcall ValidateDevice(unsigned long* passes);                    // slot 38 (+0x98)
};
// The camera at the render target's +0x08 (slot 30 sets the world matrix, as in
// D3DIMSoultreeShadow.cpp).
class TerrainCamera {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18(); virtual void Slot19();
    virtual void Slot20(); virtual void Slot21(); virtual void Slot22(); virtual void Slot23();
    virtual void Slot24(); virtual void Slot25(); virtual void Slot26(); virtual void Slot27();
    virtual void Slot28(); virtual void Slot29();
    virtual void SetWorldMatrix(const TerrainMatrix* m);                              // slot 30 (+0x78)
};
// The render target at GameObject::field_0x18 (PCRenderTarget; slot names as in
// D3DIMSoultreeShadow.cpp).
class TerrainRenderTarget {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5();
    virtual long GetTextureStageState(int stage, int type, int* value);  // slot 6
    virtual long SetTextureStageState(int stage, int type, int value);   // slot 7
    virtual void SetRenderState(int state, int value, int force);        // slot 8
    virtual long GetRenderState(int state, int* value);                  // slot 9
    void* display;                                             // +0x04
    TerrainCamera* camera;                                     // +0x08
    char pad_0x0c[0x38 - 0x0c];
    int transformCount;                                        // +0x38 printed as "Total Transforms" (difference over the call)
    char pad_0x3c[0x44 - 0x3c];
    int triangleCount;                                         // +0x44 printed as "Total Triangles"
    char pad_0x48[0x50 - 0x48];
    TerrainDevice* device;                                     // +0x50
};
// The profile page (DebugOverlay in src/reconstructed; cdecl member printers).
class TerrainDebugOverlay {
public:
    void UnknownFunction447f40(int row, const char* format, ...);   // 0x00447f40: one line
    void UnknownFunction447fa0(int row, const char* format, ...);   // 0x00447fa0: the page title
    char pad_0x0000[0x26c0];
    int pageCount;                                             // +0x26c0 next free page
};

// Terrain timing / memory counters (tier 3 names, tier 1 addresses; defined outside this file).
extern int g_UnknownGlobal68a308;     // 0x0068a308 "AgeTime" first value
extern int g_UnknownGlobal68a30c;     // 0x0068a30c "AgeTime" second value
extern int g_UnknownGlobal68a310;     // 0x0068a310 "Cache" count
extern int g_UnknownGlobal689ff4;     // 0x00689ff4 "Cache" bytes
extern int g_UnknownGlobal68a318;     // 0x0068a318 "PrepareGeometry" time
extern int g_UnknownGlobal68a31c;     // 0x0068a31c "Render3D" time (written here)
extern int g_UnknownGlobal68a320;     // 0x0068a320 "Total" time (written here)
extern int g_UnknownGlobal68a328;     // 0x0068a328 nonzero once the dual-texture probe has run
extern int g_gridDrawMemory;          // 0x0068a300 (Griddraw.cpp)
extern int g_gridDrawMemoryPeak;      // 0x0068a2fc (Griddraw.cpp)
extern int g_UnknownGlobal574714;     // 0x00574714 profile page row, -1 until allocated
extern TerrainPeakHold g_terrainPeak0;   // 0x0068a008 (TerrainSupport.cpp)
extern TerrainPeakHold g_terrainPeak1;   // 0x0068a038
extern TerrainPeakHold g_terrainPeak2;   // 0x0068a2e0
extern TerrainPeakHold g_terrainPeak3;   // 0x0068a2c0
extern TerrainPeakHold g_terrainPeak4;   // 0x0068a048
extern TerrainPeakHold g_terrainPeak5;   // 0x0068a2b0
extern TerrainPeakHold g_terrainPeak6;   // 0x0068a018
extern TerrainPeakHold g_terrainPeak7;   // 0x0068a2d0
extern TerrainPeakHold g_terrainPeak8;   // 0x0068a068
extern TerrainPeakHold g_terrainPeak9;   // 0x0068a078
unsigned int ReadClock();                // 0x004bfa80

// 0x00506220 (slot 14; tier 3 names).  Once per run, unless software rendering, decides
// whether the detail texture can be drawn in the same pass as the base texture (registry
// DriverInfo\<driver>\CanRenderDualTextureInSinglePass and TerrainDetailTextureMethod, or a
// ValidateDevice probe with both textures bound when the first is unknown).  Then draws the
// height field with the base texture and, depending on that choice, the detail texture in a
// second stage or a second blended pass, restores the render states and prints the profile
// page when the game's debug flag 4 is set.  Retail re-reads field_0x18 for every call
// (tier 1); with a cached local, or an inline accessor, VC6 does not share the tail of the
// single-pass branch with the detail pass.
int Terrain::GameObjectVirtualSlot14()
{
    if (!g_terrainToggle718)
        return 1;
    int transforms = ((TerrainRenderTarget*)field_0x18)->transformCount;
    int triangles = ((TerrainRenderTarget*)field_0x18)->triangleCount;
    int previousTag = g_MemTagStack->Push("Terrain");
    unsigned int start = ReadClock();

    if (!g_UnknownGlobal68a328) {
        if (g_terrainGame->softwareRendering) {
            field_0xcac = 0;
            lowestQualityOverride = 0;
        } else {
            char key[256];
            sprintf(key, "DriverInfo\\%s\\CanRenderDualTextureInSinglePass",
                    g_terrainGame->display->driverName);
            int canDual = g_terrainGame->GetRegistryInt(key, -1);
            if (canDual == 0 || canDual == 1) {
                field_0xcac = canDual;
                sprintf(key, "DriverInfo\\%s\\TerrainDetailTextureMethod",
                        g_terrainGame->display->driverName);
                int method = g_terrainGame->GetRegistryInt(key, 1);
                if (method < 0 || method > 2)
                    method = 1;
                if (method == 2) {
                    lowestQualityOverride = 1;
                    g_terrainQualityValue = 1;
                } else {
                    lowestQualityOverride = 0;
                }
                if (g_pTerrainQualityTable[qualityIndex].field_0x0c && method >= 1)
                    g_terrainQualityValue = 1;
                else
                    g_terrainQualityValue = 0;
            } else {
                field_0xcac = 0;
                lowestQualityOverride = 0;
                if (field_0xc3c) {
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0xb, 1);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 0xb, 0);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 2);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 1);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 2, 2);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 3, 1);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 5, 2);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 4, 2);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 1, 0xd);
                    ((TerrainRenderTarget*)field_0x18)->device->SetTexture(0, ((TerrainSurfaceBase*)ownedObjects[0])->textureSurface);
                    ((TerrainRenderTarget*)field_0x18)->device->SetTexture(1, field_0xc3c->textureSurface);
                    unsigned long passes = 0;
                    ((TerrainRenderTarget*)field_0x18)->device->ValidateDevice(&passes);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0xb, 0);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 0xb, 1);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 1, 1);
                    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 4, 1);
                    ((TerrainRenderTarget*)field_0x18)->device->SetTexture(1, 0);
                    g_terrainGame->SetRegistryInt(key, passes == 1);
                    if (passes == 1) {
                        field_0xcac = 1;
                        lowestQualityOverride = 0;
                        sprintf(key, "DriverInfo\\%s\\TerrainDetailTextureMethod",
                                g_terrainGame->display->driverName);
                        g_terrainGame->SetRegistryInt(key, 1);
                    }
                    g_terrainQualityValue = g_pTerrainQualityTable[qualityIndex].field_0x0c;
                }
            }
            g_UnknownGlobal68a328 = 1;
        }
    }

    int savedPerspective;
    ((TerrainRenderTarget*)field_0x18)->GetRenderState(4, &savedPerspective);
    int savedShade;
    ((TerrainRenderTarget*)field_0x18)->GetRenderState(9, &savedShade);
    ((TerrainRenderTarget*)field_0x18)->camera->SetWorldMatrix(&transform);
    field_0x88 = 0;
    field_0x8c = 0;
    {
        unsigned int key = colorKey;
        unsigned int alpha = 0x109 - (key >> 24);
        unsigned int keep = 0xff - alpha;
        unsigned int red = RGBA_GETRED(key) * keep / 0xff;
        field_0xc9c = RGBA_MAKE(alpha, alpha, alpha, 0xff);
        field_0xca0 = RGBA_MAKE(red, RGBA_GETGREEN(key) * keep / 0xff, RGBA_GETBLUE(key) * keep / 0xff, 0xff);
    }

    if (!field_0xc34 && !g_terrainQualityValue) {
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
    } else {
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
        if (g_terrainGame->softwareRendering)
            ((TerrainRenderTarget*)field_0x18)->SetRenderState(9, 1, 0);
        else
            ((TerrainRenderTarget*)field_0x18)->SetRenderState(9, 2, 0);
    }
    ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 1);

    if (lowestQualityOverride && field_0xc3c) {
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x10, g_terrainGame->magFilter);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x11, g_terrainGame->minFilter);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x12, g_terrainGame->mipFilter);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 0x10, g_terrainGame->magFilter);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 0x11, g_terrainGame->minFilter);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 0x12, g_terrainGame->mipFilter);
        field_0xcb4 = 0;
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0xb, 1);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 0xb, 0);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 1);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 2, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 3, 1);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 5, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 4, 2);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 1, 0xd);
        ((TerrainRenderTarget*)field_0x18)->SetRenderState(0x1d, 0, 0);
        ((TerrainRenderTarget*)field_0x18)->device->SetTexture(1, field_0xc3c->textureSurface);
        if (!heightField->UnknownFunction480920(((TerrainRenderTarget*)field_0x18), field_0xc3c))
            return 0;
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0xb, 0);
    } else {
        if (!g_terrainGame->softwareRendering && field_0xc3c && g_terrainQualityValue) {
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 1);
            ((TerrainRenderTarget*)field_0x18)->SetRenderState(0x1d, 1, 0);
            heightField->UnknownFunction480900(((TerrainRenderTarget*)field_0x18));
            ((TerrainRenderTarget*)field_0x18)->SetRenderState(0x1d, 0, 0);
        } else {
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 1);
            ((TerrainRenderTarget*)field_0x18)->SetRenderState(0x1d, 0, 0);
            heightField->UnknownFunction480900(((TerrainRenderTarget*)field_0x18));
        }
        if (!g_terrainGame->softwareRendering && field_0xc3c && g_terrainQualityValue) {
            field_0xc3c->UnknownVirtualSlot11();
            ((TerrainRenderTarget*)field_0x18)->SetRenderState(0x1b, 1, 0);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 5, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x10, g_terrainGame->magFilter);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x11, 2);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x12, 1);
            if (!heightField->UnknownFunction480920(((TerrainRenderTarget*)field_0x18), field_0xc3c))
                return 0;
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x10, g_terrainGame->magFilter);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x11, g_terrainGame->minFilter);
            ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(0, 0x12, g_terrainGame->mipFilter);
        }
    }
    ((TerrainRenderTarget*)field_0x18)->SetRenderState(4, savedPerspective, 0);
    ((TerrainRenderTarget*)field_0x18)->SetRenderState(9, savedShade, 0);
    field_0x88 /= 3;
    GameObject::GameObjectVirtualSlot14();
    g_UnknownGlobal68a31c = ReadClock() - start;

    if ((g_terrainGame->debugFlags & 4) && g_terrainGame->overlay) {
        if (g_UnknownGlobal574714 < 0) {
            int page = g_terrainGame->overlay->pageCount++;
            g_UnknownGlobal574714 = page;
        }
        g_terrainPeak0.UnknownFunction4cb6b0(g_UnknownGlobal68a308);
        g_terrainPeak1.UnknownFunction4cb6b0(g_UnknownGlobal68a30c);
        g_terrainPeak2.UnknownFunction4cb6b0(field_0xc04);
        g_terrainPeak3.UnknownFunction4cb6b0(field_0xbfc);
        g_terrainPeak4.UnknownFunction4cb6b0(field_0xc00);
        g_terrainPeak5.UnknownFunction4cb6b0(g_UnknownGlobal68a310);
        g_terrainPeak7.UnknownFunction4cb6b0(g_UnknownGlobal68a318);
        g_terrainPeak8.UnknownFunction4cb6b0(g_UnknownGlobal68a31c);
        g_UnknownGlobal68a320 = g_UnknownGlobal68a318 + g_UnknownGlobal68a31c;
        g_terrainPeak9.UnknownFunction4cb6b0(g_UnknownGlobal68a320);
        g_terrainGame->overlay->UnknownFunction447fa0(g_UnknownGlobal574714, "Terrain Tolerance:%d",
                                                      drawDistance);
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Memory %d",
                                                      g_MemTagStack->UnknownFunction4a2d20("Terrain"));
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "AgeTime %d(%d) %d(%d)",
                                                      g_UnknownGlobal68a308, g_UnknownGlobal68a30c,
                                                      g_terrainPeak0.UnknownFunction4cb690(),
                                                      g_terrainPeak1.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Setup   %d %d",
                                                      field_0xc04, g_terrainPeak2.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Eval    %d[%d] %d",
                                                      field_0xbfc, field_0x90,
                                                      g_terrainPeak3.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Mesh    %d[%d] %d",
                                                      field_0xc00, field_0xa4,
                                                      g_terrainPeak4.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Cache   %d[%f] %d[%f]",
                                                      g_UnknownGlobal68a310,
                                                      g_UnknownGlobal689ff4 * (1.0f / 1048576.0f),
                                                      g_terrainPeak5.UnknownFunction4cb690(),
                                                      g_terrainPeak6.UnknownFunction4cb690() * (1.0f / 1048576.0f));
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "  [%f]",
                                                      (ownedObjectArrayCount << 17) * (1.0f / 786432.0f));
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Total   %d %d",
                                                      g_UnknownGlobal68a320, g_terrainPeak9.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Vertex Buffers %d (%d)",
                                                      g_gridDrawMemory, g_gridDrawMemoryPeak);
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "PrepareGeometry% 3d (%d)",
                                                      g_UnknownGlobal68a318, g_terrainPeak7.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Render3D%        3d (%d)",
                                                      g_UnknownGlobal68a31c, g_terrainPeak8.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Total%           3d (%d)",
                                                      g_UnknownGlobal68a320, g_terrainPeak9.UnknownFunction4cb690());
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Total Transforms %d",
                                                      ((TerrainRenderTarget*)field_0x18)->transformCount - transforms);
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "Total Triangles  %d",
                                                      ((TerrainRenderTarget*)field_0x18)->triangleCount - triangles);
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "CanSinglePassDual %s",
                                                      field_0xcac ? "TRUE" : "FALSE");
        g_terrainGame->overlay->UnknownFunction447f40(g_UnknownGlobal574714, "SinglePassDual    %s",
                                                      lowestQualityOverride ? "TRUE" : "FALSE");
    }
    g_MemTagStack->Pop(previousTag);
    if (lowestQualityOverride) {
        ((TerrainRenderTarget*)field_0x18)->device->SetTexture(1, 0);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 1, 1);
        ((TerrainRenderTarget*)field_0x18)->SetTextureStageState(1, 4, 1);
    }
    return 1;
}

// 0x00507bb0.
void Terrain::ComputeRatios()
{
    int a = appliedDrawDistance;
    int b = field_0x70;
    field_0x78 = (float)(a * a) / (b * b);
    field_0x74 = (float)a / b;
    field_0x7c = 1.0f / field_0x78;
    field_0x80 = 1.0f / field_0x74;
}

TerrainQualityEntry* g_pTerrainQualityTable;
int g_terrainQualityValue;

// Slot 19 (0x00507920) and slot 22 (0x004dc4c0) are the shared "return 0" stubs.
int Terrain::GameObjectVirtualSlot19(int)
{
    return 0;
}

int Terrain::GameObjectVirtualSlot22(int, int)
{
    return 0;
}

// 0x00507930: clamps at zero, marks field_0xbf4 (a dirty flag, tier 3) when the value changes.
void Terrain::SetField0xbec(int value)
{
    if (value < 0)
        value = 0;
    if (drawDistance != value) {
        drawDistance = value;
        drawDistanceDirty = 1;
    }
}

void Terrain::SelectQuality(int index)
{
    qualityIndex = index;
    g_terrainQualityValue = g_pTerrainQualityTable[index].field_0x0c;
    SetField0xbec(g_pTerrainQualityTable[qualityIndex].drawDistanceSetting);
    // if/else, not a ?: expression: VC6 merges the two stores but allocates the joined value
    // to edx only in this form (retail 0x5079a3).
    if (lowestQualityOverride)
        qualityParamA = g_pTerrainQualityTable[9].field_0x04;
    else
        qualityParamA = g_pTerrainQualityTable[qualityIndex].field_0x04;
    qualityParamB = g_pTerrainQualityTable[qualityIndex].field_0x08;
}

int g_terrainToggle314;
int g_terrainToggle718;

// Slot 23 (0x00508850): debug key handler.  The three TestInputEvent kinds 0x43, 2 and 3
// each flip one toggle and return 1 (tier 3 semantics).
int Terrain::GameObjectVirtualSlot23(int event, int)
{
    if (TestInputEvent(0x43, 0, event, 0x80)) {
        g_terrainToggle314 = 1 - g_terrainToggle314;
        if (g_terrainToggle314)
            g_terrainSharedState = *((TerrainHost*)field_0x18)->sharedState;
        return 1;
    }
    if (TestInputEvent(2, 0, event, 0x80)) {
        if (g_terrainQualityValue && field_0xcac) {
            lowestQualityOverride = 1 - lowestQualityOverride;
            if (lowestQualityOverride) {
                qualityParamA = g_pTerrainQualityTable[9].field_0x04;
                return 1;
            }
            qualityParamA = g_pTerrainQualityTable[qualityIndex].field_0x04;
        }
        g_terrainQualityValue = 1 - g_terrainQualityValue;
        return 1;
    }
    if (TestInputEvent(3, 0, event, 0x80)) {
        g_terrainToggle718 = 1 - g_terrainToggle718;
        return 1;
    }
    return 0;
}

// 0x00507510 (Y) and 0x00507590 (Z): advance `origin` along `dir` to the plane coordinate
// `limit` when the ray is heading toward it from the outside, then return the result by value
// (tier 3 names: t = (limit - origin.c) / dir.c and the other two components step by t*dir).
// Both are `inline`: retail emits these copies right after CastSegment because some of its call
// sites were not inlined.  The early `return origin` matters: with a single return after the if, VC6 sinks the
// `push esi` used by the struct copy into the tail block; with two return statements it
// stays in the prologue as in retail.
inline TerrainVec3 TerrainClipRayToPlaneY(TerrainVec3& origin, const TerrainVec3& dir, float limit)
{
    if (!((dir.y > 0.0f && origin.y < limit) || (dir.y < 0.0f && origin.y > limit)))
        return origin;
    float t = (limit - origin.y) / dir.y;
    origin.x += t * dir.x;
    origin.y = limit;
    origin.z += t * dir.z;
    return origin;
}

inline TerrainVec3 TerrainClipRayToPlaneZ(TerrainVec3& origin, const TerrainVec3& dir, float limit)
{
    if (!((dir.z > 0.0f && origin.z < limit) || (dir.z < 0.0f && origin.z > limit)))
        return origin;
    float t = (limit - origin.z) / dir.z;
    origin.x += t * dir.x;
    origin.y += t * dir.y;
    origin.z = limit;
    return origin;
}

// The four per-file vector constants (0x00689ff8, 0x0068a028, 0x0068a058,
// 0x00689fe8). Their initializers 0x005089a0..0x00508adc follow this file's
// GetHeightRange (0x00508970). .CRT$XCU lists them just before this file's
// camera and timer initializers (0x00505480..0x005055e0), their .bss is
// interleaved with those timers, and this file's code reads the y axis
// (0x00507f15 onwards: dotted with a face normal to orient it). See
// docs/INITIALIZERS.md. The original header type is unknown; TerrainVec3 stands in.
static const TerrainVec3 kVec3Zero = TerrainVec3(0.0f, 0.0f, 0.0f);
static const TerrainVec3 kVec3XAxis = TerrainVec3(1.0f, 0.0f, 0.0f);
static const TerrainVec3 kVec3YAxis = TerrainVec3(0.0f, 1.0f, 0.0f);
static const TerrainVec3 kVec3ZAxis = TerrainVec3(0.0f, 0.0f, 1.0f);

// Truncates v to an int through *dst.  Retail does this with an inline `fistp` (the caller
// subtracts 0.5 first, so it floors), and the inlined helper materialises both arguments in
// stack temporaries (`lea eax,[ix]; mov [tmp],eax; fstp [tmp2]; fld [tmp2]; fistp [eax]`).
// NOTE: a plain (int) cast compiles to a call to _ftol under the gate profile. Spilling both
// arguments to stack temporaries is the shape of an inlined `__asm { fld f; mov edx,p;
// fistp [edx] }` FloatToInt helper (tier 3); /QIfist alone would emit a bare fistp. Inline
// asm is out of bounds here, so QueryGround stays partial (see targets.json).
static inline void FloatToInt(int* dst, float v)
{
    *dst = (int)v;
}

static inline TerrainVec3 TerrainNormalize(const TerrainVec3& v)
{
    float lenSq = TerrainDot(v, v);
    if (lenSq == 1.0f)
        return v;
    return v * FastInvSqrt(lenSq);
}

// 0x00507c10 (tier 3 names throughout).  x/z are scaled by field_0xc30 (grid units per world
// unit), floored to a cell (ix, iz), and the four corners are fetched.  The point's offsets
// (u, v) inside the cell give a squared distance to each corner (w[0..3]); the smallest picks
// the surface byte.  The cell is split into two triangles along a diagonal that alternates in a
// checkerboard ((ix ^ iz) & 1), the nearer-corner test picks the triangle, and the height is
// linear over it.  The normal is either the triangle's face normal (flatShaded) or the
// inverse-squared-distance weighted blend of the three corner normals.
void Terrain::QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded,
                          unsigned char* surface)
{
    if (!heightField)
        return;

    float fx = pos->x * invGridCellSize;
    float fz = pos->z * invGridCellSize;
    if (fx < 0.0f)
        fx = 0.0f;
    if (fz < 0.0f)
        fz = 0.0f;

    int ix;
    int iz;
    FloatToInt(&ix, fx - 0.5f);
    FloatToInt(&iz, fz - 0.5f);
    if (ix < 0 || iz < 0)
        return;

    float x0 = (float)ix;
    float x1 = x0 + 1.0f;
    float z0 = (float)iz;
    float z1 = z0 + 1.0f;

    float heights[4];
    TerrainVec3 n[4];
    unsigned char kinds[4];
    TerrainVec3* cornerNormals = 0;
    if (normal)
        cornerNormals = n;
    heightField->GetCellCorners(ix, iz, heights, cornerNormals, kinds);

    float y0 = heights[0] * gridCellSize;
    float y1 = heights[1] * gridCellSize;
    float y2 = heights[2] * gridCellSize;
    float y3 = heights[3] * gridCellSize;
    TerrainVec3 v[4];
    v[0] = TerrainVec3(x0, y0, z0);
    v[1] = TerrainVec3(x1, y1, z0);
    v[2] = TerrainVec3(x0, y2, z1);
    v[3] = TerrainVec3(x1, y3, z1);

    float u = fx - x0;
    float t = fz - z0;
    float w[4];
    w[0] = u * u + t * t;
    w[1] = (1.0f - u) * (1.0f - u) + t * t;
    w[2] = u * u + (1.0f - t) * (1.0f - t);
    w[3] = (1.0f - u) * (1.0f - u) + (1.0f - t) * (1.0f - t);
    if (0.001f > w[0])
        w[0] = 0.001f;
    if (0.001f > w[1])
        w[1] = 0.001f;
    if (0.001f > w[2])
        w[2] = 0.001f;
    if (0.001f > w[3])
        w[3] = 0.001f;

    if (surface) {
        int nearest;
        float best = 3.4028235e+38f;
        for (int k = 0; k < 4; k++) {
            if (best > w[k]) {
                best = w[k];
                nearest = k;
            }
        }
        *surface = kinds[nearest];
    }

    if ((iz ^ ix) & 1) {
        if (w[0] < w[3]) {
            // triangle (0, 1, 2)
            pos->y = y0 + (y1 - y0) * (fx - ix) + (y2 - y0) * (fz - iz);
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[0], &v[1], &v[2], &face, 0);
                if (TerrainDot(face, kVec3YAxis) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[0] / w[0] + n[1] / w[1] + n[2] / w[2]);
        } else {
            // triangle (1, 3, 2)
            pos->y = y2 + (y3 - y2) * (fx - ix) + (y1 - y3) * (1.0f - (fz - iz));
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[1], &v[3], &v[2], &face, 0);
                if (TerrainDot(face, kVec3YAxis) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[1] / w[1] + n[3] / w[3] + n[2] / w[2]);
        }
    } else {
        if (w[1] < w[2]) {
            // triangle (0, 1, 3)
            pos->y = y0 + (y1 - y0) * (fx - ix) + (y3 - y1) * (fz - iz);
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[0], &v[1], &v[3], &face, 0);
                if (TerrainDot(face, kVec3YAxis) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[0] / w[0] + n[1] / w[1] + n[3] / w[3]);
        } else {
            // triangle (0, 3, 2)
            pos->y = y2 + (y3 - y2) * (fx - ix) + (y0 - y2) * (1.0f - (fz - iz));
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[0], &v[3], &v[2], &face, 0);
                if (TerrainDot(face, kVec3YAxis) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[0] / w[0] + n[3] / w[3] + n[2] / w[2]);
        }
    }
}

// X-axis sibling of TerrainClipRayToPlaneY/Z (0x00507510 / 0x00507590): retail has no
// out-of-line copy, every call site in CastSegment is inlined (tier 2: the same disjunction is
// visible inline at 0x00506f6d..0x00506ffb).  Same shape as Y/Z (tier 3).
inline TerrainVec3 TerrainClipRayToPlaneX(TerrainVec3& origin, const TerrainVec3& dir, float limit)
{
    if (!((dir.x > 0.0f && origin.x < limit) || (dir.x < 0.0f && origin.x > limit)))
        return origin;
    float t = (limit - origin.x) / dir.x;
    origin.x = limit;
    origin.y += t * dir.y;
    origin.z += t * dir.z;
    return origin;
}

// 0x00506e90 (tier 3 names).  Callers: SoultreePhysicsBaseObject slot 21 (0x00502632) and
// the TerrainShadow / projected-shadow code.  The world segment is divided by field_0x40 into
// grid space, each axis is clipped so both ends lie inside the box [0, W] x [yLo, yHi] x [0, W]
// (W = 16 << shift; the start point is advanced along the direction, the end point along the
// reversed direction), trivially rejected if both ends are on the outside of a face, then y and
// z are swapped (the grid stores (x, z, height)) and the grid's slot 1 does the actual cast.
//
// Inline budget (VC6 SP3 /O2, measured with probes): VC6 expands inline calls breadth-first --
// every direct call site in source order first (greedy: a callee that does not fit the remaining
// budget is skipped, later smaller ones may still fit), then the calls inside the inlined bodies
// (here the TerrainVec3 constructors), and the budget grows with the caller's own code size.
// Retail inlines ClipX x4, ClipZ for the start point, and every `-dir` but the last; the
// constructor is inlined only in the two scalings, `end - start` and the first `-dir`.  The
// first 0x3b7 bytes match; from the Z end-point clip on, this source still inlines one ClipZ
// too many (PARTIAL: the budget arithmetic, not the statement shapes, is the remaining gap).
int Terrain::CastSegment(const TerrainVec3* from, const TerrainVec3* to, TerrainVec3* out,
                         int a, int b, int c)
{
    TerrainVec3 start = *from / gridCellSize;
    TerrainVec3 end = *to / gridCellSize;
    TerrainVec3 dir = end - start;

    float size = (float)(16 << heightField->gridShift);
    float yLo = heightField->heightMin;
    float yHi = heightField->heightMax;

    // x slab
    if (dir.x > 0.0f) {
        TerrainClipRayToPlaneX(start, dir, 0.0f);
    } else if (dir.x < 0.0f) {
        TerrainClipRayToPlaneX(start, dir, size);
    }
    if (dir.x < 0.0f) {
        TerrainClipRayToPlaneX(end, -dir, 0.0f);
    } else if (dir.x > 0.0f) {
        TerrainClipRayToPlaneX(end, -dir, size);
    }

    // z slab
    if (dir.z > 0.0f) {
        TerrainClipRayToPlaneZ(start, dir, 0.0f);
    } else if (dir.z < 0.0f) {
        TerrainClipRayToPlaneZ(start, dir, size);
    }
    if (dir.z < 0.0f) {
        TerrainClipRayToPlaneZ(end, -dir, 0.0f);
    } else if (dir.z > 0.0f) {
        TerrainClipRayToPlaneZ(end, -dir, size);
    }

    // y slab
    if (dir.y > 0.0f) {
        TerrainClipRayToPlaneY(start, dir, yLo);
    } else if (dir.y < 0.0f) {
        TerrainClipRayToPlaneY(start, dir, yHi);
    }
    if (dir.y < 0.0f) {
        TerrainClipRayToPlaneY(end, -dir, yLo);
    } else if (dir.y > 0.0f) {
        TerrainClipRayToPlaneY(end, -dir, yHi);
    }

    if ((start.x <= 0.0f && end.x <= 0.0f) || (start.x >= size && end.x >= size) ||
        (start.z <= 0.0f && end.z <= 0.0f) || (start.z >= size && end.z >= size) ||
        (start.y <= yLo && end.y <= yLo) || (start.y >= yHi && end.y >= yHi))
        return 0;

    float t = start.y;
    start.y = start.z;
    start.z = t;
    t = end.y;
    end.y = end.z;
    end.z = t;
    if (!heightField->CastSegment(&start, &end, out, a, b, c))
        return 0;

    t = out->y;
    out->y = out->z;
    out->z = t;
    *out *= gridCellSize;
    return 1;
}
