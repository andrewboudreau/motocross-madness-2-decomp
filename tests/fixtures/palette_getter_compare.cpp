// Validation only: actual scalar converters and getters; dither is excluded.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "Pixtrans.h"
#include "TextureMap.h"
#include "Quantize.h"
typedef char PaletteLayout[sizeof(ColorMapper)==0x18b10 && offsetof(ColorMapper,indexFrom555)==0x710 && offsetof(ColorMapper,indexFrom565)==0x8710 ? 1:-1];
typedef char PrefixLayout[sizeof(UnknownTexturePalette)==0x8710 && offsetof(UnknownTexturePalette,indexFrom555)==0x710 ? 1:-1];
typedef char PixelLayout[sizeof(UnknownPixel24)==3 && offsetof(UnknownPixel24,red)==0 && offsetof(UnknownPixel24,green)==1 && offsetof(UnknownPixel24,blue)==2 ? 1:-1];
static BYTE referenceSpace[0x800000]={1};
static BYTE initial[0x1d000],storage[0x1d000],oracle[0x1d000];
static BYTE* page;
struct Result { BYTE bytes[0x1d000],page[4096]; DWORD result,exception,kind,address; };
static Result actual,expected;
static int checks,faults;
int DitherConvert(void*,int,int,int,int,int,void*,void*,void*,void*,UnknownTexturePalette*) { ExitProcess(90);return 0; }
static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++) {
        if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);
}

