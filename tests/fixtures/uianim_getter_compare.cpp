// Validation only: actual unchanged 20-byte getter on prepared raw storage.
// The two pages are observed completely; no UIAnim/UIFrame lifetime is constructed.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "GameUi.h"
#include "GUIManager.h"

typedef char AnimationLayout[(sizeof(UIAnim) == 0xf8 && sizeof(UIFrame) == 0x24 &&
    offsetof(UIAnim, currentFrame) == 8 && offsetof(UIAnim, frameList) == 0x28 &&
    offsetof(UIFrame, isSound) == 8 && offsetof(UIFrame, frameTexture) == 0x18 &&
    offsetof(UnknownCursorAnimation, currentFrame) == 8 &&
    offsetof(UnknownCursorAnimation, playForwards) == 0xc &&
    offsetof(UnknownCursorAnimation, frameCount) == 0x10 &&
    offsetof(UnknownCursorAnimation, lastStepTime) == 0x20 &&
    offsetof(GUICursor, cursorAnimation) == 0x5c &&
    sizeof(TextureMap* (UIAnim::*)()) == 4) ? 1 : -1];

static BYTE referenceSpace[0x800000] = {1};
static BYTE* page;
static BYTE* framePage;
static BYTE initial[8192];
static int checks, faults;
struct Result {
    DWORD value, exception, access, address;
    int complete;
};
static Result observed, expected;

static int Filter(EXCEPTION_POINTERS* p) {
    observed.exception = p->ExceptionRecord->ExceptionCode;
    if (observed.exception != EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;
    observed.access = p->ExceptionRecord->ExceptionInformation[0];
    observed.address = p->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}

static void Run(int retail, UIAnim* self) {
    memset(&observed, 0, sizeof(observed));
    __try {
        if (retail) {
            typedef TextureMap* (UIAnim::*Getter)();
            union { Getter method; DWORD address; } call;
            call.address = 0x472f90;
            observed.value = (DWORD)(self->*call.method)();
        } else {
            observed.value = (DWORD)self->GetCurrentTexture();
        }
        observed.complete = 1;
    } __except(Filter(GetExceptionInformation())) {
    }
}

static void Check(UIAnim* self, DWORD value, DWORD faultAddress) {
    memcpy(initial, page, 4096);
    memcpy(initial + 4096, framePage, 4096);
    Run(1, self);
    expected = observed;
    // Assert purity separately before the source call, so shared mutations
    // cannot cancel each other or change the second run's starting state.
    if (memcmp(initial, page, 4096) || memcmp(initial + 4096, framePage, 4096))
        ExitProcess(11);
    Run(0, self);
    checks++;
    if (memcmp(&expected, &observed, sizeof(observed)) ||
        memcmp(initial, page, 4096) || memcmp(initial + 4096, framePage, 4096))
        ExitProcess(12);
    if (faultAddress) {
        faults++;
        if (observed.complete || observed.exception != EXCEPTION_ACCESS_VIOLATION ||
            observed.access || observed.address != faultAddress)
            ExitProcess(13);
    } else if (!observed.complete || observed.exception || observed.value != value) {
        ExitProcess(14);
    }
}

static void Set(UIAnim* self, int index, UIFrame* frame, int sound, DWORD value) {
    memset(page, 0xa5, 4096);
    memset(framePage, 0x5a, 4096);
    self->currentFrame = index;
    self->frameList[index] = frame;
    frame->isSound = sound;
    frame->frameTexture = (TextureMap*)value;
}

static int MapReference(const char* path) {
    BYTE* mapped = (BYTE*)0x400000;
    DWORD old;
    if ((DWORD)referenceSpace > 0x400000 ||
        (DWORD)referenceSpace + sizeof(referenceSpace) < 0x690000 ||
        !VirtualProtect(mapped, 0x290000, PAGE_EXECUTE_READWRITE, &old))
        return 0;
    FILE* file = fopen(path, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    if (length < (long)sizeof(IMAGE_DOS_HEADER)) return 0;
    BYTE* raw = (BYTE*)malloc(length);
    if (!raw || fread(raw, 1, length, file) != (size_t)length) return 0;
    fclose(file);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)raw;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 ||
        dos->e_lfanew > length - (long)sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)(raw + dos->e_lfanew);
    if (pe->Signature != IMAGE_NT_SIGNATURE ||
        pe->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
        pe->OptionalHeader.ImageBase != 0x400000 ||
        pe->OptionalHeader.SizeOfImage > 0x290000 ||
        pe->OptionalHeader.SizeOfHeaders > (DWORD)length ||
        pe->OptionalHeader.SizeOfHeaders > pe->OptionalHeader.SizeOfImage) return 0;
    DWORD offset = (BYTE*)IMAGE_FIRST_SECTION(pe) - raw;
    if (offset > (DWORD)length || pe->FileHeader.NumberOfSections >
        ((DWORD)length - offset) / sizeof(IMAGE_SECTION_HEADER)) return 0;
    memcpy(mapped, raw, pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(pe);
    for (int i = 0; i < pe->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* section = sections + i;
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

int main(int argc, char** argv) {
    if (argc != 2 || !MapReference(argv[1])) return 1;
    page = (BYTE*)VirtualAlloc(0, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    framePage = (BYTE*)VirtualAlloc(0, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    DWORD old;
    if (!page || !framePage ||
        !VirtualProtect(page + 4096, 4096, PAGE_NOACCESS, &old) ||
        !VirtualProtect(framePage + 4096, 4096, PAGE_NOACCESS, &old)) return 2;
    UIAnim* self = (UIAnim*)(page + 128);
    const int sounds[] = {0, 1, 2, -1, 0x7fffffff, (int)0x80000000};
    const DWORD values[] = {0, 1, 0x12345678, 0x80000000, 0xffffffff};
    for (int index = 0; index < 50; index++)
        for (int sound = 0; sound < 6; sound++)
            for (int value = 0; value < 5; value++)
                for (int samePage = 0; samePage < 2; samePage++) {
                    UIFrame* frame = (UIFrame*)(samePage ? page + 1024 : framePage + 128);
                    Set(self, index, frame, sounds[sound], values[value]);
                    Check(self, sounds[sound] == 1 ? 0 : values[value], 0);
                }
    for (int available = 0; available <= 32; available++)
        for (int sound = 0; sound < 2; sound++) {
            memset(page, 0xa5, 4096);
            memset(framePage, 0x5a, 4096);
            UIFrame* frame = (UIFrame*)(framePage + 4096 - available);
            self->currentFrame = 0;
            self->frameList[0] = frame;
            if (available >= 12) frame->isSound = sound;
            if (available >= 28) frame->frameTexture = (TextureMap*)0x12345678;
            DWORD address = 0;
            if (available < 12)
                address = (DWORD)(framePage + 4096 + (available < 8 ? 8 - available : 0));
            else if (!sound && available < 28)
                address = (DWORD)(framePage + 4096 + (available < 24 ? 24 - available : 0));
            Check(self, sound ? 0 : 0x12345678, address);
        }
    Check((UIAnim*)1, 0, 9);
    if (checks != 3067 || faults != 41) return 3;
    printf("UIAnim GetCurrentTexture: %d cases, %d independently checked AV; no storage writes\n",
           checks, faults);
    return 0;
}
