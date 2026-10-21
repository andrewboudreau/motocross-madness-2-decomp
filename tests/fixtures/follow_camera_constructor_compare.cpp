#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/MemTag.h"
extern "C" __declspec(noreturn) void FixtureUnvisited() {
    puts("Unexpected unvisited method");
    ExitProcess(97);
}
// These real declarations are never reached by the constructor graph. Keep
// their vtable slots explicit and fail immediately if the tested domain grows.
bool FollowCamera::UnknownVirtualSlot45(bool,float) { FixtureUnvisited(); }
int FollowCamera::Tick(float) { FixtureUnvisited(); }
void Camera::UpdateMatrices() { FixtureUnvisited(); }

#include <malloc.h>
struct HeapBlock {void*pointer;unsigned requested,actual;int alive;};
struct AllocationEvent {int kind,index;unsigned size;BYTE object[sizeof(FollowCamera)];};
static HeapBlock blocks[4];static AllocationEvent allocationEvents[12];
static int allocatorActive,allocationCount,eventCount,failAt=-1,throwAt=-1;
static DWORD nativeFollowVptr,nativeBaseVptr;
static BYTE*observedObject;static int observedBytes;
static int PointerId(void*p){for(int i=0;i<allocationCount;i++)if(blocks[i].pointer==p&&p)return i+1;return p?0:-1;}
static void AllocationEventRecord(int kind,int index,unsigned size){
 if(eventCount>=12)ExitProcess(98);AllocationEvent&e=allocationEvents[eventCount++];memset(&e,0,sizeof(e));e.kind=kind;e.index=index;e.size=size;
 memcpy(e.object,observedObject,observedBytes);
 if(observedBytes>=4&&*(DWORD*)e.object==nativeFollowVptr)*(DWORD*)e.object=0x55282c;
 if(observedBytes>=0x2c){void*p=*(void**)(e.object+0x28);int id=PointerId(p);if(id)*(int*)(e.object+0x28)=id;}
}
extern "C" void* __cdecl FixtureMalloc(unsigned n){
 if(!allocatorActive)return malloc(n);
 int index=allocationCount++;if(index>=4)ExitProcess(99);HeapBlock&b=blocks[index];memset(&b,0,sizeof(b));b.requested=n;
 if(index==throwAt){AllocationEventRecord(3,index,n);throw 77;}
 if(index!=failAt){b.pointer=malloc(n);if(!b.pointer)ExitProcess(100);b.actual=_msize(b.pointer);b.alive=1;}
 AllocationEventRecord(1,index,n);return b.pointer;
}
extern "C" void __cdecl FixtureFree(void*p){
 if(allocatorActive){int id=PointerId(p);if(p&&!id)ExitProcess(101);AllocationEventRecord(2,id-1,0);if(id>0){if(!blocks[id-1].alive)ExitProcess(102);blocks[id-1].alive=0;}}
 free(p);
}
extern "C" unsigned __cdecl FixtureMsize(void*p){return _msize(p);}
static void BeginAllocations(BYTE*p,int n){memset(blocks,0,sizeof(blocks));memset(allocationEvents,0,sizeof(allocationEvents));allocationCount=eventCount=0;observedObject=p;observedBytes=n;allocatorActive=1;}
static void EndAllocations(){allocatorActive=0;for(int i=0;i<allocationCount;i++)if(blocks[i].alive){free(blocks[i].pointer);blocks[i].alive=0;}}
struct ConstructorEntry { void* Construct(int); };
struct CameraLayout : FollowCamera { static void Print() {
    typedef char Layout[(sizeof(FollowCamera)==0x384 && sizeof(void*(ConstructorEntry::*)(int))==4
       && offsetof(CameraLayout,classNames)==0x28 && offsetof(CameraLayout,worldMatrix)==0x2c
       && offsetof(CameraLayout,eyePosition)==0x170 && offsetof(CameraLayout,cameraState)==0x244
       && offsetof(CameraLayout,followHeading)==0x29c && offsetof(CameraLayout,points)==0x2e4
       && offsetof(CameraLayout,isPlaced)==0x275 && offsetof(CameraLayout,isCrashView)==0x276
       && offsetof(CameraLayout,isSettled)==0x274 && offsetof(CameraLayout,pointCapacity)==0x2dc
       && offsetof(CameraLayout,frameTime)==0x340)?1:-1];
    printf("FollowCamera size %x; fields %x %x %x %x\n",sizeof(FollowCamera),
        offsetof(CameraLayout,classNames),offsetof(CameraLayout,eyePosition),
        offsetof(CameraLayout,cameraState),offsetof(CameraLayout,distanceForMaximumFov));
} };

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
static void Patch(DWORD site,DWORD old,DWORD next){if(*(BYTE*)site!=0xe8||site+5+*(DWORD*)(site+1)!=old)ExitProcess(94);*(DWORD*)(site+1)=next-site-5;}
static BYTE camera[sizeof(FollowCamera)],got[sizeof(FollowCamera)],want[sizeof(FollowCamera)];
static DWORD entry;static int checks;static unsigned fpControl=_MCW_EM|_PC_64|_RC_NEAR;

