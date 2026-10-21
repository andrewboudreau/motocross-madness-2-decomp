// Private source/retail comparison; COM output is initialized only on success.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
class TrackGame;
class PCJoystickDevice{public:int ReadImmediateState(int);};
class JoystickDevice{public:void FeedAxis(int,float,float);};
class ActualJoystickDevice{public:void FeedAxis(int,float,float);};
class ControlInterface{public:void QueueEvent(int,int,int,int);};
class ActualControlInterface{public:void QueueEvent(int,int,int,int);};
static BYTE referenceSpace[0x800000]={1};
struct World{BYTE joystick[0x5ec],games[2][0x3500],controls[2][0xcd0],bindings[6][3][0x3c];DWORD lists[6][3],device[4];BYTE input[80];};
static World world;TrackGame* g_TrackGame;
struct Scenario{int seed,device,status,subtype,flip,axes[6],ranges[6],buttonPattern,previous,bindCount,aliases,queueCount,throwAt,mutateAt,mutation,invalid,round,argument;};
static Scenario scenario;
struct State{World world;int game,fp;};
struct Event{int kind,who,args[5];BYTE input[80];State before,after;};
struct Observation{int result,completed,exception,faultKind,faultAddress,count,fp;Event events[48];State state;};
static Observation observed,expected;static int checks,retailActive;static DWORD deviceTable[26];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d case%d retail%d count%d\n",code,checks,retailActive,observed.count);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.joystick)return 1;if(p==world.device)return 2;for(int n=0;n<2;++n){if(p==world.games[n])return 10+n;if(p==world.controls[n])return 20+n;}return(int)p;}
static void FromReference(){if(retailActive)g_TrackGame=*(TrackGame**)0x56e26c;}
static void ToReference(){if(retailActive)*(TrackGame**)0x56e26c=g_TrackGame;}
static void Snapshot(State& s){FromReference();s.world=world;s.game=Identity(g_TrackGame);s.fp=_statusfp();}
static Event& Begin(int kind,const void* who,int a=0,int b=0,int c=0,int d=0){FromReference();if(observed.count>=48)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;Snapshot(e.before);return e;}
static void Boundary(){if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:g_TrackGame=(TrackGame*)world.games[1];break;
 case 1:At(world.games[0],0x14)=(DWORD)world.controls[1];break;
 case 2:At(world.joystick,0x260)=0x76543210;break;
 case 3:world.joystick[0x3d]=4;break;
 case 4:world.joystick[0x3d]=0;world.joystick[0x574]^=1;break;
 case 5:At(world.joystick,0x58c+4)=0x47000000;At(world.joystick,0x58c+8)=0x47800000;break;
 case 6:At(world.joystick,0x264+31*20)=2;At(world.joystick,0x264+30*20)=0;break;
 case 7:At(world.games[0],0x14)=0;break;
 case 8:At(world.joystick,0x4fc)=0;break;
 case 9:At(world.joystick,0x4fc+4)=0;break;
 case 10:At(world.controls[0],0x38)=127;break;
 }}ToReference();if(scenario.throwAt==observed.count)throw 0x730;}
