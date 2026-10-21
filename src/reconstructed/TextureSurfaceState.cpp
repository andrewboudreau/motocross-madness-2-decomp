// 0x00689968: four writable loader-zero bytes, read as a DWORD by texture
// setup at 0x004c7096. The only located direct address reference is that read;
// this supplies mutable storage, not a constant replacement for the branch.
// PCTextureMap.h declares it. Original translation-unit ownership is unknown.
int g_DebugDumpTextureLevels;
