// Near-miss Parameterblocks.cpp candidates, kept out of src/reconstructed
// until they match. See docs/PARAMETERBLOCKS.md.
//
// UnknownParameterBlock::UnknownFunction4b7220 (0x004b7220, 157 bytes): opens
// `name` through the stream's archive list (+0x12c): find it, else add the
// archive at `path` (loading `name` from it when that fails) and find it
// again; without an entry or archives it opens the stream itself at offset 0.
// Calls, arguments, branch layout and frame match: 144 of 157 bytes
// (91.7%). Retail keeps `this` in ebx and `path` in ebp, pushing ebp only
// around the not-found block; VC6 here swaps the two registers. Early-return,
// if/else, top-declared entry, local path copy and `== 0` forms do not change
// it.
#include "../../src/reconstructed/Parameterblocks.h"

void UnknownParameterBlock::UnknownFunction4b7220(UnknownParameterStream* stream,
                                                  const char* name, const char* path,
                                                  int index)
{
    if (stream->field_0x12c) {
        UnknownParameterArchiveEntry* entry =
            stream->field_0x12c->UnknownFunction4e9360(name, 0);
        if (!entry) {
            if (!stream->field_0x12c->UnknownFunction4e9030(path, 0))
                stream->field_0x12c->UnknownFunction4e9430(name, path);
            entry = stream->field_0x12c->UnknownFunction4e9360(name, 0);
            if (!entry) {
                UnknownFunction4b77a0(stream, 0, index);
                return;
            }
        }
        UnknownFunction4b77a0(entry->field_0x14, entry->field_0x18, index);
        return;
    }
    UnknownFunction4b77a0(stream, 0, index);
}
