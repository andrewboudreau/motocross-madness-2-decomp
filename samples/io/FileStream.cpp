// FileStream.cpp -- the buffered file stream (UnknownTextureStream, declared in
// src/reconstructed/TextureMap.h), 0x00460d10..0x00461d57, after the FastMath helpers
// (samples/physics/helpers/FastMath.cpp) and before the shadow fill code 0x00461d60.
// No __FILE__ literal or RTTI: the file name is ours (tier 3), as are the names.
//
// A stream reads a file directly or a slice (+0x130, +0x04) of an inner stream (+0x1c);
// every method first follows +0x1c to the innermost stream (tail recursion, which VC6
// turns into a loop). Files written with 0x00461b90 start with "FAOE" and are encoded
// byte by byte with a running key seeded from the file name.

#include <io.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/ResourceManager.h"

// 0x0045c6a0 (cdecl): fwrite that encodes the bytes with the running key *key.
int UnknownFunction45c6a0(const void* buffer, int size, int count, FILE* file, char* key);

// The stream that holds the file: the innermost one.
static inline UnknownTextureStream* InnermostStream(UnknownTextureStream* stream)
{
    while (stream->field_0x1c) {
        stream = stream->field_0x1c;
    }
    return stream;
}

// 0x00460d10
UnknownTextureStream::UnknownTextureStream(int a)
{
    field_0x08 = 0;
    field_0x10 = 0;
    field_0x01 = 0;
    field_0x14 = 0;
    field_0x03 = 0;
    field_0x1c = 0;
    field_0x0c = 0;
    field_0x130 = 0;
    field_0x04 = 0;
    field_0x02 = 'F';
    field_0x00 = 'A';
    field_0x20 = 'O';
    field_0x18 = 'E';
    field_0x12c = (UnknownResourceManager*)a;
    field_0x124 = field_0x21;
    field_0x128 = field_0x21;
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

// 0x00460db0
int UnknownTextureStream::UnknownFunction460db0()
{
    if (fseek(field_0x14, 0, SEEK_SET)) {
        fclose(field_0x14);
        field_0x14 = 0;
        return 0;
    }
    int encoded = field_0x08;
    field_0x124 = field_0x128;
    field_0x08 = 0;
    if (UnknownFunction461980() != field_0x02 || UnknownFunction461980() != field_0x00
        || UnknownFunction461980() != field_0x20 || UnknownFunction461980() != field_0x18) {
        field_0x08 = encoded;
        return 0;
    }
    if (fseek(field_0x14, 4, SEEK_SET)) {
        fclose(field_0x14);
        field_0x14 = 0;
        return 0;
    }
    field_0x08 = encoded;
    field_0x124 = field_0x128;
    return 1;
}

// 0x00460e70
int UnknownTextureStream::UnknownFunction460e70(const char* path)
{
    char message[0x184];

    if (field_0x14) {
        fclose(field_0x14);
    }
    if (!path) {
        sprintf(message, "filename is NULL!\n");
        return 0;
    }
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

// 0x004618e0
int UnknownTextureStream::UnknownFunction4618e0(const void* buffer, int size, int count)
{
    UnknownTextureStream* stream = InnermostStream(this);
    if (stream->field_0x08) {
        long position = ftell(stream->field_0x14);
        if (fseek(stream->field_0x14, position, SEEK_SET)) {
            fclose(stream->field_0x14);
            stream->field_0x14 = 0;
        }
        if (!UnknownFunction45c6a0(buffer, size, count, stream->field_0x14, &stream->field_0x01)) {
            return count - 1;
        }
        return count;
    }
    return fwrite(buffer, size, count, stream->field_0x14);
}

// 0x00461980
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
        field_0x128 = field_0x21 + count;
    }
    if (field_0x124 >= field_0x128) {
        return -1;
    }
    if (field_0x08) {
        char key = field_0x01;
        field_0x01 = key + *field_0x124;
        *field_0x124 ^= key;
        return (unsigned char)*field_0x124++;
    }
    return (unsigned char)*field_0x124++;
}

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
    size--;
    while (count < size) {
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

// 0x00461b90
int UnknownTextureStream::UnknownFunction461b90(const char* path)
{
    char drive[_MAX_DRIVE];
    char directory[_MAX_DIR];
    char name[_MAX_PATH];
    char extension[_MAX_EXT];

    UnknownTextureStream* stream = InnermostStream(this);
    if (stream->UnknownFunction461a60(stream->field_0x02) == -1
        || stream->UnknownFunction461a60(stream->field_0x00) == -1
        || stream->UnknownFunction461a60(stream->field_0x20) == -1
        || stream->UnknownFunction461a60(stream->field_0x18) == -1) {
        return 0;
    }
    stream->field_0x08 = 1;
    stream->field_0x10 = 4;
    _splitpath(path, drive, directory, name, extension);
    strcat(name, extension);
    stream->field_0x01 = (char)0xfa;
    int length = strlen(name);
    for (int i = 0; i < length; i++) {
        stream->field_0x01 = ((stream->field_0x01 + 3) ^ name[i]) + stream->field_0x01;
    }
    stream->field_0x03 = stream->field_0x01;
    return 1;
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
