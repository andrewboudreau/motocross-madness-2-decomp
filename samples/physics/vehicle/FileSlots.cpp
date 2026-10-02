// FileSlots.cpp -- three accessors of a small stream table (retail 0x005253d0..0x0052549d).
//
// Not Vehicle code.  They sit in the bracket the brief assigns to Vehicle.cpp (0x5252ed..)
// but before Vehicle.cpp's own __FILE__ xrefs (0x525e98..); their callers are the recorder
// (0x004e71a5.., D:\aardvark\VC\krusty2\recorder.cpp), so the class is probably recorder
// support.  owner: bracket only.  All names are tier 3.
//
// Evidence (tier 1 decoded): the table holds a mode flag at +0x23c.  With the flag set the
// streams are in-memory entries of 280 bytes starting at +4 whose dwords at entry+0x110
// (position) and entry+0x114 (end) the accessors use; with the flag clear +0x240 is an
// array of CRT FILE* and the accessors forward to fseek (0x00534eac), ftell (0x005365d6)
// and FILE::_flag & _IOEOF.  (Both lock/unlock wrappers are the CRT's own.)
#include <stdio.h>

struct MemStream {
    char field_0x00[0x110];
    int position;       // entry+0x110: stored by Seek, returned by Tell
    int end;            // entry+0x114: Eof when end <= position
};

struct StreamTable {
    int count;                 // +0x000 number of in-memory entries
    MemStream streams[2];      // +0x004 (stride 280); only the first `count` are searched
    int field_0x234[2];
    int memoryMode;            // +0x23c
    FILE* files[1];            // +0x240

    int Seek(int index, int pos, int whence);   // 0x005253d0 (ret 0xc)
    int Tell(int index);                        // 0x00525440 (ret 4)
    int Eof(int index);                         // 0x005254a0 (ret 4)
};

int StreamTable::Seek(int index, int pos, int whence)
{
    if (memoryMode) {
        for (int i = 0; i < count; i++) {
            if (i == index) {
                streams[i].position = pos;
                return pos;
            }
        }
        return 0;
    }
    if (index >= 0 && files[index])
        return fseek(files[index], pos, whence);
    return 0;
}

int StreamTable::Tell(int index)
{
    if (memoryMode) {
        for (int i = 0; i < count; i++) {
            if (i == index)
                return streams[i].position;
        }
        return 0;
    }
    if (index >= 0 && files[index])
        return ftell(files[index]);
    return 0;
}

int StreamTable::Eof(int index)
{
    if (memoryMode) {
        for (int i = 0; i < count; i++) {
            if (i == index)
                return streams[i].end <= streams[i].position;
        }
        return 0;
    }
    if (index >= 0 && files[index])
        return files[index]->_flag & _IOEOF;
    return 0;
}
