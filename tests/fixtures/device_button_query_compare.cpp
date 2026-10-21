// Private full-body dispatcher comparison. No real DirectInput device is created.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class TrackGame;
struct UnknownInputEntry;
class PCControlInterface{public:virtual int IsDeviceButtonDown(int,int,int,int);};
class ActualKeyboard{public:int Query(int,int,UnknownInputEntry*);};
class ActualMouse{public:int Query(int,int,UnknownInputEntry*);};
class ActualJoystick{public:int Query(int,int,UnknownInputEntry*);};
class Probe{public:int Slot2(int,int,UnknownInputEntry*);int Slot5(int,int,UnknownInputEntry*);};
static BYTE referenceSpace[0x800000]={1};
struct World{BYTE control[0xcd0],keyboard[0x16dc],mouse[0x2e8],joysticks[8][0x600],game[0x3500];};
static World world;TrackGame* g_TrackGame;
struct Scenario{int seed,kind,control,modifier,device,mask,result,mutation,throwCallback,invalid,alias,real,state,modifiers,testKey,grave;};
static Scenario scenario;
struct Event{int slot,who,control,modifier,entry;World before,after;};
struct Observation{int result,completed,exception,faultKind,faultAddress,count,game;Event event;World initial,state;};
static Observation observed,expected;static int checks,retailActive;static DWORD keyboardTable[8],mouseTable[8],joystickTable[8];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d case%d retail%d\n",code,checks,retailActive);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(void* p){if(p==world.keyboard)return 1;if(p==world.mouse)return 2;for(int i=0;i<8;++i)if(p==world.joysticks[i])return 10+i;return(int)p;}
static int Callback(void* who,int slot,int control,int modifier,UnknownInputEntry* entry){if(observed.count++)Fail(81);Event& e=observed.event;e.slot=slot;e.who=Identity(who);e.control=control;e.modifier=modifier;e.entry=(int)entry;e.before=world;
 switch(scenario.mutation){case 1:{At(world.control,0x34)=0;At(world.control,0x30)=0;for(int j=0;j<8;++j)At(world.control,0x10+j*4)=0;break;}
 case 2:At(world.control,0x34)=(DWORD)world.mouse;At(world.control,0x30)=(DWORD)world.keyboard;break;
 case 3:memset((BYTE*)who+4,0x95,32);break;
 case 4:g_TrackGame=0;break;
 case 5:world.game[0x2d4]^=4;At(world.keyboard,0x16d8)^=0xff;break;
 case 6:At(world.keyboard,0x260+41*20)=1;break;
 }
 if(scenario.throwCallback)throw 0x732;
 int result=scenario.result;
 if(scenario.real){if(e.who==1)result=((ActualKeyboard*)who)->Query(control,modifier,entry);else if(e.who==2)result=((ActualMouse*)who)->Query(control,modifier,entry);else result=((ActualJoystick*)who)->Query(control,modifier,entry);}
 e.after=world;return result;}
int Probe::Slot2(int control,int modifier,UnknownInputEntry* entry){return Callback(this,2,control,modifier,entry);}
int Probe::Slot5(int control,int modifier,UnknownInputEntry* entry){return Callback(this,5,control,modifier,entry);}
static int MapReference(const char* path){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{int(PCControlInterface::*method)(int,int,int,int);DWORD address;}call;call.address=0x4bf4f0;PCControlInterface* receiver=(PCControlInterface*)(scenario.invalid==3?1:scenario.invalid==4?0:(DWORD)world.control);__try{observed.result=retailActive?(receiver->*call.method)(scenario.control,scenario.kind,scenario.modifier,scenario.device):receiver->PCControlInterface::IsDeviceButtonDown(scenario.control,scenario.kind,scenario.modifier,scenario.device);observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,scenario.seed&255,sizeof(world));At(world.keyboard,0)=(DWORD)keyboardTable;At(world.mouse,0)=(DWORD)mouseTable;for(int j=0;j<8;++j){At(world.joysticks[j],0)=(DWORD)joystickTable;At(world.control,0x10+j*4)=(scenario.mask&(4<<j))?(DWORD)world.joysticks[j]:0;for(int b=0;b<32;++b)At(world.joysticks[j],0x264+b*20)=scenario.state;}
 At(world.control,0x34)=(scenario.mask&1)?(DWORD)world.keyboard:0;At(world.control,0x30)=(scenario.mask&2)?(DWORD)world.mouse:0;for(int k=0;k<256;++k)At(world.keyboard,0x260+k*20)=scenario.state;for(int m=0;m<4;++m)At(world.mouse,0x260+m*20)=scenario.state;At(world.keyboard,0x260+41*20)=scenario.grave;At(world.keyboard,0x16d8)=scenario.modifiers;world.game[0x2d4]=(BYTE)((world.game[0x2d4]&~4)|(scenario.testKey?4:0));At(world.game,0x14)=(DWORD)world.control;g_TrackGame=(TrackGame*)world.game;
 if(scenario.alias==1)At(world.control,0x30)=(DWORD)world.keyboard;if(scenario.alias==2)for(int a=0;a<8;++a)At(world.control,0x10+a*4)=(DWORD)world.joysticks[0];
 if(scenario.invalid==1){At(world.control,0x34)=1;At(world.control,0x30)=1;for(int d=0;d<8;++d)At(world.control,0x10+d*4)=1;}if(scenario.invalid==2){At(world.keyboard,0)=1;At(world.mouse,0)=1;for(int z=0;z<8;++z)At(world.joysticks[z],0)=1;}
 observed.initial=world;try{Invoke();}catch(int code){observed.exception=code;}observed.state=world;observed.game=g_TrackGame==(TrackGame*)world.game?1:(int)g_TrackGame;}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d offset%d %02x/%02x kind%d device%d mask%x control%x modifier%x result%x mutation%d invalid%d real%d\n",checks,offset,a[offset],b[offset],scenario.kind,scenario.device,scenario.mask,scenario.control,scenario.modifier,scenario.result,scenario.mutation,scenario.invalid,scenario.real);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=53;scenario.mask=0x3ff;scenario.result=0x87654321;scenario.modifier=0x3f;scenario.state=1;}
