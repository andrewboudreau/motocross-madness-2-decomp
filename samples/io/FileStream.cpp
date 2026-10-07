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
#include <sys/types.h>
#include <sys/stat.h>

#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/ResourceManager.h"

// 0x0045c6a0 (cdecl): fwrite that encodes the bytes with the running key *key.
int UnknownFunction45c6a0(const void* buffer, int size, int count, FILE* file, char* key);
// 0x0045c7b0 (cdecl): fread that decodes the bytes with the running key *key.
int UnknownFunction45c7b0(unsigned char* buffer, int size, int count, FILE* file, unsigned char* key);

// The stream that holds the file: the innermost one.
static inline UnknownTextureStream* InnermostStream(UnknownTextureStream* stream)
{
    while (stream->field_0x1c) {
        stream = stream->field_0x1c;
    }
    return stream;
}

// 0x00460d10. Near miss: retail loads the argument into edx on entry and stores
// it in sequence; every ordering tried here loads it just before its store.
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

// Whether the next four bytes are the "FAOE" header. As an inline helper the
// failure path of the four tests falls through, as in retail.
static inline int HeaderMatches(UnknownTextureStream* stream)
{
    if (stream->UnknownFunction461980() != stream->field_0x02
        || stream->UnknownFunction461980() != stream->field_0x00
        || stream->UnknownFunction461980() != stream->field_0x20
        || stream->UnknownFunction461980() != stream->field_0x18) {
        return 0;
    }
    return 1;
}

