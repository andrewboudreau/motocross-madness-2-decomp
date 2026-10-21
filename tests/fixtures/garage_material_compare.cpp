// Validation only: actual complete133B provider on prepared storage.
// Resource loading is scripted; native VC6 strncpy is shared with reference.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "D3DIMSoulTree.h"
#include "SoultreeMaterial.h"
#include "SelectGamePicProcs.h"
typedef char Layout[(sizeof(D3DIMSoultreeObject) == 0x2d8 &&
                     offsetof(D3DIMSoultreeObject, materialTable) == 0x290 &&
                     offsetof(D3DIMSoultreeObject, materialCount) == 0x294 &&
                     offsetof(SoultreeMaterial, textureName) == 0x2c &&
                     offsetof(SoultreeMaterial, hasTextureName) == 0x9c &&
                     offsetof(UnknownKrustyUIModelObject, plateTexture) == 0x1a0 &&
                     offsetof(UnknownKrustyUIModel, uiModel) == 0xc0)
                        ? 1
                        : -1];
static BYTE referenceSpace[0x800000] = {1};
static BYTE *modelPage, *materialPage, *namePage;
static BYTE initial[12288], oracle[12288];
struct Result {
    DWORD result, exception, access, address, complete, calls, receiver;
    BYTE atCall[12288], finalState[12288];
};
static Result current, expected;
static int mode, checks, faults, throws;
static D3DIMSoultreeObject *model;
static SoultreeMaterial *material;
static void Snapshot(BYTE *out) {
    memcpy(out, modelPage, 4096);
    memcpy(out + 4096, materialPage, 4096);
    memcpy(out + 8192, namePage, 4096);
}
static void Restore() {
    memcpy(modelPage, initial, 4096);
    memcpy(materialPage, initial + 4096, 4096);
    memcpy(namePage, initial + 8192, 4096);
}
void SoultreeMaterial::LoadTexture() {
    ++current.calls;
    current.receiver = (DWORD)this;
    Snapshot(current.atCall);
    if (this != material || current.calls != 1)
        ExitProcess(20);
    if (mode == 1)
        this->hasTextureName = 0x13579;
    if (mode == 2)
        RaiseException(0xe0425301, 0, 0, 0);
    if (mode == 3)
        model->materialTable = 0;
}
static int Filter(EXCEPTION_POINTERS *p) {
    current.exception = p->ExceptionRecord->ExceptionCode;
    if (current.exception == EXCEPTION_ACCESS_VIOLATION) {
        current.access = p->ExceptionRecord->ExceptionInformation[0];
        current.address = p->ExceptionRecord->ExceptionInformation[1];
    } else if (current.exception != 0xe0425301)
        return EXCEPTION_CONTINUE_SEARCH;
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail, D3DIMSoultreeObject *self, int index, const char *name, int arg) {
    memset(&current, 0, sizeof(current));
    __try {
        typedef int (D3DIMSoultreeObject::*Fn)(int, const char *, int);
        union {
            Fn method;
            DWORD words[2];
        } f;
        f.method = &D3DIMSoultreeObject::ReplaceMaterialTexture;
        if (sizeof(f.method) != 8 || f.words[1])
            ExitProcess(21);
        if (retail)
            f.words[0] = 0x444c70;
        current.result = (self->*f.method)(index, name, arg);
        current.complete = 1;
    } __except (Filter(GetExceptionInformation())) {
    }
    Snapshot(current.finalState);
}
static void Check(D3DIMSoultreeObject *self, int index, const char *name, int arg, int success,
                  int length, DWORD av, DWORD access) {
    Snapshot(initial);
    memcpy(oracle, initial, sizeof(oracle));
    Run(1, self, index, name, arg);
    expected = current;
    Restore();
    Run(0, self, index, name, arg);
    ++checks;
    if (memcmp(&expected, &current, sizeof(current))) {
        printf("difference %d\n", checks);
        ExitProcess(7);
    }
    if (av) {
        ++faults;
        if (current.complete || current.exception != EXCEPTION_ACCESS_VIOLATION ||
            current.address != av || current.access != access || current.calls ||
            memcmp(current.finalState, initial, sizeof(initial)))
            ExitProcess(22);
        return;
    }
    if (success) {
        int n = length > 63 ? 63 : length;
        BYTE *dest = oracle + 4096 + ((BYTE *)material - materialPage) + 0x2c;
        memcpy(dest, name, n);
        dest[n] = 0;
        if (current.calls != 1 || current.receiver != (DWORD)material ||
            memcmp(current.atCall, oracle, sizeof(oracle)))
            ExitProcess(23);
        if (mode == 1)
            *(DWORD *)(oracle + 4096 + ((BYTE *)material - materialPage) + 0x9c) = 0x13579;
        if (mode == 3)
            *(DWORD *)(oracle + ((BYTE *)model - modelPage) + 0x290) = 0;
        if (mode == 2) {
            ++throws;
            if (current.complete || current.exception != 0xe0425301)
                ExitProcess(24);
        } else if (!current.complete || current.exception || current.result != 1)
            ExitProcess(25);
    } else if (!current.complete || current.exception || current.result || current.calls)
        ExitProcess(26);
    if (memcmp(oracle, current.finalState, sizeof(oracle)))
        ExitProcess(27);
}
static void Setup(int count, int index, int flag, int length) {
    memset(modelPage, 0xa5, 4096);
    memset(materialPage, 0x5a, 4096);
    memset(namePage, 0xcc, 4096);
    model = (D3DIMSoultreeObject *)(modelPage + 128);
    material = (SoultreeMaterial *)(materialPage + 128);
    model->materialTable = (SoultreeMaterial **)(modelPage + 2048);
    model->materialCount = count;
    for (int j = -2; j <= 2; ++j)
        model->materialTable[j] = material;
    material->hasTextureName = flag;
    for (j = 0; j < length; ++j)
        namePage[128 + j] = (BYTE)('a' + j % 26);
    namePage[128 + length] = 0;
}
static void PatchCall(DWORD site, DWORD target, DWORD replacement) {
    if (*(BYTE *)site != 0xe8 || site + 5 + *(DWORD *)(site + 1) != target)
        ExitProcess(28);
    *(DWORD *)(site + 1) = replacement - site - 5;
}
static int MapReference(const char *path) {
    BYTE *mapped = (BYTE *)0x400000;
    DWORD old;
    if ((DWORD)referenceSpace > 0x400000 ||
        (DWORD)referenceSpace + sizeof(referenceSpace) < 0x690000 ||
        !VirtualProtect(mapped, 0x290000, PAGE_EXECUTE_READWRITE, &old))
        return 0;
    FILE *file = fopen(path, "rb");
    if (!file)
        return 0;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    if (length < (long)sizeof(IMAGE_DOS_HEADER))
        return 0;
    BYTE *raw = (BYTE *)malloc(length);
    if (!raw || fread(raw, 1, length, file) != (size_t)length)
        return 0;
    fclose(file);
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)raw;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 ||
        dos->e_lfanew > length - (long)sizeof(IMAGE_NT_HEADERS))
        return 0;
    IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS *)(raw + dos->e_lfanew);
    if (pe->Signature != IMAGE_NT_SIGNATURE ||
        pe->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
        pe->OptionalHeader.ImageBase != 0x400000 || pe->OptionalHeader.SizeOfImage > 0x290000 ||
        pe->OptionalHeader.SizeOfHeaders > (DWORD)length ||
        pe->OptionalHeader.SizeOfHeaders > pe->OptionalHeader.SizeOfImage)
        return 0;
    DWORD offset = (BYTE *)IMAGE_FIRST_SECTION(pe) - raw;
    if (offset > (DWORD)length ||
        pe->FileHeader.NumberOfSections > ((DWORD)length - offset) / sizeof(IMAGE_SECTION_HEADER))
        return 0;
    memcpy(mapped, raw, pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER *sections = IMAGE_FIRST_SECTION(pe);
    for (int i = 0; i < pe->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER *section = sections + i;
        if (section->PointerToRawData > (DWORD)length ||
            section->SizeOfRawData > (DWORD)length - section->PointerToRawData ||
            section->VirtualAddress > pe->OptionalHeader.SizeOfImage ||
            section->SizeOfRawData > pe->OptionalHeader.SizeOfImage - section->VirtualAddress)
            return 0;
        memcpy(mapped + section->VirtualAddress, raw + section->PointerToRawData,
               section->SizeOfRawData);
    }
    free(raw);
    return FlushInstructionCache(GetCurrentProcess(), 0, 0);
}