static void Word(BYTE*p,int offset,DWORD value){memcpy(p+offset,&value,4);}
static void BaseExpected(BYTE*p,int flags){
 Word(p,0,0);Word(p,4,1);for(int i=8;i<=0x20;i+=4)Word(p,i,0);
 p[0x24]=0xff;p[0x25]=(p[0x25]&0xf0)|((flags&1)?3:0);Word(p,0x28,0);
}
static void CameraExpected(BYTE*p,int flags){
 BaseExpected(p,flags);
 for(int row=0;row<4;row++)for(int column=0;column<4;column++)Word(p,0x2c+(row*4+column)*4,row==column?0x3f800000:0);
 Word(p,0x16c,0x429a0000);for(int i=0x170;i<=0x194;i+=4)Word(p,i,0);
 Word(p,0x184,0x3f800000);Word(p,0x18c,0x3f800000);
 for(i=0x1a0;i<0x1b8;i+=4)Word(p,i,0);
 Word(p,0x1bc,0x3f800000);Word(p,0x1c0,0x47c35000);
 Word(p,0x1cc,0);Word(p,0x1d0,0);Word(p,0x1d4,1);Word(p,0x1d8,1);
 Word(p,0x1dc,0x42da0000);Word(p,0x1e0,0x41200000);
 for(i=0x1e4;i<0x220;i+=4)Word(p,i,0);Word(p,0x21c,0x3f800000);
}
struct Store{unsigned offset,width;DWORD value;};
// Independently transcribed complete outer-store sequence, including the real
// state-0 slot71/66 stores. Constructor source and output are not its oracle.
static const Store stores[]={
 {0x275,1,0},{0x2b4,4,0},{0x2b8,4,0},{0x2bc,4,0},
 {0x2a8,4,0},{0x2ac,4,0},{0x2b0,4,0},{0x23c,4,0},{0x2c0,4,0},
 {0x29c,4,0},{0x2a0,4,0},{0x2e4,4,0},{0x276,1,0},{0x2a4,4,0x3f800000},
 {0x2d8,4,0},{0x228,4,0x41d80000},{0x258,4,0x429a0000},{0x2f0,4,0x429a0000},
 {0x294,4,0},{0x240,4,0},{0x27c,4,0},{0x280,4,0},{0x284,4,0},
 {0x288,4,0},{0x28c,4,0},{0x290,4,0},{0x298,4,0},{0x1d4,4,0},
 {0x268,4,0},{0x26c,4,0},{0x274,1,0},{0x2dc,4,0},{0x2e0,4,0},
 {0x2d4,4,0x40e00000},{0x2ec,4,0x40400000},{0x244,4,0},
 {0x244,4,0},{0x22c,4,0x3f860a91},{0x234,4,0},{0x220,4,0x413b851f},{0x258,4,0x42aa0000},
 {0x2c4,4,0x413b851f},{0x2c8,4,0x3f860a91},{0x2cc,4,0},
 {0x25c,4,0},{0x260,4,0},{0x224,4,0x413b851f},{0x264,4,0},
 {0x248,4,0},{0x250,4,0},{0x24c,4,0x429a0000},{0x230,4,0x3f860a91},
 {0x238,4,0},{0x254,4,0x429a0000},{0x277,1,0},{0x278,1,0},
 {0x310,4,0},{0x30c,4,0},{0x304,4,0},{0x308,4,0},{0x279,1,0},
 {0x270,4,0x41700000},{0x340,4,0},{0x2f4,4,0x41200000},
 {0x2f8,4,0x438c0000},{0x2fc,4,0x428c0000},{0x300,4,0x41a00000}};
