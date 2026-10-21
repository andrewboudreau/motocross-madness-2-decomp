// Private differential validation of actual strict visibility methods.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "visibility/VisibilityQuadTree.h"
extern "C" int VisibilityCameraLayout();
static BYTE referenceSpace[0x800000]={1};
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

typedef char Layout[(sizeof(VisibilityBoxVec)==12 && sizeof(VisibilityMatrix)==64 &&
    sizeof(VisibilityCullPoint)==16 && offsetof(VisibilityCullPoint,code)==12 &&
    sizeof(VisibilityCamera)==0x220 && offsetof(VisibilityCamera,fieldOfView)==0x16c &&
    offsetof(VisibilityCamera,viewportWidth)==0x1a8 && offsetof(VisibilityCamera,viewportHeight)==0x1ac &&
    offsetof(VisibilityCamera,aspectRatio)==0x1b8 && offsetof(VisibilityCamera,nearPlane)==0x1bc)?1:-1];
struct World { DWORD guard0[8]; VisibilityCamera camera; float data[128]; DWORD guard1[8]; };
static DWORD initialStorage[(sizeof(World)+3)/4], worldStorage[(sizeof(World)+3)/4];
#define initial (*reinterpret_cast<World*>(initialStorage))
#define world (*reinterpret_cast<World*>(worldStorage))
static BYTE* page;
struct Result { BYTE state[sizeof(World)]; BYTE page[4096]; DWORD result,exception,kind,address,fp,control,faultFp; };
static Result actual, expected;
static int checks, faults;

static int Filter(EXCEPTION_POINTERS* p) {
    if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    actual.exception=p->ExceptionRecord->ExceptionCode;
    actual.kind=p->ExceptionRecord->ExceptionInformation[0];
    actual.address=p->ExceptionRecord->ExceptionInformation[1];
    actual.faultFp=p->ContextRecord->FloatSave.StatusWord&63;
    return EXCEPTION_EXECUTE_HANDLER;
}

static void Seed(unsigned seed) {
    memset(&initial,0x69,sizeof(initial));
    unsigned random=seed+1;
    for(int i=0;i<128;i++) {
        random=random*1664525+1013904223;
        initial.data[i]=((int)(random>>24)-128)*0.125f;
    }
    for(i=0;i<16;i++) initial.data[i]=(i%5==0)?1.0f:0.0f;
    // Affine, perspective, zero-w and mixed-sign projection inputs.
    if(seed&1)initial.data[3]=0.25f;
    if(seed&2)initial.data[7]=-0.5f;
    if(seed&4)initial.data[15]=0;
    if(seed&8)initial.data[12]=-1;
    if(seed&16)initial.data[14]=2;
    static const float fovs[]={10,77,90,91};
    static const DWORD widths[]={0,1,640,0x80000001};
    initial.camera.fieldOfView=fovs[seed%4];
    initial.camera.viewportWidth=widths[(seed/4)%4];
    initial.camera.viewportHeight=widths[(seed/16)%4];
    initial.camera.aspectRatio=(seed&1)?0.75f:1.25f;
    initial.camera.nearPlane=(seed&2)?-1.0f:1.0f;
    // Include signed zero, denormal, infinity and NaN on a bounded subset.
    if(seed>=64) {
        static const DWORD edge[]={0,0x80000000,1,0x7f800000,0xff800000,0x7fc12345,0x7f812345,0x7f7fffff};
        memcpy(initial.data+32,&edge[seed%8],4);
    }
}

