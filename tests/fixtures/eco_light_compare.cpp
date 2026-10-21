// Actual 52-byte lookup and 23-byte append on prepared storage; no object lifetimes.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "LightEmitter.h"

typedef char Layout[(sizeof(LightManager)==0x11c && sizeof(LightEmitter)==0xac &&
    offsetof(LightManager,lightCount)==0x2c && offsetof(LightManager,lights)==0x30 &&
    offsetof(LightEmitter,lightType)==0x2c && offsetof(LightEmitter,manager)==0xa4 &&
    offsetof(LightEmitter,field_0x54)==0x54 && offsetof(LightEmitter,lightDirection)==0x70 &&
    sizeof(LightEmitter*(LightManager::*)(int))==4 &&
    sizeof(void(LightManager::*)(LightEmitter*))==4)?1:-1];

static BYTE referenceSpace[0x800000]={1};
static BYTE* pages[2];
static BYTE initial[16384], expectedBytes[16384], actualBytes[16384];
static int readOnlyFirst, checks, faults;
struct Result { DWORD value, exception, access, address; int complete; };
static Result expected, observed;
static int Filter(EXCEPTION_POINTERS* p) {
    if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=EXCEPTION_ACCESS_VIOLATION;
    observed.access=p->ExceptionRecord->ExceptionInformation[0];
    observed.address=p->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail,int kind,LightManager* self,LightEmitter* light,int type) {
    memset(&observed,0,sizeof(observed));
    __try {
        if(!kind) {
            if(retail) {
                union { LightEmitter*(LightManager::*method)(int); DWORD address; } call;
                call.address=0x4a0190;
                observed.value=(DWORD)(self->*call.method)(type);
            } else observed.value=(DWORD)self->FindLightOfType(type);
        } else {
            if(retail) {
                union { void(LightManager::*method)(LightEmitter*); DWORD address; } call;
                call.address=0x49e470;
                (self->*call.method)(light);
            } else self->AddLight(light);
        }
        observed.complete=1;
    } __except(Filter(GetExceptionInformation())) {}
}
static void Capture(BYTE* bytes) {
    memcpy(bytes,pages[0],8192); memcpy(bytes+8192,pages[1],8192);
}
static void Restore() {
    DWORD old;
    if(!VirtualProtect(pages[0],4096,PAGE_READWRITE,&old)) ExitProcess(20);
    memcpy(pages[0],initial,8192); memcpy(pages[1],initial+8192,8192);
    if(readOnlyFirst && !VirtualProtect(pages[0],4096,PAGE_READONLY,&old)) ExitProcess(21);
}
// The oracle observes exact byte addresses and the three ordered append stores.
// Guard cases are DWORD aligned; no claim about split-page partial DWORD stores.
static int Access(DWORD address,int write,DWORD* value) {
    for(int i=0;i<2;i++) {
        DWORD offset=address-(DWORD)pages[i];
        if(offset<=8192-4) {
            if(write && !i && readOnlyFirst && offset<4096) break;
            if(write) memcpy(expectedBytes+i*8192+offset,value,4);
            else memcpy(value,expectedBytes+i*8192+offset,4);
            return 1;
        }
    }
    expected.exception=EXCEPTION_ACCESS_VIOLATION;
    expected.access=write;
    expected.address=address;
    return 0;
}
static void Model(int kind,DWORD self,DWORD light,int type) {
    memset(&expected,0,sizeof(expected));
    memcpy(expectedBytes,initial,sizeof(initial));
    DWORD count,entry,value;
    if(!Access(self+0x2c,0,&count)) return;
    if(kind) {
        value=light;
        if(!Access(self+0x30+count*4,1,&value)) return;
        value=self;
        if(!Access(light+0xa4,1,&value)) return;
        if(!Access(self+0x2c,0,&count)) return;
        count++;
        if(!Access(self+0x2c,1,&count)) return;
    } else {
        for(int i=0;i<(int)count;i++) {
            if(!Access(self+0x30+i*4,0,&entry)) return;
            if(!Access(entry+0x2c,0,&value)) return;
            if(value==(DWORD)type) { expected.value=entry; break; }
        }
    }
    expected.complete=1;
}
static void Check(int kind,LightManager* self,LightEmitter* light,int type) {
    Capture(initial); Model(kind,(DWORD)self,(DWORD)light,type);
    for(int retail=1;retail>=0;retail--) {
        Restore(); Run(retail,kind,self,light,type); Capture(actualBytes);
        if(memcmp(&expected,&observed,sizeof(expected)) ||
           memcmp(expectedBytes,actualBytes,sizeof(expectedBytes))) {
            printf("Mismatch case %d kind %d retail %d: value %08lx/%08lx fault %08lx/%08lx address %08lx/%08lx\n",
                checks,kind,retail,expected.value,observed.value,expected.exception,observed.exception,
                expected.address,observed.address); ExitProcess(3);
        }
    }
    checks++; if(expected.exception) faults++;
}
static void Setup(int seed) {
    DWORD old;
    if(!VirtualProtect(pages[0],4096,PAGE_READWRITE,&old)) ExitProcess(22);
    readOnlyFirst=0;
    for(int i=0;i<8192;i++) {
        pages[0][i]=(BYTE)(seed*19+i*37); pages[1][i]=(BYTE)(seed*53+i*11);
    }
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


int main(int argc,char**argv) {
    if(argc!=2 || !MapReference(argv[1])) return 1;
    DWORD old;
    for(int p=0;p<2;p++) {
        pages[p]=(BYTE*)VirtualAlloc(0,12288,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if(!pages[p] || !VirtualProtect(pages[p]+8192,4096,PAGE_NOACCESS,&old)) return 2;
    }
    const int counts[]={-2147483647-1,-1,0,1,2,7,49,50};
    const int types[]={-2147483647-1,-1,0,1,2,4,6,2147483647};
    for(int seed=0;seed<32;seed++)
        for(int c=0;c<8;c++)
            for(int mode=0;mode<4;mode++) {
                Setup(seed);
                LightManager* self=(LightManager*)(pages[0]+512);
                self->lightCount=counts[c];
                int wanted=types[seed%8];
                int position=mode==0?0:mode==1?counts[c]/2:counts[c]-1;
                for(int i=0;i<counts[c];i++) {
                    LightEmitter* light=(LightEmitter*)(pages[1]+128+i*128);
                    light->lightType=mode!=3 && i>=position?wanted:types[(seed+1)%8];
                    self->lights[i]=light;
                }
                Check(0,self,0,wanted);
            }
    // Every valid append index, including owner aliases to the count and slot.
    for(seed=0;seed<16;seed++) for(int count=0;count<50;count++)
        for(int alias=0;alias<4;alias++) {
            Setup(seed);
            LightManager* self=(LightManager*)(pages[0]+512);
            self->lightCount=count;
            LightEmitter* light=(LightEmitter*)(alias==0?pages[1]+128:alias==1?pages[0]+2048:
                alias==2?(BYTE*)self+0x2c-0xa4:(BYTE*)self+0x30+count*4-0xa4);
            Check(1,self,light,0);
        }
    // Three independently modeled fault stages of append: slot, owner, count.
    for(seed=0;seed<32;seed++) {
        Setup(seed); LightManager* self=(LightManager*)(pages[0]+512);
        self->lightCount=0; Check(0,(LightManager*)0,0,6);
        Setup(seed); self->lightCount=1; self->lights[0]=(LightEmitter*)(pages[1]+8192-0x2c);
        Check(0,self,0,6);
        Setup(seed); self=(LightManager*)(pages[0]+8192-0x30); self->lightCount=1;
        Check(0,self,0,6);
        Setup(seed); self=(LightManager*)(pages[0]+8192-0x30); self->lightCount=0;
        Check(0,self,0,6); // count0 does not read the inaccessible first slot
        Setup(seed); self->lightCount=0;
        Check(1,self,(LightEmitter*)(pages[1]+128),0); // first slot write faults
        Setup(seed); self=(LightManager*)(pages[0]+512); self->lightCount=0;
        Check(1,self,(LightEmitter*)(pages[1]+8192-0xa4),0); // slot write survives
        Setup(seed); self=(LightManager*)(pages[0]+4096-0x30); self->lightCount=0;
        readOnlyFirst=1;
        Check(1,self,(LightEmitter*)(pages[1]+128),0); // slot and owner survive count fault
        Setup(seed); self=(LightManager*)(pages[0]+512); self->lightCount=1;
        self->lights[0]=(LightEmitter*)(pages[1]+128); self->lights[0]->lightType=6;
        self->lights[1]=(LightEmitter*)0; self->lightCount=2;
        Check(0,self,0,6); // first match prevents reading invalid later pointer
    }
    printf("Eco light: %d cases, %d independently modeled AV\n",checks,faults);
    return 0;
}
