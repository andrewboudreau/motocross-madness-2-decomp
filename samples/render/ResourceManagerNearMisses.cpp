// Near-miss ResourceManager.cpp candidates, kept out of src/reconstructed
// until they match. The canonical file is included first so the TU-local
// declarations and globals are the same.
//
// 0x004e9030 (802 bytes, opens an "RS2" archive and adds its items): every
// instruction matches except that retail also keeps `archive` in the stack
// slot of the `new` temporary ([esp+0x14], stored right after the EH state
// reset) and returns it from there; VC6 here keeps it in ebx only. Tried
// without effect: declaration order, separate declaration and assignment,
// a second `result` variable, a single shared return.

#include "../../src/reconstructed/ResourceManager.cpp"

// 0x004e9030
UnknownTextureStream* UnknownResourceManager::UnknownFunction4e9030(const char* path, int flags)
{
    int count;
    char magic[4];
    int i;

    UnknownTextureStream* archive = new(__FILE__, 212) UnknownTextureStream((int)this);
    if (archive->UnknownFunction460f50(path, "rb", flags || archive->UnknownFunction460e70(path))) {
        if (archive->UnknownFunction461640(magic, 4, 1) == 1 && !_stricmp(magic, "RS2")
            && archive->UnknownFunction461640(&count, 4, 1) == 1) {
            if (field_0x08 + count > field_0x00) {
                field_0x14 = (ResourceItem**)DebugRealloc(field_0x14, (field_0x08 + count) * 4, __FILE__, 231);
                if (field_0x14) {
                    for (i = field_0x00; i < field_0x08 + count; i++)
                        field_0x14[i] = 0;
                    field_0x00 = field_0x08 + count;
                }
            }
            if (field_0x0c + 1 > field_0x04) {
                field_0x10 = (UnknownTextureStream**)DebugRealloc(field_0x10, (field_0x0c + 8) * 4, __FILE__, 246);
                if (field_0x10) {
                    field_0x04 = field_0x0c + 8;
                    field_0x10[field_0x0c++] = archive;
                }
            }
            if (field_0x10 && field_0x14) {
                for (i = 0; i < count; i++) {
                    field_0x14[field_0x08 + i] = new(__FILE__, 261) ResourceItem;
                    archive->UnknownFunction461640(&field_0x14[field_0x08 + i]->field_0x0c, 4, 1);
                    field_0x14[field_0x08 + i]->field_0x08 =
                        (char*)DebugRealloc(field_0x14[field_0x08 + i]->field_0x08,
                                            field_0x14[field_0x08 + i]->field_0x0c, __FILE__, 268);
                    archive->UnknownFunction461640(field_0x14[field_0x08 + i]->field_0x08,
                                                   field_0x14[field_0x08 + i]->field_0x0c, 1);
                    archive->UnknownFunction461640(&field_0x14[field_0x08 + i]->field_0x18, 4, 1);
                    archive->UnknownFunction461640(&field_0x14[field_0x08 + i]->field_0x1c, 4, 1);
                    field_0x14[field_0x08 + i]->field_0x14 = archive;
                    if (UnknownFunction4e8f40(field_0x14[field_0x08 + i])) {
                        field_0x14[field_0x08 + i]->Release();
                        i--;
                        count--;
                    }
                }
            }
            field_0x08 += count;
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