void JoystickDevice::FeedAxis(int axis,float value,float range){Event& e=Begin(2,this,axis,*(int*)&value,*(int*)&range);Boundary();((ActualJoystickDevice*)this)->FeedAxis(axis,value,range);Snapshot(e.after);}
void ControlInterface::QueueEvent(int control,int kind,int pressed,int device){Event& e=Begin(3,this,control,kind,pressed,device);Boundary();((ActualControlInterface*)this)->QueueEvent(control,kind,pressed,device);Snapshot(e.after);}
static long __stdcall GetDeviceState(void* who,unsigned long size,void* data){Event& e=Begin(1,who,size);if(size!=80||!data)Fail(82);if(scenario.status>=0){memcpy(data,world.input,80);memcpy(e.input,data,80);}Boundary();Snapshot(e.after);return scenario.status;}
static DWORD Replacement(DWORD va){if(va==0x489c00)return Address(&JoystickDevice::FeedAxis);if(va==0x43cea0)return Address(&ControlInterface::QueueEvent);Fail(83);return 0;}
static int MapReference(const char* path,const char* patches){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int count=0;while(fread(entry,sizeof(entry),1,f)==1){BYTE* site=(BYTE*)entry[0];if(entry[0]<0x4c3790||entry[0]+5>0x4c3956||*site!=0xe8||entry[2]!=0xe8||(DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1])return 0;*(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);++count;}fclose(f);return count==7&&FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{int(PCJoystickDevice::*method)(int);DWORD address;}call;call.method=&PCJoystickDevice::ReadImmediateState;if(retailActive)call.address=0x4c3790;__try{observed.result=(((PCJoystickDevice*)world.joystick)->*call.method)(scenario.argument);observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,0x53+scenario.seed%39,sizeof(world));world.device[0]=(DWORD)deviceTable;At(world.joystick,0x25c)=scenario.device?(DWORD)world.device:0;At(world.joystick,0x260)=17;world.joystick[0x3d]=(BYTE)scenario.subtype;world.joystick[0x574]=(BYTE)((world.joystick[0x574]&~1)|(scenario.flip&1));
 for(int a=0;a<6;++a){At(world.input,a*4)=scenario.axes[a];At(world.joystick,0x58c+a*4)=scenario.ranges[a];At(world.joystick,0x4fc+a*20)=scenario.bindCount;At(world.joystick,0x500+a*20)=(DWORD)world.lists[a];
  for(int b=0;b<3;++b){world.lists[a][b]=(DWORD)world.bindings[scenario.aliases?0:a][scenario.aliases?0:b];At(world.bindings[a][b],0x28)=0x3f000000;At(world.bindings[a][b],0x2c)=0;At(world.bindings[a][b],0x30)=0x3f800000;At(world.bindings[a][b],0x34)=0x3f0ccccd;At(world.bindings[a][b],0x38)=0x3ee66666;}}
 At(world.input,24)=0x12345678;At(world.input,28)=0xabcdef01;for(int p=0;p<4;++p)At(world.input,32+p*4)=scenario.seed==19?(p?9000*p:0xffffffff):(0x9e3779b9U*(scenario.seed+p));
 for(int k=0;k<32;++k){BYTE value;switch(scenario.buttonPattern){case 0:value=0;break;case 1:value=0xff;break;case 2:value=(BYTE)(k&1?0x80:0x7f);break;default:value=(BYTE)(scenario.seed+k*37);break;}world.input[48+k]=value;At(world.joystick,0x264+k*20)=scenario.previous==3?((value>>7)^1):scenario.previous;}
 for(int n=0;n<2;++n){At(world.games[n],0x14)=(DWORD)world.controls[n];At(world.controls[n],0x38)=scenario.queueCount;}
 g_TrackGame=(TrackGame*)world.games[0];switch(scenario.invalid){case 1:g_TrackGame=0;break;case 2:At(world.games[0],0x14)=0;break;case 3:At(world.joystick,0x25c)=1;break;case 4:At(world.joystick,0x500)=0;break;case 5:world.lists[0][0]=0;break;}
 ToReference();_fpreset();_controlfp(scenario.round,_MCW_RC);try{Invoke();}catch(int code){observed.exception=code;}observed.fp=_statusfp();Snapshot(observed.state);_fpreset();}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d offset%d %02x/%02x subtype%d flip%d pattern%d previous%d bindings%d alias%d status%d round%x mutation%d/%d throw%d invalid%d events%d/%d fp%x/%x\n",checks,offset,a[offset],b[offset],scenario.subtype,scenario.flip,scenario.buttonPattern,scenario.previous,scenario.bindCount,scenario.aliases,scenario.status,scenario.round,scenario.mutateAt,scenario.mutation,scenario.throwAt,scenario.invalid,expected.count,observed.count,expected.fp,observed.fp);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.device=1;scenario.bindCount=2;scenario.buttonPattern=2;scenario.previous=3;scenario.argument=12345;for(int a=0;a<6;++a){scenario.axes[a]=a*8192;scenario.ranges[a]=0x47800000;}}
