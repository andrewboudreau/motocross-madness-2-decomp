// FileStream.cpp -- the buffered file stream (UnknownTextureStream, declared in
// TextureMap.h), 0x00460d10..0x00461d57, after the FastMath helpers
// (samples/physics/helpers/FastMath.cpp) and before the shadow fill code 0x00461d60.
// No __FILE__ literal or RTTI: the file name is ours (tier 3), as are the names.
//
// A stream reads a file directly or a slice (+0x130, +0x04) of an inner stream (+0x1c);
// every method first follows +0x1c to the innermost stream (tail recursion, which VC6
// turns into a loop). Files written with 0x00461b90 start with "FAOE" and are encoded
// byte by byte with a running key seeded from the file name.
//
// This file holds the methods that match retail byte for byte. The near misses
// (constructor, header check, open, seek, read, write, write-header) stay in
// samples/io/FileStream.cpp with their notes; docs/FILESTREAM.md has the table.

#include <io.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "TextureMap.h"
#include "ResourceManager.h"

// The stream that holds the file: the innermost one.
static inline UnknownTextureStream* InnermostStream(UnknownTextureStream* stream)
{
    while (stream->field_0x1c) {
        stream = stream->field_0x1c;
    }
    return stream;
}

// 0x00460d60
UnknownTextureStream::~UnknownTextureStream()
{
    if (field_0x14) {
        fclose(field_0x14);
    }
}

// 0x00460d70
UnknownTextureStream* UnknownTextureStream::UnknownFunction460d70(const char* name)
{
    if (field_0x12c) {
        ResourceItem* item = field_0x12c->UnknownFunction4e9360(name, 0);
        if (item) {
            field_0x130 = item->field_0x18;
            field_0x04 = item->field_0x1c;
            return item->field_0x14;
        }
    }
    return 0;
}

// 0x00460e70. The NULL test guards the rest, so its message comes last.
int UnknownTextureStream::UnknownFunction460e70(const char* path)
{
    char message[0x184];

    if (field_0x14) {
        fclose(field_0x14);
    }
    if (path) {
        if (*path && (field_0x14 = fopen(path, "rb")) != 0) {
            if (!UnknownFunction460db0()) {
                fclose(field_0x14);
                field_0x14 = 0;
                return 0;
            }
            if (field_0x14) {
                fclose(field_0x14);
                field_0x14 = 0;
            }
            return 1;
        }
        sprintf(message, "Error opening %s.\n", path);
        return 0;
    }
    sprintf(message, "filename is NULL!\n");
    return 0;
}

// 0x00461310
int UnknownTextureStream::UnknownFunction461310(int unused)
{
    FILE* file = InnermostStream(this)->field_0x14;
    return _setmode(file->_file, _O_BINARY);
}

// 0x00461600
int UnknownTextureStream::UnknownFunction461600()
{
    UnknownTextureStream* stream = InnermostStream(this);
    int position = ftell(stream->field_0x14) + (stream->field_0x124 - stream->field_0x128);
    if (position >= stream->field_0x10) {
        return position - stream->field_0x10;
    }
    return -1;
}

// 0x00461980: the next byte, decoded with the running key in an encoded file; -1 at
// the end. The decode reads the byte into a temporary and the key straight from the
// field (no key temporary): that is the only spelling under which VC6 loads the key
// first and copies it, as retail does; naming the key makes it copy the byte instead.
// inline_depth(0) keeps the UnknownFunction430ff0 call out of line, as retail has it.
#pragma inline_depth(0)
int UnknownTextureStream::UnknownFunction461980()
{
    if (field_0x1c) {
        if (field_0x04 > 0 && UnknownFunction461600() >= field_0x130 + field_0x04) {
            return -1;
        }
        if (field_0x1c->UnknownFunction430ff0()) {
            return -1;
        }
        return field_0x1c->UnknownFunction461980();
    }
    if (field_0x124 == field_0x128) {
        int count = fread(field_0x21, 1, 0x100, field_0x14);
        if (count != 0x100 && (count == 0 || (field_0x14->_flag & _IOERR))) {
            return -1;
        }
        field_0x124 = field_0x21;
        field_0x128 = field_0x124 + count;
    }
    char* p = field_0x124;
    if (p < field_0x128) {
        if (field_0x08) {
            char c = *p;
            char decoded = field_0x01 ^ c;
            field_0x01 += c;
            *p = decoded;
            return (unsigned char)*field_0x124++;
        }
        return (unsigned char)*field_0x124++;
    }
    return -1;
}
#pragma inline_depth()

// 0x00461a60
int UnknownTextureStream::UnknownFunction461a60(int c)
{
    UnknownTextureStream* stream = InnermostStream(this);
    char byte = (char)c;
    if (!stream->UnknownFunction4618e0(&byte, 1, 1)) {
        return -1;
    }
    return c;
}

// 0x00461aa0: reads a line of at most size - 1 characters; "\r\n" ends it as "\n".
#pragma inline_depth(0)
int UnknownTextureStream::UnknownFunction461aa0(char* buffer, int size)
{
    if (field_0x1c) {
        if (field_0x04 > 0 && UnknownFunction461600() >= field_0x130 + field_0x04) {
            return 0;
        }
        if (field_0x1c->UnknownFunction430ff0()) {
            return 0;
        }
        if (field_0x04 > 0) {
            int left = field_0x130 - UnknownFunction461600() + field_0x04 + 1;
            if (left < size) {
                size = left;
            }
        }
        return field_0x1c->UnknownFunction461aa0(buffer, size);
    }
    char* p = buffer;
    *p = (char)UnknownFunction461980();
    if (*p == (char)-1) {
        return 0;
    }
    int count = 1;
    while (count < size - 1) {
        if (*p == '\n') {
            if (p[-1] == '\n' && count > 1) {
                *p = 0;
                return (int)buffer;
            }
            break;
        }
        if (*p == '\r') {
            *p = '\n';
        }
        p++;
        *p = (char)UnknownFunction461980();
        if (*p == (char)-1) {
            *p = 0;
            break;
        }
        count++;
    }
    p[1] = 0;
    return (int)buffer;
}
#pragma inline_depth()

// The modification time and size of the stream's file; _fstat's result.
static inline int FileTimes(UnknownTextureStream* stream, int* time, int* size)
{
    struct _stat status;
    int result = _fstat(stream->field_0x14->_file, &status);
    if (result) {
        *time = 0;
        *size = 0;
    } else {
        *time = status.st_mtime;
        *size = status.st_size;
    }
    return result;
}

// 0x00461cb0. Writing the _fstat call inline keeps the recursion a call;
// through FileTimes VC6 turns it into the loop retail has.
int UnknownTextureStream::UnknownFunction461cb0(int* time, int* size)
{
    if (!time || !size) {
        return -1;
    }
    if (field_0x1c) {
        return field_0x1c->UnknownFunction461cb0(time, size);
    }
    return FileTimes(this, time, size);
}

// 0x00461d20
void UnknownTextureStream::UnknownFunction461d20()
{
    if (field_0x1c) {
        field_0x1c->UnknownFunction461d20();
        return;
    }
    field_0x01 = field_0x03;
}

// 0x00461d40: fprintf.
int UnknownFunction461d40(FILE* file, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    return vfprintf(file, format, arguments);
}
