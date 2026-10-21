// Validation-only full attachment operation with actual scene/cache/rotation helpers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "vehicle/Vehicle.h"
#include "TireAttachmentViews.h"
static BYTE referenceSpace[0x800000]={1};
struct World { BYTE wheel[0x2e0]; BYTE slider[0x120]; BYTE hinge[0x120]; BYTE nodes[3][0x400]; };
static World world,initial;
struct Observation { World world; BYTE guard[4096]; DWORD exception,kind,address,faultStatus,control,status; };
static Observation observed,expected;
struct Scenario { int seed,mode,caches,alias,invalid,guardRole,cut,family;unsigned rc,pc; };
static Scenario scenario;
static BYTE* pages;static int checks;
static SoultreeObject* Node(int i){return (SoultreeObject*)(world.nodes[i]+16);}
static VehicleWheel* Wheel(){return scenario.invalid==1?0:(VehicleWheel*)(world.wheel+16);}
static TireAttachA* Slider(){return scenario.alias==1?(TireAttachA*)((BYTE*)Node(0)-8):(TireAttachA*)(world.slider+16);}
static TireAttachB* Hinge(){return scenario.alias==2?(TireAttachB*)((BYTE*)Node(0)+0xb4):(TireAttachB*)(world.hinge+16);}
static float Bits(DWORD bits){float value;memcpy(&value,&bits,4);return value;}
static void Identity(Matrix4& m){memset(&m,0,sizeof(m));m._11=m._22=m._33=m._44=1;}
static float Value(int i){static const DWORD edges[]={0,0x80000000,0x3f800000,0xbf800000,0x3eaaaaab,0x00800000,1,0x7f7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345,0x41200000,0xc0400000,0x3f000000,0x40000000};
 if(scenario.family==1)return Bits(edges[(scenario.seed+i)%16]);
 if(scenario.family==2){DWORD r=(scenario.seed*1664525u+1013904223u)*(i*97+1);return Bits((r&0x807fffff)|(((r>>24)%254)<<23));}
 return ((scenario.seed+i*13)%31-15)*0.125f;}
