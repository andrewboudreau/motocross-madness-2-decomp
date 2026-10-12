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
// (header check, open, seek, read, write, write-header) stay in
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
    while (stream->innerStream) {
        stream = stream->innerStream;
    }
    return stream;
}

// 0x00460d10. The buffer pointer is named before the argument store: only then does VC6
// load the argument at entry (edx) and keep the buffer address in ecx for both pointer
// stores, as retail does.
UnknownTextureStream::UnknownTextureStream(int a)
{
    fileEncoded = 0;
    headerSize = 0;
    textMode = 0;
    fileHandle = 0;
    nameKey = 0;
    innerStream = 0;
    keyStartOffset = 0;
    sliceOffset = 0;
    sliceLength = 0;
    headerByte0 = 'F';
    headerByte1 = 'A';
    headerByte2 = 'O';
    headerByte3 = 'E';
    char* buffer = readBuffer;
    archives = (UnknownResourceManager*)a;
    bufferPos = buffer;
    bufferEnd = buffer;
}

// 0x00460d60
UnknownTextureStream::~UnknownTextureStream()
{
    if (fileHandle) {
        fclose(fileHandle);
    }
}

// 0x00460d70
UnknownTextureStream* UnknownTextureStream::FindArchive(const char* name)
{
    if (archives) {
        ResourceItem* item = archives->FindItem(name, 0);
        if (item) {
            sliceOffset = item->offset;
            sliceLength = item->size;
            return item->archive;
        }
    }
    return 0;
}

// 0x00460e70. The NULL test guards the rest, so its message comes last.
int UnknownTextureStream::IsValidFile(const char* path)
{
    char message[0x184];

    if (fileHandle) {
        fclose(fileHandle);
    }
    if (path) {
        if (*path && (fileHandle = fopen(path, "rb")) != 0) {
            if (!CheckHeader()) {
                fclose(fileHandle);
                fileHandle = 0;
                return 0;
            }
            if (fileHandle) {
                fclose(fileHandle);
                fileHandle = 0;
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
int UnknownTextureStream::SetBufferSize(int unused)
{
    FILE* file = InnermostStream(this)->fileHandle;
    return _setmode(file->_file, _O_BINARY);
}

// 0x00461600
int UnknownTextureStream::Tell()
{
    UnknownTextureStream* stream = InnermostStream(this);
    int position = ftell(stream->fileHandle) + (stream->bufferPos - stream->bufferEnd);
    if (position >= stream->headerSize) {
        return position - stream->headerSize;
    }
    return -1;
}

// 0x00461980: the next byte, decoded with the running key in an encoded file; -1 at
// the end. The decode reads the byte into a temporary and the key straight from the
// field (no key temporary): that is the only spelling under which VC6 loads the key
// first and copies it, as retail does; naming the key makes it copy the byte instead.
// inline_depth(0) keeps the AtEnd call out of line, as retail has it.
#pragma inline_depth(0)
int UnknownTextureStream::ReadByte()
{
    if (innerStream) {
        if (sliceLength > 0 && Tell() >= sliceOffset + sliceLength) {
            return -1;
        }
        if (innerStream->AtEnd()) {
            return -1;
        }
        return innerStream->ReadByte();
    }
    if (bufferPos == bufferEnd) {
        int count = fread(readBuffer, 1, 0x100, fileHandle);
        if (count != 0x100 && (count == 0 || (fileHandle->_flag & _IOERR))) {
            return -1;
        }
        bufferPos = readBuffer;
        bufferEnd = bufferPos + count;
    }
    char* p = bufferPos;
    if (p < bufferEnd) {
        if (fileEncoded) {
            char c = *p;
            char decoded = textMode ^ c;
            textMode += c;
            *p = decoded;
            return (unsigned char)*bufferPos++;
        }
        return (unsigned char)*bufferPos++;
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
    if (innerStream) {
        if (sliceLength > 0 && Tell() >= sliceOffset + sliceLength) {
            return 0;
        }
        if (innerStream->AtEnd()) {
            return 0;
        }
        if (sliceLength > 0) {
            int left = sliceOffset - Tell() + sliceLength + 1;
            if (left < size) {
                size = left;
            }
        }
        return innerStream->ReadLine(buffer, size);
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
    int result = _fstat(stream->fileHandle->_file, &status);
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
    if (innerStream) {
        return innerStream->GetFileTimes(time, size);
    }
    return FileTimes(this, time, size);
}

// 0x00461d20
void UnknownTextureStream::ResetTextMode()
{
    if (innerStream) {
        innerStream->ResetTextMode();
        return;
    }
    textMode = nameKey;
}

// 0x00461d40: fprintf.
int FilePrintf(FILE* file, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    return vfprintf(file, format, arguments);
}