static void Run(int retail,int method,int output,int count,int fault,int rounding,int precision,int receiver) {
    world=initial;memset(&actual,0,sizeof(actual));
    for(int i=0;i<4096;i++)page[i]=(BYTE)(i*17+5);
    VisibilityClipper* self=reinterpret_cast<VisibilityClipper*>(receiver?0:page+4096);
    const VisibilityCamera* camera=&world.camera;
    const VisibilityMatrix* matrix=reinterpret_cast<VisibilityMatrix*>(world.data);
    const VisibilityBoxVec* input=reinterpret_cast<VisibilityBoxVec*>(world.data+32);
    VisibilityBoxVec* destination=reinterpret_cast<VisibilityBoxVec*>(world.data+output);
    unsigned* code=reinterpret_cast<unsigned*>(world.data+100);
    if(output==120)destination=reinterpret_cast<VisibilityBoxVec*>(&world.camera.fieldOfView);
    if(output==121)code=reinterpret_cast<unsigned*>(destination);
    if(output==122)destination=0;
    if(output==123)code=0;
    if(fault==1)camera=0;
    if(fault==2)matrix=0;
    if(fault==3)input=0;
    if(fault>=4 && fault<=6) {
        input=reinterpret_cast<VisibilityBoxVec*>(page+4096-4*(fault-4));
        memcpy((void*)input,world.data+32,4*(fault-4));
    }
    if(fault>=7 && fault<=9)destination=reinterpret_cast<VisibilityBoxVec*>(page+4096-4*(fault-7));
    if(fault==10)code=reinterpret_cast<unsigned*>(page+4096);
    _fpreset();_controlfp(rounding|precision,_MCW_RC|_MCW_PC);
    __try {
        if(method==0) {
            union { void(VisibilityClipper::*fn)(const VisibilityBoxVec*,VisibilityBoxVec*,const VisibilityMatrix*,int); DWORD address; } f;
            f.fn=&VisibilityClipper::TransformVectors;if(sizeof(f.fn)!=4)ExitProcess(11);
            if(retail)f.address=0x52f0a0;
            (self->*f.fn)(input,destination,matrix,count);
        } else if(method==1) {
            union { int(VisibilityClipper::*fn)(const VisibilityCamera*,const VisibilityBoxVec*,const VisibilityBoxVec*); DWORD address; } f;
            f.fn=&VisibilityClipper::TestDot;if(sizeof(f.fn)!=4)ExitProcess(11);
            if(retail)f.address=0x52f140;
            actual.result=(self->*f.fn)(camera,input,destination);
        } else if(method==2) {
            union { int(VisibilityClipper::*fn)(const VisibilityCamera*,const VisibilityMatrix*,const VisibilityBoxVec*,VisibilityBoxVec*,unsigned*); DWORD address; } f;
            f.fn=&VisibilityClipper::ProjectPoint;if(sizeof(f.fn)!=4)ExitProcess(11);
            if(retail)f.address=0x52f340;
            actual.result=(self->*f.fn)(camera,matrix,input,destination,code);
        } else if(method==3) {
            union { int(VisibilityClipper::*fn)(const VisibilityCamera*,const VisibilityBoxVec*,int,int,int,int); DWORD address; } f;
            f.fn=&VisibilityClipper::CullQuad;if(sizeof(f.fn)!=4)ExitProcess(11);
            if(retail)f.address=0x52f4d0;
            actual.result=(self->*f.fn)(camera,input,0,count&3,(count+1)&3,(count+2)&3);
        } else {
            union { int(VisibilityClipper::*fn)(const VisibilityCamera*,VisibilityCullPoint*,int); DWORD address; } f;
            f.fn=&VisibilityClipper::CullPolygon;if(sizeof(f.fn)!=4)ExitProcess(11);
            if(retail)f.address=0x52fac0;
            actual.result=(self->*f.fn)(camera,reinterpret_cast<VisibilityCullPoint*>(const_cast<VisibilityBoxVec*>(input)),count);
        }
    } __except(Filter(GetExceptionInformation())) {}
    actual.fp=_statusfp();actual.control=_controlfp(0,0);
    memcpy(actual.state,&world,sizeof(world));memcpy(actual.page,page,4096);_fpreset();
}

static void Check(int method,int output,int count,int fault,int r,int p,int receiver) {
    Run(1,method,output,count,fault,r,p,receiver);expected=actual;
    Run(0,method,output,count,fault,r,p,receiver);
    if(memcmp(&actual,&expected,sizeof(actual)) || (!fault && actual.exception)) {
        printf("FAIL case%d method%d out%d count%d fault%d RC%x PC%x\n",checks,method,output,count,fault,r,p);exit(1);
    }
    if(actual.exception)faults++;
    checks++;
}

int main(int argc,char**argv) {
    if(argc!=2||!MapReference(argv[1])||!VisibilityCameraLayout())return 3;
    page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;
    if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 4;
    const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
    const int precisions[]={_PC_64,_PC_53,_PC_24};
    const int outputs[]={12,29,31,32,33,35,48,120};
    const int counts[]={-1,0,1,4,8};
    for(int r=0;r<4;r++)for(int p=0;p<3;p++)for(int seed=0;seed<80;seed++) {
        Seed(seed);
        for(int output=0;output<8;output++)for(int count=0;count<5;count++)
            Check(0,outputs[output],counts[count],0,rounds[r],precisions[p],seed&1);
        for(output=0;output<8;output++) {
            Check(1,outputs[output],1,0,rounds[r],precisions[p],seed&1);
            Check(2,outputs[output],1,0,rounds[r],precisions[p],seed&1);
        }
        for(output=121;output<=123;output++)Check(2,output,1,0,rounds[r],precisions[p],seed&1);
        for(int count=0;count<5;count++) {
            Check(3,48,counts[count],0,rounds[r],precisions[p],seed&1);
            Check(4,48,counts[count],0,rounds[r],precisions[p],seed&1);
        }
    }
    for(int method=0;method<5;method++)for(int fault=1;fault<=10;fault++)
        for(r=0;r<4;r++)for(int p=0;p<3;p++) {
            Seed(3);Check(method,48,4,fault,rounds[r],precisions[p],0);
        }
    // Independent identity/inside/outside and cull outcomes, beyond differential equality.
    Seed(0);for(int i=32;i<44;i++)initial.data[i]=0.25f;
    initial.camera.viewportWidth=640;initial.camera.viewportHeight=480;
    Check(0,48,1,0,_RC_NEAR,_PC_64,1);
    if(world.data[48]!=.25f||world.data[49]!=.25f||world.data[50]!=.25f)return 5;
    Check(2,48,1,0,_RC_NEAR,_PC_64,1);
    if(actual.result!=1||world.data[48]!=160||world.data[49]!=120||*(DWORD*)(world.data+100))return 6;
    initial.data[32]=-1;Check(2,48,1,0,_RC_NEAR,_PC_64,1);
    if(actual.result||*(DWORD*)(world.data+100)!=1)return 7;
    Seed(0);for(i=0;i<4;i++){initial.data[32+i*3]=0;initial.data[33+i*3]=0;initial.data[34+i*3]=2;}
    Check(3,48,3,0,_RC_NEAR,_PC_64,1);if(actual.result!=1)return 8;
    for(i=0;i<4;i++)initial.data[34+i*3]=0;
    Check(3,48,3,0,_RC_NEAR,_PC_64,1);if(actual.result)return 9;
    if(!faults)return 10;
    printf("%d visibility scenarios match (%d access faults)\n",checks,faults);return 0;
}
