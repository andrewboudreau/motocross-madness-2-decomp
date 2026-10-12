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
// (header check, seek, read, write, write-header) stay in
// samples/io/FileStream.cpp with their notes; docs/FILESTREAM.md has the table.

#include <io.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// 0x00460d10. The buffer pointer is named before the argument store: only then does VC6
// load the argument at entry (edx) and keep the buffer address in ecx for both pointer
// stores, as retail does.
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
    char* buffer = field_0x21;
    field_0x12c = (UnknownResourceManager*)a;
    field_0x124 = buffer;
    field_0x128 = buffer;
}

// 0x00460d60
UnknownTextureStream::~UnknownTextureStream()
{
    if (field_0x14) {
        fclose(field_0x14);
    }
}

// 0x00460d70
UnknownTextureStream* UnknownTextureStream::FindArchive(const char* name)
{
    if (field_0x12c) {
        ResourceItem* item = field_0x12c->FindItem(name, 0);
        if (item) {
            field_0x130 = item->offset;
            field_0x04 = item->size;
            return item->archive;
        }
    }
    return 0;
}

// 0x00460e70. The NULL test guards the rest, so its message comes last.
int UnknownTextureStream::IsValidFile(const char* path)
{
    char message[0x184];

    if (field_0x14) {
        fclose(field_0x14);
    }
    if (path) {
        if (*path && (field_0x14 = fopen(path, "rb")) != 0) {
            if (!CheckHeader()) {
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

// 0x00460f50: opens `path` with fopen `mode` ("r"/"a" read, "w" write) or as a
// slice of an archive (0x00460d70); `encoded` asks for the "FAOE" encoding (a
// write mode gains '+'). The messages go to a local buffer that is never shown;
// _splitpath reuses the read branch's buffer for the name. Three source forms
// matter. Each branch declares its own message buffer: with one buffer for the
// function VC6 cross-jumps the write branch's header-failure block and the two
// "filename is NULL!" blocks into their read-branch twins. The key loop reads
// the byte into a `char` first (as 0x00461b90 does). Retail never stores
// access[0], which strncpy overwrites: only bytes 1..7 are cleared.
int UnknownTextureStream::Open(const char* path, const char* mode, int encoded)
{
    char drive[4];
    char extension[0x100];
    char directory[0x100];

    if (field_0x14) {
        fclose(field_0x14);
        field_0x08 = 0;
        field_0x10 = 0;
        field_0x01 = 0;
        field_0x14 = 0;
        field_0x03 = 0;
        field_0x1c = 0;
    }
    field_0x1c = FindArchive(path);
    if (field_0x1c) {
        field_0x1c->field_0x01 = field_0x1c->field_0x03;
        SetBufferSize(0x8000);
        Seek(field_0x130, SEEK_SET, 0);
        return 1;
    }
    if (*mode == 'r' || *mode == 'a') {
        char buffer[0x184];
        if (!encoded && IsValidFile(path)) {
            field_0x08 = 1;
            if (field_0x12c && (field_0x1c = field_0x12c->AddArchive(path, 0)) != 0) {
                return 1;
            }
        }
        if (path) {
            if (*path && (field_0x14 = fopen(path, mode)) != 0) {
                if (!encoded && !field_0x08) {
                    if (fseek(field_0x14, 0, SEEK_SET)) {
                        fclose(field_0x14);
                        field_0x14 = 0;
                        sprintf(buffer, "Fseek failed for %s.\n", path);
                        return 0;
                    }
                } else {
                    if (!CheckHeader()) {
                        fclose(field_0x14);
                        field_0x14 = 0;
                        sprintf(buffer, "%s NOT an encrypted file, but expected to be.\n", path);
                        return 0;
                    }
                    field_0x08 = 1;
                    field_0x10 = 4;
                    _splitpath(path, drive, directory, buffer, extension);
                    strcat(buffer, extension);
                    field_0x01 = (char)0xfa;
                    int length = strlen(buffer);
                    char* p = buffer;
                    for (int i = 0; i < length; i++) {
                        char c = *p++;
                        field_0x01 = ((field_0x01 + 3) ^ c) + field_0x01;
                    }
                    field_0x03 = field_0x01;
                }
            } else {
                sprintf(buffer, "Error opening %s.\n", path);
                return 0;
            }
        } else {
            sprintf(buffer, "filename is NULL!\n");
            return 0;
        }
    } else if (*mode == 'w') {
        char access[8];
        char buffer[0x184];
        memset(access + 1, 0, sizeof(access) - 1);
        strncpy(access, mode, 8);
        if (encoded && mode[1] != '+') {
            for (int i = 7; i > 1; i--) {
                access[i] = access[i - 1];
            }
            access[1] = '+';
        }
        if (path) {
            if (*path && (field_0x14 = fopen(path, access)) != 0) {
                if (fseek(field_0x14, 0, SEEK_SET)) {
                    fclose(field_0x14);
                    field_0x14 = 0;
                    sprintf(buffer, "Fseek failed for %s.\n", path);
                    return 0;
                }
                if (encoded && !WriteHeader(path)) {
                    fclose(field_0x14);
                    field_0x14 = 0;
                    sprintf(buffer, "Write of encrypted file header failed for %s.\n", path);
                    return 0;
                }
            } else {
                sprintf(buffer, "Error opening %s.\n", path);
                return 0;
            }
        } else {
            sprintf(buffer, "filename is NULL!\n");
            return 0;
        }
    }
    SetBufferSize(0x8000);
    Seek(0, SEEK_SET, field_0x08);
    return 1;
}

// 0x00461310
int UnknownTextureStream::SetBufferSize(int unused)
{
    FILE* file = InnermostStream(this)->field_0x14;
    return _setmode(file->_file, _O_BINARY);
}

// 0x00461600
int UnknownTextureStream::Tell()
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
int UnknownTextureStream::ReadByte()
{
    if (field_0x1c) {
        if (field_0x04 > 0 && Tell() >= field_0x130 + field_0x04) {
            return -1;
        }
        if (field_0x1c->UnknownFunction430ff0()) {
            return -1;
        }
        return field_0x1c->ReadByte();
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
int UnknownTextureStream::WriteByte(int c)
{
    UnknownTextureStream* stream = InnermostStream(this);
    char byte = (char)c;
    if (!stream->Write(&byte, 1, 1)) {
        return -1;
    }
    return c;
}

// 0x00461aa0: reads a line of at most size - 1 characters; "\r\n" ends it as "\n".
#pragma inline_depth(0)
int UnknownTextureStream::ReadLine(char* buffer, int size)
{
    if (field_0x1c) {
        if (field_0x04 > 0 && Tell() >= field_0x130 + field_0x04) {
            return 0;
        }
        if (field_0x1c->UnknownFunction430ff0()) {
            return 0;
        }
        if (field_0x04 > 0) {
            int left = field_0x130 - Tell() + field_0x04 + 1;
            if (left < size) {
                size = left;
            }
        }
        return field_0x1c->ReadLine(buffer, size);
    }
    char* p = buffer;
    *p = (char)ReadByte();
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
        *p = (char)ReadByte();
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
int UnknownTextureStream::GetFileTimes(int* time, int* size)
{
    if (!time || !size) {
        return -1;
    }
    if (field_0x1c) {
        return field_0x1c->GetFileTimes(time, size);
    }
    return FileTimes(this, time, size);
}

// 0x00461d20
void UnknownTextureStream::ResetTextMode()
{
    if (field_0x1c) {
        field_0x1c->ResetTextMode();
        return;
    }
    field_0x01 = field_0x03;
}

// 0x00461d40: fprintf.
int FilePrintf(FILE* file, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    return vfprintf(file, format, arguments);
}
