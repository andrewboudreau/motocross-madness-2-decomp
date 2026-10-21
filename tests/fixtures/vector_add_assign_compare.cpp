// Native proof of the actual BVH provider and an identical canonical-header emission.
// Test-only member pointers use the independently decoded thiscall/ret4 boundary.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "../../src/reconstructed/VectorAddAssign.h"
extern Vec3& (Vec3::*g_CanonicalAdd)(const Vec3&);
typedef char Layout[(sizeof(Vec3AddAssignCall)==12 && sizeof(Vec3)==12 &&
    offsetof(Vec3AddAssignCall,x)==0 && offsetof(Vec3AddAssignCall,y)==4 &&
    offsetof(Vec3AddAssignCall,z)==8)?1:-1];
static BYTE referenceSpace[0x800000]={1};
static DWORD initial[32],storage[32];
static BYTE* page;
struct Result { DWORD words[32]; BYTE page[4096]; DWORD result,exception,kind,address,fp,control,faultFp; };
static Result observed,expected;
static int checks;
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

static int Filter(EXCEPTION_POINTERS* info) {
    if(info->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=info->ExceptionRecord->ExceptionCode;
    observed.kind=info->ExceptionRecord->ExceptionInformation[0];
    observed.address=info->ExceptionRecord->ExceptionInformation[1];
    observed.faultFp=info->ContextRecord->FloatSave.StatusWord&63;
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int reference,int provider,int out,int fault,int rounding,int precision) {
    memset(&observed,0,sizeof(observed)); memcpy(storage,initial,sizeof(storage));
    for(int i=0;i<4096;++i)page[i]=(BYTE)(i*13+7);
    Vec3* input=(Vec3*)(storage+8); Vec3AddAssignCall* output=(Vec3AddAssignCall*)(storage+out);
    if(fault==1)input=0;
    if(fault==2)output=0;
    if(fault==3){input=0;output=0;}
    if(fault>=4 && fault<=6) {
        input=(Vec3*)(page+4096-4*(fault-4));
        memcpy((void*)input,storage+8,4*(fault-4));
    }
    if(fault>=7 && fault<=9) {
        output=(Vec3AddAssignCall*)(page+4096-4*(fault-7));
        memcpy(output,storage+out,4*(fault-7));
    }
    union { Vec3AddAssignCall&(Vec3AddAssignCall::*method)(const Vec3&); DWORD address; } call;
    typedef char MemberPointerSize[sizeof(call.method)==sizeof(DWORD)?1:-1];
    memset(&call,0,sizeof(call)); call.method=&Vec3AddAssignCall::operator+=;
    if(reference)call.address=0x428060;
    _fpreset(); _controlfp(rounding|precision,_MCW_RC|_MCW_PC);
    __try {
        if(reference || provider==0)observed.result=(DWORD)&((output->*call.method)(*input));
        else observed.result=(DWORD)&((((Vec3*)output)->*g_CanonicalAdd)(*input));
        if(observed.result!=(DWORD)output)ExitProcess(8);
    } __except(Filter(GetExceptionInformation())) {}
    observed.fp=_statusfp(); observed.control=_controlfp(0,0);
    memcpy(observed.words,storage,sizeof(storage)); memcpy(observed.page,page,4096); _fpreset();
}
static void Check(int provider,int out,int fault,int rounding,int precision) {
    Run(1,provider,out,fault,rounding,precision); expected=observed;
    Run(0,provider,out,fault,rounding,precision);
    if(memcmp(&observed,&expected,sizeof(observed))) {
        printf("FAIL case%d provider%d out%d fault%d RC%x PC%x\n",checks,provider,out,fault,rounding,precision);
        exit(1);
    }
    ++checks;
}
static void Seed(unsigned seed) {
    const DWORD edges[]={0,0x80000000,0x3f800000,0xbf800000,0x67800000,0xe7800000,
        0x00800000,1,0x7f7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345,
        0xffc54321,0x3eaaaaab,0xbeaaaaab};
    DWORD random=seed+1;
    for(int i=0;i<32;++i) {random=random*1664525+1013904223;initial[i]=seed<16?edges[(seed+i)%16]:random;}
}
int main(int argc,char** argv) {
    if(argc!=2 || !MapReference(argv[1]))return 3;
    page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;
    if(!page || !VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 4;
    Vec3AddAssignCall probe;
    if((void*)static_cast<Vec3*>(&probe)!=(void*)&probe)return 5;
    const int outputs[]={0,6,7,8,9,10,18,19,20,21,22};
    const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},precision[]={_PC_64,_PC_53,_PC_24};
    for(int provider=0;provider<2;++provider) {
        for(int r=0;r<4;++r)for(int p=0;p<3;++p)for(int seed=0;seed<128;++seed) {
            Seed(seed);
            for(int out=0;out<11;++out)Check(provider,outputs[out],0,rounds[r],precision[p]);
        }
        for(int fault=1;fault<=9;++fault)for(int r=0;r<4;++r)for(int p=0;p<3;++p) {
            Seed(17);Check(provider,8,fault,rounds[r],precision[p]);
        }
        Seed(17);((float*)initial)[0]=10;((float*)initial)[8]=1;((float*)initial)[9]=2;((float*)initial)[10]=3;((float*)initial)[11]=4;
        Check(provider,9,0,_RC_NEAR,_PC_64);
        if(observed.exception || ((float*)storage)[9]!=3 || ((float*)storage)[10]!=6 || ((float*)storage)[11]!=10)return 6;
        Check(provider,8,3,_RC_NEAR,_PC_64);
        if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.kind || observed.address)return 7;
        Check(provider,0,5,_RC_NEAR,_PC_64);
        float first=*(float*)initial+1;
        if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.kind || observed.address!=(DWORD)(page+4096) ||
           memcmp(storage,&first,4) || memcmp(storage+1,initial+1,7*4))return 9;
    }
    printf("Vector add-assign: %d alias, FP and partial-fault cases match\n",checks);
    return 0;
}
