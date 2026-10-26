#pragma once

#include "BlockAllocator.h"

#include "ContainerList.h"
#include "GameObject.h"

class ManagedTexture;
class TextureMap;
class TextureMapManager;

// Intrusive list of TextureMaps through their +0x08 (next) and +0x0c
// (previous) links, with a cursor. Its methods are out of line among
// TextureMapManager.cpp's code; the original name is unknown.
class UnknownTextureMapList {
public:
    UnknownTextureMapList();                  // 0x00510a50
    ~UnknownTextureMapList();                 // 0x00510a60: unlinks every texture
    TextureMap* First();                      // 0x00510a90
    TextureMap* Last();                       // 0x00510aa0
    TextureMap* Next();                       // 0x00510ab0
    TextureMap* Previous();                   // 0x00510ad0
    void Append(TextureMap* texture);         // 0x00510af0
    void Remove(TextureMap* texture);         // 0x00510b30
    void AppendList(UnknownTextureMapList* other); // 0x00510b70: moves `other`'s textures here

    TextureMap* m_head;
    TextureMap* m_tail;
    TextureMap* m_cursor;
    int m_count;
};

// qsort comparators of ManagedTexture pointers (TextureCache.cpp).
int UnknownCompare50ee70(const void* first, const void* second);
int UnknownCompare50eeb0(const void* first, const void* second);
int UnknownCompare50ef00(const void* first, const void* second);

// 0x0050d800 (cdecl): fits a size list's planned levels into `budget` texels.
int FitTexturesToBudget(ContainerList<ManagedTexture*>* list, int budget);

// Interval timer (PCVideoCard.cpp; constructor 0x004cb670 records the
// current time and the interval), 12 bytes.
class UnknownTimer {
public:
    explicit UnknownTimer(int interval);
    int LargestRecentSample();              // 0x004cb690: the largest recent sample
    void RecordSample(int sample);   // 0x004cb6b0: records a sample

    int timerStart;                           // +0x00 start time
    int field_0x04;
    int timerInterval;                        // +0x08 interval
};

class CacheTexture;
class PCRenderTarget;
class PCTextureMap;

// RTTI: ManagedTextureGroup : BaseObject (vtable 0x005583c0), 0x258 bytes.
// Its code cites TextureCache.cpp. It owns ManagedTextures (+0x44) of one
// format and the CacheTexture pages (+0x54) they are packed onto; the
// manager keeps the groups in its +0x44 and steps through them with debug
// keys (slot 23).
class ManagedTextureGroup : public BaseObject {
public:
    // 0x0050bed0
    ManagedTextureGroup(TextureMapManager* manager, int format, int addressU, int addressV);
    virtual ~ManagedTextureGroup();           // 0x0050c2e0 (deleting wrapper 0x0050c2c0)
    int SetPageCount(int count);     // 0x0050c4a0: sets the page count; 0 on failure
    void AddTexture(ManagedTexture* texture); // 0x0050c6c0: adds a texture
    void ClearUseRecords();             // 0x0050c760: clears the textures' use records
    void RestoreLostPages();             // 0x0050c790: restores lost pages
    void WriteDebugReport();             // 0x0050c7e0: appends a report to C:\temp\TM_debug.txt
    void RepackPages();             // 0x0050c8c0: repacks the pages
    void RepackWholePages();             // 0x0050c960 (near miss in samples/render)
    void RepackWithPartialBlits();             // 0x0050dad0 (near miss in samples/render)
    void UnknownFunction464e90();             // 0x00464e90 (shared empty body): out of page space
    // 0x0050ef70 (manager slot 15): draws the debug-key selection into
    // `target`; 0 unless this is the manager's selected group.
    int DrawSelection(PCRenderTarget* target);

