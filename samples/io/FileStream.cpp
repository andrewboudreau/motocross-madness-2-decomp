// FileStream.cpp -- near misses of the buffered file stream (UnknownTextureStream,
// declared in src/reconstructed/TextureMap.h), 0x00460d10..0x00461d57. The twelve
// methods that match retail are in src/reconstructed/FileStream.cpp; these six
// (header check, open, seek, read, write, write-header) stay here with
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
//   countdown loop shape moves those two. Swept without gain (257/282 best):
//   the seed store at each of 168 positions with (char)0xfa, -6, '\xfa' and
//   0xfa, for/split-p++/countdown loops with early or late `p`, inline seed
//   helpers taking the stream, a char* or a char&, a char& key alias for some
//   or all uses and `p` set before strlen; strcpy(name + strlen) gives 166.
//   Storing the seed before _splitpath matches the lea esi but moves the store ahead
//   of the call; storing it after strlen also leaves 2 differences. The
//   store after `char* p = name;` puts the lea esi where retail has it (one
//   instruction difference, 255/282) but sinks the store to just before the
//   jle; `strlen(strcat(..))` (183), `p = strcat(..)` (172) and strcat or
//   strlen through `p` (225) are worse.
// - 0x004618e0: retail loads buffer/size/count into registers before the pushes;
//   VC6 here pushes them from the stack. Register locals, a helper for the fseek
//   block and an unsigned count do not change it.
//   A `FILE* file = stream->fileHandle;` local in the fwrite tail does schedule all
//   loads before the pushes, but in ecx/edx/eax where retail uses edx/eax/ecx
//   (a register tie-break); not kept. 136/152: the scratch registers are
//   rotated one step against retail throughout (fclose argument eax vs edx,
//   encoded call's file and key edx/ecx vs ecx/eax, fwrite's file esi vs
//   ecx). Also unchanged:
//   the tail-call form `if (innerStream) return innerStream->...`, every
//   int/unsigned/long/size_t count and const void*/void*/const char* buffer
//   type, a 0..63 typedef and 0..19 prototype prefix, a standalone TU,
//   while/for/next InnermostStream loops, `!= 0`/NULL tests, int position,
//   a result local or ternary for the encoded return, a key local, inline
//   helpers for the close, resync, encoded write and plain write, dead
//   locals or empty statements at six positions and an else-return copy.
//   FILE* locals in the seek block and a nested fseek(f, ftell(f)) are worse.
//   A later 88-variant sweep (fseek `!= 0`, a result local, FILE* locals or
//   references for some or all reads of fileHandle, comma zeroing, ternary,
//   else and positive encoded returns, the top-level branch inverted) stays
//   at 136 or below. Locals for buffer, size, count and the file in all 24
//   read orders keep the fwrite tail at 136; the same for the encoded call
//   (with a `char* key` local) gives 75.
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
int WriteXorEncoded(const void* buffer, int size, int count, FILE* file, char* key);
// 0x0045c7b0 (cdecl): fread that decodes the bytes with the running key *key.
int ReadXorDecoded(unsigned char* buffer, int size, int count, FILE* file, unsigned char* key);

// The stream that holds the file: the innermost one.
static inline UnknownTextureStream* InnermostStream(UnknownTextureStream* stream)
{
    while (stream->innerStream) {
        stream = stream->innerStream;
    }
    return stream;
}

// Whether the next four bytes are the "FAOE" header. As an inline helper the
// failure path of the four tests falls through, as in retail.
static inline int HeaderMatches(UnknownTextureStream* stream)
{
    if (stream->ReadByte() != stream->headerByte0
        || stream->ReadByte() != stream->headerByte1
        || stream->ReadByte() != stream->headerByte2
        || stream->ReadByte() != stream->headerByte3) {
        return 0;
    }
    return 1;
}

