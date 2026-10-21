// Private differential validation only. Production never loads reference code.
// All allocation/stream/merge operations below are scripted test dependencies.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "../../src/reconstructed/ResourceManager.h"
#include "../../src/reconstructed/FileStream.h"
#include "../../src/reconstructed/DebugAlloc.h"

extern "C" void __cdecl __CxxFrameHandler();

struct Scenario {
    int count, items, itemCapacity, archives, archiveCapacity;
    int flags, valid, opens, badMagic, failRead, failRealloc, mergeMask;
    int throwKind, throwAt;
};
struct Block { int size, alive; unsigned char bytes[1024]; };
struct Observation {
    int result, exception, events, blocks;
    int trace[512][6];
    unsigned char manager[sizeof(UnknownResourceManager)];
    Block heap[48];
};
static Scenario scenario;
static Observation observed, referenceResult;
static Block heap[48];
static int blocks, reads, reallocs, entries, constructors, allocations;
static UnknownResourceManager* manager;
static union { double alignment; unsigned char bytes[sizeof(UnknownResourceManager)]; } managerStorage;
static unsigned char referenceSpace[0x200000];

static void Event(int op, int a = 0, int b = 0, int c = 0, int d = 0, int e = 0) {
    if (observed.events >= 512) ExitProcess(90);
    int* out = observed.trace[observed.events++];
    out[0] = op; out[1] = a; out[2] = b; out[3] = c; out[4] = d; out[5] = e;
}
static int Identity(const void* p) {
    if (!p) return 0;
    if (p == manager) return -1;
    for (int i = 0; i < blocks; ++i)
        if ((DWORD)p >= (DWORD)heap[i].bytes && (DWORD)p < (DWORD)heap[i].bytes + 1024)
            return (i + 1) * 2048 + (DWORD)p - (DWORD)heap[i].bytes;
    return -2; // A stack output buffer, never a retained pointer.
}
static unsigned int Hash(const void* data, int size) {
    unsigned int value = 2166136261U;
    for (int i = 0; i < size; ++i) value = (value ^ ((const unsigned char*)data)[i]) * 16777619U;
    return value;
}
static void* Allocate(int size) {
    if (size < 0 || size > 1024 || blocks >= 48) ExitProcess(91);
    Block* b = heap + blocks++;
    b->size = size; b->alive = 1;
    memset(b->bytes, 0xa5, sizeof(b->bytes));
    return b->bytes;
}
static void Retire(void* p) {
    if (!p) return;
    for (int i = 0; i < blocks; ++i)
        if (p == heap[i].bytes) {
            if (!heap[i].alive) ExitProcess(92);
            heap[i].alive = 0;
            return;
        }
    ExitProcess(93);
}
// Diagnostic source paths differ between rebuilt and retail code. Validate
// each expected path before normalizing that diagnostic-only argument.
static int SourceFile(const char* file) {
    const char* name = strrchr(file, '\\');
    const char* slash = strrchr(file, '/');
    if (!name || (slash && slash > name)) name = slash;
    name = name ? name + 1 : file;
    if (strcmp(name, "ResourceManager.cpp") && strcmp(name, "ResourceManagerNearMisses.cpp")) ExitProcess(96);
    return 1;
}
void* operator new(unsigned int size, const char* file, int line) {
    Event(1, size, line, ++allocations, SourceFile(file));
    if ((scenario.throwKind == 1 && line == 212) ||
        (scenario.throwKind == 3 && line == 261 && allocations - 1 == scenario.throwAt))
        throw 71;
    void* p = Allocate(size);
    Event(2, Identity(p));
    return p;
}
void operator delete(void* p) {
    if (!p) return;
    if (Identity(p) == -2) { free(p); return; }
    Event(3, Identity(p)); Retire(p);
}
void operator delete(void* p, const char* file, int line) {
    Event(4, Identity(p), line, SourceFile(file)); Retire(p);
}
void* DebugRealloc(void* p, unsigned int size, const char* file, int line) {
    Event(5, Identity(p), size, line, ++reallocs, SourceFile(file));
    if (reallocs == scenario.failRealloc) return 0;
    void* result = Allocate(size);
    if (p) {
        int index = Identity(p) / 2048 - 1;
        if (index < 0) ExitProcess(94);
        memcpy(result, p, heap[index].size < (int)size ? heap[index].size : size);
        Retire(p);
    }
    Event(6, Identity(result));
    return result;
}
BaseObject::BaseObject() { refCount = 1; }
BaseObject::~BaseObject() {}
int BaseObject::AddRef() { return ++refCount; }
int BaseObject::GetRefCount() { return refCount; }
int BaseObject::Release() {
    Event(7, Identity(this), refCount);
    if (!--refCount) { delete this; return 0; }
    return refCount;
}
ResourceItem::ResourceItem() {
    Event(8, Identity(this), ++constructors);
    if (scenario.throwKind == 4 && constructors == scenario.throwAt) throw 73;
    nameSize = 1; name = (char*)Allocate(1); name[0] = 0;
    object = 0; archive = 0; offset = 0; size = 0;
}
ResourceItem::~ResourceItem() {
    Event(9, Identity(this), Identity(name));
    Retire(name);
}
UnknownTextureStream::UnknownTextureStream(UnknownResourceManager* owner) {
    Event(10, Identity(this), Identity(owner));
    if (scenario.throwKind == 2) throw 72;
    memset(this, 0, sizeof(*this));
    archives = owner;
}
UnknownTextureStream::~UnknownTextureStream() { Event(11, Identity(this)); }
int UnknownTextureStream::IsValidFile(const char* path) {
    Event(12, Identity(this), Hash(path, strlen(path)), scenario.valid);
    return scenario.valid;
}
int UnknownTextureStream::Open(const char* path, const char* mode, int flag) {
    Event(13, Identity(this), Hash(path, strlen(path)), Hash(mode, strlen(mode)), flag);
    return scenario.opens;
}
int UnknownTextureStream::Read(void* buffer, int size, int count) {
    int number = ++reads, total = size * count;
    if (total < 0 || total > 64) ExitProcess(95);
    unsigned char data[64]; memset(data, 0, sizeof(data));
    if (number == 1) memcpy(data, scenario.badMagic == 1 ? "BAD" : (scenario.badMagic == 2 ? "rs2" : "RS2"), 4);
    else if (number == 2) memcpy(data, &scenario.count, 4);
    else {
        int entry = (number - 3) / 4, field = (number - 3) % 4;
        int value;
        if (field == 0) { value = 8; memcpy(data, &value, 4); }
        if (field == 1) { sprintf((char*)data, "entry%d", entry); }
        if (field == 2) { value = 113 + entry * 37; memcpy(data, &value, 4); }
        if (field == 3) { value = 211 + entry * 53; memcpy(data, &value, 4); }
    }
    Event(14, Identity(this), Identity(buffer), size, count, number);
    // Failed reads leave constructor-initialized fields untouched. Later
    // per-entry read failures are intentionally ignored by the retail body.
    // A null destination from failed name realloc is a scripted failure,
    // not evidence that the real stream tolerates allocation failure.
    if (number == scenario.failRead || !buffer) { Event(15, 0); return 0; }
    memcpy(buffer, data, total);
    Event(15, count, Hash(data, total));
    return count;
}
int UnknownResourceManager::MergeWithExisting(ResourceItem* item) {
    int ordinal = entries++;
    Event(16, Identity(this), Identity(item), (item->name ? Hash(item->name, item->nameSize) : 0), item->offset, item->size);
    int merged = (scenario.mergeMask >> ordinal) & 1;
    if (merged && itemCount) {
        // Model the mutation that makes a duplicate replace existing metadata.
        items[0]->archive = item->archive;
        items[0]->offset = item->offset;
        items[0]->size = item->size;
    }
    Event(17, merged);
    return merged;
}

