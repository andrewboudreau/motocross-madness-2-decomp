// Private complete GetDc comparison; real strict region/release helpers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct CameraRect {int left,top,right,bottom;};
class BackgroundImage {public:int GetDc(void**,CameraRect*,int,int,int*,int*,CameraRect*);void MarkRegionPending(int);void SetRegionRect(int,CameraRect*);int ReleaseDc();};
extern int g_GetDcRegionIndex;extern CameraRect g_GetDcClipRect;
static BYTE referenceSpace[0x800000]={1};
struct Region {int framesLeft;CameraRect rects[3];int pending,lastFrame,owner;};
struct Surface {DWORD table;int gets,releases,lastDC;};
struct World {BYTE object[0x74],target[0x80];Region regions[4];Surface surfaces[3];DWORD dc;int frames,a;CameraRect rect,clip;};
struct State {World world;int regionIndex;CameraRect scratch;};
struct Scenario {int dirty,copyValid,frames,next,last,copied,capacity,frameIndex,pattern,held,index,status,releaseStatus,writeDC,shape,alias,invalid,steps,throwAt,mutateAt,mutation,seed;};
struct Event {int kind,who,input,result,output;State before,after;};
struct Observation {int results[8],completed,exception,faultKind,faultAddress,count;Event events[32];State steps[8],state;};
static World world;static Scenario scenario;static Observation observed,expected;static int checks,retailActive;static DWORD surfaceTable[27];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static int& RegionIndex(){return retailActive?*(int*)0x5777a0:g_GetDcRegionIndex;}
static CameraRect& Scratch(){return retailActive?*(CameraRect*)0x5777a8:g_GetDcClipRect;}
static void Fail(int code){printf("Invariant%d case%d\n",code,checks);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(void* p){if(!p)return 0;for(int i=0;i<3;++i)if(p==&world.surfaces[i])return i+1;return(int)p;}
static void Snapshot(State& state){state.world=world;state.regionIndex=RegionIndex();state.scratch=Scratch();}
static Event& Begin(int kind,void* who){if(observed.count>=32)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);Snapshot(e.before);return e;}
static void Boundary(){if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:At(world.target,0x48)=(DWORD)&world.surfaces[2];break;
 case 1:At(world.object,0x3c)=(DWORD)&world.surfaces[2];break;
 case 2:At(world.object,0x6c)=(DWORD)&world.surfaces[2];break;
 case 3:At(world.object,0x70)=0xabc;break;
 case 4:At(world.object,0x40)=0;At(world.object,0x60)=0;break;
 case 5:RegionIndex()=99;Scratch().left=-7;break;
 case 6:world.frames=0;world.rect.right=2;break;
 case 7:At(world.object,0x18)=0;break;
 case 8:At(world.object,0x64)=-1;At(world.object,0x68)=1;break;
 case 9:world.regions[0].lastFrame=9;world.regions[0].rects[0].left=19;break;
 }}if(scenario.throwAt==observed.count)throw 0x736;}