static unsigned StoresExpected(BYTE*p,unsigned available){
 for(unsigned i=0;i<sizeof(stores)/sizeof(stores[0]);i++){
  const Store&t=stores[i];if(t.offset+t.width>available)return t.offset>available?t.offset:available;
  memcpy(p+t.offset,&t.value,t.width);
 }
 return 0;
}
static void NormalOracle(const BYTE*p,int seed,int flags){
 BYTE expected[sizeof(FollowCamera)];for(unsigned i=0;i<sizeof(expected);i++)expected[i]=(BYTE)(seed*17+i*73);
 CameraExpected(expected,flags);if(StoresExpected(expected,sizeof(expected)))ExitProcess(34);
 if(memcmp(expected,p,sizeof(expected))){unsigned at=0;while(expected[at]==p[at])at++;printf("Normal oracle offset%x %02x/%02x\n",at,expected[at],p[at]);ExitProcess(35);}
}
static void Run(int retail,int seed,int flags,BYTE*out){
 _fpreset();_controlfp(fpControl,_MCW_EM|_MCW_PC|_MCW_RC);
 for(unsigned i=0;i<sizeof(camera);i++)camera[i]=(BYTE)(seed*17+i*73);
 union {void*(ConstructorEntry::*method)(int);DWORD address;} call;
 call.address=retail?0x462ee0:entry;
 BeginAllocations(camera,sizeof(camera));
 void*result=(((ConstructorEntry*)camera)->*call.method)(flags);
 if(result!=camera||strcmp(*(char**)(camera+0x28),"GameObject,"))ExitProcess(5);
 memcpy(out,camera,sizeof(camera));
 if(*(DWORD*)out!=(retail?0x55282c:nativeFollowVptr))ExitProcess(33);
 *(DWORD*)out=0;*(DWORD*)(out+0x28)=0;NormalOracle(out,seed,flags);
 if(allocationCount!=2||blocks[0].requested!=1||blocks[1].requested!=12||blocks[0].alive||!blocks[1].alive)ExitProcess(31);
 DebugFree(*(char**)(camera+0x28),__FILE__,__LINE__);EndAllocations();
}