static int Filter(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    actual.exception=e->ExceptionRecord->ExceptionCode;
    actual.kind=e->ExceptionRecord->ExceptionInformation[0];
    actual.address=e->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
typedef int (__cdecl* Convert)(void*,void*,int,int,int,int,int,UnknownTexturePalette*);
static void Seed(unsigned seed) {
    DWORD random=seed+1;
    for(int i=0;i<sizeof(initial);i++) {random=random*1664525+1013904223;initial[i]=(BYTE)(random>>24);}
}
static int SourceOffset(){return 0x1a100;}
static int DestinationOffset(int mode) {
    const int offsets[]={0x1b100,0x1a100,0x1a101,0x1a0ff,0x1800,0x9800};return offsets[mode];
}
static void Run(int retail,int format,int width,int height,int ds,int ss,int alias,int fault) {
    memset(&actual,0,sizeof(actual));memcpy(storage,initial,sizeof(storage));
    for(int i=0;i<4096;i++)page[i]=(BYTE)(i*17+3);
    BYTE* src=storage+SourceOffset();BYTE* dst=storage+DestinationOffset(alias);
    BYTE* palette=storage+0x1000;
    if(fault==1)src=0;
    if(fault==2)dst=0;
    if(fault==3)palette=0;
    if(fault>=4&&fault<=6)src=page+4096-(fault-4);
    if(fault==7)dst=page+4096;
    if(fault==8)dst=page+4095;
    if(fault==9) {
        memset(src,0,3);
        palette=page+4096-(format==24?0x710:0x8710);
    }
    if(fault==10) {src=0;dst=0;palette=0;}
    Convert function=retail?(Convert)(format==24?0x4d0aa0:0x4d0b90):(format==24?&Convert24To8:&Convert565To8);
    __try {actual.result=function(dst,src,width,height,ds,ss,0,(UnknownTexturePalette*)palette);}
    __except(Filter(GetExceptionInformation())) {}
    memcpy(actual.bytes,storage,sizeof(storage));memcpy(actual.page,page,4096);
}
static void Oracle(int format,int width,int height,int ds,int ss,int alias) {
    memcpy(oracle,initial,sizeof(oracle));
    int bytes=format==24?3:2;
    for(int y=0;y<height;y++)for(int x=0;x<width;x++) {
        BYTE* src=oracle+SourceOffset()+y*ss*bytes+x*bytes;
        unsigned index=format==24?((unsigned(src[0])/8)*1024+(unsigned(src[1])/8)*32+unsigned(src[2])/8):(unsigned(src[0])+unsigned(src[1])*256);
        BYTE value=oracle[0x1000+(format==24?0x710:0x8710)+index];
        oracle[DestinationOffset(alias)+y*ds+x]=value;
    }
}
static void Check(int format,int width,int height,int ds,int ss,int alias,int fault) {
    Run(1,format,width,height,ds,ss,alias,fault);expected=actual;
    Run(0,format,width,height,ds,ss,alias,fault);
    if(memcmp(&actual,&expected,sizeof(actual))) {printf("FAIL %d format%d w%d h%d ds%d ss%d alias%d fault%d\n",checks,format,width,height,ds,ss,alias,fault);exit(1);}
    if(!fault) {
        Oracle(format,width,height,ds,ss,alias);
        if(actual.exception||actual.result!=1||memcmp(actual.bytes,oracle,sizeof(oracle)))ExitProcess(6);
    } else if(width>0&&height>0) {
        if(actual.exception!=EXCEPTION_ACCESS_VIOLATION)ExitProcess(7);
        faults++;
    } else if(actual.exception||actual.result!=1)ExitProcess(8);
    checks++;
}
static void Getters() {
    typedef unsigned char* (ColorMapper::*Getter)();
    union {Getter member;DWORD address;} function;
    if(sizeof(Getter)!=4)ExitProcess(10);
    const DWORD values[]={0,1,0x1000,0x7fffffff,0xfffffff8,0xffffffff};
    for(int table=0;table<2;table++)for(int i=0;i<6;i++) {
        ColorMapper* self=(ColorMapper*)values[i];
        function.address=table?0x4de290:0x4de280;
        DWORD reference=(DWORD)(self->*function.member)();
        DWORD result=(DWORD)(table?self->GetIndexFrom565():self->GetIndexFrom555());
        if(result!=reference||result!=values[i]+(table?0x8710:0x710))ExitProcess(11);
        checks++;
    }
    ColorMapper* palette=(ColorMapper*)(storage+0x1000);
    if(palette->GetIndexFrom555()!=storage+0x1710||palette->GetIndexFrom565()!=storage+0x9710)ExitProcess(12);
    checks++;
}
int main(int argc,char**argv) {
    if(argc!=2||!MapReference(argv[1]))return 3;
    page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;
    if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 4;
    Getters();
    const int widths[]={-1,0,1,2,7,16,31},heights[]={-1,0,1,3};
    for(int seed=0;seed<8;seed++) {Seed(seed);
        for(int f=0;f<2;f++)for(int w=0;w<7;w++)for(int h=0;h<4;h++)for(int stride=0;stride<3;stride++)for(int alias=0;alias<6;alias++) {
            int step=stride==0?40:(stride==1?0:-40);
            Check(f?565:24,widths[w],heights[h],step,step,alias,0);
        }
    }
    for(int f=0;f<2;f++)for(int fault=1;fault<=10;fault++) {
        Seed(31);Check(f?565:24,3,2,8,8,0,fault);
        Check(f?565:24,0,2,8,8,0,fault);Check(f?565:24,3,0,8,8,0,fault);
    }
    // Destination guard allows exactly one write before the next pixel faults.
    Seed(17);Check(24,3,1,8,8,0,8);
    unsigned index=(initial[SourceOffset()]/8)*1024+(initial[SourceOffset()+1]/8)*32+initial[SourceOffset()+2]/8;
    if(actual.kind!=1||actual.address!=(DWORD)(page+4096)||page[4095]!=initial[0x1710+index])return 13;
    // Simultaneously bad operands read source first; table getters themselves do not dereference.
    Check(24,1,1,1,1,0,10);
    if(actual.kind!=0||actual.address!=0)return 14;
    printf("Palette getters: %d cases, %d access faults, scalar buffers and aliases match\n",checks,faults);
    return 0;
}
