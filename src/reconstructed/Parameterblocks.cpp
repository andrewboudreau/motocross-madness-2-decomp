// Parameterblocks.cpp -- reconstruction of D:\aardvark\VC\krusty2\Parameterblocks.cpp
// (0x004b6f30..0x004b8465). See Parameterblocks.h and docs/PARAMETERBLOCKS.md.
// Line numbers passed to the debug allocator are the retail literals.

#include "Parameterblocks.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"

#define PB_LINE_SIZE 0x400
#define PB_MIN(a, b) (((a) < (b)) ? (a) : (b))

// Retail stops the scan at the first letter by setting the index to the
// length (not with `break`).
int UnknownFunction4b6f30(const char* text)
{
    int length = strlen(text);
    int hasLetter = 0;
    int hasDot = 0;

    for (int i = 0; i < length; i++) {
        if (isalpha(text[i])) {
            hasLetter = 1;
            i = length;
        } else if (text[i] == '.') {
            hasDot = 1;
        }
    }
    if (hasLetter)
        return 2;
    if (hasDot)
        return 1;
    return 0;
}

// The string case computes the size into a local before the call; passing
// `strlen(text) + 1` directly schedules the pushes differently.
void UnknownFunction4b6fb0(const char* text, UnknownParameterValue* value)
{
    switch (value->type = UnknownFunction4b6f30(text)) {
    case 2: {
        int size = strlen(text) + 1;
        value->s = (char*)DebugMalloc(size, __FILE__, 44);
        strcpy(value->s, text);
        break;
    }
    case 1:
        value->f = (float)atof(text);
        break;
    case 0:
        value->i = atoi(text);
        break;
    }
}

// Retail copies the raw 32 bits for both numeric types (no float-to-int
// conversion), in two separate case bodies.
void UnknownFunction4b7040(UnknownParameterValue value, int* out)
{
    char* end;

    switch (value.type) {
    case 2:
        *out = strtol(value.s, &end, 0);
        break;
    case 1:
        *out = value.i;
        break;
    case 0:
        *out = value.i;
        break;
    }
}

// As above: both numeric types load the raw bits as a float.
void UnknownFunction4b7080(UnknownParameterValue value, float* out)
{
    switch (value.type) {
    case 2:
        *out = (float)atof(value.s);
        break;
    case 1:
        *out = value.f;
        break;
    case 0:
        *out = value.f;
        break;
    }
}

void UnknownFunction4b70c0(UnknownParameterValue value, char* out)
{
    switch (value.type) {
    case 2:
        strcpy(out, value.s);
        break;
    case 1:
        sprintf(out, "%f", value.f);
        break;
    case 0:
        sprintf(out, "%i", value.i);
        break;
    }
}

UnknownParameterBlock::UnknownParameterBlock()
{
    field_0x000 = 0;
    field_0x004 = 0;
    field_0x008 = 0;
    field_0x00c = 0;
    field_0x118 = 0;
    field_0x11c = 0;
    field_0x120 = 0;
    field_0x124 = 0;
    field_0x128 = 0;
    field_0x12c = 0;
    field_0x530 = 0;
    field_0x534 = 0;
    field_0x538 = 0;
    field_0x53c = 0;
    field_0x540 = 0;
    field_0x114 = 0;
    field_0x010[0] = 0;
}

// Tier 2: callers `delete` the block by calling this and then the plain
// operator delete (for example 0x00445931).
UnknownParameterBlock::~UnknownParameterBlock()
{
    UnknownFunction4b7890();
    if (field_0x120) {
        operator delete(field_0x120, __FILE__, 141);
        field_0x120 = 0;
        field_0x118 = 0;
    }
    for (int i = 0; i < field_0x540; i++) {
        if (field_0x544[i].type == 2)
            operator delete(field_0x544[i].s, __FILE__, 147);
        field_0x544[i].s = 0;
    }
    field_0x540 = 0;
}

// 0x004b7220 (open through the stream's archives) is a near miss; see
// samples/parameterblocks/ParameterblocksNearMisses.cpp.