extern "C" void __cdecl __CxxFrameHandler();
extern int g_UnclaimedBytes;
static BYTE*page;static int exception,access,complete;static DWORD fault;
static int Filter(EXCEPTION_POINTERS*p){exception=p->ExceptionRecord->ExceptionCode;if(exception==EXCEPTION_ACCESS_VIOLATION){access=p->ExceptionRecord->ExceptionInformation[0];fault=p->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
struct GuardResult{BYTE page[4096];int exception,access,complete,allocated;DWORD fault;int allocations,events;unsigned sizes[4],actual[4];DWORD vptr,rawName;int nameId;char name[32];BYTE first[4];int alive[4];AllocationEvent transcript[12];};
static GuardResult guardGot,guardWant;
static void GuardRun(int retail,int available,GuardResult*out){
 _fpreset();_controlfp(fpControl,_MCW_EM|_MCW_PC|_MCW_RC);memset(out,0,sizeof(*out));memset(page,0xa5,4096);BYTE*obj=page+4096-available;
 exception=access=complete=0;fault=0;int before=g_UnclaimedBytes;
 union {void*(ConstructorEntry::*method)(int);DWORD address;} call;call.address=retail?0x462ee0:entry;
 BeginAllocations(obj,available);
 __try{(((ConstructorEntry*)obj)->*call.method)(1);complete=1;}
 __except(Filter(GetExceptionInformation())){}
 memcpy(out->page,page,4096);*(DWORD*)(out->page+4096-available)=0;*(DWORD*)(out->page+4096-available+0x28)=0;
 out->exception=exception;out->access=access;out->fault=fault;out->complete=complete;out->allocated=g_UnclaimedBytes-before;

 out->vptr=*(DWORD*)obj==nativeFollowVptr?0x55282c:*(DWORD*)obj;out->nameId=PointerId(*(void**)(obj+0x28));out->rawName=out->nameId==0?*(DWORD*)(obj+0x28):0;
 if(complete&&out->nameId>0){strncpy(out->name,*(char**)(obj+0x28),31);}
 out->allocations=allocationCount;out->events=eventCount;memcpy(out->transcript,allocationEvents,sizeof(allocationEvents));
 for(int k=0;k<allocationCount;k++){out->sizes[k]=blocks[k].requested;out->actual[k]=blocks[k].actual;out->alive[k]=blocks[k].alive;if(blocks[k].alive)out->first[k]=*(BYTE*)blocks[k].pointer;}
 EndAllocations();

}

static void GuardOracle(int available,const GuardResult&r){
 BYTE expected[4096];memset(expected,0xa5,sizeof(expected));BYTE*object=expected+4096-available;
 CameraExpected(object,1);unsigned failed=StoresExpected(object,available);
 if(memcmp(expected,r.page,4096)){unsigned at=0;while(expected[at]==r.page[at])at++;printf("Guard oracle available%x at%x %02x/%02x\n",available,at,expected[at],r.page[at]);ExitProcess(36);}
 if(r.allocations!=2||r.sizes[0]!=1||r.sizes[1]!=12||r.alive[0])ExitProcess(37);
 int cleanup=failed&&available>=0x276;
 if(failed){
  if(r.complete||r.exception!=EXCEPTION_ACCESS_VIOLATION||r.access!=1||r.fault!=(DWORD)(page+4096-available+failed)||r.vptr!=(cleanup?nativeBaseVptr:0x55282c)||r.alive[1]!=!cleanup||r.events!=(cleanup?4:3))ExitProcess(38);
 }else{
  if(!r.complete||r.exception||r.vptr!=0x55282c||!r.alive[1]||r.events!=3||strcmp(r.name,"GameObject,"))ExitProcess(39);
 }
 int expectedAllocation=1-r.actual[0]+12-(cleanup?r.actual[1]:0);
 if(r.allocated!=expectedAllocation)ExitProcess(40);
 if(r.transcript[0].kind!=1||r.transcript[0].index!=0||r.transcript[0].size!=1||r.transcript[1].kind!=1||r.transcript[1].index!=1||r.transcript[1].size!=12||r.transcript[2].kind!=2||r.transcript[2].index!=0)ExitProcess(41);
 if(cleanup&&(r.transcript[3].kind!=2||r.transcript[3].index!=1))ExitProcess(42);
}

static void AllocationOracle(int mode,const GuardResult&r){
 BYTE expected[sizeof(FollowCamera)];memset(expected,0xa5,sizeof(expected));BaseExpected(expected,1);
 if(memcmp(expected,r.page+4096-sizeof(expected),sizeof(expected)))ExitProcess(43);
 if(r.complete||r.vptr!=nativeBaseVptr||r.exception!=(mode==0?EXCEPTION_ACCESS_VIOLATION:0xe06d7363))ExitProcess(45);
 if(mode==0&&(r.access!=1||r.fault!=0||r.nameId!=-1||r.allocated!=0))ExitProcess(46);
 if(mode==2&&(r.nameId!=0||r.rawName!=0xa5a5a5a5||r.allocated!=0))ExitProcess(47);
 if(mode==3&&(r.nameId!=1||!r.alive[0]||r.first[0]!=0||r.allocated!=1-(int)r.actual[0]))ExitProcess(48);
 if(r.allocations!=(mode==3?2:1)||r.events!=(mode==3?2:1))ExitProcess(49);
 if(r.sizes[0]!=1||r.transcript[0].kind!=(mode==2?3:1)||r.transcript[0].index!=0||r.transcript[0].size!=1)ExitProcess(50);
 if(mode==3&&(r.sizes[1]!=12||r.transcript[1].kind!=3||r.transcript[1].index!=1||r.transcript[1].size!=12))ExitProcess(51);
}
int main(int argc,char**argv){
 setvbuf(stdout,0,_IONBF,0);if(argc!=11||!MapReference(argv[1]))return 1;
 CameraLayout::Print();entry=strtoul(argv[2],0,16);nativeFollowVptr=strtoul(argv[8],0,16);nativeBaseVptr=strtoul(argv[10],0,16);
 Patch(0x462f03,0x4bed80,strtoul(argv[3],0,16));
 Patch(0x46302e,0x466e50,strtoul(argv[4],0,16));
 for(int i=0;i<3;i++){int slot=i==0?58:i==1?61:66;*(DWORD*)(0x55282c+slot*4)=strtoul(argv[5+i],0,16);}
 memset((void*)0x65b448,0,12);float*z=(float*)0x65b438;z[0]=z[1]=0;z[2]=1;

 if(*(BYTE*)0x54ab6d!=0xe9||0x54ab72+*(DWORD*)0x54ab6e!=0x53471a)return 21;
 *(DWORD*)0x54ab6e=(DWORD)&__CxxFrameHandler-0x54ab72;
 if(*(BYTE*)0x54ab63!=0xe9||0x54ab68+*(DWORD*)0x54ab64!=0x4624d0)return 22;
 *(DWORD*)0x54ab64=strtoul(argv[9],0,16)-0x54ab68;
 FlushInstructionCache(GetCurrentProcess(),0,0);
 int flags[]={0,1,2,3,-1,0x7fffffff,0x80000000,0x55aa55aa};
 unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 int totalGuards=0,totalFaults=0,totalAllocations=0;
 for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++){
 fpControl=_MCW_EM|pcs[pc]|rcs[rc];failAt=throwAt=-1;
 for(int seed=0;seed<32;seed++)for(i=0;i<8;i++){
  Run(1,seed,flags[i],want);Run(0,seed,flags[i],got);checks++;
  if(memcmp(want,got,sizeof(got))){unsigned at=0;while(want[at]==got[at])at++;printf("Mismatch %d/%d at%x %02x/%02x\n",seed,flags[i],at,want[at],got[at]);return 3;}
 }


 if(!page){page=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 20;}
 MemTagStack*saved=g_MemTagStack;g_MemTagStack=0;
 int faults=0;for(int available=0x220;available<=0x384;available++){
  GuardRun(1,available,&guardWant);GuardRun(0,available,&guardGot);faults+=guardGot.exception!=0;GuardOracle(available,guardWant);GuardOracle(available,guardGot);
  if(memcmp(&guardWant,&guardGot,sizeof(guardGot))){unsigned at=0;while(((BYTE*)&guardWant)[at]==((BYTE*)&guardGot)[at])at++;
   printf("Guard mismatch available%x at%x values%02x/%02x fault%x/%x allocated%d/%d\n",available,at,((BYTE*)&guardWant)[at],((BYTE*)&guardGot)[at],guardWant.fault,guardGot.fault,guardWant.allocated,guardGot.allocated);for(int j=0;j<0x40;j+=4)printf("+%x %08x/%08x\n",j,*(DWORD*)(guardWant.page+4096-available+j),*(DWORD*)(guardGot.page+4096-available+j));return 3;}
 }

 totalGuards+=357;totalFaults+=faults;
 // A NULL second allocation is deliberately excluded: retail DebugRealloc
 // returns the old 1-byte-request block, then strcat needs 12 bytes. Its
 // native _msize can be only 8, making an ordinary run overwrite the CRT heap.
 const int allocationModes[]={0,2,3};
 for(int allocationCase=0;allocationCase<3;allocationCase++){
  int mode=allocationModes[allocationCase];failAt=mode==0?0:-1;throwAt=mode>=2?mode-2:-1;
  GuardRun(1,0x384,&guardWant);GuardRun(0,0x384,&guardGot);
  if(memcmp(&guardWant,&guardGot,sizeof(guardGot))){printf("Allocation differential mode%d\n",mode);return 32;}
  AllocationOracle(mode,guardWant);AllocationOracle(mode,guardGot);
  totalAllocations++;
 }
 g_MemTagStack=saved;
 }
 printf("FollowCamera constructor: %d normal, %d guarded (%d AV), %d allocation cases\n",checks,totalGuards,totalFaults,totalAllocations);return 0;
}
