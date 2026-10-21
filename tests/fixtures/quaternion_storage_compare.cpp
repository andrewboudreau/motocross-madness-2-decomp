#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "../../samples/physics/rigidbody/PhysicsBody.h"

typedef Quat* (__cdecl *NormalizeFn)(Quat*,const Quat*);
struct ResetCall { void Reset(); };
typedef char Layout[(sizeof(Quat)==16 && offsetof(Quat,w)==0 && offsetof(Quat,x)==4 &&
    offsetof(Quat,y)==8 && offsetof(Quat,z)==12 && sizeof(PhysicsRigidBody)==0x2a8 &&
    offsetof(PhysicsRigidBody,initialVelocity)==0x13c &&
    offsetof(PhysicsRigidBody,initialAngularVelocity)==0x148 &&
    offsetof(PhysicsRigidBody,dOrientation)==0x26c &&
    sizeof(void(ResetCall::*)())==4)?1:-1];

static BYTE referenceSpace[0x800000];
static int MapReference(const char*path){
    BYTE*mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE*f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE*raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER*dos=(IMAGE_DOS_HEADER*)raw;if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER*section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}
    free(raw);return 1;
}


static unsigned control;
static DWORD globals[4], patterns[][4]={
    {0,0,0,0}, {0x3f800000,0,0,0},
    {0x80000000,0x7fc12345,0xff800000,0x12345678},
    {0x11111111,0x22222222,0x33333333,0x44444444}};