// Scans forward for `[name]` and loads the section's non-empty,
// non-comment lines. `lines` must be cleared before the line count and
// capacity: other orders let VC6 keep a zero register for the whole function.
int UnknownParameterBlock::UnknownFunction4b72c0(const char* name)
{
    char stripped[PB_LINE_SIZE];
    char line[PB_LINE_SIZE];
    char header[PB_LINE_SIZE];
    char** lines;
    int capacity;

    if (!field_0x008)
        return 0;
    for (;;) {
        if (!field_0x000->UnknownFunction461aa0(line, PB_LINE_SIZE))
            return 0;
        UnknownFunction4b8360(line, stripped);
        if (stripped[0] == '[') {
            int length = strcspn(stripped + 1, "]\n\r");
            strncpy(header, stripped + 1, length);
            header[length] = 0;
            if (_stricmp(name, header) == 0)
                break;
        }
    }

    if (field_0x12c)
        UnknownFunction4b7890();
    lines = 0;
    field_0x128 = 0;
    capacity = 0;
    int nameLength = strlen(name);
    int length = PB_MIN(PB_LINE_SIZE - 1, nameLength);
    strncpy(field_0x130, name, length);
    field_0x130[length] = 0;
    while (field_0x000->UnknownFunction461aa0(line, PB_LINE_SIZE)) {
        UnknownFunction4b8360(line, stripped);
        if (stripped[0] == '[')
            break;
        if (stripped[0] != 0 && stripped[0] != ';') {
            if (field_0x128 >= capacity) {
                capacity += 20;
                lines = (char**)DebugRealloc(lines, capacity * sizeof(char*), __FILE__, 273);
            }
            lines[field_0x128] = new (__FILE__, 275) char[PB_LINE_SIZE];
            strcpy(lines[field_0x128], stripped);
            field_0x128++;
        }
    }
    if (field_0x128 == 0)
        return 0;
    field_0x12c = new (__FILE__, 285) char*[field_0x128];
    for (int i = 0; i < field_0x128; i++)
        field_0x12c[i] = lines[i];
    operator delete(lines, __FILE__, 290);
    field_0x530 = 0;
    return 1;
}

// Records every new `[header]`: its stream position, mode byte and, once the
// next header (or the end) is seen, its line count.
void UnknownParameterBlock::UnknownFunction4b7560(UnknownParameterStream* stream)
{
    char header[PB_LINE_SIZE];
    char stripped[PB_LINE_SIZE];
    char line[PB_LINE_SIZE];
    int lineIndex = 0;

    while (stream->UnknownFunction461aa0(line, PB_LINE_SIZE)) {
        UnknownFunction4b8360(line, stripped);
        if (stripped[0] == '[') {
            int length = strcspn(stripped + 1, "]\n\r");
            strncpy(header, stripped + 1, length);
            header[length] = 0;
            if (UnknownFunction4b7830(header) == -1) {
                if (field_0x120) {
                    if (field_0x118 >= field_0x11c) {
                        field_0x11c += 10;
                        field_0x120 = (UnknownParameterSection*)DebugRealloc(
                            field_0x120, field_0x11c * sizeof(UnknownParameterSection),
                            __FILE__, 318);
                    }
                } else {
                    field_0x120 = (UnknownParameterSection*)DebugMalloc(
                        10 * sizeof(UnknownParameterSection), __FILE__, 321);
                    field_0x11c = 10;
                }
                strcpy(field_0x120[field_0x118].name, header);
                field_0x120[field_0x118].offset = stream->UnknownFunction461600();
                field_0x120[field_0x118].lineCount = lineIndex;
                field_0x120[field_0x118].mode = stream->UnknownFunction43e9e0();
                if (field_0x118 > 0)
                    field_0x120[field_0x118 - 1].lineCount =
                        lineIndex - field_0x120[field_0x118 - 1].lineCount - 1;
                field_0x118++;
            }
        }
        lineIndex++;
    }
    if (field_0x118 > 0)
        field_0x120[field_0x118 - 1].lineCount =
            lineIndex - field_0x120[field_0x118 - 1].lineCount - 1;
}

void UnknownParameterBlock::UnknownFunction4b77a0(UnknownParameterStream* stream, int offset,
                                                  int index)
{
    field_0x124 = offset;
    field_0x008 = 1;
    field_0x534 = 0;
    field_0x00c = 0;
    field_0x000 = stream;
    if (field_0x120) {
        operator delete(field_0x120, __FILE__, 362);
        field_0x120 = 0;
        field_0x118 = 0;
    }
    stream->UnknownFunction461340(stream->field_0x130 + field_0x124, 0, 0);
    if (index)
        UnknownFunction4b7560(stream);
    field_0x530 = 0;
}

int UnknownParameterBlock::UnknownFunction4b7830(const char* name)
{
    for (int i = 0; i < field_0x118; i++) {
        if (_stricmp(name, field_0x120[i].name) == 0)
            return i;
    }
    return -1;
}