static long __stdcall ProbeGetDC(void* who,void** out){Event& e=Begin(1,who);if(out!=(void**)(world.object+0x70))Fail(82);e.input=(int)*out;++((Surface*)who)->gets;if(scenario.writeDC==1)*out=(void*)(0x900+e.who);if(scenario.writeDC==2)*out=0;Boundary();e.result=scenario.status;e.output=(int)*out;Snapshot(e.after);return scenario.status;}
static long __stdcall ProbeReleaseDC(void* who,void* dc){Event& e=Begin(2,who);e.input=(int)dc;++((Surface*)who)->releases;((Surface*)who)->lastDC=(int)dc;Boundary();e.result=scenario.releaseStatus;Snapshot(e.after);return scenario.releaseStatus;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int PatchReference(){const DWORD sites[]={0x404be3,0x404bfa,0x404c27},targets[]={0x404cb0,0x404240,0x404c80};DWORD replacements[]={Address(&BackgroundImage::MarkRegionPending),Address(&BackgroundImage::SetRegionRect),Address(&BackgroundImage::ReleaseDc)};for(int i=0;i<3;++i){BYTE* p=(BYTE*)sites[i];if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=targets[i])return 0;*(DWORD*)(p+1)=replacements[i]-(DWORD)(p+5);}return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){void** dc=(void**)&world.dc;int* a=&world.a;int* frames=&world.frames;CameraRect* rect=&world.rect;CameraRect* clip=&world.clip;BackgroundImage* object=(BackgroundImage*)world.object;
 switch(scenario.alias){case 1:a=frames;break;case 2:dc=(void**)frames;break;case 3:dc=(void**)a;break;case 4:clip=rect;break;case 5:frames=(int*)(world.object+0x60);break;case 6:a=(int*)(world.object+0x40);break;case 7:dc=(void**)(world.object+0x70);break;case 8:clip=&world.regions[0].rects[scenario.frameIndex];break;case 9:rect=&world.regions[0].rects[scenario.frameIndex];break;case 10:frames=(int*)(world.object+0x48);break;case 11:clip=(CameraRect*)(world.object+0x64);break;}
 switch(scenario.invalid){case 1:dc=(void**)1;break;case 2:a=(int*)1;break;case 3:frames=(int*)1;break;case 4:rect=(CameraRect*)1;break;case 5:clip=(CameraRect*)1;break;case 6:object=0;break;}
 __try{for(int step=0;step<scenario.steps;++step){union{int(BackgroundImage::*p)(void**,CameraRect*,int,int,int*,int*,CameraRect*);DWORD a;}call;call.a=0x4049d0;observed.results[step]=retailActive?(object->*call.p)(dc,rect,scenario.index,scenario.dirty,frames,a,clip):object->GetDc(dc,rect,scenario.index,scenario.dirty,frames,a,clip);++observed.completed;Snapshot(observed.steps[step]);}}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));memset(&world,scenario.seed&255,sizeof(world));for(int j=0;j<8;++j)observed.results[j]=-999;At(world.object,0x18)=(DWORD)world.target;At(world.object,0x3c)=(DWORD)&world.surfaces[1];At(world.object,0x40)=scenario.copyValid;At(world.object,0x48)=scenario.capacity;At(world.object,0x50)=(DWORD)world.regions;At(world.object,0x60)=scenario.next;At(world.object,0x64)=scenario.last;At(world.object,0x68)=scenario.copied;At(world.object,0x6c)=scenario.held<0?0:(DWORD)&world.surfaces[scenario.held];At(world.object,0x70)=0x123;At(world.target,0xc)=6;At(world.target,0x10)=5;At(world.target,0x14)=3;At(world.target,0x18)=scenario.frameIndex;At(world.target,0x1c)=7;At(world.target,0x48)=(DWORD)&world.surfaces[0];
 for(int i=0;i<3;++i){world.surfaces[i].table=(DWORD)surfaceTable;world.surfaces[i].gets=world.surfaces[i].releases=0;world.surfaces[i].lastDC=-1;}
 for(i=0;i<4;++i){world.regions[i].framesLeft=i%2;world.regions[i].pending=-1;world.regions[i].lastFrame=(scenario.pattern&(1<<i))?7:6;for(int frame=0;frame<3;++frame){CameraRect& r=world.regions[i].rects[frame];r.left=i*2;r.top=frame;r.right=i*2+7;r.bottom=frame+7;}}
 world.dc=0xface;world.a=17;world.frames=scenario.frames;world.rect.left=1;world.rect.top=1;world.rect.right=8;world.rect.bottom=8;world.clip.left=world.clip.top=world.clip.right=world.clip.bottom=-71;
 if(scenario.shape==1)world.rect.left=world.rect.right=3;if(scenario.shape==2){world.rect.left=20;world.rect.top=20;world.rect.right=30;world.rect.bottom=30;}if(scenario.shape==3){world.rect.left=8;world.rect.top=9;world.rect.right=-2;world.rect.bottom=-3;}if(scenario.shape==4){world.rect.left=-8;world.rect.top=-8;world.rect.right=8;world.rect.bottom=8;}if(scenario.shape==5){world.rect.left=0;world.rect.top=0;world.rect.right=0;world.rect.bottom=0;}
 switch(scenario.invalid){case 7:At(world.object,0x18)=0;break;case 8:At(world.object,0x50)=0;break;case 9:At(world.target,0x48)=1;break;case 10:At(world.object,0x6c)=1;break;case 11:At(world.target,0x48)=0;break;case 12:At(world.object,0x3c)=0;break;}
 RegionIndex()=33;Scratch().left=-11;Scratch().top=-12;Scratch().right=-13;Scratch().bottom=-14;try{Invoke();}catch(int code){observed.exception=code;}Snapshot(observed.state);}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d offset%d %02x/%02x dirty%d copy%d frames%d next%d last%d copied%d capacity%d frame%d pattern%x held%d index%d alias%d invalid%d mutate%d/%d throw%d fault%x/%x\n",checks,offset,a[offset],b[offset],scenario.dirty,scenario.copyValid,scenario.frames,scenario.next,scenario.last,scenario.copied,scenario.capacity,scenario.frameIndex,scenario.pattern,scenario.held,scenario.index,scenario.alias,scenario.invalid,scenario.mutateAt,scenario.mutation,scenario.throwAt,expected.faultAddress,observed.faultAddress);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.dirty=scenario.copyValid=scenario.copied=1;scenario.frames=1;scenario.last=-1;scenario.capacity=3;scenario.pattern=7;scenario.held=-1;scenario.index=-1;scenario.writeDC=1;scenario.steps=1;scenario.seed=19;}
