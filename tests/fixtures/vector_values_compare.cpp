// Validation only: separately compiled source hidden-output ABI and full retail helper.
// Direct ABI calls exercise overlapping results that ordinary C++ return expressions
// cannot request. The decoded retail stack layout and complete 69-byte source match
// establish this test-only call boundary. Floating-point exceptions remain masked.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "math/Vec3.h"
#include "../../src/reconstructed/MatrixUtil.h"
Vec3 SegmentAdd(const Vec3&,const Vec3&);
Vec3 SegmentSubtract(const Vec3&,const Vec3&);
Vector3 WreckerSubtractValue(const Vector3&,const Vector3&);
typedef char OtherVectorLayout[sizeof(Vector3)==12&&offsetof(Vector3,x)==0&&offsetof(Vector3,y)==4&&offsetof(Vector3,z)==8?1:-1];
typedef char VectorLayout[sizeof(Vec3)==12&&offsetof(Vec3,x)==0&&offsetof(Vec3,y)==4&&offsetof(Vec3,z)==8?1:-1];
static BYTE referenceSpace[0x800000]={1};
static DWORD initial[32],storage[32];static BYTE* page;
struct Result {DWORD words[32];BYTE page[4096];DWORD result,exception,kind,address,fp,control,faultFp;};
static Result observed,expected;static int checks;
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

static int Filter(EXCEPTION_POINTERS* p){if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=p->ExceptionRecord->ExceptionCode;observed.kind=p->ExceptionRecord->ExceptionInformation[0];observed.address=p->ExceptionRecord->ExceptionInformation[1];observed.faultFp=p->ContextRecord->FloatSave.StatusWord&63;return EXCEPTION_EXECUTE_HANDLER;}
typedef void*(__cdecl* Binary)(void*,const void*,const void*);
static void Run(int retail,int operation,int out,int right,int fault,int rounding,int precision){
 memset(&observed,0,sizeof(observed));memcpy(storage,initial,sizeof(storage));
 for(int i=0;i<4096;i++)page[i]=(BYTE)(i*13+7);
 void* result=storage+out;const void* a=storage+8;const void* b=storage+right;
 if(fault==1)a=0;if(fault==2)b=0;if(fault==3)result=0;if(fault==4){a=0;b=0;result=0;}
 if(fault>=5&&fault<=7){a=page+4096-4*(fault-5);memcpy((void*)a,storage+8,4*(fault-5));}
 if(fault>=8&&fault<=10){b=page+4096-4*(fault-8);memcpy((void*)b,storage+right,4*(fault-8));}
 if(fault>=11&&fault<=13)result=page+4096-4*(fault-11);
 _fpreset();_controlfp(rounding|precision,_MCW_RC|_MCW_PC);
 Binary functions[]={(Binary)&SegmentAdd,(Binary)&SegmentSubtract,(Binary)&WreckerSubtractValue};
 Binary function=retail?(Binary)(operation==0?0x421cb0:0x421d00):functions[operation];
 __try{observed.result=(DWORD)function(result,a,b);if(observed.result!=(DWORD)result)ExitProcess(8);}__except(Filter(GetExceptionInformation())){}
 observed.fp=_statusfp();observed.control=_controlfp(0,0);memcpy(observed.words,storage,sizeof(storage));memcpy(observed.page,page,4096);_fpreset();
}
static void Check(int op,int out,int right,int fault,int r,int p){Run(1,op,out,right,fault,r,p);expected=observed;Run(0,op,out,right,fault,r,p);if(memcmp(&observed,&expected,sizeof(observed))){printf("FAIL case%d op%d out%d right%d fault%d RC%x PC%x\n",checks,op,out,right,fault,r,p);exit(1);}checks++;}
static void Seed(unsigned seed){const DWORD edges[]={0,0x80000000,0x3f800000,0xbf800000,0x67800000,0xe7800000,0x00800000,1,0x7f7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345,0xffc54321,0x3eaaaaab,0xbeaaaaab};DWORD random=seed+1;for(int i=0;i<32;i++){random=random*1664525+1013904223;initial[i]=seed<16?edges[(seed+i)%16]:random;}}
int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 3;page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 4;
 Vec3 a(1.5f,-2.0f,.25f),b(-.5f,3.0f,.75f);Vec3 add=SegmentAdd(a,b),sub=SegmentSubtract(a,b);
 Vector3 va(1.5f,-2.0f,.25f),vb(-.5f,3.0f,.75f),vs=WreckerSubtractValue(va,vb);
 if(add.x!=1||add.y!=1||add.z!=1||sub.x!=2||sub.y!=-5||sub.z!=-.5f||vs.x!=2||vs.y!=-5||vs.z!=-.5f||a.x!=1.5f||a.y!=-2||a.z!=.25f||b.x!=-.5f||b.y!=3||b.z!=.75f)return 5;checks++;
 const int outputs[]={0,6,7,8,9,10,18,19,20,21,22};const int rights[]={6,7,8,9,10,20};const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},precisions[]={_PC_64,_PC_53,_PC_24};
 for(int op=0;op<3;op++)for(int r=0;r<4;r++)for(int p=0;p<3;p++)for(int seed=0;seed<64;seed++){Seed(seed);for(int out=0;out<11;out++)for(int b=0;b<6;b++)Check(op,outputs[out],rights[b],0,rounds[r],precisions[p]);}
 for(op=0;op<3;op++)for(int fault=1;fault<=13;fault++)for(int r=0;r<4;r++)for(int p=0;p<3;p++){Seed(17);Check(op,8,20,fault,rounds[r],precisions[p]);}
 Seed(17);Check(0,8,20,4,_RC_NEAR,_PC_64);if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address)return 6;
 Seed(17);initial[8]=0x3fc00000;initial[20]=0x3f000000;Check(1,8,20,12,_RC_NEAR,_PC_64);if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)(page+4096)||*(DWORD*)(page+4092)!=0x3f800000)return 7;
 printf("Vector values: %d hidden-output, overlap, raw-float, FP and fault cases match\n",checks);return 0;}