void UnknownParameterBlock::UnknownFunction4b7890()
{
    for (int i = 0; i < field_0x128; i++)
        delete field_0x12c[i];
    delete field_0x12c;
    field_0x128 = 0;
    field_0x12c = 0;
}

// Without an index it rewinds and scans; with one it seeks to the section
// and reads its recorded line count.
int UnknownParameterBlock::UnknownFunction4b78f0(const char* name)
{
    char stripped[PB_LINE_SIZE];
    char line[PB_LINE_SIZE];

    if (!field_0x008)
        return 0;
    if (!field_0x120) {
        field_0x000->UnknownFunction461340(field_0x000->field_0x130 + field_0x124, 0, 0);
        field_0x000->UnknownFunction461d20();
        return UnknownFunction4b72c0(name);
    }
    int index = UnknownFunction4b7830(name);
    if (index == -1)
        return 0;
    field_0x000->UnknownFunction461340(field_0x120[index].offset, 0, 0);
    field_0x000->UnknownFunction43e9b0(field_0x120[index].mode);
    if (field_0x12c)
        UnknownFunction4b7890();
    int count = field_0x120[index].lineCount;
    field_0x12c = new (__FILE__, 459) char*[count];
    field_0x128 = 0;
    int nameLength = strlen(name);
    int length = PB_MIN(PB_LINE_SIZE - 1, nameLength);
    strncpy(field_0x130, name, length);
    field_0x130[length] = 0;
    for (int i = 0; i < count; i++) {
        field_0x000->UnknownFunction461aa0(line, PB_LINE_SIZE);
        UnknownFunction4b8360(line, stripped);
        if (stripped[0] != 0 && stripped[0] != ';') {
            field_0x12c[field_0x128] = new (__FILE__, 471) char[PB_LINE_SIZE];
            strcpy(field_0x12c[field_0x128], stripped);
            field_0x128++;
        }
    }
    field_0x530 = 0;
    return 1;
}

// Looks `key` up among the loaded `key=value` lines, strips one pair of
// quotes and copies at most `size` characters (-1: unbounded). The clamps
// need the length in its own local and the limit first in the min.
int UnknownParameterBlock::UnknownFunction4b7b30(const char* key, char* out, int size)
{
    char text[PB_LINE_SIZE];
    char name[PB_LINE_SIZE];

    for (int i = 0; i < field_0x128; i++) {
        int lineLength = strlen(field_0x12c[i]);
        int length = PB_MIN(PB_LINE_SIZE - 1, lineLength);
        strncpy(text, field_0x12c[i], length);
        text[length] = 0;
        UnknownTokenizer tokens(text);
        char* token = tokens.UnknownFunction515df0("=\n");
        int tokenLength = strlen(token);
        length = PB_MIN(PB_LINE_SIZE - 1, tokenLength);
        strncpy(name, token, length);
        name[length] = 0;
        char* value = tokens.UnknownFunction515df0("\n");
        if (value && _stricmp(name, key) == 0) {
            if (*value == '"')
                value++;
            int last = strlen(value) - 1;
            if (last >= 0 && value[last] == '"')
                value[last] = 0;
            if (size != -1) {
                strncpy(out, value, size);
                int copied = strlen(value);
                out[PB_MIN(size, copied)] = 0;
            } else {
                strcpy(out, value);
            }
            return 1;
        }
    }
    strcpy(out, "");
    return 0;
}

int UnknownParameterBlock::UnknownFunction4b7cf0(const char* key, int* out)
{
    char* end;
    char text[PB_LINE_SIZE];

    if (UnknownFunction4b7b30(key, text, -1)) {
        if (_strnicmp(text, "T", 1) == 0) {
            *out = 1;
            return 1;
        }
        if (_strnicmp(text, "F", 1) == 0) {
            *out = 0;
            return 1;
        }
        if (_stricmp(text, "ON") == 0) {
            *out = 1;
            return 1;
        }
        if (_stricmp(text, "OFF") == 0) {
            *out = 0;
            return 1;
        }
        if (_strnicmp(text, "Y", 1) == 0) {
            *out = 1;
            return 1;
        }
        if (_strnicmp(text, "N", 1) == 0) {
            *out = 0;
            return 1;
        }
        *out = strtol(text, &end, 0);
        return 1;
    }
    return 0;
}

