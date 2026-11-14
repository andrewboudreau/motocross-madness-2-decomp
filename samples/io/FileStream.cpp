// FileStream.cpp -- near misses of the buffered file stream (FileStream,
// declared in src/reconstructed/TextureMap.h), 0x00460d10..0x00461d57. The thirteen
// methods that match retail are in src/reconstructed/FileStream.cpp; these three
// (seek, read, write-header) stay here with
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
//   retail's loop body (85% -> 90%); it is also the spelling of 0x00460f50's
//   loop, which matches once each branch declares its own message buffer.
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
//   strlen through `p` (225) are worse. Session 17: the seed at every statement
//   position with `unsigned char`, a for-init seed and a SetTextMode-style helper
//   stay at 257 or below; VC6 keeps the store out of the rep movsd/movsb pair.
//   Session 21: the seed between strcat and strlen lands after strlen's
//   `or ecx, -1` (257, lea esi hoisted with it); /ML keeps 255.
// - 0x00460db0 (CheckHeader) is exact in src/reconstructed/FileStream.cpp for
//   the same reason as Write: both fseek calls take the handle from a FILE*
//   local. That puts the first fclose argument in eax and the second in edx,
//   so the two failure blocks no longer merge and the first stays inline.
//   About 60 control-flow shapes (||, &&, goto, nested, result locals, seek
//   helpers, re-tests) had scored 25-29/186.
// - 0x004618e0 (Write) is exact in src/reconstructed/FileStream.cpp: the file
//   handle goes through a FILE* local for ftell, fseek and fwrite (not for
//   fclose or the encoded write). VC6 hands expression temporaries eax, ecx
//   and edx in turn; a value read into a named local takes no turn, so the
//   locals shift every later scratch register by one step, which was the
//   whole 136/152 gap. Source shapes, statement order, helpers, types and a
//   0..63 declaration prefix never moved it.
// - 0x00461340: retail loads the decoded position (+0x0c) once and keeps the next
//   pointer in a different register (see the function's note).
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

// 0x0045c7b0 (cdecl): fread that decodes the bytes with the running key *key.
int ReadXorDecoded(unsigned char* buffer, int size, int count, FILE* file, unsigned char* key);

// The stream that holds the file: the innermost one.
static inline FileStream* InnermostStream(FileStream* stream)
{
    while (stream->innerStream) {
        stream = stream->innerStream;
    }
    return stream;
}

