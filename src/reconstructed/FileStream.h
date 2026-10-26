#pragma once

#include <stdio.h>

// The stream textures load from: 0x00461600 returns the read position,
// 0x00461640 reads `count` items of `size` bytes and returns how many it
// read. Both follow +0x1c to the innermost stream first.
class UnknownResourceManager;

class UnknownTextureStream {
public:
    UnknownTextureStream(UnknownResourceManager* manager);              // 0x00460d10
    ~UnknownTextureStream();                  // 0x00460d60
    int Open(const char* path, const char* mode, int encoded); // 0x00460f50: opens `path`
    int Seek(int offset, int origin, int flag); // 0x00461340: seeks
    int Tell();
    int Read(void* buffer, int size, int count);
    int ReadByte();              // 0x00461980: reads a byte
    int ReadLine(char* buffer, int size); // 0x00461aa0: reads a line (CarProcedural.cpp)
    int GetFileTimes(int* time, int* size); // 0x00461cb0: _fstat times (SceneManager 0x004ea0fd)
    // 0x00460e70: 1 when `path` opens and passes 0x00460db0's check; the
    // file is closed again (ResourceManager.cpp passes it to 0x00460f50).
    int IsValidFile(const char* path);
    // 0x00460d70: the archive holding `name` (its offset and length go to
    // +0x130 and +0x04), or 0 (FileStream.cpp).
    UnknownTextureStream* FindArchive(const char* name);
    int CheckHeader();              // 0x00460db0: checks the "FAOE" header
    int SetBufferSize(int unused);    // 0x00461310: binary mode
    int Write(const void* buffer, int size, int count); // 0x004618e0: writes
    int WriteByte(int c);         // 0x00461a60: writes a byte
    int WriteHeader(const char* path); // 0x00461b90: writes the header, keyed by the name
    // 0x00461d20: recursive accessor; SelectSection inlines its outer steps.
    void ResetTextMode()
    {
        if (innerStream)
            innerStream->ResetTextMode();
        else
            textMode = nameKey;
    }
    // 0x0043e9e0 (out-of-line copy): the text-mode byte.
    char GetTextMode()
    {
        if (innerStream)
            return innerStream->GetTextMode();
        return textMode;
    }
    // 0x0043e9b0 (out-of-line copy): sets the text-mode byte.
    void SetTextMode(char mode)
    {
        if (innerStream)
            innerStream->SetTextMode(mode);
        else
            textMode = mode;
    }
    // 0x004e9960 (out-of-line copy after ResourceManager.cpp's code):
    // the innermost stream's encoded flag (+0x08).
    int IsEncoded()
    {
        if (innerStream)
            return innerStream->IsEncoded();
        return fileEncoded;
    }
    // 0x00430ff0 (out-of-line copy): whether the stream is at its end: past
    // its length within the inner stream, else the buffer is drained and the
    // file is at end-of-file.
    int AtEnd()
    {
        if (innerStream) {
            if (sliceLength > 0 && Tell() >= sliceOffset + sliceLength)
                return 1;
            return innerStream->AtEnd();
        }
        if (bufferPos != bufferEnd)
            return 0;
        return feof(fileHandle);
    }

    char headerByte1;                         // +0x00 'A', second byte of the "FAOE" header
    char textMode;                            // +0x01 text mode; the running key of an encoded file
    char headerByte0;                         // +0x02 'F', first header byte
    char nameKey;                             // +0x03 key derived from the file name (0x00461b90)
    int sliceLength;                          // +0x04 length within the inner stream (0: to its end)
    int fileEncoded;                          // +0x08 1: the file is encoded
    int keyStartOffset;                       // +0x0c offset where Seek last restarted the key (0x00461340)
    int headerSize;                           // +0x10 header size before the data (4 when encoded)
    FILE* fileHandle;                         // +0x14 the open file
    char headerByte3;                         // +0x18 'E', fourth header byte
    unsigned char field_0x019[0x1c - 0x19];
    UnknownTextureStream* innerStream;        // +0x1c inner stream
    char headerByte2;                         // +0x20 'O', third header byte
    char readBuffer[0x124 - 0x21];            // +0x21 read buffer
    char* bufferPos;                          // +0x124 buffer position
    char* bufferEnd;                          // +0x128 buffer end
    UnknownResourceManager* archives;         // +0x12c the archives (constructor argument)
    int sliceOffset;                          // +0x130 start offset in the inner stream
};

// Parameter readers use the same 0x134-byte archive stream.
typedef UnknownTextureStream UnknownParameterStream;
