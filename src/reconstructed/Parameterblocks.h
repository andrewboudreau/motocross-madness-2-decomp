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
    UnknownParameterStream* holderStream;     // +0x14 stream holding the file
    int holderOffset;                         // +0x18 offset of the file in it
};

// The archive list a stream points to at +0x12c. Tier 2: the same retail
// object as UnknownResourceManager (UnknownResourceManager.h), whose shared
// declaration types 0x004e9030 as void; Parameterblocks.cpp tests its result,
// so this view declares the three members it calls with their own addresses.
class UnknownParameterArchive {
public:
    UnknownParameterArchiveEntry* FindItem(const char* name, int a); // 0x004e9360: finds `name`
    int AddArchive(const char* path, int flags);   // 0x004e9030: adds an archive
    void AddLooseFile(const char* name, const char* path); // 0x004e9430: loads `name` from `path`
};

// The stream a parameter block reads. Tier 2: the same retail object as
// UnknownTextureStream (TextureMap.h, whose header this TU must not edit);
// this view adds the line reader and the three recursive inline accessors
// whose out-of-line copies retail calls (0x0043e9e0, 0x0043e9b0, 0x00461d20).
// Every accessor follows +0x1c to the innermost stream first.
class UnknownParameterStream {
public:
    int Seek(int offset, int a, int origin); // 0x00461340: seeks
    int Tell();                              // 0x00461600: read position
    int ReadLine(char* buffer, int size);        // 0x00461aa0: reads a line, 0 at the end
    int Read(void* buffer, int size, int count); // 0x00461640: reads `count` items (SoultreeMaterial.cpp)

    // 0x0043e9e0 (out-of-line copy): the text-mode byte.
    char GetTextMode()
    {
        if (innerStream)
            return innerStream->GetTextMode();
        return textMode;
    }
    // 0x0043e9b0 (out-of-line copy): sets the text-mode byte.
    void SetTextMode(char mode)
    {
        if (innerStream)
            innerStream->SetTextMode(mode);
        else
            textMode = mode;
    }
    // 0x00461d20 (out-of-line copy): restores the text-mode byte from +3.
    void ResetTextMode()
    {
        if (innerStream)
            innerStream->ResetTextMode();
        else
            textMode = initialTextMode;
    }

    char field_0x00;
    char textMode;                         // +0x01 text-mode byte
    char field_0x02;
    char initialTextMode;                  // +0x03 its initial value
    unsigned char field_0x04[0x1c - 0x04];
    UnknownParameterStream* innerStream;      // +0x1c inner stream
    unsigned char field_0x020[0x12c - 0x20];
    UnknownParameterArchive* archives;        // +0x12c archives
    int startOffset;                          // +0x130 start offset in the inner stream
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
int ClassifyParameterValue(const char* text);
// 0x004b6fb0: classifies and parses `text` into `value`.
void ParseParameterValue(const char* text, UnknownParameterValue* value);
// 0x004b7040/0x004b7080/0x004b70c0: convert a value to int, float or text.
void ParameterValueToInt(UnknownParameterValue value, int* out);
void ParameterValueToFloat(UnknownParameterValue value, float* out);
void ParameterValueToString(UnknownParameterValue value, char* out);

// 0x5c4 bytes (operator new(0x5c4) at 0x004458b5). No vtable; callers
// `delete` it through 0x004b7190 and the plain operator delete.
class UnknownParameterBlock {
public:
    UnknownParameterBlock();                         // 0x004b7130
    ~UnknownParameterBlock();                        // 0x004b7190
    // 0x004b7220: opens `name` through the stream's archives, else the stream itself.
    void OpenFromArchives(UnknownParameterStream* stream, const char* name,
                               const char* path, int index);
    int ScanSection(const char* name);     // loads a section by scanning
    void IndexSections(UnknownParameterStream* stream); // indexes the sections
    void Attach(UnknownParameterStream* stream, int offset, int index); // opens
    int FindSection(const char* name);     // section index or -1
    void FreeLines();                    // frees the loaded lines
    int SelectSection(const char* name);     // selects a section
    int ReadString(const char* key, char* out, int size); // raw value of `key`
    int ReadInt(const char* key, int* out);
    int ReadFloat(const char* key, float* out);
    int GetString(const char* key, const char* def, char* out, int size);
    int GetInt(const char* key, int def, int* out);
    int GetFloat(const char* key, float def, float* out);
    int BeginRows(const char* name);     // starts reading a section's rows
    int ReadRow(char* raw);            // reads the next row
    int GetRowInt(int index, int* out);
    int GetRowFloat(int index, float* out);
    int GetRowString(int index, char* out);
    int GetSectionNames(char* out, int size);  // section names, '?'-terminated
    void StripBlanks(const char* in, char* out); // strips blanks outside [] and ""

    UnknownParameterStream* blockStream;      // +0x000 stream
    int field_0x004;
    int isOpen;                               // +0x008 open
    int field_0x00c;
    char field_0x010[0x104];
    int field_0x114;
    int sectionCount;                         // +0x118 section count
    int sectionCapacity;                      // +0x11c section capacity
    UnknownParameterSection* sectionTable;    // +0x120 sections
    int fileOffset;                           // +0x124 file offset in the stream
    int loadedLineCount;                      // +0x128 loaded line count
    char** loadedLines;                       // +0x12c loaded lines
    char loadedSectionName[0x400];            // +0x130 loaded section name
    int readingRows;                          // +0x530 reading rows
    int field_0x534;
    int rowCount;                             // +0x538 row count
    int rowIndex;                             // +0x53c row index
    int valueCount;                           // +0x540 value count
    UnknownParameterValue rowValues[16];      // +0x544
};
