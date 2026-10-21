// Private native comparison of the actual standalone VectorCalls.cpp object.
// These four retail functions have no relocations, imports or callees. Only the
// test maps the hash-pinned image; the production functions are ordinary C++.
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/Math3D.h"

static unsigned char referenceSpace[0x800000];
static float storage[32];
static unsigned long initial[32], expected[32];

static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000; DWORD old;
    if ((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
        !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* f=fopen(path,"rb"); if (!f) return 0;
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    BYTE* raw=(BYTE*)malloc(size); if (!raw || fread(raw,1,size,f)!=(size_t)size) return 0;
    fclose(f); IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if (pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000) return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for (int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if (section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
            section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage) return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw); return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

static void Seed(unsigned int seed) {
    const unsigned long edges[]={0,0x80000000,0x3f800000,0xbf800000,0x67800000,
        0xe7800000,0x00800000,0x00000001,0x7f7fffff,0x7f800000,0xff800000,
        0x7fc12345,0x7f812345,0xffc54321,0x3eaaaaab,0xbeaaaaab};
    unsigned int random=seed+1;
    for (int i=0;i<32;++i) {
        random=random*1664525u+1013904223u;
        initial[i]=seed<16?edges[(seed+i)%16]:random;
    }
}

typedef Vec3* (__cdecl* Binary)(Vec3*,const Vec3*,const Vec3*);
typedef Vec3* (__cdecl* Scale)(Vec3*,const Vec3*,float);
typedef float (__cdecl* Dot)(const Vec3*,const Vec3*);

static int CheckBinary(Binary source,Binary retail,int b,int out) {
    Vec3* a=(Vec3*)(storage+8); Vec3* other=(Vec3*)(storage+b); Vec3* result=(Vec3*)(storage+out);
    memcpy(storage,initial,sizeof(storage)); _clearfp();
    if (retail(result,a,other)!=result) return 0;
    memcpy(expected,storage,sizeof(storage));
    memcpy(storage,initial,sizeof(storage)); _clearfp();
    return source(result,a,other)==result && !memcmp(storage,expected,sizeof(storage));
}

static int CheckScale(int out,unsigned long bits) {
    float scale; memcpy(&scale,&bits,4);
    Vec3* a=(Vec3*)(storage+8); Vec3* result=(Vec3*)(storage+out);
    memcpy(storage,initial,sizeof(storage)); _clearfp();
    if (((Scale)0x5015b0)(result,a,scale)!=result) return 0;
    memcpy(expected,storage,sizeof(storage));
    memcpy(storage,initial,sizeof(storage)); _clearfp();
    return Vec3ScaleCall(result,a,scale)==result && !memcmp(storage,expected,sizeof(storage));
}

static int CheckDot(int b) {
    Vec3* a=(Vec3*)(storage+8); Vec3* other=(Vec3*)(storage+b);
    memcpy(storage,initial,sizeof(storage)); _clearfp();
    float retail=((Dot)0x40ae30)(a,other);
    memcpy(expected,storage,sizeof(storage));
    memcpy(storage,initial,sizeof(storage)); _clearfp();
    float source=Vec3DotCall(a,other);
    return !memcmp(&retail,&source,4) && !memcmp(storage,expected,sizeof(storage));
}

int main(int argc,char** argv) {
    if (argc!=2 || !MapReference(argv[1])) return 3;
    const int inputs[]={6,7,8,9,10,20,22};
    const int outputs[]={0,6,7,8,9,10,18,19,20,21,22};
    const unsigned long scales[]={0,0x80000000,0x3f800000,0xbf800000,
        0x67800000,0x00800000,0x7f800000,0x7fc12345};
    int checks=0;
    for (int seed=0;seed<128;++seed) {
        Seed(seed);
        for (int b=0;b<7;++b) {
            for (int out=0;out<11;++out) {
                if (!CheckBinary(Vec3AddCall,(Binary)0x421cb0,inputs[b],outputs[out]) ||
                    !CheckBinary(Vec3SubtractCall,(Binary)0x421d00,inputs[b],outputs[out])) {
                    printf("Binary mismatch seed%d b%d out%d\n",seed,b,out); return 2;
                }
                checks+=2;
            }
            if (!CheckDot(inputs[b])) { printf("Dot mismatch seed%d b%d\n",seed,b); return 2; }
            ++checks;
        }
        for (int out=0;out<11;++out) for (int s=0;s<8;++s) {
            if (!CheckScale(outputs[out],scales[s])) {
                printf("Scale mismatch seed%d s%d out%d\n",seed,s,out); return 2;
            }
            ++checks;
        }
    }
    // 2^80 cancels exactly before z is added. Grouping z with -2^80 first
    // loses the one even at x87 extended precision; do not route to DotProduct.
    Vec3 a(1.2089258196146292e24f,-1.2089258196146292e24f,1.0f), b(1.0f,1.0f,1.0f);
    _clearfp();
    if (Vec3DotCall(&a,&b)!=1.0f || ((Dot)0x40ae30)(&a,&b)!=1.0f) return 4;
    ++checks;
    printf("VectorCalls: %d alias, return and arithmetic scenarios match\n",checks);
    return 0;
}