// 0x00460db0: 1 when the file starts with the header; the file is closed when a
// seek fails. Near miss (16%): retail keeps the first fseek-failure block inline and
// cross-jumps only from its fclose call into the second one (the two load +0x14
// into different registers), and stores fileEncoded = 0 as an immediate; VC6 here
// merges the whole first block into the second and stores fseek's zero result.
// The plain ||, &&, goto, nested, result-variable and seek-helper shapes compile
// to the same or worse code; so do inline SeekOrClose/SeekOnly helpers for either
// or both blocks and `!= 0` tests (27/186 over the whole 186-byte extent).
// Also swept (20 forms, best 28): a result local, `!= 0`, `0L`, an extra
// `&& fileHandle` and an unsigned `> 0` test for the first seek, each with the
// `encoded`/+0x124/`fileEncoded = 0` statements in three orders and a cast zero.
// Nested `if (!fseek(...)) { ... } else { fclose ...}` forms, with the header
// test negated or not, and an if/else-if chain give 25-28.
int UnknownTextureStream::CheckHeader()
{
    if (fseek(fileHandle, 0, SEEK_SET)) {
        fclose(fileHandle);
        fileHandle = 0;
        return 0;
    }
    int encoded = fileEncoded;
    bufferPos = bufferEnd;
    fileEncoded = 0;
    if (!HeaderMatches(this)) {
        fileEncoded = encoded;
        return 0;
    }
    if (fseek(fileHandle, 4, SEEK_SET)) {
        fclose(fileHandle);
        fileHandle = 0;
        return 0;
    }
    fileEncoded = encoded;
    bufferPos = bufferEnd;
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
// Session 10 (284/957 strict, 34 instruction differences): the extra store is the
// access[0] zero; `= ""` (string literal), memset(access, 0, 8) and a zeroing loop
// give 208. The merged tail is the header-failure block of the write branch, which
// VC6 cross-jumps into the read branch NOT-encrypted block by moving the
// fileHandle store ahead of the format push. A CloseFile(FILE*&) helper, NULL,
// `if (!encoded) {} else if` and a FILE* local closed after the reset (190) do not
// keep it apart. `char c = *p++;` in the key loop matches the loop and the
// key reload after it but merges two more tails (201).
int UnknownTextureStream::Open(const char* path, const char* mode, int encoded)
{
    char drive[4];
    char buffer[0x184];
    char extension[0x100];
    char directory[0x100];

    if (fileHandle) {
        fclose(fileHandle);
        fileEncoded = 0;
        headerSize = 0;
        textMode = 0;
        fileHandle = 0;
        nameKey = 0;
        innerStream = 0;
    }
    innerStream = FindArchive(path);
    if (innerStream) {
        innerStream->textMode = innerStream->nameKey;
        SetBufferSize(0x8000);
        Seek(sliceOffset, SEEK_SET, 0);
        return 1;
    }
    if (*mode == 'r' || *mode == 'a') {
        if (!encoded && IsValidFile(path)) {
            fileEncoded = 1;
            if (archives && (innerStream = archives->AddArchive(path, 0)) != 0) {
                return 1;
            }
        }
        if (path) {
            if (*path && (fileHandle = fopen(path, mode)) != 0) {
                if (!encoded && !fileEncoded) {
                    if (fseek(fileHandle, 0, SEEK_SET)) {
                        fclose(fileHandle);
                        fileHandle = 0;
                        sprintf(buffer, "Fseek failed for %s.\n", path);
                        return 0;
                    }
                } else {
                    if (!CheckHeader()) {
                        fclose(fileHandle);
                        fileHandle = 0;
                        sprintf(buffer, "%s NOT an encrypted file, but expected to be.\n", path);
                        return 0;
                    }
                    fileEncoded = 1;
                    headerSize = 4;
                    _splitpath(path, drive, directory, buffer, extension);
                    strcat(buffer, extension);
                    textMode = (char)0xfa;
                    int length = strlen(buffer);
                    char* p = buffer;
                    for (int i = 0; i < length; i++) {
                        textMode = ((textMode + 3) ^ *p++) + textMode;
                    }
                    nameKey = textMode;
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
            if (*path && (fileHandle = fopen(path, access)) != 0) {
                if (fseek(fileHandle, 0, SEEK_SET)) {
                    fclose(fileHandle);
                    fileHandle = 0;
                    sprintf(buffer, "Fseek failed for %s.\n", path);
                    return 0;
                }
                if (encoded && !WriteHeader(path)) {
                    fclose(fileHandle);
                    fileHandle = 0;
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
    Seek(0, SEEK_SET, fileEncoded);
    return 1;
}

// Restarts the key of `stream`: 0x00461d20 inlined one level.
static inline void RestartKey(UnknownTextureStream* stream)
{
    if (stream->innerStream)
        stream->innerStream->ResetTextMode();
    else
        stream->textMode = stream->nameKey;
}

// 0x00461340: seeks to `offset` (origin as fseek). An encoded file cannot
// fseek into the middle of its key stream, so with `flag` it rewinds (or keeps
// the decoded position +0x0c) and decodes forward through a scratch buffer.
// Near miss (286 of 702; candidate 705 bytes): the control flow and the inner
// loop match, but retail keeps the next pointer and the zero in other registers
// and swaps the origin/offset registers (ebp/ebx), so most instructions differ
// by register. The 0x418-byte buffer reproduces retail's 0x41c-byte frame;
// `result = 0` inside the loop reproduces retail's store (the source was likely
// tail recursion). The decoded position (+0x0c) read once into `position` and
// cleared when not below `offset` gives retail's load-compare-clear (201 with
// the ternary, which reads +0x0c twice); retail also threads the SEEK_CUR
// test into the clear, which VC6 here repeats. A `decoded` local (194), a
// `next` pointer for the chain walk (194), if/else-if origin checks (196 to
// 208) and a goto past the decoded read (206) are worse.
int UnknownTextureStream::Seek(int offset, int origin, int flag)
{
    unsigned char buffer[0x418];
    int result = 0;
    UnknownTextureStream* stream = this;
    while (stream->innerStream) {
        result = 0;
        flag = flag && offset != stream->sliceOffset;
        stream = stream->innerStream;
    }
    if (origin == SEEK_SET && offset == stream->Tell()) {
        if (stream->fileEncoded && !flag) {
            RestartKey(stream);
            stream->keyStartOffset = offset;
        }
        stream->bufferPos = stream->bufferEnd;
        return 0;
    }
    if (stream->fileEncoded && flag) {
        if ((origin == SEEK_END && offset != 0) || (origin == SEEK_CUR && offset < 0))
            return 1;
        int position = 0;
        if (origin != SEEK_CUR) {
            position = stream->keyStartOffset;
            if (position >= offset)
                position = 0;
        }
        if (origin == SEEK_SET || origin == SEEK_END) {
            result = fseek(stream->fileHandle, stream->headerSize + position, SEEK_SET);
            if (result) {
                fclose(stream->fileHandle);
                stream->fileHandle = 0;
                return result;
            }
            stream->textMode = stream->nameKey;
        }
        if (origin == SEEK_SET || origin == SEEK_CUR) {
            int skip = offset - position;
            if (skip) {
                int blocks = skip / 0x400;
                int tail = skip % 0x400;
                for (; blocks; blocks--) {
                    if (!ReadXorDecoded(buffer, 0x400, 1, stream->fileHandle, (unsigned char*)&stream->textMode))
                        return -1;
                }
                if (tail && !ReadXorDecoded(buffer, tail, 1, stream->fileHandle, (unsigned char*)&stream->textMode))
                    return -1;
            }
        } else if (origin == SEEK_END) {
            while (ReadXorDecoded(buffer, 0x400, 1, stream->fileHandle, (unsigned char*)&stream->textMode))
                ;
            if (ferror(stream->fileHandle))
                return -1;
            return !feof(stream->fileHandle);
        }
    } else {
        if (stream->fileEncoded && !flag) {
            RestartKey(stream);
            if (origin == SEEK_SET)
                stream->keyStartOffset = offset;
        }
        result = fseek(stream->fileHandle, stream->headerSize + offset, origin);
        if (result) {
            fclose(stream->fileHandle);
            stream->fileHandle = 0;
        }
    }
    stream->bufferPos = stream->bufferEnd;
    return result;
}

// The running-key decode used by 0x00461640 (the 0x00461980 shape).
static inline void DecodeBytes(UnknownTextureStream* stream, char* p, int count)
{
    for (int i = 0; i < count; i++) {
        char c = *p;
        char decoded = stream->textMode ^ c;
        stream->textMode += c;
        *p++ = decoded;
    }
}

// 0x00461640: reads `count` items of `size` bytes, from the buffer first, and
// returns how many it read. Near miss (389 of 671): a separate `out` pointer
// for the advanced buffer (202 when `buffer` itself is advanced) gives
// retail's ecx copy homed in the dead `buffer` slot. Left: retail loads the
// file pointer (+0x14) into eax or ebx where VC6 here uses edx, cross-jumps
// the empty-buffer encoded call into the later one (VC6 here keeps three
// 45c7b0 calls) and keeps the two `(done + read) / size` failure returns
// apart (VC6 here merges them). A key pointer local (24) and `left`
// computed before the encoded test (372) are worse.
int UnknownTextureStream::Read(void* buffer, int size, int count)
{
    UnknownTextureStream* stream = InnermostStream(this);
    char* p = stream->bufferPos;
    if (p != stream->bufferEnd) {
        int available = stream->bufferEnd - p;
        int total = size * count;
        if (total <= available) {
            if (stream->fileEncoded) {
                DecodeBytes(stream, p, total);
            }
            memcpy(buffer, stream->bufferPos, total);
            stream->bufferPos += total;
            return count;
        }
        if (stream->fileEncoded) {
            DecodeBytes(stream, p, available);
        }
        memcpy(buffer, stream->bufferPos, available);
        stream->bufferPos = stream->bufferEnd;
        char* out = (char*)buffer + available;
        int part = (unsigned)available % (unsigned)size;
        if (part) {
            if (stream->fileEncoded) {
                if (!ReadXorDecoded((unsigned char*)out, part, 1, stream->fileHandle, (unsigned char*)&stream->textMode)) {
                    return 0;
                }
            } else if (!fread(out, part, 1, stream->fileHandle)) {
                return (unsigned)available / (unsigned)size;
            }
            if (part == total - available) {
                return count;
            }
            out += part;
        }
        if (stream->fileEncoded) {
            if (!ReadXorDecoded((unsigned char*)out, size, count - (unsigned)(part + available) / (unsigned)size, stream->fileHandle, (unsigned char*)&stream->textMode)) {
                return count - 1;
            }
            return count;
        }
        int left = count - (unsigned)(part + available) / (unsigned)size;
        int read = fread(out, size, left, stream->fileHandle);
        if (read == left) {
            return count;
        }
        return count - read;
    }
    if (stream->fileEncoded) {
        if (!ReadXorDecoded((unsigned char*)buffer, size, count, stream->fileHandle, (unsigned char*)&stream->textMode)) {
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
        int read = fread(out, 1, 0x400, stream->fileHandle);
        if (read != 0x400) {
            return (unsigned)(done + read) / (unsigned)size;
        }
        blocks--;
        out += read;
        done += read;
    }
    if (tail) {
        int read = fread(out, 1, tail, stream->fileHandle);
        if (read != tail) {
            return (unsigned)(done + read) / (unsigned)size;
        }
    }
    return count;
}

// 0x004618e0. Near miss (88%): retail loads every argument into a register before
// the pushes; VC6 here pushes from memory.
int UnknownTextureStream::Write(const void* buffer, int size, int count)
{
    UnknownTextureStream* stream = InnermostStream(this);
    if (stream->fileEncoded) {
        long position = ftell(stream->fileHandle);
        if (fseek(stream->fileHandle, position, SEEK_SET)) {
            fclose(stream->fileHandle);
            stream->fileHandle = 0;
        }
        if (!WriteXorEncoded(buffer, size, count, stream->fileHandle, &stream->textMode)) {
            return count - 1;
        }
        return count;
    }
    return fwrite(buffer, size, count, stream->fileHandle);
}

// Writes the "FAOE" header; 0 when a write fails. As an inline helper the failure
// path falls through, as in retail.
static inline int PutHeader(UnknownTextureStream* stream)
{
    if (stream->WriteByte(stream->headerByte0) == -1
        || stream->WriteByte(stream->headerByte1) == -1
        || stream->WriteByte(stream->headerByte2) == -1
        || stream->WriteByte(stream->headerByte3) == -1) {
        return 0;
    }
    return 1;
}

// 0x00461b90: writes the header and seeds the key from the file name (name plus
// extension). Near miss (90%): the loop body matches with the name byte in a
// temporary; retail stores the 0xfa seed inside the inlined strcat and sets up
// the name pointer right after strlen, where VC6 here schedules both later.
int UnknownTextureStream::WriteHeader(const char* path)
{
    char drive[_MAX_DRIVE];
    char directory[_MAX_DIR];
    char name[_MAX_PATH];
    char extension[_MAX_EXT];

    UnknownTextureStream* stream = InnermostStream(this);
    if (!PutHeader(stream)) {
        return 0;
    }
    stream->fileEncoded = 1;
    stream->headerSize = 4;
    _splitpath(path, drive, directory, name, extension);
    strcat(name, extension);
    stream->textMode = (char)0xfa;
    int length = strlen(name);
    char* p = name;
    for (int i = 0; i < length; i++) {
        char c = *p++;
        stream->textMode = ((stream->textMode + 3) ^ c) + stream->textMode;
    }
    stream->nameKey = stream->textMode;
    return 1;
}