int UnknownParameterBlock::UnknownFunction4b7e70(const char* key, float* out)
{
    char text[PB_LINE_SIZE];

    if (UnknownFunction4b7b30(key, text, -1)) {
        *out = (float)atof(text);
        return 1;
    }
    return 0;
}

int UnknownParameterBlock::UnknownFunction4b7ec0(const char* key, const char* def, char* out,
                                                 int size)
{
    if (UnknownFunction4b7b30(key, out, size))
        return 1;
    strcpy(out, def);
    return 0;
}

int UnknownParameterBlock::UnknownFunction4b7f10(const char* key, int def, int* out)
{
    if (UnknownFunction4b7cf0(key, out))
        return 1;
    *out = def;
    return 0;
}

int UnknownParameterBlock::UnknownFunction4b7f40(const char* key, float def, float* out)
{
    if (UnknownFunction4b7e70(key, out))
        return 1;
    *out = def;
    return 0;
}

// Retail looks the section up twice and uses the first result.
int UnknownParameterBlock::UnknownFunction4b7f70(const char* name)
{
    int index = UnknownFunction4b7830(name);
    if (UnknownFunction4b7830(name) != -1) {
        field_0x000->UnknownFunction461340(field_0x120[index].offset, 0, 0);
        field_0x000->UnknownFunction43e9b0(field_0x120[index].mode);
        field_0x530 = 1;
        field_0x53c = 0;
        return field_0x538 = field_0x120[index].lineCount;
    }
    return 0;
}

int UnknownParameterBlock::UnknownFunction4b8010(char* raw)
{
    char line[PB_LINE_SIZE];
    char stripped[PB_LINE_SIZE];

    if (!field_0x008)
        return 0;
    if (!field_0x530)
        return 0;
    if (field_0x53c > field_0x538 - 1)
        return 0;
    for (int i = 0; i < field_0x540; i++) {
        if (field_0x544[i].type == 2)
            operator delete(field_0x544[i].s, __FILE__, 802);
        field_0x544[i].s = 0;
    }
    field_0x000->UnknownFunction461aa0(line, PB_LINE_SIZE);
    if (raw)
        strcpy(raw, line);
    UnknownFunction4b8360(line, stripped);
    UnknownTokenizer tokens(stripped);
    char* token = tokens.UnknownFunction515df0(",\n");
    field_0x540 = 0;
    while (token) {
        UnknownFunction4b6fb0(token, &field_0x544[field_0x540]);
        token = tokens.UnknownFunction515df0(",\n");
        field_0x540++;
    }
    field_0x53c++;
    return field_0x540;
}

// The int and float getters accept index == count; the string getter does
// not (retail's `jle` and `jl`).
int UnknownParameterBlock::UnknownFunction4b8180(int index, int* out)
{
    if (index > field_0x540)
        return 0;
    UnknownFunction4b7040(field_0x544[index], out);
    return 1;
}

int UnknownParameterBlock::UnknownFunction4b81c0(int index, float* out)
{
    if (index > field_0x540)
        return 0;
    UnknownFunction4b7080(field_0x544[index], out);
    return 1;
}

int UnknownParameterBlock::UnknownFunction4b8200(int index, char* out)
{
    if (index >= field_0x540)
        return 0;
    UnknownFunction4b70c0(field_0x544[index], out);
    return 1;
}

int UnknownParameterBlock::UnknownFunction4b8240(char* out, int size)
{
    int total = 0;
    char* cursor = out;

    for (int i = 0; i < field_0x118; i++) {
        int length = strlen(field_0x120[i].name) + 1;
        if (total + length + 1 >= size)
            break;
        total += length;
        strcpy(cursor, field_0x120[i].name);
        strcat(cursor, "?");
        cursor += length;
    }
    out[total] = 0;
    return total;
}

// Drops blanks outside brackets and quotes. The declaration order puts
// `length` in the dead `in` slot, as retail does.
void UnknownParameterBlock::UnknownFunction4b8360(const char* in, char* out)
{
    int count = 0;
    int quoted = 0;
    int depth = 0;
    int length = strlen(in);

    *out = 0;
    if (length) {
        for (int i = 0; i < length; i++) {
            char c = in[i];
            switch (c) {
            case '[':
                depth++;
                break;
            case '"':
                quoted = 1 - quoted;
                break;
            case ']':
                depth--;
                break;
            case '\t':
            case '\n':
            case '\r':
            case ' ':
                if (depth <= 0 && !quoted)
                    continue;
                break;
            }
            out[count++] = c;
        }
    }
    out[count] = 0;
}