static void Oracle(int result,int who,int calls){if(observed.exception||observed.completed!=scenario.steps||observed.results[0]!=result||observed.count!=calls)Fail(90);if(who>=0&&Identity((void*)At(world.object,0x6c))!=who)Fail(91);}
int main(int argc,char** argv){CameraRect zero={0,0,0,0};if(g_GetDcRegionIndex||memcmp(&g_GetDcClipRect,&zero,16))return 6;if(argc!=2||!MapReference(argv[1])||!PatchReference())return 3;surfaceTable[17]=(DWORD)ProbeGetDC;surfaceTable[26]=(DWORD)ProbeReleaseDC;
 Base();Compare();Oracle(1,1,1);if(world.frames||At(world.object,0x60)!=1||world.dc!=0x901)Fail(92);Base();scenario.next=1;Compare();Oracle(1,2,1);if(world.frames!=1||world.a!=1||At(world.object,0x60))Fail(93);
 Base();scenario.held=0;Compare();Oracle(1,1,0);if(world.dc!=0x123)Fail(94);Base();scenario.held=2;Compare();Oracle(1,1,2);if(observed.events[0].kind!=2||observed.events[0].who!=3||observed.events[1].kind!=1||world.surfaces[2].lastDC!=0x123)Fail(95);
 Base();scenario.frames=0;Compare();Oracle(1,1,1);if(world.clip.left!=1||world.clip.top!=1||world.clip.right!=7||world.clip.bottom!=7||world.a!=1||At(world.object,0x64)||At(world.object,0x68))Fail(96);
 Base();scenario.frames=0;scenario.pattern=0;Compare();Oracle(0,0,0);if(At(world.object,0x64)!=0xffffffff||At(world.object,0x68)!=1)Fail(97);
 Base();scenario.dirty=0;scenario.index=1;Compare();Oracle(1,1,1);if(world.frames!=3||world.regions[1].lastFrame!=7||world.regions[1].rects[0].right!=6||world.regions[1].rects[0].bottom!=5)Fail(98);
 Base();scenario.frames=2;scenario.steps=4;Compare();if(observed.exception||observed.completed!=4||observed.count!=5)Fail(101);for(int step=0;step<4;++step){if(observed.results[step]!=1||observed.steps[step].world.frames!=(step<2?1:0)||observed.steps[step].world.dc!=(step==1?0x902:0x901))Fail(102);}
 Base();scenario.frames=0;scenario.capacity=1;scenario.pattern=1;scenario.steps=3;Compare();if(observed.exception||observed.completed!=3||observed.results[0]!=1||observed.results[1]!=1||observed.results[2]!=0||world.dc||At(world.object,0x64)!=0xffffffff||At(world.object,0x68)!=1)Fail(103);
 const int frameValues[]={-1,0,1,2};for(int dirty=0;dirty<2;++dirty)for(int copy=0;copy<2;++copy)for(int frames=0;frames<4;++frames)for(int next=0;next<2;++next)for(int held=-1;held<3;++held){Base();scenario.dirty=dirty;scenario.copyValid=copy;scenario.frames=frameValues[frames];scenario.next=next;scenario.held=held;Compare();}
 for(int capacity=-1;capacity<=4;++capacity)for(int frame=0;frame<3;++frame)for(int pattern=0;pattern<16;++pattern)for(int last=-1;last<=4;++last)for(int copied=0;copied<2;++copied)for(int shape=0;shape<6;++shape){Base();scenario.frames=0;scenario.capacity=capacity;scenario.frameIndex=frame;scenario.pattern=pattern;scenario.last=last;scenario.copied=copied;scenario.shape=shape;Compare();}
 const int statuses[]={0,1,-1,0x80000000,0x7fffffff};for(int status=0;status<5;++status)for(int release=0;release<5;++release)for(int write=0;write<3;++write)for(int cache=-1;cache<3;++cache){Base();scenario.held=cache;scenario.status=statuses[status];scenario.releaseStatus=statuses[release];scenario.writeDC=write;Compare();Oracle(1,1,cache==0?0:(cache<0?1:2));int expectedDC=cache==0||(!write&&cache<0)?0x123:(write==1?0x901:0);if(world.dc!=expectedDC)Fail(99);}
 for(int alias=0;alias<12;++alias)for(int mode=0;mode<4;++mode)for(int idx=-1;idx<4;++idx){Base();scenario.alias=alias;scenario.index=idx;if(mode==1)scenario.frames=0;if(mode==2)scenario.next=1;if(mode==3)scenario.dirty=0;Compare();}
 for(int path=0;path<4;++path){Base();if(path==1){scenario.frames=0;scenario.held=2;}if(path==2){scenario.next=1;scenario.held=0;}if(path==3)scenario.dirty=0;Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();if(observed.exception!=0x736||observed.count!=at)Fail(100);scenario.throwAt=0;for(int mutation=0;mutation<10;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=12;++invalid)for(int path2=0;path2<4;++path2){Base();scenario.invalid=invalid;if(path2==1)scenario.frames=0;if(path2==2)scenario.next=1;if(path2==3){scenario.dirty=0;scenario.index=1;}Compare();}
 for(int mask=0;mask<16;++mask)for(int count=0;count<5;++count){Base();scenario.frames=0;scenario.capacity=count;scenario.pattern=mask;scenario.steps=8;Compare();}
 printf("BackgroundDc: %d region, actual-helper, COM, alias and fault scenarios match\n",checks);return 0;}
