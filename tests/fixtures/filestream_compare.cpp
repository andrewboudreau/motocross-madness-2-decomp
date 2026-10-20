// Private differential validation only. Production never loads reference code.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/TextureMap.h"

// The separately compiled sample imports the ordinary CRT names. Its calls
// are redirected by /D dependency-name substitutions in the test object only.
struct Scenario {
    int method, encoded, layers, available, size, count;
    int offset, origin, flag, position, keyStart, failure, headerMismatch;
    const char* path;
};
struct Observation {
    int result, position, closed, fileFlags[3], eventCount, written;
    int events[2048][5];
    unsigned char streamBytes[3 * sizeof(UnknownTextureStream)];
    unsigned char output[4096];
    unsigned char writtenBytes[4096];
};
static Scenario scenario;
static Observation observed;
static FILE mockFiles[3];
static int position, reads, seeks, bytes;
static union { double alignment; unsigned char bytes[3 * sizeof(UnknownTextureStream)]; } storage;
static unsigned char output[4096];
static unsigned char referenceSpace[0x200000];

static UnknownTextureStream* Stream(int index) {
    return (UnknownTextureStream*)(storage.bytes + index * sizeof(UnknownTextureStream));
}
static void Record(int operation, int a, int b, int c, int d = 0) {
    if (observed.eventCount >= 2048) ExitProcess(90);
    int* event = observed.events[observed.eventCount++];
    event[0] = operation; event[1] = a; event[2] = b; event[3] = c; event[4] = d;
}
static int FileIdentity(FILE* file) {
    if (!file) return -1;
    for (int i = 0; i < 3; ++i)
        if (file == mockFiles + i) return i;
    ExitProcess(93);
    return -2;
}
static void RecordWritten(const void* data, int count) {
    if (count < 0 || observed.written + count > sizeof(observed.writtenBytes)) ExitProcess(94);
    memcpy(observed.writtenBytes + observed.written, data, count);
    observed.written += count;
}
static int BufferOffset(const void* buffer) {
    if ((DWORD)buffer >= (DWORD)output && (DWORD)buffer <= (DWORD)output + sizeof(output))
        return (DWORD)buffer - (DWORD)output;
    return -1; // Seek's private scratch buffer; its address is not observable.
}
extern "C" int __cdecl ProbeSeek(FILE* file, long offset, int origin) {
    Record(1, offset, origin, FileIdentity(file));
    if (++seeks == scenario.failure) return -3;
    position = origin == SEEK_SET ? offset : origin == SEEK_CUR ? position + offset : 2048 + offset;
    return 0;
}
extern "C" int __cdecl ProbeClose(FILE* file) {
    Record(2, FileIdentity(file), 0, 0); observed.closed++; return 0;
}
extern "C" long __cdecl ProbeTell(FILE* file) {
    Record(3, FileIdentity(file), 0, 0); return position;
}
extern "C" size_t __cdecl ProbeRead(void* buffer, size_t size, size_t count, FILE* file) {
    Record(4, size, count, BufferOffset(buffer), FileIdentity(file));
    int result = count;
    if (++reads == scenario.failure && result) result--;
    if (size * result > sizeof(output)) ExitProcess(91);
    for (unsigned int i = 0; i < size * result; ++i)
        ((unsigned char*)buffer)[i] = (unsigned char)((position + i) * 37 + FileIdentity(file) * 11 + 9);
    position += size * result;
    return result;
}
extern "C" size_t __cdecl ProbeWrite(const void* buffer, size_t size, size_t count, FILE* file) {
    Record(5, size, count, BufferOffset(buffer), FileIdentity(file));
    RecordWritten(buffer, size * count);
    if (++reads == scenario.failure && count) count--;
    position += size * count;
    return count;
}
extern "C" void __cdecl ProbeSplit(const char* path, char* drive, char* dir, char* name, char* ext) {
    Record(6, strlen(path), 0, 0);
    _splitpath(path, drive, dir, name, ext);
}
int WriteXorEncoded(const unsigned char* buffer, int size, int count, FILE* file, unsigned char* key) {
    Record(7, size, count, BufferOffset(buffer), FileIdentity(file));
    RecordWritten(buffer, size * count);
    for (int i = 0; i < size * count; ++i) *key += buffer[i] ^ *key;
    position += size * count;
    return ++reads != scenario.failure;
}
int ReadXorDecoded(unsigned char* buffer, int size, int count, FILE* file, unsigned char* key) {
    Record(8, size, count, BufferOffset(buffer), FileIdentity(file));
    int failed = ++reads == scenario.failure;
    // SEEK_END's loop terminates through this scripted dependency, with both
    // EOF and error flag paths exercised. These mocks are shared by both sides.
    if (scenario.method == 1 && scenario.origin == SEEK_END && reads >= 3) failed = 1;
    if (failed) {
        file->_flag |= scenario.failure & 1 ? 0x20 : 0x10;
        return 0;
    }
    if (size * count > sizeof(output) || size < 0 || count < 0) ExitProcess(92);
    for (int i = 0; i < size * count; ++i) {
        unsigned char value = (unsigned char)((position + i) * 37 + FileIdentity(file) * 11 + 9);
        buffer[i] = *key ^ value;
        *key += value;
    }
    position += size * count;
    return 1;
}
int UnknownTextureStream::Tell() {
    Record(9, (char*)this - (char*)Stream(0), 0, 0); return scenario.position;
}
void UnknownTextureStream::ResetTextMode() {
    Record(10, (char*)this - (char*)Stream(0), 0, 0); textMode = nameKey;
}
int UnknownTextureStream::ReadByte() {
    Record(11, (char*)this - (char*)Stream(0), 0, 0);
    int value = "FAOE"[bytes++ % 4];
    if (bytes == scenario.headerMismatch) value = -1;
    bufferPos++;
    return value;
}
int UnknownTextureStream::WriteByte(int value) {
    Record(12, value, (char*)this - (char*)Stream(0), 0);
    unsigned char byte = value;
    RecordWritten(&byte, 1);
    return ++bytes == scenario.failure ? -1 : value;
}