// 0x00461340: seeks to `offset` (origin as fseek). An encoded file cannot
// fseek into the middle of its key stream, so with `flag` it rewinds (or keeps
// the decoded position +0x0c) and decodes forward through a scratch buffer.
// Session 21 (343 of 702, candidate 695 bytes): the plain branch's key
// reset tests only `fileEncoded`, as retail does. That branch is the else
// of `fileEncoded && flag`, so the dropped `&& !flag` was always true there
// (342, 699 bytes with it). The head now matches through the SEEK_CUR test.
// ResetTextMode is now an
// in-class inline (FileStream.h), so calling it on the stream itself gives
// retail's one inlined level and the out-of-line 0x00461d20 call; the old
// RestartKey helper (which called it on the inner stream) now inlined a
// second level (730 bytes, 132). The SEEK_END tail written `if (feof(..))
// return 0; return 1;` (also as if/else-if/else) gives retail's `not al;
// movsx eax, al; and 0x10; shr 4`; `!feof`, `feof == 0`, `?:`, `!=
// _IOEOF`, char casts of the flag or of feof, and a `flags` local do not.
// Still left: retail gives the fseek result edi and the position ebx (offset
// reloaded from its home), VC6 here the reverse; the declaration order of
// stream/buffer/result, `if ((result = fseek(..)) != 0)`, `offset -=
// position` or `position = offset - position` for the skip, a
// function-scope position, a tail-recursive Seek (not turned into a loop:
// 732 bytes), the split `return 1` tests and four other position forms
// do not move it; nor do those skip forms or two position forms (an
// if/else and a `position = 0` default) once the redundant test is gone.
// Earlier (before the inline change; 300 of 652 masked; same 702 bytes): the
// inner loop match, but retail keeps the next pointer and the zero in other
// registers and swaps the origin/offset registers (ebp/ebx), so most
// instructions differ by register. The 0x418-byte buffer reproduces
// retail's 0x41c-byte frame; `result = 0` inside the loop reproduces
// retail's store (the source was likely tail recursion). Reading the
// decoded position (+0x0c) inside the condition, with SEEK_CUR and a
// position not below `offset` sharing one clear, gives retail's single
// load and its SEEK_CUR branch threaded into the clear (705 -> 702 bytes,
// 41 -> 32 hunks). Retail puts the position in ebx, which held `offset`,
// and reloads `offset`; VC6 here uses edi. The nested-if form (286), the
// ternary (201), an if/else on the position, a `decoded` local (194), a
// `next` pointer for the chain walk (194), if/else-if origin checks (196 to
// 208) and a goto past the decoded read (206) are worse.
int FileStream::Seek(int offset, int origin, int flag)
{
    FileStream* stream = this;
    unsigned char buffer[0x418];
    int result = 0;
    while (stream->innerStream) {
        result = 0;
        flag = flag && offset != stream->sliceOffset;
        stream = stream->innerStream;
    }
    if (origin == SEEK_SET && offset == stream->Tell()) {
        if (stream->fileEncoded && !flag) {
            stream->ResetTextMode();
            stream->keyStartOffset = offset;
        }
        stream->bufferPos = stream->bufferEnd;
        return 0;
    }
    if (stream->fileEncoded && flag) {
        if ((origin == SEEK_END && offset != 0) || (origin == SEEK_CUR && offset < 0))
            return 1;
        int position;
        if (origin == SEEK_CUR || (position = stream->keyStartOffset) >= offset)
            position = 0;
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
            if (feof(stream->fileHandle))
                return 0;
            return 1;
        }
    } else {
        if (stream->fileEncoded) {
            stream->ResetTextMode();
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
static inline void DecodeBytes(FileStream* stream, char* p, int count)
{
    for (int i = 0; i < count; i++) {
        char c = *p;
        char decoded = stream->textMode ^ c;
        stream->textMode += c;
        *p++ = decoded;
    }
}

// 0x00461640: reads `count` items of `size` bytes, from the buffer first, and
// returns how many it read. Near miss (467 of 670): a separate `out` pointer
// for the advanced buffer (202 when `buffer` itself is advanced) gives
// retail's ecx copy homed in the dead `buffer` slot. The empty-buffer
// encoded read returns a `?:` (389 as `if (!...) return count - 1; return
// count;`): the buffered encoded read then keeps its own `return count`
// after its failure return, like retail, instead of jumping to the later
// one. Left: retail loads the file pointer (+0x14) into eax or ebx where
// VC6 here uses edx, cross-jumps the empty-buffer encoded call into the
// buffered one (VC6 here keeps both 45c7b0 calls) and keeps the two
// `(done + read) / size` failure returns apart (VC6 here merges them;
// unsigned `read`, `(read + done)`, `done += read` first and a `goto
// failed` for the block loop do not change that). A key pointer local (24)
// and `left` computed before the encoded test (372) are worse.
int FileStream::Read(void* buffer, int size, int count)
{
    FileStream* stream = InnermostStream(this);
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
        return ReadXorDecoded((unsigned char*)buffer, size, count, stream->fileHandle, (unsigned char*)&stream->textMode) ? count : count - 1;
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

// Writes the "FAOE" header; 0 when a write fails. As an inline helper the failure
// path falls through, as in retail.
static inline int PutHeader(FileStream* stream)
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
// temporary. Seeding the key after `p = name` sets up the name pointer right
// after strlen as retail does (seeding before strlen hoists the pointer into
// the strlen); retail still stores the 0xfa seed inside the inlined strcat,
// where VC6 here stores it before the loop. Seeding before strcat or before
// _splitpath moves the store to the start of the strcat or ahead of the call;
// `name[i]`, a count-down loop and `length` in the for-init are no closer.
int FileStream::WriteHeader(const char* path)
{
    char drive[_MAX_DRIVE];
    char directory[_MAX_DIR];
    char name[_MAX_PATH];
    char extension[_MAX_EXT];

    FileStream* stream = InnermostStream(this);
    if (!PutHeader(stream)) {
        return 0;
    }
    stream->fileEncoded = 1;
    stream->headerSize = 4;
    _splitpath(path, drive, directory, name, extension);
    strcat(name, extension);
    int length = strlen(name);
    char* p = name;
    stream->textMode = (char)0xfa;
    for (int i = 0; i < length; i++) {
        char c = *p++;
        stream->textMode = ((stream->textMode + 3) ^ c) + stream->textMode;
    }
    stream->nameKey = stream->textMode;
    return 1;
}
