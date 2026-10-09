// FileStream.cpp -- near misses of the buffered file stream (UnknownTextureStream,
// declared in src/reconstructed/TextureMap.h), 0x00460d10..0x00461d57. The eleven
// methods that match retail are in src/reconstructed/FileStream.cpp; these seven
// (constructor, header check, open, seek, read, write, write-header) stay here with
// their notes. The bindings file covers both.
//
// A stream reads a file directly or a slice (+0x130, +0x04) of an inner stream (+0x1c);
// every method first follows +0x1c to the innermost stream (tail recursion, which VC6
// turns into a loop). Files written with 0x00461b90 start with "FAOE" and are encoded
// byte by byte with a running key seeded from the file name.
//
// What moved the scores, and what did not:
// - The byte decode `c = *p; decoded = key ^ c; key += c; *p = decoded;` with the key
//   read straight from the field (no key temporary) makes VC6 load the key first and
//   copy it, as retail does; this matched 0x00461980 and is the DecodeBytes shape.
// - In the seed loop of 0x00461b90, `char c = *p++;` before the key update gives
//   retail's loop body (85% -> 90%); the same spelling in 0x00460f50 regresses it,
//   where retail keeps `*p++` inside the expression.
// - 0x00461b90: retail stores the 0xfa seed inside the inlined strcat and sets up the
//   name pointer (lea esi) right after strlen; no statement order, helper, index or
//   countdown loop shape moves those two.
//   Storing the seed before _splitpath matches the lea esi but moves the store ahead
//   of the call; storing it after strlen also leaves 2 differences.
// - 0x004618e0: retail loads buffer/size/count into registers before the pushes;
//   VC6 here pushes them from the stack. Register locals, a helper for the fseek
//   block and an unsigned count do not change it.
//   A `FILE* file = stream->field_0x14;` local in the fwrite tail does schedule all
//   loads before the pushes, but in ecx/edx/eax where retail uses edx/eax/ecx
//   (a register tie-break); not kept.
// - 0x00460d10: a member-initialiser list stores in declaration order (62% with every
//   field in it), but retail's order is the body order below with the argument
//   loaded into edx at entry; neither form, a local copy nor chained assignments
//   reproduce the entry load.
// - 0x00461340: retail loads the decoded position (+0x0c) once and keeps the next
//   pointer in a different register; the `position` ternary, an explicit if/else
//   and assigning position before the zero test all give the same or worse code.
// - /G6 makes every one of these worse.

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

// 0x00460d10. Near miss (22%): retail loads the argument into edx on entry and stores
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
// seek fails. Near miss (16%): retail keeps the first fseek-failure block inline and
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
// loop match, but retail keeps the next pointer and the zero in other registers
// and loads the decoded position +0x0c once, so most instructions differ by
// register. The 0x418-byte buffer reproduces retail's 0x41c-byte frame;
// `result = 0` inside the loop reproduces retail's store (the source was likely
// tail recursion).
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

// The running-key decode used by 0x00461640 (the 0x00461980 shape).
static inline void DecodeBytes(UnknownTextureStream* stream, char* p, int count)
{
    for (int i = 0; i < count; i++) {
        char c = *p;
        char decoded = stream->field_0x01 ^ c;
        stream->field_0x01 += c;
        *p++ = decoded;
    }
}

// 0x00461640: reads `count` items of `size` bytes, from the buffer first, and
// returns how many it read. Near miss (about 32%): retail keeps the advanced
// buffer pointer in ecx and tail-merges the two encoded 45c7b0 calls; VC6 here
// does neither.
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

// 0x004618e0. Near miss (88%): retail loads every argument into a register before
// the pushes; VC6 here pushes from memory.
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
// extension). Near miss (90%): the loop body matches with the name byte in a
// temporary; retail stores the 0xfa seed inside the inlined strcat and sets up
// the name pointer right after strlen, where VC6 here schedules both later.
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
        char c = *p++;
        stream->field_0x01 = ((stream->field_0x01 + 3) ^ c) + stream->field_0x01;
    }
    stream->field_0x03 = stream->field_0x01;
    return 1;
}
