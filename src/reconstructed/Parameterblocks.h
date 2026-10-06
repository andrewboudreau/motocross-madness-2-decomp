#pragma once

// Parameterblocks.cpp (D:\aardvark\VC\krusty2\Parameterblocks.cpp): an
// INI-style reader. A file holds `[section]` headers followed by lines of
// `key=value` or comma-separated rows; ';' starts a comment line. Names are
// provisional (tier 3); behaviour and layout come from 0x004b6f30..0x004b8465.

#include "UnknownTokenizer.h"

class UnknownParameterStream;

// What 0x004e9360 returns: an archive entry naming the stream and offset
// that hold a file.
struct UnknownParameterArchiveEntry {
    unsigned char field_0x00[0x14];
    UnknownParameterStream* field_0x14;   // stream holding the file
    int field_0x18;                       // offset of the file in it
};

// The archive list a stream points to at +0x12c. Tier 2: the same retail
// object as UnknownResourceManager (UnknownResourceManager.h), whose shared
// declaration types 0x004e9030 as void; Parameterblocks.cpp tests its result,
// so this view declares the three members it calls with their own addresses.
class UnknownParameterArchive {
public:
    UnknownParameterArchiveEntry* UnknownFunction4e9360(const char* name, int a); // 0x004e9360: finds `name`
    int UnknownFunction4e9030(const char* path, int flags);   // 0x004e9030: adds an archive
    void UnknownFunction4e9430(const char* name, const char* path); // 0x004e9430: loads `name` from `path`
};

// The stream a parameter block reads. Tier 2: the same retail object as
// UnknownTextureStream (TextureMap.h, whose header this TU must not edit);
// this view adds the line reader and the three recursive inline accessors
// whose out-of-line copies retail calls (0x0043e9e0, 0x0043e9b0, 0x00461d20).
// Every accessor follows +0x1c to the innermost stream first.
class UnknownParameterStream {
public:
    int UnknownFunction461340(int offset, int a, int origin); // 0x00461340: seeks
    int UnknownFunction461600();                              // 0x00461600: read position
    int UnknownFunction461aa0(char* buffer, int size);        // 0x00461aa0: reads a line, 0 at the end
    int UnknownFunction461640(void* buffer, int size, int count); // 0x00461640: reads `count` items (SoultreeMaterial.cpp)

    // 0x0043e9e0 (out-of-line copy): the text-mode byte.
    char UnknownFunction43e9e0()
    {
        if (field_0x1c)
            return field_0x1c->UnknownFunction43e9e0();
        return field_0x01;
    }
    // 0x0043e9b0 (out-of-line copy): sets the text-mode byte.
    void UnknownFunction43e9b0(char mode)
    {
        if (field_0x1c)
            field_0x1c->UnknownFunction43e9b0(mode);
        else
            field_0x01 = mode;
    }
    // 0x00461d20 (out-of-line copy): restores the text-mode byte from +3.
    void UnknownFunction461d20()
    {
        if (field_0x1c)
            field_0x1c->UnknownFunction461d20();
        else
            field_0x01 = field_0x03;
    }

    char field_0x00;
    char field_0x01;                       // text-mode byte
    char field_0x02;
    char field_0x03;                       // its initial value
    unsigned char field_0x04[0x1c - 0x04];
    UnknownParameterStream* field_0x1c;    // inner stream
    unsigned char field_0x020[0x12c - 0x20];
    UnknownParameterArchive* field_0x12c;  // archives
    int field_0x130;                       // start offset in the inner stream
};

// One parsed row value: 0 int, 1 float, 2 string (owned, DebugMalloc).
struct UnknownParameterValue {
    int type;
    union {
        int i;
        float f;
        char* s;
    };
};

// One indexed section (0x40c bytes).
struct UnknownParameterSection {
    int offset;          // stream position after the header line
    int lineCount;       // lines up to the next header
    char name[0x400];
    char mode;           // the stream's text-mode byte at the header
};

// 0x004b6f30: 2 if `text` has a letter, 1 if it has a '.', else 0.
int UnknownFunction4b6f30(const char* text);
// 0x004b6fb0: classifies and parses `text` into `value`.
void UnknownFunction4b6fb0(const char* text, UnknownParameterValue* value);
// 0x004b7040/0x004b7080/0x004b70c0: convert a value to int, float or text.
void UnknownFunction4b7040(UnknownParameterValue value, int* out);
void UnknownFunction4b7080(UnknownParameterValue value, float* out);
void UnknownFunction4b70c0(UnknownParameterValue value, char* out);

// 0x5c4 bytes (operator new(0x5c4) at 0x004458b5). No vtable; callers
// `delete` it through 0x004b7190 and the plain operator delete.
class UnknownParameterBlock {
public:
    UnknownParameterBlock();                         // 0x004b7130
    ~UnknownParameterBlock();                        // 0x004b7190
    // 0x004b7220: opens `name` through the stream's archives, else the stream itself.
    void UnknownFunction4b7220(UnknownParameterStream* stream, const char* name,
                               const char* path, int index);
    int UnknownFunction4b72c0(const char* name);     // loads a section by scanning
    void UnknownFunction4b7560(UnknownParameterStream* stream); // indexes the sections
    void UnknownFunction4b77a0(UnknownParameterStream* stream, int offset, int index); // opens
    int UnknownFunction4b7830(const char* name);     // section index or -1
    void UnknownFunction4b7890();                    // frees the loaded lines
    int UnknownFunction4b78f0(const char* name);     // selects a section
    int UnknownFunction4b7b30(const char* key, char* out, int size); // raw value of `key`
    int UnknownFunction4b7cf0(const char* key, int* out);
    int UnknownFunction4b7e70(const char* key, float* out);
    int UnknownFunction4b7ec0(const char* key, const char* def, char* out, int size);
    int UnknownFunction4b7f10(const char* key, int def, int* out);
    int UnknownFunction4b7f40(const char* key, float def, float* out);
    int UnknownFunction4b7f70(const char* name);     // starts reading a section's rows
    int UnknownFunction4b8010(char* raw);            // reads the next row
    int UnknownFunction4b8180(int index, int* out);
    int UnknownFunction4b81c0(int index, float* out);
    int UnknownFunction4b8200(int index, char* out);
    int UnknownFunction4b8240(char* out, int size);  // section names, '?'-terminated
    void UnknownFunction4b8360(const char* in, char* out); // strips blanks outside [] and ""

    UnknownParameterStream* field_0x000;  // stream
    int field_0x004;
    int field_0x008;                      // open
    int field_0x00c;
    char field_0x010[0x104];
    int field_0x114;
    int field_0x118;                      // section count
    int field_0x11c;                      // section capacity
    UnknownParameterSection* field_0x120; // sections
    int field_0x124;                      // file offset in the stream
    int field_0x128;                      // loaded line count
    char** field_0x12c;                   // loaded lines
    char field_0x130[0x400];              // loaded section name
    int field_0x530;                      // reading rows
    int field_0x534;
    int field_0x538;                      // row count
    int field_0x53c;                      // row index
    int field_0x540;                      // value count
    UnknownParameterValue field_0x544[16];
};
