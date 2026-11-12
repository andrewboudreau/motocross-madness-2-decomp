// Standalone AddArchive candidate, admitted to the game as reviewed nonexact.
// Retail 0x004e9030 spans 804 bytes; authentic VC6 /O2 /MT emits 798.
// Retail additionally homes the archive result in the new temporary's stack
// slot and returns it from there; the candidate keeps it in EBX. The final
// itemCount store/pop scheduling also differs. Both have two unwind states,
// each leading to -1, and no try blocks. See docs/RESOURCEMANAGER.md for the
// bounded profile experiment and private behavioral validation boundary.
// The other manager methods and storage belong to the canonical source.
// s20: a separate `result = archive` local returned at the end is copy-propagated
// back into EBX (unchanged, 120 strict of 804). The archives append only in the
// grown-storage branch is retail behavior (checked against 0x004e9030).
// s21 (all 120 strict unless noted; the copy is propagated into EBX):
// `archive = result = new`, `result = archive = new`, `result = new;
// archive = result`, `result = 0` or `archive = 0` first, a reference
// `FileStream*& result = archive`, an inline NewArchive helper
// (two forms) and a cast return. `if ((archive = new ...) != 0) result =
// archive; else result = 0;` is worse (131/820); a dead `slot = &result`
// is eliminated. Retail homes the ResourceItem temporary in the dead
// `flags` argument slot ([esp + 0x2c]); QuarryStuntEvent 0x004de590 (also a
// near miss) shows the same new-temporary homing.

#include <string.h>
#include "../../src/reconstructed/ResourceManager.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/FileStream.h"

// 0x004e9030
FileStream* UnknownResourceManager::AddArchive(const char* path, int flags)
{
    int count;
    char magic[4];
    int i;

    FileStream* archive = new(__FILE__, 212) FileStream(this);
    if (archive->Open(path, "rb", flags || archive->IsValidFile(path))) {
        if (archive->Read(magic, 4, 1) == 1 && !_stricmp(magic, "RS2")
            && archive->Read(&count, 4, 1) == 1) {
            if (itemCount + count > itemCapacity) {
                items = (ResourceItem**)DebugRealloc(items, (itemCount + count) * 4, __FILE__, 231);
                if (items) {
                    for (i = itemCapacity; i < itemCount + count; i++)
                        items[i] = 0;
                    itemCapacity = itemCount + count;
                }
            }
            if (archiveCount + 1 > archiveCapacity) {
                archives = (FileStream**)DebugRealloc(archives, (archiveCount + 8) * 4, __FILE__, 246);
                if (archives) {
                    archiveCapacity = archiveCount + 8;
                    archives[archiveCount++] = archive;
                }
            }
            if (archives && items) {
                for (i = 0; i < count; i++) {
                    items[itemCount + i] = new(__FILE__, 261) ResourceItem;
                    archive->Read(&items[itemCount + i]->nameSize, 4, 1);
                    items[itemCount + i]->name =
                        (char*)DebugRealloc(items[itemCount + i]->name,
                                            items[itemCount + i]->nameSize, __FILE__, 268);
                    archive->Read(items[itemCount + i]->name,
                                                   items[itemCount + i]->nameSize, 1);
                    archive->Read(&items[itemCount + i]->offset, 4, 1);
                    archive->Read(&items[itemCount + i]->size, 4, 1);
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