static DWORD Address(int (UnknownTextureStream::*method)()) {
    union { int (UnknownTextureStream::*method)(); DWORD address; } value;
    value.method = method; return value.address;
}
static DWORD Address(void (UnknownTextureStream::*method)()) {
    union { void (UnknownTextureStream::*method)(); DWORD address; } value;
    value.method = method; return value.address;
}
static DWORD Address(int (UnknownTextureStream::*method)(int)) {
    union { int (UnknownTextureStream::*method)(int); DWORD address; } value;
    value.method = method; return value.address;
}
static DWORD Replacement(DWORD target) {
    switch (target) {
    case 0x534eac: return (DWORD)ProbeSeek;
    case 0x534c3d: return (DWORD)ProbeClose;
    case 0x5365d6: return (DWORD)ProbeTell;
    case 0x534f65: return (DWORD)ProbeRead;
    case 0x5350b7: return (DWORD)ProbeWrite;
    case 0x5359d2: return (DWORD)ProbeSplit;
    case 0x45c6a0: return (DWORD)WriteXorEncoded;
    case 0x45c7b0: return (DWORD)ReadXorDecoded;
    case 0x461600: return Address(&UnknownTextureStream::Tell);
    case 0x461d20: return Address(&UnknownTextureStream::ResetTextMode);
    case 0x461980: return Address(&UnknownTextureStream::ReadByte);
    case 0x461a60: return Address(&UnknownTextureStream::WriteByte);
    }
    return 0;
}
static int MapReference(const char* path, const char* patches) {
    FILE* file = fopen(path, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    unsigned char* image = new unsigned char[size];
    if (fread(image, 1, size, file) != (size_t)size) return 0;
    fclose(file);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)image;
    if (size < sizeof(*dos) || dos->e_magic != IMAGE_DOS_SIGNATURE ||
        dos->e_lfanew < 0 || dos->e_lfanew > size - sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(image + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != 0x14c ||
        nt->OptionalHeader.ImageBase != 0x400000) return 0;
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    for (DWORD page = 0x460000; page <= 0x461000; page += 0x1000) {
        if (page < (DWORD)referenceSpace ||
            page + 0x1000 > (DWORD)referenceSpace + sizeof(referenceSpace)) return 0;
        DWORD oldProtection, rva = page - 0x400000;
        if (!VirtualProtect((void*)page, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtection)) return 0;
        int copied = 0;
        for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER* section = sections + i;
            if (rva >= section->VirtualAddress && rva + 0x1000 <= section->VirtualAddress + section->SizeOfRawData) {
                DWORD offset = section->PointerToRawData + rva - section->VirtualAddress;
                if (offset + 0x1000 > (DWORD)size) return 0;
                memcpy((void*)page, image + offset, 0x1000);
                copied = 1;
            }
        }
        if (!copied) return 0;
    }
    delete[] image;
    file = fopen(patches, "rb");
    if (!file) return 0;
    DWORD patch[2];
    while (fread(patch, sizeof(patch), 1, file) == 1) {
        DWORD at = patch[0], target = patch[1], replacement = Replacement(target);
        if (at < 0x460000 || at + 5 > 0x462000 || !replacement ||
            *(unsigned char*)at != 0xe8 || at + 5 + *(DWORD*)(at + 1) != target) return 0;
        *(DWORD*)(at + 1) = replacement - at - 5;
    }
    fclose(file);
    FlushInstructionCache(GetCurrentProcess(), (void*)0x460000, 0x2000);
    return 1;
}
static void Run(int reference) {
    memset(&observed, 0, sizeof(observed));
    memset(mockFiles, 0, sizeof(mockFiles));
    memset(storage.bytes, 0, sizeof(storage.bytes));
    for (int n = 0; n < sizeof(output); ++n) output[n] = (unsigned char)(n * 23 + n / 7 + 17);
    position = scenario.position; reads = seeks = bytes = 0;
    for (int i = 0; i < 3; ++i) {
        UnknownTextureStream* stream = Stream(i);
        stream->headerByte0 = 'F'; stream->headerByte1 = 'A';
        stream->headerByte2 = 'O'; stream->headerByte3 = 'E';
        stream->textMode = 73 + i; stream->nameKey = 19 + i;
        stream->fileEncoded = scenario.encoded;
        stream->headerSize = scenario.encoded ? 4 : 0;
        stream->fileHandle = mockFiles + i;
        stream->keyStartOffset = scenario.keyStart;
        stream->sliceOffset = i ? 5 : 0;
        stream->innerStream = i < scenario.layers ? Stream(i + 1) : 0;
        for (int j = 0; j < sizeof(stream->readBuffer); ++j)
            stream->readBuffer[j] = (char)(j * 13 + i * 17 + 7);
        stream->bufferPos = stream->readBuffer;
        stream->bufferEnd = stream->readBuffer + scenario.available;
    }
    union { int (UnknownTextureStream::*method)(void*, int, int); DWORD address; } read;
    union { int (UnknownTextureStream::*method)(int, int, int); DWORD address; } seek;
    union { int (UnknownTextureStream::*method)(); DWORD address; } check;
    union { int (UnknownTextureStream::*method)(const void*, int, int); DWORD address; } write;
    union { int (UnknownTextureStream::*method)(const char*); DWORD address; } header;
    read.method = &UnknownTextureStream::Read; seek.method = &UnknownTextureStream::Seek;
    check.method = &UnknownTextureStream::CheckHeader; write.method = &UnknownTextureStream::Write;
    header.method = &UnknownTextureStream::WriteHeader;
    if (reference) {
        read.address = 0x461640; seek.address = 0x461340; check.address = 0x460db0;
        write.address = 0x4618e0; header.address = 0x461b90;
    }
    switch (scenario.method) {
    case 0: observed.result = (Stream(0)->*read.method)(output, scenario.size, scenario.count); break;
    case 1: observed.result = (Stream(0)->*seek.method)(scenario.offset, scenario.origin, scenario.flag); break;
    case 2: observed.result = (Stream(0)->*check.method)(); break;
    case 3: observed.result = (Stream(0)->*write.method)(output, scenario.size, scenario.count); break;
    case 4: observed.result = (Stream(0)->*header.method)(scenario.path); break;
    }
    observed.position = position;
    for (int f = 0; f < 3; ++f) observed.fileFlags[f] = mockFiles[f]._flag;
    memcpy(observed.streamBytes, storage.bytes, sizeof(storage.bytes));
    memcpy(observed.output, output, sizeof(output));
}
static int checks;
static void Compare() {
    Run(1);
    Observation original = observed;
    Run(0);
    if (memcmp(&original, &observed, sizeof(observed))) {
        printf("Mismatch method=%d encoded=%d layers=%d available=%d size=%d count=%d "
               "offset=%d origin=%d flag=%d pos=%d key=%d failure=%d header=%d\n",
               scenario.method, scenario.encoded, scenario.layers, scenario.available,
               scenario.size, scenario.count, scenario.offset, scenario.origin, scenario.flag,
               scenario.position, scenario.keyStart, scenario.failure, scenario.headerMismatch);
        ExitProcess(2);
    }
    checks++;
}
int main(int argc, char** argv) {
    if (argc != 3 || !MapReference(argv[1], argv[2])) {
        puts("Cannot map private reference"); return 1;
    }
    const int sizes[] = {0, 1, 2, 3, 4, 7};
    const int counts[] = {0, 1, 2, 9, 146, 147, 293, 1024, 2048};
    const int available[] = {0, 1, 2, 7, 19};
    const int offsets[] = {-1, 0, 5, 1024, 2049};
    const char* paths[] = {"", "track.dat", "C:\\game\\route.long", "a.b.c", ".hidden"};
    for (scenario.encoded = 0; scenario.encoded < 2; ++scenario.encoded)
        for (scenario.layers = 0; scenario.layers < 3; ++scenario.layers)
            for (scenario.failure = 0; scenario.failure < 5; ++scenario.failure) {
                scenario.method = 0;
                for (int a = 0; a < 5; ++a)
                    for (int s = 0; s < 6; ++s)
                        for (int c = 0; c < 9; ++c) {
                            if (sizes[s] * counts[c] > 3072) continue;
                            scenario.available = available[a];
                            scenario.size = sizes[s]; scenario.count = counts[c];
                            Compare();
                        }
                scenario.method = 1;
                for (int o = 0; o < 5; ++o)
                    for (scenario.origin = 0; scenario.origin < 4; ++scenario.origin)
                        for (scenario.flag = 0; scenario.flag < 2; ++scenario.flag)
                            for (int p = 0; p < 2; ++p)
                                for (int k = 0; k < 2; ++k) {
                                    scenario.offset = offsets[o];
                                    // Negative seeks in an encoded stream with an
                                    // unknown origin can produce unbounded retail
                                    // loops; valid SET/CUR/END failures are covered.
                                    if (scenario.encoded && scenario.flag &&
                                        scenario.offset < 0 && scenario.origin != SEEK_CUR &&
                                        scenario.origin != SEEK_END) continue;
                                    scenario.position = p ? 5 : 0;
                                    scenario.keyStart = k ? 10 : 0;
                                    Compare();
                                }
                scenario.method = 2;
                for (scenario.headerMismatch = 0; scenario.headerMismatch < 5; ++scenario.headerMismatch)
                    Compare();
                scenario.method = 3;
                for (int c = 0; c < 7; ++c) {
                    scenario.size = 3; scenario.count = counts[c]; Compare();
                }
                scenario.method = 4;
                for (int n = 0; n < 5; ++n) {
                    scenario.path = paths[n]; Compare();
                }
            }
    printf("FileStream: %d scenarios match results, state, bytes and ordered calls\n", checks);
    return 0;
}
