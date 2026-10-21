// Private differential test; the candidate is the actual standalone object.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/Display.h"

static unsigned char referenceSpace[0x800000];
static unsigned char displayStorage[sizeof(UnknownDisplay)];
static UnknownDisplayMode modes[3];
struct Observation {
    int result, calls, kind, argument;
    unsigned char before[sizeof(UnknownDisplay)], after[sizeof(UnknownDisplay)];
    UnknownDisplayMode beforeModes[3], afterModes[3];
};
static Observation observed, expected;
static int callbackResult, mutate;
static UnknownDisplay* Display() { return (UnknownDisplay*)displayStorage; }
static int Callback(UnknownDisplay* self,int kind,int argument) {
    if(self!=Display() || observed.calls++)ExitProcess(81);
    observed.kind=kind;observed.argument=argument;
    memcpy(observed.before,self,sizeof(displayStorage));
    memcpy(observed.beforeModes,modes,sizeof(modes));
    if(mutate) { self->videoMemoryBudget^=0x12345678;self->currentDisplayMode=2;modes[1].field_0x10=-17; }
    return callbackResult;
}
int UnknownDisplay::CreateFlipChain(int buffers) { return Callback(this,1,buffers); }
int UnknownDisplay::CreateSystemRenderSurface(int buffers) { return Callback(this,2,buffers); }
template<class T> static DWORD Address(T method) {
    if(sizeof(method)!=4)ExitProcess(82);DWORD value;memcpy(&value,&method,4);return value;
}
static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    const DWORD sites[]={0x4ca56b,0x4ca582}, targets[]={0x4c9f30,0x4ca130};
    DWORD replacements[]={Address(&UnknownDisplay::CreateFlipChain),Address(&UnknownDisplay::CreateSystemRenderSurface)};
    for(i=0;i<2;++i) {
        if(*(BYTE*)sites[i]!=0xe8 || sites[i]+5+*(DWORD*)(sites[i]+1)!=targets[i])return 0;
        *(DWORD*)(sites[i]+1)=replacements[i]-sites[i]-5;
    }
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static void Run(int retail,int width,int height,int depth,int buffers,int budget,int full) {
    memset(&observed,0,sizeof(observed));memset(displayStorage,0x53,sizeof(displayStorage));
    memset(modes,0x61,sizeof(modes));Display()->displayModes=modes;Display()->currentDisplayMode=1;
    Display()->displayModeCount=3;Display()->videoMemoryBudget=budget;
    modes[1].width=width;modes[1].height=height;modes[1].bitDepth=depth;modes[1].field_0x10=buffers;
    union { int(UnknownDisplay::*method)(int);DWORD address; } call;
    call.method=&UnknownDisplay::CreateModeSurfaces;if(retail)call.address=0x4ca520;
    observed.result=(Display()->*call.method)(full);
    memcpy(observed.after,displayStorage,sizeof(displayStorage));memcpy(observed.afterModes,modes,sizeof(modes));
}
static int checks, fullCalls, windowCalls, rejected;
static void Compare(int width,int height,int depth,int buffers,int budget,int full) {
    Run(1,width,height,depth,buffers,budget,full);expected=observed;
    Run(0,width,height,depth,buffers,budget,full);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))) {
        printf("Mismatch case%d dimensions%d/%d depth%d buffers%d budget%d full%d callback%d mutation%d\n",
               checks,width,height,depth,buffers,budget,full,callbackResult,mutate);ExitProcess(2);
    }
    if(observed.result!=0 && observed.result!=1)ExitProcess(83);
    if(!full && (observed.calls!=1 || observed.kind!=2))ExitProcess(84);
    if(observed.calls && observed.result!=(callbackResult!=0))ExitProcess(85);
    if(observed.kind==1)++fullCalls;else if(observed.kind==2)++windowCalls;else ++rejected;
}
int main(int argc,char** argv) {
    if(argc!=2 || !MapReference(argv[1]))return 3;
    if(sizeof(UnknownDisplay)!=0xb78 || sizeof(UnknownDisplayMode)!=36)return 4;
    const int values[]={0,1,-1,2,-2,7,8,15,16,24,32,480,640,1024,0x7fffffff,(int)0x80000000};
    const int flags[]={0,1,-3}, returns[]={0,1,-1,7};unsigned int random=0x12345678;
    for(int seed=0;seed<16384;++seed) {
        random=random*1664525u+1013904223u;
        int width=values[seed%16],height=values[(seed/16)%16],depth=values[(seed/256)%16];
        int buffers=values[(random>>16)%16],budget=(int)random;
        for(int f=0;f<3;++f)for(int r=0;r<4;++r) {
            callbackResult=returns[r];mutate=(seed+r)&1;
            Compare(width,height,depth,buffers,budget,flags[f]);
        }
    }
    // Exact budget boundary and its neighbors for ordinary display modes.
    const int widths[]={320,640,800,1024,1280},heights[]={240,480,600,768},depths[]={8,16,24,32};
    for(int w=0;w<5;++w)for(int h=0;h<4;++h)for(int d=0;d<4;++d)for(int b=1;b<=4;++b)
    for(int delta=-1;delta<=1;++delta)for(int r=0;r<4;++r) {
        int pixels=widths[w]*heights[h];callbackResult=returns[r];mutate=r&1;
        Compare(widths[w],heights[h],depths[d],b,pixels*depths[d]/8*b+pixels*2+delta,1);
    }
    if(!fullCalls || !windowCalls || !rejected)return 5;
    printf("ModeSurfaces: %d arithmetic, boundary and callback scenarios match\n",checks);return 0;
}