// Constructors cannot be addressed in C++; these test-only thiscall entry
// points call the identical scripted constructors at the supplied address.
struct ConstructorCalls {
    UnknownTextureStream* Stream(UnknownResourceManager* owner) {
        return ::new(this) UnknownTextureStream(owner);
    }
    ResourceItem* Item() { return ::new(this) ResourceItem; }
};
template<class T> static DWORD Address(T method) {
    union { T method; DWORD address; } value;
    value.method = method; return value.address;
}
struct DestructorCall { void Stream() { ((UnknownTextureStream*)this)->~UnknownTextureStream(); } };
static DWORD Replacement(DWORD target) {
    switch (target) {
    case 0x4a3010: return (DWORD)(void* (__cdecl*)(unsigned int, const char*, int))operator new;
    case 0x4a3060: return (DWORD)(void (__cdecl*)(void*, const char*, int))operator delete;
    case 0x4a30c0: return (DWORD)(void (__cdecl*)(void*))operator delete;
    case 0x460d10: return Address(&ConstructorCalls::Stream);
    case 0x460d60: return Address(&DestructorCall::Stream);
    case 0x4e8d70: return Address(&ConstructorCalls::Item);
    case 0x460e70: return Address(&UnknownTextureStream::IsValidFile);
    case 0x460f50: return Address(&UnknownTextureStream::Open);
    case 0x461640: return Address(&UnknownTextureStream::Read);
    case 0x4a2ec0: return (DWORD)DebugRealloc;
    case 0x4e8f40: return Address(&UnknownResourceManager::MergeWithExisting);
    case 0x536070: return (DWORD)_stricmp;
    case 0x53471a: return (DWORD)__CxxFrameHandler;
    }
    return 0;
}
static int MapReference(const char* path, const char* patchPath) {
    FILE* file = fopen(path, "rb"); if (!file) return 0;
    fseek(file, 0, SEEK_END); long size = ftell(file); rewind(file);
    unsigned char* image = (unsigned char*)malloc(size);
    if (!image || fread(image, 1, size, file) != (size_t)size) return 0;
    fclose(file);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)image;
    if (size < sizeof(*dos) || dos->e_magic != IMAGE_DOS_SIGNATURE ||
        dos->e_lfanew < 0 || dos->e_lfanew > size - sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(image + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != 0x14c ||
        nt->OptionalHeader.ImageBase != 0x400000) return 0;
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    // Function, unwind funclets/handler, FuncInfo/unwind map, and literals.
    const DWORD pages[] = {0x4e9000, 0x54d000, 0x563000, 0x566000, 0x572000};
    for (int n = 0; n < sizeof(pages) / sizeof(*pages); ++n) {
        DWORD page = pages[n], rva = page - 0x400000, protection;
        if (page < (DWORD)referenceSpace || page + 0x1000 > (DWORD)referenceSpace + sizeof(referenceSpace)) return 0;
        if (!VirtualProtect((void*)page, 0x1000, PAGE_EXECUTE_READWRITE, &protection)) return 0;
        int copied = 0;
        for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER* section = sections + i;
            if (rva >= section->VirtualAddress && rva + 0x1000 <= section->VirtualAddress + section->SizeOfRawData) {
                DWORD offset = section->PointerToRawData + rva - section->VirtualAddress;
                if (offset + 0x1000 > (DWORD)size) return 0;
                memcpy((void*)page, image + offset, 0x1000); copied = 1;
            }
        }
        if (!copied) return 0;
    }
    free(image);
    file = fopen(patchPath, "rb"); if (!file) return 0;
    DWORD patch[3];
    while (fread(patch, sizeof(patch), 1, file) == 1) {
        DWORD at = patch[0], target = patch[1], replacement = Replacement(target);
        int allowed = (at >= 0x4e9030 && at + 5 <= 0x4e9354) ||
                      (at >= 0x54daa0 && at + 5 <= 0x54dad8);
        if (!allowed || (patch[2] != 0xe8 && patch[2] != 0xe9) ||
            !replacement || *(unsigned char*)at != patch[2] ||
            at + 5 + *(DWORD*)(at + 1) != target) return 0;
        *(DWORD*)(at + 1) = replacement - at - 5;
    }
    fclose(file);
    FlushInstructionCache(GetCurrentProcess(), (void*)0x4e9000, 0x1000);
    FlushInstructionCache(GetCurrentProcess(), (void*)0x54d000, 0x1000);
    return 1;
}
static void Run(int reference) {
    memset(&observed, 0, sizeof(observed)); memset(heap, 0, sizeof(heap));
    memset(managerStorage.bytes, 0, sizeof(managerStorage.bytes));
    manager = (UnknownResourceManager*)managerStorage.bytes;
    blocks = reads = reallocs = entries = constructors = allocations = 0;
    manager->itemCount = scenario.items; manager->itemCapacity = scenario.itemCapacity;
    manager->archiveCount = scenario.archives; manager->archiveCapacity = scenario.archiveCapacity;
    if (scenario.itemCapacity) {
        manager->items = (ResourceItem**)Allocate(scenario.itemCapacity * 4);
        memset(manager->items, 0, scenario.itemCapacity * 4);
        for (int i = 0; i < scenario.items; ++i) {
            manager->items[i] = ::new(Allocate(sizeof(ResourceItem))) ResourceItem;
            manager->items[i]->offset = 900 + i; manager->items[i]->size = 700 + i;
        }
    }
    if (scenario.archiveCapacity) {
        manager->archives = (UnknownTextureStream**)Allocate(scenario.archiveCapacity * 4);
        memset(manager->archives, 0, scenario.archiveCapacity * 4);
        for (int j = 0; j < scenario.archives; ++j)
            manager->archives[j] = ::new(Allocate(sizeof(UnknownTextureStream))) UnknownTextureStream(manager);
    }
    observed.events = 0; constructors = allocations = 0;
    union { UnknownTextureStream* (UnknownResourceManager::*method)(const char*, int); DWORD address; } call;
    call.method = &UnknownResourceManager::AddArchive;
    if (reference) call.address = 0x4e9030;
    try {
        observed.result = Identity((manager->*call.method)("C:\\game\\test.rs2", scenario.flags));
    } catch (int code) { observed.exception = code; }
    observed.blocks = blocks;
    memcpy(observed.manager, manager, sizeof(*manager));
    memcpy(observed.heap, heap, sizeof(heap));
}
static int checks;
static void Compare() {
    Run(1); referenceResult = observed; Run(0); ++checks;
    if (memcmp(&referenceResult, &observed, sizeof(observed))) {
        const int* r = (const int*)&referenceResult; const int* c = (const int*)&observed;
        int first = 0; while (first < sizeof(observed) / 4 && r[first] == c[first]) ++first;
        printf("Mismatch %d dword %d: %x/%x count%d items%d/%d archives%d/%d flag%d valid%d open%d magic%d read%d realloc%d merge%d throw%d/%d\n",
               checks, first, r[first], c[first], scenario.count, scenario.items, scenario.itemCapacity,
               scenario.archives, scenario.archiveCapacity, scenario.flags, scenario.valid, scenario.opens,
               scenario.badMagic, scenario.failRead, scenario.failRealloc, scenario.mergeMask,
               scenario.throwKind, scenario.throwAt);
        ExitProcess(2);
    }
}
int main(int argc, char** argv) {
    if (argc != 3 || !MapReference(argv[1], argv[2])) return 3;
    const int itemCounts[] = {0, 1, 2}, extraCapacities[] = {0, 1, 4};
    const int counts[] = {-1, 0, 1, 3}, flags[] = {0, 1, -7};
    for (int i = 0; i < 3; ++i) for (int cap = 0; cap < 3; ++cap)
    for (int ac = 0; ac < 2; ++ac) for (int spare = 0; spare < 2; ++spare)
    for (int n = 0; n < 4; ++n) for (int f = 0; f < 3; ++f)
    for (int valid = 0; valid < 2; ++valid) for (int mode = 0; mode < 20; ++mode) {
        memset(&scenario, 0, sizeof(scenario));
        scenario.items = itemCounts[i]; scenario.itemCapacity = scenario.items + extraCapacities[cap];
        scenario.archives = ac; scenario.archiveCapacity = ac + spare * 8;
        scenario.count = counts[n]; scenario.flags = flags[f]; scenario.valid = valid; scenario.opens = 1;
        if (mode == 1) scenario.opens = 0;
        if (mode == 2) scenario.badMagic = 1;
        if (mode >= 3 && mode <= 12) scenario.failRead = mode - 2;
        if (mode >= 13 && mode <= 15) scenario.failRealloc = mode - 12;
        if (mode == 16) scenario.mergeMask = 1;
        if (mode == 17) scenario.mergeMask = 2;
        if (mode == 18) scenario.mergeMask = 7;
        if (mode == 19) scenario.badMagic = 2;
        Compare();
    }
    // Throw paths start with no pre-existing objects so setup itself cannot throw.
    for (int kind = 1; kind <= 4; ++kind) for (int at = 1; at <= 3; ++at) {
        memset(&scenario, 0, sizeof(scenario)); scenario.opens = 1; scenario.count = 3;
        scenario.throwKind = kind; scenario.throwAt = at; Compare();
    }
    printf("AddArchive: %d scenarios match state, allocation ownership and ordered calls\n", checks);
    return 0;
}