static int ExpectedModifier(){if(!scenario.testKey&&(scenario.modifier&0x80))return 0;if(scenario.modifier==0x3f)return 1;if(scenario.testKey&&scenario.modifier==0x80&&scenario.grave==1)return 1;if(scenario.modifier==(int)0x80000000)return scenario.modifiers==0;if(scenario.modifier&0x40)return(scenario.modifiers|0x40)==scenario.modifier;return(scenario.modifiers&scenario.modifier)!=0;}
static void BasicOracle(){int selected=0,who=0;if(scenario.kind==0){selected=scenario.mask&1;who=1;}else if(scenario.kind==1){selected=scenario.mask&2;who=2;}else if(scenario.kind==2||scenario.kind==3){selected=scenario.mask&(4<<scenario.device);who=10+scenario.device;}if(memcmp(&world,&observed.initial,sizeof(world))||!observed.completed||observed.exception||observed.count!=(selected?1:0)||observed.result!=(selected?scenario.result:0))Fail(90);if(selected&&(observed.event.slot!=(scenario.kind<2?5:2)||observed.event.who!=who||observed.event.control!=scenario.control||observed.event.modifier!=scenario.modifier||observed.event.entry||memcmp(&world,&observed.event.before,sizeof(world))))Fail(91);}
int main(int argc,char** argv){if(argc!=2||!MapReference(argv[1]))return 3;keyboardTable[5]=mouseTable[5]=Address(&Probe::Slot5);joystickTable[2]=Address(&Probe::Slot2);
 const int kinds[]={0,1,2,3,-1,4,0x7fffffff,0x80000000};const int values[]={0,1,-1,0x80000000,0x7fffffff,0x87654321};
 for(int kind=0;kind<8;++kind)for(int device=0;device<8;++device)for(int presence=0;presence<2;++presence)for(int value=0;value<6;++value){Base();scenario.kind=kinds[kind];scenario.device=device;scenario.mask=presence?0x3ff:0;scenario.control=values[value];scenario.modifier=values[5-value];scenario.result=values[value];Compare();BasicOracle();}
 for(int mask=0;mask<1024;++mask)for(int kind2=0;kind2<4;++kind2){Base();scenario.kind=kind2;scenario.mask=mask;scenario.device=(mask>>4)&7;Compare();BasicOracle();}
 for(int kind3=0;kind3<4;++kind3)for(int dev=0;dev<8;++dev)for(int mutation=0;mutation<7;++mutation)for(int throwing=0;throwing<2;++throwing){Base();scenario.kind=kind3;scenario.device=dev;scenario.mutation=mutation;scenario.throwCallback=throwing;Compare();if(throwing&&observed.exception!=0x732)Fail(92);}
 for(int alias=1;alias<=2;++alias)for(int kind4=0;kind4<4;++kind4)for(int dev2=0;dev2<8;++dev2){Base();scenario.alias=alias;scenario.kind=kind4;scenario.device=dev2;Compare();}
 for(int invalid=1;invalid<=4;++invalid)for(int kind5=0;kind5<8;++kind5){Base();scenario.kind=kinds[kind5];scenario.invalid=invalid;Compare();if(kind5<4){if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)Fail(93);}else if(observed.exception||observed.result||observed.count)Fail(94);}
 const int modifiers[]={0,1,2,4,8,0x3f,0x40,0x41,0x7f,0x80,0xff,0x80000000,-1};
 for(int type=0;type<4;++type)for(int held=0;held<3;++held)for(int mods=0;mods<256;++mods)for(int request=0;request<13;++request){Base();scenario.real=1;scenario.kind=type;scenario.control=2;scenario.state=held;scenario.modifiers=mods;scenario.modifier=modifiers[request];scenario.testKey=(mods>>1)&1;scenario.grave=(mods>>2)&1;Compare();int want=held==1&&ExpectedModifier();if(observed.exception||observed.result!=want||!observed.completed||observed.event.entry||memcmp(&world,&observed.event.before,sizeof(world)))Fail(95);}
 for(int type2=1;type2<4;++type2)for(int held2=0;held2<3;++held2)for(int request2=0;request2<13;++request2){Base();scenario.real=1;scenario.mask&=~1;scenario.kind=type2;scenario.control=2;scenario.state=held2;scenario.modifier=modifiers[request2];Compare();int allowed=type2==1||scenario.modifier==0||scenario.modifier==0x3f||scenario.modifier==(int)0x80000000;if(observed.exception||observed.result!=(held2==1&&allowed))Fail(96);}
 for(int kind6=0;kind6<4;++kind6)for(int mut=0;mut<7;++mut){Base();scenario.real=1;scenario.kind=kind6;scenario.control=2;scenario.mutation=mut;Compare();}
 printf("DeviceButtonQuery: %d dispatch, real-helper, callback and fault scenarios match\n",checks);return 0;}