int main(int argc, char **argv) {
    if (argc != 2 || !MapReference(argv[1]))
        return 1;
    union {
        void (SoultreeMaterial::*method)();
        DWORD address;
    } load;
    load.method = &SoultreeMaterial::LoadTexture;
    if (sizeof(load.method) != 4)
        return 2;
    PatchCall(0x444ce4, 0x4ff620, load.address);
    // Reference CRT boundary uses the same native VC6 strncpy as source.
    PatchCall(0x444cc5, 0x534b10, (DWORD)&strncpy);
    FlushInstructionCache(GetCurrentProcess(), 0, 0);
    BYTE **pages[] = {&modelPage, &materialPage, &namePage};
    DWORD old;
    for (int p = 0; p < 3; ++p) {
        *pages[p] = (BYTE *)VirtualAlloc(0, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!*pages[p] || !VirtualProtect(*pages[p] + 4096, 4096, PAGE_NOACCESS, &old))
            return 3;
    }
    const int flags[] = {0, 1, -1, 0x7fffffff};
    const int lengths[] = {0, 1, 31, 62, 63, 64, 65, 127};
    const int args[] = {0, 1, (int)0xdeadbeef};
    for (int count = -3; count <= 2; ++count)
        for (int index = -2; index <= 2; ++index)
            for (int f = 0; f < 4; ++f)
                for (int l = 0; l < 8; ++l)
                    for (mode = 0; mode < 4; ++mode)
                        for (int a = 0; a < 3; ++a) {
                            Setup(count, index, flags[f], lengths[l]);
                            Check(model, index, (char *)namePage + 128, args[a],
                                  index <= count && flags[f] != 0, lengths[l], 0, 0);
                        }
    mode = 0;
    Setup(0, 0, 1, 1);
    Check((D3DIMSoultreeObject *)1, 0, (char *)namePage + 128, 0, 0, 0, 1 + 0x294, 0);
    Setup(0, 0, 1, 1);
    model->materialTable = (SoultreeMaterial **)1;
    Check(model, 0, (char *)namePage + 128, 0, 0, 0, 1, 0);
    Setup(0, 0, 1, 1);
    model->materialTable[0] = (SoultreeMaterial *)1;
    Check(model, 0, (char *)namePage + 128, 0, 0, 0, 1 + 0x9c, 0);
    Setup(0, 0, 1, 1);
    Check(model, 0, (char *)1, 0, 0, 0, 1, 0);
    // Every prefix length still lacks a NUL: strlen must fault before any write/callback.
    for (int n = 0; n <= 64; ++n) {
        Setup(0, 0, 1, 1);
        memset(namePage + 4096 - n, 'x', n);
        Check(model, 0, (char *)namePage + 4096 - n, 0, 0, 0, (DWORD)(namePage + 4096), 0);
    }
    if (checks != 11589 || faults != 69 || throws != 1080)
        return 4;
    printf("ReplaceMaterialTexture: %d cases, %d AV, %d scripted throws\n", checks, faults, throws);
    return 0;
}