    unsigned char noRepacking;                // +0x08 toggled by debug key 0x21; set: no repacking
    int groupFormat;                          // +0x0c pixel format
    int groupAddressU;                        // +0x10 address mode u
    int groupAddressV;                        // +0x14 address mode v
    int pagesByLevel[9];                      // +0x18 region slots per size level kept by the last partial-blit repack
    int pageMemory;                           // +0x3c pages' memory
    TextureMapManager* groupManager;          // +0x40
    UnknownTextureMapList managedTextures;    // +0x44 ManagedTextures, stepped through by +0x1c8
    UnknownTextureMapList cachePages;         // +0x54 CacheTexture pages, stepped through by +0x1c4
    UnknownTextureMapList field_0x64;
    int blitBudget;                           // +0x74 ctor 0x10000; the partial-blit repack lowers mip levels while more texels than this are queued
    int maxEmptiedPages;                      // +0x78 6 / bytes per pixel; the whole-page repack empties at most this many pages once every unplaced texture fits
    ContainerList<ManagedTexture*> usedTextures;     // +0x7c textures used since the last repack, at their planned level
    ContainerList<ManagedTexture*> unusedByLevel[9]; // +0x90 unused textures that still hold a region, by allocated level
    ContainerList<ManagedTexture*> blitTextures;     // +0x144 textures whose mip levels the partial-blit repack copies
    ContainerList<ManagedTexture*> unplacedTextures; // +0x158 textures without a region, placed into the emptied pages
    ContainerList<ManagedTexture*> growingTextures;  // +0x16c textures allocated below their planned level
    ContainerList<ManagedTexture*> pageTextures;     // +0x180 scratch: one page's textures while its wantedTexels is summed
    ContainerList<ManagedTexture*> evictedTextures;  // +0x194 scratch: the textures of a page being emptied
    ContainerList<ManagedTexture*> dumpTextures;     // +0x1a8 scratch: one page's textures for the debug page dump
    void* groupPalette;                       // +0x1bc palette of 8-bit groups
    PCTextureMap* surfaceTexture;             // +0x1c0 256x256 texture whose system surface the pages share
    int pageCursor;                           // +0x1c4 debug key cursor (pages)
    int textureCursor;                        // +0x1c8 debug key cursor (textures)
    int placePasses;                          // +0x1cc PlaceTextures calls in the last repack
    int texelsBlitted;                        // +0x1d0 "TexMem Blted" (x bytes/pixel); CacheTexture adds each blit rect's area
    UnknownTimer texelsBlittedHistory;        // +0x1d4 samples of +0x1d0; its largest is "TexMem Blted"'s peak
    UnknownTimer blitTimeHistory;             // +0x1e0 samples of now - +0x250; its largest is "BltTime"'s (%d)
    UnknownTimer texturesBlittedHistory;      // +0x1ec samples of the blitted count; "Textures Blted %d (%d)"
    void* gdiObject;                          // +0x1f8 GDI object (DeleteObject)
    int requestedBySize[9];                   // +0x1fc per size level; "Size Requested Granted" column 1
    int grantedBySize[9];                     // +0x220 per planned level; "Size Requested Granted" column 2
    int texelsRequested;                      // +0x244 "TexMemRequested"; sum of the requested levels' areas
    int texelsGranted;                        // +0x248 "TexMemGranted"; sum of the planned levels' areas
    int texelsManaged;                        // +0x24c "TexMemManaged"; sum of the textures' full-size areas
    int blitStartTime;                        // +0x250 clock before the blits; "BltTime" prints now minus it
    int debugPage;                            // +0x254 debug overlay page (-1 until NewPage), titled "TextureManager ..."
};

// RTTI: TextureMapManager : GameObject (vtable 0x0055848c; 0x7c bytes, the
// size Game's initialiser allocates). Its code sits among
// TextureMapManager.cpp's literals. Names are provisional.
class TextureMapManager : public GameObject {
public:
    TextureMapManager();                      // 0x00510bd0
    virtual ~TextureMapManager();             // 0x00510cf0 (deleting wrapper 0x00510cd0)
    virtual int Tick(float frameTime);                 // 0x00510dd0: counts frames
    virtual int PrepareGeometry();            // 0x00510df0
    virtual int RenderPre3D();                // 0x00510e30
    virtual int RenderPost3D();               // 0x00510e90
    virtual int OnActivate();                 // 0x00511290: restores every texture
    // 0x00510ee0: debug keys stepping through the texture caches.
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);

    // 0x00511180: creates and registers a texture group with `pages` pages.
    ManagedTextureGroup* CreateGroup(int format, int pages, int addressU, int addressV);
    void RegisterTexture(TextureMap* texture); // 0x005112f0: registers a texture
    void UnregisterTexture(TextureMap* texture); // 0x00511300: unregisters it
    // 0x005113d0: shares the texture cache limit out among the groups (on
    // AGP displays it restores every grouped texture instead).
    int ShareCacheLimit();
    void CountTextureMemory();             // 0x00511580 (PCTextureMap slot 12 and destructor)
    // 0x00511310 / 0x00511370: grow-only scratch buffers of at least
    // `bytes`; 0 when the allocation fails.
    void* GrowScratchBuffer(unsigned int bytes);
    void* GrowSecondScratchBuffer(unsigned int bytes);

    UnknownTextureMapList allTextures;        // +0x2c every registered texture
    ManagedTextureGroup* debugGroup;          // +0x3c cache selected by the debug keys
    int debugGroupIndex;                      // +0x40 its index in +0x44
    ContainerList<ManagedTextureGroup*> groups; // +0x44  ManagedTextureGroups: CreateGroup adds, the group destructor removes, ShareCacheLimit and the debug keys walk them
    int debugKeyMode;                         // +0x58 debug key mode (0-2)
    void* scratchBuffer;                      // +0x5c scratch buffer (0x00511310)
    void* secondScratchBuffer;                // +0x60 scratch buffer (0x00511370)
    unsigned int scratchBufferSize;           // +0x64 +0x5c's size
    unsigned int secondScratchBufferSize;     // +0x68 +0x60's size
    int field_0x6c;
    BlockAllocator* regionAllocator;          // +0x70 0x30-byte region blocks
    int frameCount;                           // +0x74 frame count (slot 10)
    int field_0x78;
};
