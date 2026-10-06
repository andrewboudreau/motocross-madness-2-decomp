#pragma once

// D:\aardvark\VC\krusty2\ResourceManager.cpp (__FILE__ 0x00572b64).
// Names are provisional except ResourceItem (RTTI).
//
// This header is the full declaration of the class other TUs see through
// UnknownResourceManager.h. That header stays a reduced view because
// declaring the destructor and fields there flips a register choice in
// TrackGame 0x00520ab0 (docs/SCENEMANAGER.md); never include both.

#include "BaseObject.h"

class UnknownTextureStream;

// RTTI: ResourceItem : BaseObject (vtable 0x00557858, COL 0x0055ec78), 0x20
// bytes (operator new(0x20) at 0x004e91e1, 0x004e957f, 0x004e9718). One
// named file: the archive holding it and where, or the object it was
// loaded as. UnknownResourceEntry (UnknownResourceManager.h) is the view
// other TUs use.
class ResourceItem : public BaseObject {
public:
    ResourceItem();                       // 0x004e8d70
    virtual ~ResourceItem();              // 0x004e8e10 (deleting 0x004e8dc0)
    void UnknownFunction4e8de0();         // 0x004e8de0: empty name, no archive

    char* field_0x08;                     // name (DebugMalloc'd)
    int field_0x0c;                       // name size including the NUL
    void* field_0x10;                     // object loaded from the entry
    UnknownTextureStream* field_0x14;     // archive holding the file
    int field_0x18;                       // offset of the file in it
    int field_0x1c;                       // read after the offset (size?)
};

// The 0x18-byte object at 0x00689c78 (g_UnknownResourceManager572b44 points
// at it); SceneManager 0x004f0d20 also keeps one on its stack. Two grown
// arrays: the items and the archives they come from.
class UnknownResourceManager {
public:
    UnknownResourceManager();             // 0x004e8e80
    ~UnknownResourceManager();            // 0x004e8ea0

    // 0x004e8f40: when an item named like `item` exists, moves `item`'s
    // location onto it (deleting the old archive when unused) and returns
    // 1; 0 when there is none or the existing item's archive chain has a
    // nonzero field_0x08.
    int UnknownFunction4e8f40(ResourceItem* item);
    // 0x004e9010: records `object` on `entry`; drops the entry when both
    // the object and its archive are gone.
    void UnknownFunction4e9010(void* entry, void* object);
    // 0x004e9030: opens the "RS2" archive `path` and adds its items.
    UnknownTextureStream* UnknownFunction4e9030(const char* path, int flags);
    ResourceItem* UnknownFunction4e9360(const char* name, int a); // finds `name`
    void* UnknownFunction4e93f0(void* object);       // the item recording `object`
    int UnknownFunction4e9430(const char* name, const char* path); // adds a loose file
    int UnknownFunction4e96b0(const char* name, void* object);     // adds an object
    int UnknownFunction4e9830(UnknownTextureStream* archive);     // removes an archive's items
    void UnknownFunction4e9900(ResourceItem* item);               // releases `item`

    // Inline (no out-of-line copy): whether an item still comes from
    // `archive`. Written inline, 0x004e8f40 saves ebp only on the path that
    // uses it, as retail does; open-coded, VC6 saves it in the prologue.
    int UnknownInlineArchiveUsed(UnknownTextureStream* archive)
    {
        for (int i = 0; i < field_0x08; i++) {
            if (archive == field_0x14[i]->field_0x14)
                return 1;
        }
        return 0;
    }

    int field_0x00;                       // field_0x14 capacity
    int field_0x04;                       // field_0x10 capacity
    int field_0x08;                       // field_0x14 count
    int field_0x0c;                       // field_0x10 count
    UnknownTextureStream** field_0x10;    // archives
    ResourceItem** field_0x14;            // items
};

extern UnknownResourceManager g_UnknownResourceManager689c78;
extern UnknownResourceManager* g_UnknownResourceManager572b44;
