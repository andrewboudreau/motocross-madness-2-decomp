#pragma once

class UnknownTextureStream;

// What 0x004e9360 returns: an archive entry naming the stream and offset
// that hold a file, and the object loaded from it (PCAudio.cpp keeps a
// Sound there).
struct UnknownResourceEntry {
    unsigned char field_0x00[0x10];
    void* field_0x10;                     // object loaded from the entry
    UnknownTextureStream* field_0x14;     // stream holding the file
    int field_0x18;                       // offset of the file in it
};

// Global at 0x00572b44; its code is among ResourceManager.cpp's literals.
// No RTTI names it. Its constructor, destructor and fields are declared on
// UnknownSceneResourceManager (SceneManager.h); ResourceManager.h has the
// full declaration ResourceManager.cpp compiles against (do not include both).
class UnknownResourceManager {
public:
    UnknownResourceEntry* UnknownFunction4e9360(const char* name, int a); // 0x004e9360: finds `name`
    // 0x004e9030: adds an archive and returns it; 0x004e9830 removes one.
    UnknownTextureStream* UnknownFunction4e9030(const char* path, int flags);
    void UnknownFunction4e9830(UnknownTextureStream* archive);
    // 0x004e9430 loads `name` from `path`; 0x004e96b0 takes `name` over from
    // the global manager (SceneManager 0x004f0310).
    void UnknownFunction4e9430(const char* name, const char* path);
    void UnknownFunction4e96b0(const char* name, int a);
    // 0x004e93f0 / 0x004e9010: the entry recorded for `object` (0 if none),
    // and its removal (TextureMap's destructor).
    void* UnknownFunction4e93f0(void* object);
    void UnknownFunction4e9010(void* entry, int flags);
    // PCAudio.cpp passes the object to record as the second argument (one
    // retail function; declared twice to keep TextureMap's call unchanged).
    void UnknownFunction4e9010(void* entry, void* object);
};
extern UnknownResourceManager* g_UnknownResourceManager572b44;
