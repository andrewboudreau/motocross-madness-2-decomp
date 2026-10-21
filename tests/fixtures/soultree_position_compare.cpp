// Complete source setter and real hierarchy-cache helpers; no scripted calls.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "math/Math3D.h"
#include "core/SoultreeObject.h"
// Compiled separately with the actual rendering declaration, avoiding a second
// declaration of SoultreeObject in this validation translation unit.
extern "C" void RenderingPositionBoundary(void*,const void*);

static BYTE referenceSpace[0x800000] = {1};
typedef char NodeSize[(sizeof(SoultreeObject)==0x1a4)?1:-1];
typedef char NodeFields[(offsetof(SoultreeObject,localMatrix)==0xb8 &&
    offsetof(SoultreeObject,worldValid)==0x138 && offsetof(SoultreeObject,parent)==0x13c &&
    offsetof(SoultreeObject,firstChild)==0x140 && offsetof(SoultreeObject,nextSibling)==0x144 &&
    offsetof(SoultreeObject,subtreeDirty)==0x18c && sizeof(Vec3)==12)?1:-1];
struct World { BYTE nodes[7][0x1c4]; DWORD input[3]; };
struct Result { World world; BYTE faultPage[4096]; DWORD exception, kind, address, status, control; };
static World world, initial;
static Result observed, expected;
static BYTE* faultPages;
static SoultreeObject* receiver;
static Vec3* input;
static int faultRole, faultCut, checks;
static SoultreeObject* Node(int index) { return (SoultreeObject*)(world.nodes[index]+16); }
static SoultreeObject* FaultNode() { return (SoultreeObject*)(faultPages+4096-faultCut); }
static DWORD Bits(int seed) {
    static const DWORD values[]={0,0x80000000,0x3f800000,0xbf800000,0x00000001,0x007fffff,
        0x00800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc12345,
        0x7f812345,0xff812345,0x3eaaaaab,0xc1234567};
    return values[seed&15];
}
static void Setup(int mask,int seed,int alias,int role,int cut) {
    memset(&world,(seed*37+21)&255,sizeof(world));
    for(int i=0;i<7;i++) {
        SoultreeObject* n=Node(i);
        n->parent=0;n->firstChild=0;n->nextSibling=0;n->prevSibling=0;
        n->worldValid=(mask>>i)&1;n->subtreeDirty=(seed&1)?-7:0;
        DWORD bits[3]={Bits(seed+i),Bits(seed+i+5),Bits(seed+i+9)};
        memcpy(&n->localMatrix._41,bits,12);memcpy(&n->worldMatrix._41,bits,12);
    }
    Node(1)->parent=Node(0);Node(2)->parent=Node(1);
    Node(2)->firstChild=Node(3);Node(2)->nextSibling=Node(6);
    Node(3)->parent=Node(2);Node(3)->nextSibling=Node(4);Node(3)->firstChild=Node(5);
    Node(4)->parent=Node(2);Node(5)->parent=Node(3);
    world.input[0]=Bits(seed);world.input[1]=Bits(seed+5);world.input[2]=Bits(seed+9);
    input=alias==0?(Vec3*)world.input:alias==1?(Vec3*)&Node(2)->localMatrix._41:
        alias==2?(Vec3*)&Node(1)->localMatrix._41:alias==3?(Vec3*)&Node(2)->worldMatrix._41:
        alias==4?(Vec3*)((BYTE*)&Node(2)->localMatrix._41-4):(Vec3*)&Node(2)->localMatrix._42;
    receiver=Node(2);faultRole=role;faultCut=cut;
    if(role==1)receiver=FaultNode();
    if(role==2)Node(2)->parent=FaultNode();
    if(role==3)Node(2)->firstChild=FaultNode();
    if(role==4)Node(3)->nextSibling=FaultNode();
    memcpy(&initial,&world,sizeof(world));
}
static int Filter(EXCEPTION_POINTERS* info) {
    if(info->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=info->ExceptionRecord->ExceptionCode;
    observed.kind=info->ExceptionRecord->ExceptionInformation[0];
    observed.address=info->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int reference,unsigned rounding,unsigned precision) {
    DWORD previous;
    if(!VirtualProtect(faultPages,8192,PAGE_READWRITE,&previous))ExitProcess(10);
    memset(faultPages,0x5a,8192);memcpy(&world,&initial,sizeof(world));
    if(faultRole) {
        // Prepare a real node before protecting the page containing its tail.
        BYTE storage[sizeof(SoultreeObject)];
        SoultreeObject* node=(SoultreeObject*)storage;
        memcpy(node,Node(2),sizeof(storage));
        node->parent=0;node->firstChild=0;node->nextSibling=0;
        memcpy(FaultNode(),node,sizeof(storage));
    }
    if(!VirtualProtect(faultPages+4096,4096,PAGE_NOACCESS,&previous))ExitProcess(11);
    memset(&observed,0,sizeof(observed));_fpreset();
    _controlfp(rounding|precision,_MCW_RC|_MCW_PC);
    __try {
        if(reference==1) {
            void(SoultreeObject::*method)(Vec3);DWORD address=0x4fc630;
            if(sizeof(method)!=4)ExitProcess(12);memcpy(&method,&address,4);
            (receiver->*method)(*input);
        } else if(reference==2)RenderingPositionBoundary(receiver,input);
        else receiver->SetPositionVec3(*input);
    } __except(Filter(GetExceptionInformation())) {}
    observed.status=_statusfp();observed.control=_controlfp(0,0);
    memcpy(&observed.world,&world,sizeof(world));memcpy(observed.faultPage,faultPages,4096);
    _fpreset();
}
static int Check(unsigned rounding,unsigned precision) {
    Run(1,rounding,precision);memcpy(&expected,&observed,sizeof(expected));
    for(int variant=0;variant<=2;variant+=2) {
        Run(variant,rounding,precision);
        if(memcmp(&observed,&expected,sizeof(expected))) {
            unsigned i=0;while(i<sizeof(expected)&&((BYTE*)&observed)[i]==((BYTE*)&expected)[i])i++;
            printf("case=%d variant=%d difference=%u exception=%08x/%08x address=%08x/%08x\n",checks,variant,i,
                observed.exception,expected.exception,observed.address,expected.address);return 0;
        }
    }
    ++checks;return 1;
}

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

int main(int argc,char** argv) {
    if(argc!=2||!MapReference(argv[1]))return 1;
    faultPages=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(!faultPages)return 2;
    const unsigned rounding[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
    const unsigned precision[]={_PC_24,_PC_53,_PC_64};
    for(int mask=0;mask<128;mask++)for(int seed=0;seed<16;seed++)for(int alias=0;alias<6;alias++)
        for(int r=0;r<4;r++)for(int p=0;p<3;p++) {
            Setup(mask,seed,alias,0,0);DWORD argumentWords[3];memcpy(argumentWords,input,12);
            if(!Check(rounding[r],precision[p]))return 3;
            if(observed.exception || observed.status || memcmp(&Node(2)->localMatrix._41,argumentWords,12))return 4;
        }
    const int cuts[]={0,0xe8,0xe9,0xeb,0xec,0xed,0xef,0xf0,0xf1,0xf3,0xf4,
        0x138,0x13c,0x140,0x144,0x148,0x18c,0x18d,0x18f,0x190,0x1a4};
    for(int role=1;role<=4;role++)for(int c=0;c<sizeof(cuts)/sizeof(cuts[0]);c++)
        for(int seed=0;seed<16;seed++)for(int mask=0;mask<8;mask++) {
            Setup(mask*17,seed,0,role,cuts[c]);if(!Check(_RC_NEAR,_PC_64))return 5;
        }
    // Independent outcomes: cache propagation excludes this node's siblings.
    Setup(127,12,0,0,0);DWORD bits[3];memcpy(bits,world.input,12);
    if(!Check(_RC_NEAR,_PC_64)||memcmp(&Node(2)->localMatrix._41,bits,12))return 6;
    for(int n=0;n<7;n++) {
        if(Node(n)->subtreeDirty!=(n<3?1:0))return 7;
        if(Node(n)->worldValid!=((n<2||n==6)?1:0))return 8;
    }
    // An already-invalid target still dirties ancestors, but skips descendants.
    Setup(123,0,0,0,0);if(!Check(_RC_NEAR,_PC_64))return 9;
    if(Node(2)->worldValid||Node(0)->subtreeDirty!=1||Node(1)->subtreeDirty!=1||
       Node(3)->worldValid!=1||Node(4)->worldValid!=1||Node(5)->worldValid!=1)return 13;
    // A stale first child blocks its children, but not that child's next sibling.
    Setup(119,0,0,0,0);if(!Check(_RC_NEAR,_PC_64))return 14;
    if(Node(3)->worldValid||Node(4)->worldValid||Node(5)->worldValid!=1)return 15;
    // Translation writes occur before dirtying. A protected third component
    // leaves x/y written and faults at the first inaccessible write address.
    Setup(127,15,0,1,0xf0);if(!Check(_RC_NEAR,_PC_64))return 16;
    if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||
       observed.address!=(DWORD)(faultPages+4096)||memcmp(faultPages+4096-8,initial.input,8))return 17;
    printf("Soultree position: %d raw-bit, cache and partial-fault cases passed\n",checks);
    return 0;
}
