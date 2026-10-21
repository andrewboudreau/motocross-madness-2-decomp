// Validation only: actual rendering scale and real cache helpers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "D3DIMSoulTree.h"
static BYTE referenceSpace[0x800000]={1};
struct World {BYTE nodes[7][0x2f8];DWORD scales[3];};
struct Result {World world;BYTE pages[8192];DWORD exception,kind,address,status,control,faultStatus,faultControl;};
static World world,initial;static Result observed,expected;static BYTE* faultPages;
static D3DIMSoultreeObject* receiver;static int faultRole,faultCut,checks,selectedFamily;
static unsigned fpMask=_MCW_EM;
static D3DIMSoultreeObject* Node(int index){return (D3DIMSoultreeObject*)(world.nodes[index]+16);}
static D3DIMSoultreeObject* FaultNode(){return (D3DIMSoultreeObject*)(faultPages+4096-faultCut);}
static DWORD Bits(unsigned seed,int family){
 static const DWORD finite[]={0,0x80000000,0x3f800000,0xbf800000,1,0x007fffff,0x00800000,0x7f7fffff,0xff7fffff,0x3f000000,0x3eaaaaab,0xc1234567};
 static const DWORD nonfinite[]={0x7f800000,0xff800000,0x7fc12345,0xffc54321,0x7f812345,0xff812345,0,0x3f800000};
 if(family==1)return finite[seed%12];if(family==2)return nonfinite[seed%8];
 unsigned v=seed*1664525u+1013904223u;
 if(family==3)return (v&0x807fffff)|(((v>>24)%254)<<23);
 float f=((int)(v%113)-55)*0.0625f;DWORD b;memcpy(&b,&f,4);return b;
}
static void Setup(int mask,int seed,int family,int alias,int role,int cut){
 memset(&world,0x39,sizeof(world));selectedFamily=family;
 for(int i=0;i<7;i++){
  D3DIMSoultreeObject* n=Node(i);n->parent=0;n->firstChild=0;n->nextSibling=0;
  n->worldValid=(mask>>i)&1;n->subtreeDirty=(seed&1)?-7:0;
  for(int j=0;j<16;j++){DWORD bits=Bits(seed+i*17+j*7,family);memcpy(((BYTE*)&n->localMatrix)+j*4,&bits,4);}
  memcpy(&n->worldMatrix,&n->localMatrix,64);
 }
 Node(1)->parent=Node(0);Node(2)->parent=Node(1);Node(2)->firstChild=Node(3);Node(2)->nextSibling=Node(6);
 Node(3)->parent=Node(2);Node(3)->nextSibling=Node(4);Node(3)->firstChild=Node(5);Node(4)->parent=Node(2);Node(5)->parent=Node(3);
 for(i=0;i<3;i++)world.scales[i]=Bits(seed+i*11,family);
 if(alias)memcpy(world.scales,((BYTE*)&Node(2)->localMatrix)+(alias-1)*4,12);
 receiver=Node(2);faultRole=role;faultCut=cut;
 if(role==1||role==5)receiver=FaultNode();if(role==6)receiver=0;
 if(role==2)Node(2)->parent=FaultNode();if(role==3)Node(2)->firstChild=FaultNode();if(role==4)Node(3)->nextSibling=FaultNode();
 initial=world;
}
static int Filter(EXCEPTION_POINTERS* p){DWORD code=p->ExceptionRecord->ExceptionCode;
 if(code!=EXCEPTION_ACCESS_VIOLATION&&!(code>=EXCEPTION_FLT_DENORMAL_OPERAND&&code<=EXCEPTION_FLT_UNDERFLOW))return EXCEPTION_CONTINUE_SEARCH;
 observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.kind=p->ExceptionRecord->ExceptionInformation[0];observed.address=p->ExceptionRecord->ExceptionInformation[1];}
 observed.faultStatus=p->ContextRecord->FloatSave.StatusWord&63;observed.faultControl=p->ContextRecord->FloatSave.ControlWord;
 if(code!=EXCEPTION_ACCESS_VIOLATION)_clearfp();return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail,unsigned rc,unsigned pc){
 DWORD old;if(!VirtualProtect(faultPages,8192,PAGE_READWRITE,&old))ExitProcess(10);memset(faultPages,0x5a,8192);world=initial;
 if(faultRole&&faultRole!=6){BYTE storage[sizeof(D3DIMSoultreeObject)];D3DIMSoultreeObject* n=(D3DIMSoultreeObject*)storage;memcpy(n,Node(2),sizeof(storage));n->parent=0;n->firstChild=0;n->nextSibling=0;memcpy(FaultNode(),n,sizeof(storage));}
 if(!VirtualProtect(faultPages+4096,4096,faultRole==5?PAGE_READONLY:PAGE_NOACCESS,&old))ExitProcess(11);
 memset(&observed,0,sizeof(observed));_fpreset();_control87(rc|pc|fpMask,_MCW_RC|_MCW_PC|_MCW_EM);
 float scales[3];memcpy(scales,world.scales,12);
 __try{if(retail){void(D3DIMSoultreeObject::*method)(float,float,float)=&D3DIMSoultreeObject::Scale;DWORD words[2];if(sizeof(method)!=8)ExitProcess(12);memcpy(words,&method,8);if(words[1])ExitProcess(13);words[0]=0x4fd340;memcpy(&method,words,8);(receiver->*method)(scales[0],scales[1],scales[2]);}
 else receiver->Scale(scales[0],scales[1],scales[2]);}__except(Filter(GetExceptionInformation())){}
 observed.status=_status87();observed.control=_control87(0,0);observed.world=world;memcpy(observed.pages,faultPages,faultRole==5?8192:4096);_fpreset();
}
static void Check(unsigned rc,unsigned pc){Run(1,rc,pc);expected=observed;Run(0,rc,pc);
 if(memcmp(&observed,&expected,sizeof(observed))){unsigned i=0;while(((BYTE*)&observed)[i]==((BYTE*)&expected)[i])i++;
 printf("FAIL case%d byte%u family%d role%d cut%x RC%x PC%x mask%x exc%x/%x fp%x/%x\n",checks,i,selectedFamily,faultRole,faultCut,rc,pc,fpMask,observed.exception,expected.exception,observed.status,expected.status);exit(1);}checks++;
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

int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 2;faultPages=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!faultPages)return 3;
 if(sizeof(D3DIMSoultreeObject)!=0x2d8||offsetof(D3DIMSoultreeObject,localMatrix)!=0xb8||offsetof(D3DIMSoultreeObject,subtreeDirty)!=0x18c)return 4;
 const unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},pc[]={_PC_64,_PC_53,_PC_24};
 for(int mask=0;mask<128;mask++)for(int seed=0;seed<16;seed++)for(int family=0;family<4;family++)for(int r=0;r<4;r++)for(int p=0;p<3;p++){
  Setup(mask,seed,family,0,0,0);Check(rc[r],pc[p]);if(observed.exception)return 5;
  if(memcmp(&Node(2)->localMatrix._41,&((D3DIMSoultreeObject*)(initial.nodes[2]+16))->localMatrix._41,12))return 6;
 }
 for(int alias=1;alias<=14;alias++)for(int family=0;family<4;family++)for(int r=0;r<4;r++)for(int p=0;p<3;p++){Setup(127,7,family,alias,0,0);Check(rc[r],pc[p]);}
 const int cuts[]={0,1,0xb8,0xb9,0xbc,0xc0,0xc4,0xc8,0xcc,0xd0,0xd4,0xd8,0xdc,0xe0,0xe4,0xe8,0xec,0xf0,0xf4,0xf8,0x138,0x13c,0x140,0x144,0x148,0x18c,0x190,0x2d8};
 for(int role=1;role<=5;role++)for(int c=0;c<sizeof(cuts)/sizeof(cuts[0]);c++)for(int mask=0;mask<4;mask++){Setup(mask*42,5,0,0,role,cuts[c]);Check(_RC_NEAR,_PC_64);}
 const unsigned exceptions[]={_EM_INVALID,_EM_ZERODIVIDE,_EM_OVERFLOW,_EM_UNDERFLOW,_EM_INEXACT,_EM_DENORMAL};
 for(int ex=0;ex<6;ex++)for(int family=0;family<4;family++)for(int r=0;r<4;r++)for(int p=0;p<3;p++){Setup(127,7,family,0,0,0);fpMask=_MCW_EM&~exceptions[ex];Check(rc[r],pc[p]);}fpMask=_MCW_EM;
 // Independent exact simple-matrix and dirty-cache oracle.
 Setup(127,0,0,0,0,0);Matrix4 identity;memset(&identity,0,64);identity._11=identity._22=identity._33=identity._44=1;identity._41=2;identity._42=3;identity._43=4;
 memcpy(&((D3DIMSoultreeObject*)(initial.nodes[2]+16))->localMatrix,&identity,64);initial.scales[0]=0x40000000;initial.scales[1]=0x40400000;initial.scales[2]=0x40800000;Check(_RC_NEAR,_PC_64);
 identity._11=2;identity._22=3;identity._33=4;if(observed.exception||memcmp(&Node(2)->localMatrix,&identity,64))return 7;
 for(int n=0;n<7;n++){if(Node(n)->subtreeDirty!=(n<3?1:0)||Node(n)->worldValid!=((n<2||n==6)?1:0))return 8;}
 // Read-only output page allows the full input snapshot, then faults after two stores.
 Setup(127,0,0,0,5,0xc0);D3DIMSoultreeObject* initialNode=(D3DIMSoultreeObject*)(initial.nodes[2]+16);memcpy(&initialNode->localMatrix,&identity,64);initial.scales[0]=initial.scales[1]=initial.scales[2]=0x3f800000;Check(_RC_NEAR,_PC_64);
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)(faultPages+4096)||*(DWORD*)(faultPages+4096-8)!=0x40000000||*(DWORD*)(faultPages+4096-4)!=0)return 9;
 Setup(127,0,0,0,6,0);Check(_RC_NEAR,_PC_64);if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address!=0xb8)return 14;
 printf("Soultree scale: %d complete-state, cache, FP and fault cases match\n",checks);return 0;}
