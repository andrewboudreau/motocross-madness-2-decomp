#pragma once

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

// RTTI: ManagedTextureGroup : BaseObject (vtable 0x005583c0; constructor
// 0x0050bed0 writes it, destructor 0x0050c2e0, deleting wrapper 0x0050c2c0),
// 0x258 bytes. Its code sits among TextureCache.cpp's literals. The manager
// keeps them in +0x44 and steps through them with debug keys (slot 23).
class ManagedTextureGroup : public BaseObject {
public:
    ManagedTextureGroup(TextureMapManager* manager, int a, int b, int c);
    virtual ~ManagedTextureGroup();
    int UnknownFunction50c4a0(int value);     // 0x0050c4a0: 0 on failure
    void UnknownFunction50c6c0(ManagedTexture* texture); // 0x0050c6c0: adds a texture
    void UnknownFunction50c760();             // 0x0050c760 (manager slot 12)
    void UnknownFunction50c790();             // 0x0050c790 (manager slot 18)
    void UnknownFunction50c7e0();             // 0x0050c7e0 (debug key 0x15)
    void UnknownFunction50c8c0();             // 0x0050c8c0 (manager slot 13)
    void UnknownFunction50ef70(void* value);  // 0x0050ef70 (manager slot 15)

    unsigned char field_0x08;                 // toggled by debug key 0x21
    unsigned char field_0x09[0x44 - 0x09];
    UnknownTextureMapList field_0x44;         // its textures (ManagedTextures), stepped through by +0x1c8
    unsigned char field_0x54[0x60 - 0x54];
    int field_0x60;                           // count stepped through by +0x1c4
    unsigned char field_0x64[0x1c4 - 0x64];
    int field_0x1c4;
    int field_0x1c8;
    unsigned char field_0x1cc[0x258 - 0x1cc];
};

// Fixed-size block pool (BlockAllocator.cpp; constructor 0x00423f70 takes the
// block size and count, destructor 0x00423fb0), 0x28 bytes.
class UnknownBlockAllocator {
public:
    UnknownBlockAllocator(unsigned int blockSize, unsigned int count);
    ~UnknownBlockAllocator();

    unsigned char field_0x00[0x28];
};

// RTTI: TextureMapManager : GameObject (vtable 0x0055848c; 0x7c bytes, the
// size Game's initialiser allocates). Its code sits among
// TextureMapManager.cpp's literals. Names are provisional.
class TextureMapManager : public GameObject {
public:
    TextureMapManager();                      // 0x00510bd0
    virtual ~TextureMapManager();             // 0x00510cf0 (deleting wrapper 0x00510cd0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00510dd0: counts frames
    virtual int UnknownVirtualSlot12();       // 0x00510df0
    virtual int UnknownVirtualSlot13();       // 0x00510e30
    virtual int UnknownVirtualSlot15();       // 0x00510e90
    virtual int UnknownVirtualSlot18();       // 0x00511290: restores every texture
    // 0x00510ee0: debug keys stepping through the texture caches.
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);

    // 0x00511180: creates and registers a texture cache.
    ManagedTextureGroup* UnknownFunction511180(int a, int b, int c, int d);
    void UnknownFunction5112f0(TextureMap* texture); // 0x005112f0: registers a texture
    void UnknownFunction511300(TextureMap* texture); // 0x00511300: unregisters it
    // 0x005113d0: shares the texture cache limit out among the groups (on
    // AGP displays it restores every grouped texture instead).
    int UnknownFunction5113d0();
    void UnknownFunction511580();             // 0x00511580 (PCTextureMap slot 12 and destructor)
    // 0x00511310 / 0x00511370: grow-only scratch buffers of at least
    // `bytes`; 0 when the allocation fails.
    void* UnknownFunction511310(unsigned int bytes);
    void* UnknownFunction511370(unsigned int bytes);

    UnknownTextureMapList field_0x2c;         // every registered texture
    ManagedTextureGroup* field_0x3c;          // cache selected by the debug keys
    int field_0x40;                           // its index in +0x44
    ContainerList<ManagedTextureGroup*> field_0x44;
    int field_0x58;                           // debug key mode (0-2)
    void* field_0x5c;                         // scratch buffer (0x00511310)
    void* field_0x60;                         // scratch buffer (0x00511370)
    unsigned int field_0x64;                  // +0x5c's size
    unsigned int field_0x68;                  // +0x60's size
    int field_0x6c;
    UnknownBlockAllocator* field_0x70;
    int field_0x74;                           // frame count (slot 10)
    int field_0x78;
};