static int Count(int kind){int count=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++count;return count;}
static void Oracle(){if(!observed.completed||observed.exception||observed.result!=1||Count(1)!=1||Count(2)!=6||Count(3)!=32||At(world.controls[0],0x38)!=32)Fail(90);
 for(int a=0;a<6;++a){float raw=(float)(a*8192),feed=a==2?65536-raw:raw;Event& e=observed.events[1+a];if(e.kind!=2||e.args[0]!=a||e.args[1]!=*(int*)&feed||e.args[2]!=0x47800000||At(world.joystick,0x4e4+a*4)!=*(DWORD*)&raw)Fail(91);float mapped=feed/65536;for(int b=0;b<2;++b)if(At(world.bindings[a][b],0x24)!=*(DWORD*)&mapped)Fail(92);}
 for(int k=0;k<32;++k){int pressed=k&1;BYTE* button=world.joystick+0x264+k*20;BYTE* before=observed.events[0].before.world.joystick+0x264+k*20;if(At(button,0)!=(DWORD)pressed||At(button,pressed?8:4)||At(button,pressed?16:12)||At(button,pressed?4:8)!=At(before,pressed?4:8)||At(button,pressed?12:16)!=At(before,pressed?12:16))Fail(93);BYTE* event=world.controls[0]+0x3c+k*20;if(At(event,0)!=(DWORD)k||At(event,4)!=2||At(event,8)!=17||At(event,12)!=(DWORD)pressed||At(event,16)!=0x3f)Fail(94);}
 if(memcmp(world.joystick+0x5d8,world.input+32,16))Fail(95);
}
int main(int argc,char** argv){if(argc!=3||!MapReference(argv[1],argv[2]))return 3;deviceTable[9]=(DWORD)GetDeviceState;
 Base();Compare();Oracle();Base();scenario.subtype=4;Compare();if(Count(2)||Count(3)!=32||memcmp(world.joystick+0x4e4,observed.events[0].before.world.joystick+0x4e4,24))return 4;
 Base();scenario.status=-1;Compare();if(observed.result||observed.count!=1||memcmp(&world,&observed.events[0].before.world,sizeof(world)))return 5;
 Base();scenario.device=0;Compare();if(observed.result||observed.count||observed.exception)return 6;
 Base();scenario.buttonPattern=0;scenario.previous=0;scenario.invalid=1;scenario.argument=-2147483647-1;Compare();if(!observed.completed||observed.result!=1||Count(3)||At(world.controls[0],0x38)||memcmp(world.joystick+0x264,observed.events[0].before.world.joystick+0x264,32*20)||memcmp(world.joystick+0x5d8,world.input+32,16))Fail(97);
 Base();scenario.status=-1;scenario.invalid=1;Compare();if(!observed.completed||observed.result||observed.exception||observed.count!=1)Fail(98);
 Base();scenario.device=0;scenario.invalid=1;Compare();if(!observed.completed||observed.result||observed.exception||observed.count)Fail(99);
 Base();scenario.flip=1;Compare();if(observed.events[2].args[1]!=0x47600000||At(world.joystick,0x4e8)!=0x46000000)Fail(100);
 const int values[]={-2147483647-1,-16777219,-65536,-1,0,1,32768,65535,65536,16777217,16777219,2147483647};const int ranges[]={0,0x3f800000,0x47800000,0xbf800000,0x7f800000,0xff800000,0x7fc01234};const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 for(int v=0;v<12;++v)for(int range=0;range<7;++range)for(int flip=0;flip<2;++flip)for(int round=0;round<4;++round){Base();for(int axis=0;axis<6;++axis){scenario.axes[axis]=values[(v+axis)%12];scenario.ranges[axis]=ranges[(range+axis)%7];}scenario.flip=flip;scenario.round=rounds[round];scenario.seed=checks;Compare();}
 for(int buttons=0;buttons<256;++buttons)for(int previous=0;previous<4;++previous)for(int subtype=0;subtype<2;++subtype){Base();scenario.seed=buttons;scenario.buttonPattern=3;scenario.previous=previous;scenario.subtype=subtype?4:0;Compare();}
 const int statuses[]={0,1,-1,(-2147483647-1),2147483647};for(int status=0;status<5;++status)for(int subtype2=0;subtype2<256;++subtype2){Base();scenario.status=statuses[status];scenario.subtype=subtype2;Compare();if((status==2||status==3)?(observed.result!=0||observed.count!=1):(observed.result!=1||Count(2)!=(subtype2==4?0:6)))Fail(96);}
 for(int bindings=-1;bindings<=3;++bindings)for(int alias=0;alias<2;++alias)for(int queued=0;queued<=128;queued+=32){Base();scenario.bindCount=bindings;scenario.aliases=alias;scenario.queueCount=queued;Compare();}
 for(int path=0;path<3;++path){Base();if(path==1)scenario.subtype=4;if(path==2){scenario.buttonPattern=0;scenario.previous=0;}Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<11;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=5;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 7;}
 printf("JoystickImmediate: %d complete axes, bindings, buttons, queues, POV, FP and fault scenarios match\n",checks);return 0;
}