// 0x00460db0: 1 when the file starts with the header; the file is closed when a
// seek fails. Near miss: retail keeps the first fseek-failure block inline and
// cross-jumps only from its fclose call into the second one (the two load +0x14
// into different registers), and stores field_0x08 = 0 as an immediate; VC6 here
// merges the whole first block into the second and stores fseek's zero result.
// The plain ||, &&, goto, nested, result-variable and seek-helper shapes compile
// to the same or worse code.
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
    if (!HeaderMatches(this)) {
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

// 0x00460f50: opens `path` with fopen `mode` ("r"/"a" read, "w" write) or as a
// slice of an archive (0x00460d70); `encoded` asks for the "FAOE" encoding (a
// write mode gains '+'). The messages go to a local buffer that is never shown.
// The buffer is 0x184 bytes, as in 0x00460e70; _splitpath reuses it for the name.
// Near miss (about 30%, 933 vs 957 bytes): VC6 here stores access[0] as well
// (retail zeroes only bytes 1..7), shares the fclose/sprintf tails of error
// blocks that retail keeps apart, and swaps dl/cl in the key loop as in
// 0x00461b90; the rest matches up to branch targets.
int UnknownTextureStream::UnknownFunction460f50(const char* path, const char* mode, int encoded)
{
    char drive[4];
    char buffer[0x184];
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
    field_0x1c = UnknownFunction460d70(path);
    if (field_0x1c) {
        field_0x1c->field_0x01 = field_0x1c->field_0x03;
        UnknownFunction461310(0x8000);
        UnknownFunction461340(field_0x130, SEEK_SET, 0);
        return 1;
    }
    if (*mode == 'r' || *mode == 'a') {
        if (!encoded && UnknownFunction460e70(path)) {
            field_0x08 = 1;
            if (field_0x12c && (field_0x1c = field_0x12c->UnknownFunction4e9030(path, 0)) != 0) {
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
                    if (!UnknownFunction460db0()) {
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
                        field_0x01 = ((field_0x01 + 3) ^ *p++) + field_0x01;
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
        char access[8] = {0};
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
                if (encoded && !UnknownFunction461b90(path)) {
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
    UnknownFunction461310(0x8000);
    UnknownFunction461340(0, SEEK_SET, field_0x08);
    return 1;
}

// 0x00461310
int UnknownTextureStream::UnknownFunction461310(int unused)
{
    FILE* file = InnermostStream(this)->field_0x14;
    return _setmode(file->_file, _O_BINARY);
}

// Restarts the key of `stream`: 0x00461d20 inlined one level.
static inline void RestartKey(UnknownTextureStream* stream)
{
    if (stream->field_0x1c)
        stream->field_0x1c->UnknownFunction461d20();
    else
        stream->field_0x01 = stream->field_0x03;
}

// 0x00461340: seeks to `offset` (origin as fseek). An encoded file cannot
// fseek into the middle of its key stream, so with `flag` it rewinds (or keeps
// the decoded position +0x0c) and decodes forward through a scratch buffer.
// Near miss (about 29%, 700 vs 702 bytes): the control flow and the inner
// loop match, but retail keeps `offset` in ebx and `origin` in ebp where VC6
// here swaps them, so most instructions differ by register. The 0x418-byte
// buffer reproduces retail's 0x41c-byte frame; `result = 0` inside the loop
// reproduces retail's store (the source was likely tail recursion).
int UnknownTextureStream::UnknownFunction461340(int offset, int origin, int flag)
{
    unsigned char buffer[0x418];
    int result = 0;
    UnknownTextureStream* stream = this;
    while (stream->field_0x1c) {
        result = 0;
        flag = flag && offset != stream->field_0x130;
        stream = stream->field_0x1c;
    }
    if (origin == SEEK_SET && offset == stream->UnknownFunction461600()) {
        if (stream->field_0x08 && !flag) {
            RestartKey(stream);
            stream->field_0x0c = offset;
        }
        stream->field_0x124 = stream->field_0x128;
        return 0;
    }
    if (stream->field_0x08 && flag) {
        if ((origin == SEEK_END && offset != 0) || (origin == SEEK_CUR && offset < 0))
            return 1;
        int position = origin != SEEK_CUR && stream->field_0x0c < offset ? stream->field_0x0c : 0;
        if (origin == SEEK_SET || origin == SEEK_END) {
            result = fseek(stream->field_0x14, stream->field_0x10 + position, SEEK_SET);
            if (result) {
                fclose(stream->field_0x14);
                stream->field_0x14 = 0;
                return result;
            }
            stream->field_0x01 = stream->field_0x03;
        }
        if (origin == SEEK_SET || origin == SEEK_CUR) {
            int skip = offset - position;
            if (skip) {
                int blocks = skip / 0x400;
                int tail = skip % 0x400;
                for (; blocks; blocks--) {
                    if (!UnknownFunction45c7b0(buffer, 0x400, 1, stream->field_0x14, (unsigned char*)&stream->field_0x01))
                        return -1;
                }
                if (tail && !UnknownFunction45c7b0(buffer, tail, 1, stream->field_0x14, (unsigned char*)&stream->field_0x01))
                    return -1;
            }
        } else if (origin == SEEK_END) {
            while (UnknownFunction45c7b0(buffer, 0x400, 1, stream->field_0x14, (unsigned char*)&stream->field_0x01))
                ;
            if (ferror(stream->field_0x14))
                return -1;
            return !feof(stream->field_0x14);
        }
    } else {
        if (stream->field_0x08 && !flag) {
            RestartKey(stream);
            if (origin == SEEK_SET)
                stream->field_0x0c = offset;
        }
        result = fseek(stream->field_0x14, stream->field_0x10 + offset, origin);
        if (result) {
            fclose(stream->field_0x14);
            stream->field_0x14 = 0;
        }
    }
    stream->field_0x124 = stream->field_0x128;
    return result;
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

// The running-key decode used by 0x00461640.
static inline void DecodeBytes(UnknownTextureStream* stream, char* p, int count)
{
    for (int i = 0; i < count; i++) {
        char key = stream->field_0x01;
        char c = *p ^ key;
        stream->field_0x01 = key + *p;
        *p++ = c;
    }
}

// 0x00461640: reads `count` items of `size` bytes, from the buffer first, and
// returns how many it read. Near miss (about 30%): retail loads the key before
// the byte in the decode loop, keeps the advanced buffer pointer in ecx and
// tail-merges the two encoded 45c7b0 calls; VC6 here does none of these.
int UnknownTextureStream::UnknownFunction461640(void* buffer, int size, int count)
{
    UnknownTextureStream* stream = InnermostStream(this);
    char* p = stream->field_0x124;
    if (p != stream->field_0x128) {
        int available = stream->field_0x128 - p;
        int total = size * count;
        if (total <= available) {
            if (stream->field_0x08) {
                DecodeBytes(stream, p, total);
            }
            memcpy(buffer, stream->field_0x124, total);
            stream->field_0x124 += total;
            return count;
        }
        if (stream->field_0x08) {
            DecodeBytes(stream, p, available);
        }
        memcpy(buffer, stream->field_0x124, available);
        stream->field_0x124 = stream->field_0x128;
        buffer = (char*)buffer + available;
        int part = (unsigned)available % (unsigned)size;
        if (part) {
            if (stream->field_0x08) {
                if (!UnknownFunction45c7b0((unsigned char*)buffer, part, 1, stream->field_0x14, (unsigned char*)&stream->field_0x01)) {
                    return 0;
                }
            } else if (!fread(buffer, part, 1, stream->field_0x14)) {
                return (unsigned)available / (unsigned)size;
            }
            if (part == total - available) {
                return count;
            }
            buffer = (char*)buffer + part;
        }
        if (stream->field_0x08) {
            if (!UnknownFunction45c7b0((unsigned char*)buffer, size, count - (unsigned)(part + available) / (unsigned)size, stream->field_0x14, (unsigned char*)&stream->field_0x01)) {
                return count - 1;
            }
            return count;
        }
        int left = count - (unsigned)(part + available) / (unsigned)size;
        int read = fread(buffer, size, left, stream->field_0x14);
        if (read == left) {
            return count;
        }
        return count - read;
    }
    if (stream->field_0x08) {
        if (!UnknownFunction45c7b0((unsigned char*)buffer, size, count, stream->field_0x14, (unsigned char*)&stream->field_0x01)) {
            return count - 1;
        }
        return count;
    }
    int total = size * count;
    int done = 0;
    int blocks = total / 0x400;
    int tail = total % 0x400;
    char* out = (char*)buffer;
    while (blocks) {
        int read = fread(out, 1, 0x400, stream->field_0x14);
        if (read != 0x400) {
            return (unsigned)(done + read) / (unsigned)size;
        }
        blocks--;
        out += read;
        done += read;
    }
    if (tail) {
        int read = fread(out, 1, tail, stream->field_0x14);
        if (read != tail) {
            return (unsigned)(done + read) / (unsigned)size;
        }
    }
    return count;
}

// 0x004618e0. Near miss: retail loads every argument into a register before the
// pushes; VC6 here pushes from memory.
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

// 0x00461980: the next byte, decoded with the running key in an encoded file; -1 at
// the end. Near miss (98%): retail loads the key before the byte; every ordering of
// the decode tried here loads the byte first. inline_depth(0) keeps the
// UnknownFunction430ff0 call out of line, as retail has it.
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
            char key = field_0x01;
            char c = *p ^ key;
            field_0x01 = key + *p;
            *p = c;
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

// Writes the "FAOE" header; 0 when a write fails. As an inline helper the failure
// path falls through, as in retail.
static inline int PutHeader(UnknownTextureStream* stream)
{
    if (stream->UnknownFunction461a60(stream->field_0x02) == -1
        || stream->UnknownFunction461a60(stream->field_0x00) == -1
        || stream->UnknownFunction461a60(stream->field_0x20) == -1
        || stream->UnknownFunction461a60(stream->field_0x18) == -1) {
        return 0;
    }
    return 1;
}

// 0x00461b90: writes the header and seeds the key from the file name (name plus
// extension). Near miss (85%): retail stores the 0xfa seed inside the inlined
// strcat, sets up the name pointer after strlen and holds key + 3 in dl and the
// name byte in cl; VC6 here swaps those two registers and schedules the seed and
// pointer later. The index, countdown, operand-order and helper shapes of the
// loop do not change it.
int UnknownTextureStream::UnknownFunction461b90(const char* path)
{
    char drive[_MAX_DRIVE];
    char directory[_MAX_DIR];
    char name[_MAX_PATH];
    char extension[_MAX_EXT];

    UnknownTextureStream* stream = InnermostStream(this);
    if (!PutHeader(stream)) {
        return 0;
    }
    stream->field_0x08 = 1;
    stream->field_0x10 = 4;
    _splitpath(path, drive, directory, name, extension);
    strcat(name, extension);
    stream->field_0x01 = (char)0xfa;
    int length = strlen(name);
    char* p = name;
    for (int i = 0; i < length; i++) {
        stream->field_0x01 = ((stream->field_0x01 + 3) ^ *p++) + stream->field_0x01;
    }
    stream->field_0x03 = stream->field_0x01;
    return 1;
}

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
