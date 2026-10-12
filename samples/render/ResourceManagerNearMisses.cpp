// Near-miss ResourceManager.cpp candidates, kept out of src/reconstructed
// until they match. The canonical file is included first so the TU-local
// declarations and globals are the same.
//
// 0x004e9030 (802 bytes, opens an "RS2" archive and adds its items): every
// instruction matches except that retail also keeps `archive` in the stack
// slot of the `new` temporary ([esp+0x14], stored right after the EH state
// reset) and returns it from there; VC6 here keeps it in ebx only. Tried
// without effect: declaration order, separate declaration and assignment,
// a second `result` variable (declared at the top or at the `new`, assigned
// from `archive` or receiving the `new` and copied to `archive`; all 95 of
// 798 by match.py), a single shared return.

#include "../../src/reconstructed/ResourceManager.cpp"

// 0x004e9030
UnknownTextureStream* UnknownResourceManager::AddArchive(const char* path, int flags)
{
    int count;
    char magic[4];
    int i;

    UnknownTextureStream* archive = new(__FILE__, 212) UnknownTextureStream((int)this);
    if (archive->UnknownFunction460f50(path, "rb", flags || archive->UnknownFunction460e70(path))) {
        if (archive->UnknownFunction461640(magic, 4, 1) == 1 && !_stricmp(magic, "RS2")
            && archive->UnknownFunction461640(&count, 4, 1) == 1) {
            if (itemCount + count > itemCapacity) {
                items = (ResourceItem**)DebugRealloc(items, (itemCount + count) * 4, __FILE__, 231);
                if (items) {
                    for (i = itemCapacity; i < itemCount + count; i++)
                        items[i] = 0;
                    itemCapacity = itemCount + count;
                }
            }
            if (archiveCount + 1 > archiveCapacity) {
                archives = (UnknownTextureStream**)DebugRealloc(archives, (archiveCount + 8) * 4, __FILE__, 246);
                if (archives) {
                    archiveCapacity = archiveCount + 8;
                    archives[archiveCount++] = archive;
                }
            }
            if (archives && items) {
                for (i = 0; i < count; i++) {
                    items[itemCount + i] = new(__FILE__, 261) ResourceItem;
                    archive->UnknownFunction461640(&items[itemCount + i]->nameSize, 4, 1);
                    items[itemCount + i]->name =
                        (char*)DebugRealloc(items[itemCount + i]->name,
                                            items[itemCount + i]->nameSize, __FILE__, 268);
                    archive->UnknownFunction461640(items[itemCount + i]->name,
                                                   items[itemCount + i]->nameSize, 1);
                    archive->UnknownFunction461640(&items[itemCount + i]->offset, 4, 1);
                    archive->UnknownFunction461640(&items[itemCount + i]->size, 4, 1);
                    items[itemCount + i]->archive = archive;
                    if (MergeWithExisting(items[itemCount + i])) {
                        items[itemCount + i]->Release();
                        i--;
                        count--;
                    }
                }
            }
            itemCount += count;
            return archive;
        }
        if (archive)
            delete archive;
        return 0;
    }
    if (archive)
        delete archive;
    return 0;
}