static void Prepare(){
 memset(&world,0x6b,sizeof(world));DWORD old;VirtualProtect(pages,8192,PAGE_READWRITE,&old);memset(pages,0x59,8192);
 for(int i=0;i<3;i++){SoultreeObject*n=Node(i);Identity(n->localMatrix);Identity(n->worldMatrix);n->parent=0;n->firstChild=0;n->nextSibling=0;n->prevSibling=0;n->worldValid=(scenario.caches>>i)&1;n->subtreeDirty=0;n->localMatrix._41=i+7;n->localMatrix._42=i+8;n->localMatrix._43=i+9;}
 Node(0)->parent=Node(1);Node(0)->firstChild=Node(2);Node(2)->parent=Node(0);
 TireAttachA*a=Slider();TireAttachB*b=Hinge();a->node=(TireNode*)Node(0);a->position=Value(0);a->direction.x=Value(1);a->direction.y=Value(2);a->direction.z=Value(3);
 b->node=(TireNode*)Node(0);b->position=Value(4);b->scale=Value(5);b->axis.x=1;b->axis.y=scenario.seed&1?0:0.5f;b->axis.z=scenario.seed&2?0:0.25f;
 Matrix4 rest;Identity(rest);rest._41=7;rest._42=8;rest._43=9;
 if(scenario.family==3){rest._11=.5f;rest._12=.25f;rest._23=-.5f;rest._31=.125f;}
 memcpy(&b->restMatrix,&rest,64);
 VehicleWheel*w=(VehicleWheel*)(world.wheel+16);w->primaryAux=(scenario.mode==1||scenario.mode==3)?(int)a:0;w->secondaryAux=(scenario.mode==2||scenario.mode==3)?(int)b:0;
 if(scenario.invalid==2)w->primaryAux=1;if(scenario.invalid==3){w->primaryAux=0;w->secondaryAux=1;}
 if(scenario.invalid==4)a->node=0;if(scenario.invalid==5)b->node=0;
 if(scenario.guardRole){BYTE* dest=pages+4096-scenario.cut;
  if(scenario.guardRole==1){memcpy(dest,a,sizeof(*a));w->primaryAux=(int)dest;}
  if(scenario.guardRole==2){memcpy(dest,b,sizeof(*b));w->primaryAux=0;w->secondaryAux=(int)dest;}
  if(scenario.guardRole==3){memcpy(dest,Node(0),sizeof(SoultreeObject));a->node=(TireNode*)dest;b->node=(TireNode*)dest;((SoultreeObject*)dest)->parent=0;((SoultreeObject*)dest)->firstChild=0;}
 }
 VirtualProtect(pages+4096,4096,PAGE_NOACCESS,&old);initial=world;
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.kind=e->ExceptionRecord->ExceptionInformation[0];observed.address=e->ExceptionRecord->ExceptionInformation[1];observed.faultStatus=e->ContextRecord->FloatSave.StatusWord&63;return EXCEPTION_EXECUTE_HANDLER;}
static void Run(int retail){Prepare();memset(&observed,0,sizeof(observed));_fpreset();_control87(scenario.rc|scenario.pc,_MCW_RC|_MCW_PC);
 union {void(VehicleWheel::*method)();DWORD address;} call;call.method=&VehicleWheel::UpdateAttachment;if(retail)call.address=0x515660;
 __try{(Wheel()->*call.method)();}__except(Filter(GetExceptionInformation())){}
 observed.world=world;memcpy(observed.guard,pages,4096);observed.control=_control87(0,0);observed.status=_status87();_fpreset();
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){unsigned n=0;while(((BYTE*)&expected)[n]==((BYTE*)&observed)[n])n++;printf("Mismatch case%d offset%u seed%d mode%d family%d alias%d invalid%d guard%d/%d RC%x PC%x exception%x/%x\n",checks,n,scenario.seed,scenario.mode,scenario.family,scenario.alias,scenario.invalid,scenario.guardRole,scenario.cut,scenario.rc,scenario.pc,expected.exception,observed.exception);ExitProcess(20);}}
static void Reset(){memset(&scenario,0,sizeof(scenario));scenario.pc=_PC_64;scenario.caches=7;}
static int MapReference(const char* path){DWORD old;BYTE*mapped=(BYTE*)0x400000;
 if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE*f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;
 BYTE*raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
 IMAGE_DOS_HEADER*dos=(IMAGE_DOS_HEADER*)raw;if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
 IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 IMAGE_SECTION_HEADER*sections=IMAGE_FIRST_SECTION(pe);DWORD offset=(BYTE*)sections-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(*sections))return 0;
 memset(mapped,0,pe->OptionalHeader.SizeOfImage);memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
 for(int n=0;n<pe->FileHeader.NumberOfSections;++n){IMAGE_SECTION_HEADER&s=sections[n];if(s.PointerToRawData>(DWORD)size||s.SizeOfRawData>(DWORD)size-s.PointerToRawData||s.VirtualAddress>pe->OptionalHeader.SizeOfImage||s.SizeOfRawData>pe->OptionalHeader.SizeOfImage-s.VirtualAddress)return 0;memcpy(mapped+s.VirtualAddress,raw+s.PointerToRawData,s.SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 1;pages=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!pages)return 2;
 if(sizeof(VehicleWheel)!=0x2c0||offsetof(VehicleWheel,primaryAux)!=0x2b0||offsetof(VehicleWheel,secondaryAux)!=0x2ac||offsetof(TireAttachA,position)!=0x98||offsetof(TireAttachB,axis)!=0xd4)return 3;
 unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},pc[]={_PC_24,_PC_53,_PC_64};int seed,mode,family,cache,r,p;
 for(seed=0;seed<24;seed++)for(mode=0;mode<4;mode++)for(family=0;family<4;family++)for(cache=0;cache<8;cache++)for(r=0;r<4;r++)for(p=0;p<3;p++){
 Reset();scenario.seed=seed;scenario.mode=mode;scenario.family=family;scenario.caches=cache;scenario.rc=rc[r];scenario.pc=pc[p];Compare();}
 for(int alias=1;alias<=2;alias++)for(seed=0;seed<16;seed++)for(mode=1;mode<4;mode++)for(cache=0;cache<8;cache++)for(r=0;r<4;r++)for(p=0;p<3;p++){
 Reset();scenario.alias=alias;scenario.seed=seed;scenario.mode=mode;scenario.caches=cache;scenario.rc=rc[r];scenario.pc=pc[p];Compare();}
 for(int invalid=1;invalid<=5;invalid++)for(mode=0;mode<4;mode++)for(family=0;family<4;family++)for(r=0;r<4;r++){
 Reset();scenario.invalid=invalid;scenario.mode=mode;scenario.family=family;scenario.seed=9;scenario.rc=rc[r];Compare();}
 const int cuts[]={0,4,0x44,0x48,0x98,0x9c,0xc0,0xc4,0xc8,0xcc,0xd4,0xd8,0xdc,0xe0,0xe8,0xec,0xf0,0xf4,0x138,0x13c,0x140,0x144,0x18c,0x190,0x1a4};
 for(int role=1;role<=3;role++)for(int cut=0;cut<25;cut++)for(mode=1;mode<=2;mode++)for(r=0;r<4;r++){
 Reset();scenario.guardRole=role;scenario.cut=cuts[cut];scenario.mode=mode;scenario.seed=15;scenario.rc=rc[r];Compare();}
 // Independent finite slider oracle and cache effects. Seed15 gives position0,
 // seed23 gives position1 and direction(-1.25,.375,-1.875).
 Reset();scenario.seed=23;scenario.mode=3;Compare();
 if(observed.exception||Node(0)->localMatrix._41!=1.25f||Node(0)->localMatrix._42!=-.375f||Node(0)->localMatrix._43!=1.875f||Node(0)->worldValid||Node(2)->worldValid||Node(1)->subtreeDirty!=1) return 30;
 Reset();scenario.mode=0;Compare();if(observed.exception||memcmp(&world,&initial,sizeof(world)))return 31;
 // A nonfinite quotient must skip both matrix restoration and rotation, including an unused null node.
 Reset();scenario.mode=2;scenario.family=1;scenario.seed=4;scenario.invalid=5;Compare();
 if(observed.exception||memcmp(&world,&initial,sizeof(world)))return 32;
 Reset();scenario.mode=2;scenario.seed=25;Compare();
 if(observed.exception||memcmp(&Node(0)->localMatrix,&Hinge()->restMatrix,64)||Node(0)->worldValid||Node(2)->worldValid||Node(1)->subtreeDirty!=1)return 33;
 // Matrix/cache updates precede an inaccessible rotation-axis argument read.
 Reset();scenario.mode=2;scenario.seed=15;scenario.guardRole=2;scenario.cut=0xd8;Compare();
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address!=(DWORD)(pages+4096)||Node(0)->localMatrix._41!=7||Node(0)->localMatrix._42!=8||Node(0)->localMatrix._43!=9||Node(0)->worldValid||Node(1)->subtreeDirty!=1)return 34;
 // First translation store survives a fault on the second component.
 Reset();scenario.mode=1;scenario.seed=23;scenario.guardRole=3;scenario.cut=0xec;Compare();
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)(pages+4096)||*(float*)(pages+4092)!=1.25f)return 35;
 printf("Wheel attachment: %d complete-state, actual-helper and partial-fault scenarios match\n",checks);return 0;
}
