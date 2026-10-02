#pragma once

// Global at 0x00572b44; its code is among ResourceManager.cpp's literals.
// No RTTI names it.
class UnknownResourceManager {
public:
    void UnknownFunction4e9030(const char* path, int flags);  // 0x004e9030: adds an archive
    // 0x004e93f0 / 0x004e9010: the entry recorded for `object` (0 if none),
    // and its removal (TextureMap's destructor).
    void* UnknownFunction4e93f0(void* object);
    void UnknownFunction4e9010(void* entry, int flags);
};
extern UnknownResourceManager* g_UnknownResourceManager572b44;