static BYTE world[128],before[128],actual[128],expected[128];
static DWORD code,access,fault;
static int complete;
static LONG Filter(EXCEPTION_POINTERS*p){
    code=p->ExceptionRecord->ExceptionCode;
    access=code==EXCEPTION_ACCESS_VIOLATION?p->ExceptionRecord->ExceptionInformation[0]:0;
    fault=code==EXCEPTION_ACCESS_VIOLATION?p->ExceptionRecord->ExceptionInformation[1]:0;
    return EXCEPTION_EXECUTE_HANDLER;
}
static void SetGlobals(int pattern){
    memcpy(globals,patterns[pattern],16);
    memcpy(&g_QuatIdentity,globals,16);
    memcpy((void*)0x6850b0,globals,16);
}
static void ResetFp(){_fpreset();_control87(control,_MCW_PC|_MCW_RC|_MCW_EM);}
static void RunNormalize(NormalizeFn fn,Quat*out,const Quat*input){
    code=access=fault=0;complete=0;ResetFp();
    __try { if(fn(out,input)!=out)ExitProcess(10);complete=1; }
    __except(Filter(GetExceptionInformation())){}
}
static void RunReset(DWORD fn,void*object){
    union {DWORD address;void(ResetCall::*method)();}call;call.address=fn;
    code=access=fault=0;complete=0;
    __try {(((ResetCall*)object)->*call.method)();complete=1;}
    __except(Filter(GetExceptionInformation())){}
}
static int Put(BYTE*p,int available,int offset,DWORD value,int&failed){
    if(failed)return 0;
    if(offset+4>available){failed=offset;return 0;}
    memcpy(p+offset,&value,4);return 1;
}
static DWORD Get(const BYTE*p,int offset){DWORD v;memcpy(&v,p+offset,4);return v;}
static int ResetExpected(BYTE*p,int available){
    int failed=0,i;
    for(i=0;i<3;i++)Put(p,available,0x210+4*i,0,failed);
    for(i=0;i<3;i++)Put(p,available,0x21c+4*i,0,failed);
    for(i=0;i<3;i++)Put(p,available,0x154+4*i,Get(p,0x13c+4*i),failed);
    for(i=0;i<3;i++)Put(p,available,0x160+4*i,Get(p,0x13c+4*i),failed);
    for(i=0;i<3;i++)Put(p,available,0x16c+4*i,Get(p,0x148+4*i),failed);
    for(int j=0;j<2;j++){
        for(i=0;i<3;i++)Put(p,available,0x23c+12*j+4*i,0,failed);
        for(i=0;i<4;i++)Put(p,available,0x26c+16*j+4*i,globals[i],failed);
        for(i=0;i<3;i++)Put(p,available,0x254+12*j+4*i,0,failed);
        for(i=0;i<3;i++)Put(p,available,0x28c+12*j+4*i,0,failed);
    }
    return failed;
}
static BYTE body[0x2a8],bodyWant[0x2a8],bodyGot[0x2a8];
static BYTE*page;
static BYTE pageWant[4096],pageGot[4096],pageExpected[4096];
static void GuardCheck(int failed,BYTE*base){
    if(failed){if(complete||code!=EXCEPTION_ACCESS_VIOLATION||access!=1||fault!=(DWORD)(base+failed))ExitProcess(20);}
    else if(!complete||code)ExitProcess(21);
}
int main(int argc,char**argv){
    if(argc!=4||!MapReference(argv[1]))return 1;
    NormalizeFn source=(NormalizeFn)strtoul(argv[2],0,16),retail=(NormalizeFn)0x4a1d60;
    DWORD resetSource=strtoul(argv[3],0,16);
    const DWORD zero[4]={0,0,0,0};
    if(memcmp(&g_QuatIdentity,zero,16)||memcmp((void*)0x6850b0,zero,16))return 2;
    memset((void*)0x689a00,0,12); // Existing strict zero-vector initializer value.
    // Observe both consumers before the first deliberate test write to the
    // shared quaternion. The local oracle globals also begin loader-zero.
    control=_MCW_EM|_PC_64|_RC_NEAR;
    for(int initialSide=0;initialSide<2;initialSide++){
        memset(world,0,sizeof(world));memset(world,0xa5,16);
        RunNormalize(initialSide?source:retail,(Quat*)world,(Quat*)(world+32));
        if(!complete||code||memcmp(world,zero,16))return 11;
        memset(body,0xa5,sizeof(body));memcpy(bodyWant,body,sizeof(body));ResetExpected(bodyWant,sizeof(body));
        RunReset(initialSide?resetSource:0x4cc500,body);
        if(!complete||code||memcmp(body,bodyWant,sizeof(body)))return 12;
    }
    unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
    int normal=0,resetCases=0,guardCases=0,guardFaults=0;
    for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++){
        control=_MCW_EM|pcs[pc]|rcs[rc];
        for(int pattern=0;pattern<4;pattern++)for(int signs=0;signs<16;signs++)for(int destination=0;destination<=48;destination+=4){
            SetGlobals(pattern);memset(before,0xa5,sizeof(before));
            for(int k=0;k<4;k++){DWORD v=(signs&(1<<k))?0x80000000:0;memcpy(before+32+4*k,&v,4);}
            memcpy(expected,before,sizeof(expected));memcpy(expected+destination,globals,16);
            for(int side=0;side<2;side++){
                memcpy(world,before,sizeof(world));RunNormalize(side?source:retail,(Quat*)(world+destination),(Quat*)(world+32));
                if(!complete||code||memcmp(world,expected,sizeof(world))||memcmp(&g_QuatIdentity,globals,16))return 3;
            }
            normal++;
        }
        for(int component=0;component<4;component++)for(int sign=0;sign<2;sign++)for(int exponent=-60;exponent<=60;exponent+=15){
            SetGlobals(0);memset(before,0,sizeof(before));DWORD value=((exponent+127)<<23)|(sign?0x80000000:0);
            memcpy(before+32+component*4,&value,4);memcpy(expected,before,sizeof(before));value=0x3f800000|(sign?0x80000000:0);memcpy(expected+component*4,&value,4);
            for(int side=0;side<2;side++){memcpy(world,before,sizeof(world));RunNormalize(side?source:retail,(Quat*)world,(Quat*)(world+32));if(!complete||code||memcmp(world,expected,sizeof(world)))return 4;}
            normal++;
        }
    }
    for(int pattern=0;pattern<4;pattern++)for(int seed=0;seed<32;seed++){
        SetGlobals(pattern);for(int i=0;i<sizeof(body);i++)body[i]=(BYTE)(seed*37+i*13);
        memcpy(bodyWant,body,sizeof(body));ResetExpected(bodyWant,sizeof(body));
        RunReset(0x4cc500,body);if(!complete||code||memcmp(body,bodyWant,sizeof(body)))return 5;
        memcpy(bodyGot,body,sizeof(body));for(i=0;i<sizeof(body);i++)body[i]=(BYTE)(seed*37+i*13);
        RunReset(resetSource,body);if(!complete||code||memcmp(body,bodyWant,sizeof(body))||memcmp(body,bodyGot,sizeof(body)))return 6;
        if(memcmp(&g_QuatIdentity,globals,16))return 7;resetCases++;
    }
    page=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;
    if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 8;
    for(pattern=0;pattern<4;pattern++)for(int available=0x210;available<=0x2a8;available+=4){
        SetGlobals(pattern);memset(pageExpected,0xa5,4096);int failed=ResetExpected(pageExpected+4096-available,available);
        for(int side=0;side<2;side++){
            memset(page,0xa5,4096);RunReset(side?resetSource:0x4cc500,page+4096-available);GuardCheck(failed,page+4096-available);
            if(memcmp(page,pageExpected,4096))return 9;
        }
        guardCases++;guardFaults+=failed!=0;
    }
    SetGlobals(0);
    printf("Quaternion storage: 2 initial, %d normalize, %d reset, %d guarded reset (%d AV) cases\n",normal,resetCases,guardCases,guardFaults);
    return 0;
}
