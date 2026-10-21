#pragma once

// Parameterblocks.cpp (D:\aardvark\VC\krusty2\Parameterblocks.cpp): an
// INI-style reader. A file holds `[section]` headers followed by lines of
// `key=value` or comma-separated rows; ';' starts a comment line. Names are
// provisional (tier 3); behaviour and layout come from 0x004b6f30..0x004b8465.

#include "UnknownTokenizer.h"

#include "FileStream.h"

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
