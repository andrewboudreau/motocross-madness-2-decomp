// Private source/retail comparison; COM output is initialized only on success.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
class TrackGame;
class PCJoystickDevice{public:int ReadBufferedState(int);int ButtonIndexFromOffset(int);int ApplyAxisDeadZone(int,int);};
class ActualPCJoystickDevice{public:int ButtonIndexFromOffset(int);int ApplyAxisDeadZone(int,int);};
void ActualReportError(long,const char*,int);
extern "C" void _ftol();
class JoystickDevice{public:void FeedAxis(int,float,float);};
class ActualJoystickDevice{public:void FeedAxis(int,float,float);};
class ControlInterface{public:void QueueEvent(int,int,int,int);};
class ActualControlInterface{public:void QueueEvent(int,int,int,int);};
static BYTE referenceSpace[0x800000]={1};
struct Record{DWORD offset,data,timeStamp,sequence;};
struct World{BYTE joystick[0x5ec],games[2][0x3500],controls[2][0xcd0],bindings[6][3][0x3c];DWORD lists[6][3],device[2][4];Record records[2][16];};
static World world;TrackGame* g_TrackGame;
struct Scenario{int seed,device,status[2],counts[2],acquire,subtype,flip,filter,current[6],previous[6],ranges[6],scales[6],bindCount,aliases,queueCount,throwAt,mutateAt,mutation,invalid,round,argument;Record records[2][16];};
static Scenario scenario;static const char* candidateFile;
struct State{World world;int game,fp,count,valid;Record records[16];};
struct Event{int kind,who,args[5];Record records[16];State before,after;};
struct Observation{int result,completed,exception,faultKind,faultAddress,count,fp;Event events[80];State state;};
static Observation observed,expected;static int checks,retailActive;static DWORD deviceTable[26];static long* countPointer;static Record* outputPointer;static int validRecords,dataCalls;
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d case%d retail%d count%d\n",code,checks,retailActive,observed.count);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.joystick)return 1;if(p==world.device[0])return 2;if(p==world.device[1])return 3;for(int n=0;n<2;++n){if(p==world.games[n])return 10+n;if(p==world.controls[n])return 20+n;}return(int)p;}
static void FromReference(){if(retailActive)g_TrackGame=*(TrackGame**)0x56e26c;}
static void ToReference(){if(retailActive)*(TrackGame**)0x56e26c=g_TrackGame;}
static void Snapshot(State& s){FromReference();s.world=world;s.game=Identity(g_TrackGame);s.fp=_statusfp();s.count=countPointer?*countPointer:0;s.valid=validRecords;if(validRecords)memcpy(s.records,outputPointer,validRecords*sizeof(Record));}
static Event& Begin(int kind,const void* who,int a=0,int b=0,int c=0,int d=0){FromReference();if(observed.count>=80)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;Snapshot(e.before);return e;}
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
 case 11:if(validRecords){outputPointer[validRecords-1].data^=0x87654321;outputPointer[validRecords-1].timeStamp^=0xdeadbeef;}break;
 case 12:if(countPointer)*countPointer=0;break;
 case 13:At(world.joystick,0x25c)=(DWORD)world.device[1];break;
 case 14:world.joystick[0x5d5]^=1;break;
 case 15:At(world.joystick,0x5a4+4)=0x47000000;break;
 case 16:At(world.joystick,0x5bc)=0;At(world.joystick,0x5bc+4)=0xbf800000;break;
 }}ToReference();if(scenario.throwAt==observed.count)throw 0x730;}
void JoystickDevice::FeedAxis(int axis,float value,float range){Event& e=Begin(2,this,axis,*(int*)&value,*(int*)&range);Boundary();((ActualJoystickDevice*)this)->FeedAxis(axis,value,range);Snapshot(e.after);}
void ControlInterface::QueueEvent(int control,int kind,int pressed,int device){Event& e=Begin(3,this,control,kind,pressed,device);Boundary();((ActualControlInterface*)this)->QueueEvent(control,kind,pressed,device);Snapshot(e.after);}
int PCJoystickDevice::ButtonIndexFromOffset(int offset){Event& e=Begin(4,this,offset);Boundary();int result=((ActualPCJoystickDevice*)this)->ButtonIndexFromOffset(offset);e.args[4]=result;Snapshot(e.after);return result;}
int PCJoystickDevice::ApplyAxisDeadZone(int axis,int value){Event& e=Begin(5,this,axis,value);Boundary();int result=((ActualPCJoystickDevice*)this)->ApplyAxisDeadZone(axis,value);e.args[4]=result;Snapshot(e.after);return result;}
void UnknownReportError(long result,const char* file,int line){if(line!=317||!file)Fail(84);if(strcmp(file,retailActive?"D:\\aardvark\\VC\\krusty2\\PCInputDeviceType.cpp":candidateFile))Fail(85);Event& e=Begin(6,0,result,line);Boundary();ActualReportError(result,file,line);Snapshot(e.after);}
static long __stdcall Acquire(void* who){Event& e=Begin(7,who);Boundary();Snapshot(e.after);return scenario.acquire;}
static long __stdcall GetDeviceData(void* who,unsigned long size,Record* data,long* count,unsigned long flags){if(dataCalls>=2||size!=16||flags||!data||!count)Fail(82);countPointer=count;outputPointer=data;Event& e=Begin(1,who,size,*count,flags,dataCalls);int call=dataCalls++;int returned=scenario.counts[call];if(scenario.status[call]>=0){if(returned>16)Fail(86);if(returned>0&&returned>e.args[1])returned=e.args[1]>0?e.args[1]:0;validRecords=returned>0?returned:0;if(validRecords){memcpy(data,world.records[call],validRecords*sizeof(Record));memcpy(e.records,data,validRecords*sizeof(Record));}}*count=returned;Boundary();Snapshot(e.after);return scenario.status[call];}
static DWORD Replacement(DWORD va){switch(va){case 0x489c00:return Address(&JoystickDevice::FeedAxis);case 0x43cea0:return Address(&ControlInterface::QueueEvent);case 0x4c2ef0:return Address(&PCJoystickDevice::ButtonIndexFromOffset);case 0x4c3090:return Address(&PCJoystickDevice::ApplyAxisDeadZone);case 0x4bf6a0:return(DWORD)UnknownReportError;case 0x534a88:return(DWORD)_ftol;}Fail(83);return 0;}
static int MapReference(const char* path,const char* patches){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int count=0;while(fread(entry,sizeof(entry),1,f)==1){BYTE* site=(BYTE*)entry[0];if(entry[0]<0x4c3100||entry[0]+5>0x4c36f5||*site!=0xe8||entry[2]!=0xe8||(DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1])return 0;*(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);++count;}fclose(f);return count==19&&FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{int(PCJoystickDevice::*method)(int);DWORD address;}call;call.method=&PCJoystickDevice::ReadBufferedState;if(retailActive)call.address=0x4c3100;__try{observed.result=(((PCJoystickDevice*)world.joystick)->*call.method)(scenario.argument);observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;countPointer=0;outputPointer=0;validRecords=0;dataCalls=0;memset(&world,0x53+scenario.seed%39,sizeof(world));world.device[0][0]=world.device[1][0]=(DWORD)deviceTable;At(world.joystick,0x25c)=scenario.device?(DWORD)world.device[0]:0;At(world.joystick,0x260)=17;world.joystick[0x3d]=(BYTE)scenario.subtype;world.joystick[0x574]=(BYTE)((world.joystick[0x574]&~1)|(scenario.flip&1));world.joystick[0x5d5]=(BYTE)scenario.filter;
 for(int a=0;a<6;++a){At(world.joystick,0x4e4+a*4)=scenario.current[a];At(world.joystick,0x5a4+a*4)=scenario.previous[a];At(world.joystick,0x58c+a*4)=scenario.ranges[a];At(world.joystick,0x5bc+a*4)=scenario.scales[a];At(world.joystick,0x4fc+a*20)=scenario.bindCount;At(world.joystick,0x500+a*20)=(DWORD)world.lists[a];
  for(int b=0;b<3;++b){world.lists[a][b]=(DWORD)world.bindings[scenario.aliases?0:a][scenario.aliases?0:b];At(world.bindings[a][b],0x28)=0x3f000000;At(world.bindings[a][b],0x2c)=0;At(world.bindings[a][b],0x30)=0x3f800000;At(world.bindings[a][b],0x34)=0x3f0ccccd;At(world.bindings[a][b],0x38)=0x3ee66666;}}
 memcpy(world.records,scenario.records,sizeof(world.records));for(int n=0;n<2;++n){At(world.games[n],0x14)=(DWORD)world.controls[n];At(world.controls[n],0x38)=scenario.queueCount;}
 g_TrackGame=(TrackGame*)world.games[0];switch(scenario.invalid){case 1:g_TrackGame=0;break;case 2:At(world.games[0],0x14)=0;break;case 3:At(world.joystick,0x25c)=1;break;case 4:At(world.joystick,0x500)=0;break;case 5:world.lists[0][0]=0;break;}
 ToReference();_fpreset();_controlfp(scenario.round,_MCW_RC);try{Invoke();}catch(int code){observed.exception=code;}observed.fp=_statusfp();countPointer=0;outputPointer=0;validRecords=0;Snapshot(observed.state);_fpreset();}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d offset%d %02x/%02x subtype%d flip%d filter%d status%x/%x count%d/%d rounding%x mutation%d/%d throw%d invalid%d events%d/%d fp%x/%x\n",checks,offset,a[offset],b[offset],scenario.subtype,scenario.flip,scenario.filter,scenario.status[0],scenario.status[1],scenario.counts[0],scenario.counts[1],scenario.round,scenario.mutateAt,scenario.mutation,scenario.throwAt,scenario.invalid,expected.count,observed.count,expected.fp,observed.fp);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.device=1;scenario.bindCount=2;scenario.counts[0]=scenario.counts[1]=16;scenario.argument=12345;for(int a=0;a<6;++a){scenario.current[a]=scenario.previous[a]=0x47000000;scenario.ranges[a]=0x47800000;scenario.scales[a]=0x3f800000;}for(int n=0;n<2;++n)for(int i=0;i<16;++i){Record& r=scenario.records[n][i];r.offset=i<6?i*4:(i<10?0x20+(i-6)*4:0x30+(i-10)/2);r.data=i<6?i*8192:(i<10?9000*(i-6):(i&1?0x80:0));r.timeStamp=0xfffffff0+i;r.sequence=0x12345678+i;}}
static int Count(int kind){int count=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++count;return count;}
static void Oracle(){if(!observed.completed||observed.exception||observed.result!=1||Count(1)!=1||Count(2)!=6||Count(3)!=6||Count(4)!=16||Count(5)!=2||At(world.controls[0],0x38)!=6)Fail(90);for(int a=0;a<6;++a){float raw=(float)(a*8192),feed=a==2?65536-raw:raw,mapped=feed/65536;if(At(world.joystick,0x4e4+a*4)!=*(DWORD*)&raw||At(world.joystick,0x5a4+a*4)!=*(DWORD*)&raw)Fail(91);for(int b=0;b<2;++b)if(At(world.bindings[a][b],0x24)!=*(DWORD*)&mapped)Fail(92);}for(int p=0;p<4;++p)if(At(world.joystick,0x5d8+p*4)!=9000*p)Fail(93);for(int k=0;k<6;++k){BYTE* event=world.controls[0]+0x3c+k*20;if(At(event,0)!=k/2||At(event,4)!=2||At(event,8)!=17||At(event,12)!=(k&1)||At(event,16)!=0x3f)Fail(94);}for(int button=0;button<3;++button){BYTE* now=world.joystick+0x264+button*20;BYTE* before=observed.events[0].before.world.joystick+0x264+button*20;if(At(now,0)!=1||At(now,4)!=0xfffffffa+button*2||At(now,8)!=0xfffffffb+button*2||At(now,12)!=At(before,4)||At(now,16)!=At(before,8))Fail(95);}}
int main(int argc,char** argv){if(argc!=4)return 3;candidateFile=argv[3];if(!MapReference(argv[1],argv[2])||memcmp((void*)_ftol,(void*)0x534a88,39))return 3;deviceTable[7]=(DWORD)Acquire;deviceTable[10]=(DWORD)GetDeviceData;
 Base();Compare();Oracle();Base();scenario.subtype=4;Compare();if(Count(2)||Count(5)||Count(3)!=6||memcmp(world.joystick+0x5a4,observed.events[0].before.world.joystick+0x5a4,24))Fail(96);
 Base();scenario.device=0;scenario.invalid=1;Compare();if(observed.result||observed.count||observed.exception)Fail(97);
 Base();scenario.subtype=4;scenario.counts[0]=1;scenario.records[0][0].data=0xffffffff;Compare();if(At(world.joystick,0x4e4)!=0x4f800000||Count(2)||Count(5))Fail(105);
 Base();for(int duplicate=0;duplicate<16;++duplicate){scenario.records[0][duplicate].offset=0x35;scenario.records[0][duplicate].data=0x80;scenario.records[0][duplicate].timeStamp=duplicate;}Compare();if(Count(3)!=16||At(world.controls[0],0x38)!=16||At(world.joystick,0x264+5*20+8)!=15||At(world.joystick,0x264+5*20+16)!=14)Fail(106);for(int eventIndex=0;eventIndex<16;++eventIndex){BYTE* event=world.controls[0]+0x3c+eventIndex*20;if(At(event,0)!=5||At(event,12)!=1)Fail(107);}
 for(int edge=0;edge<2;++edge){Base();scenario.filter=1;scenario.counts[0]=1;scenario.records[0][0].offset=12;scenario.records[0][0].data=42768+edge;Compare();float filtered=edge?32768.0f:42768.0f;float raw=(float)(42768+edge);if(At(world.joystick,0x4e4+12)!=*(DWORD*)&filtered||At(world.joystick,0x5a4+12)!=*(DWORD*)&raw)Fail(108);}
 const int statuses[]={0,1,-1,0x8007001e,0x8007000c,0x80000000,0x7fffffff};
 for(int status=0;status<7;++status)for(int acquisition=0;acquisition<3;++acquisition)for(int retry=0;retry<7;++retry){Base();scenario.status[0]=statuses[status];scenario.acquire=acquisition==0?-1:acquisition-1;scenario.status[1]=statuses[retry];Compare();int lost=status==3||status==4;if(lost&&scenario.acquire<0){if(observed.result||Count(6)!=1||Count(7)!=1||dataCalls!=1)Fail(98);Event& e=observed.events[2];if(e.kind!=6||e.args[0]!=statuses[status]||e.args[1]!=317)Fail(99);}else if(lost){if(dataCalls!=2||Count(7)!=1||Count(6)||observed.result!=(statuses[retry]>=0))Fail(100);}else if(dataCalls!=1||Count(7)||Count(6)||observed.result!=(statuses[status]>=0))Fail(101);}
 for(int count=-2;count<=16;++count)for(int retryPath=0;retryPath<2;++retryPath){Base();scenario.counts[0]=scenario.counts[1]=count;if(retryPath)scenario.status[0]=0x8007001e;Compare();if(observed.result!=1||Count(4)!=(count>0?count:0))Fail(102);if(retryPath&&observed.events[2].args[1]!=count)Fail(103);}
 const int values[]={0,1,9999,10000,10001,32768,65535,65536,16777217,0x7fffffff,0x80000000,0xffffffff};const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 for(int axis=0;axis<6;++axis)for(int v=0;v<12;++v)for(int filter=0;filter<2;++filter)for(int subtype=0;subtype<2;++subtype)for(int round=0;round<4;++round){Base();scenario.counts[0]=1;scenario.records[0][0].offset=axis*4;scenario.records[0][0].data=values[v];scenario.filter=filter;scenario.subtype=subtype?4:0;scenario.round=rounds[round];Compare();}
 for(int offset=0;offset<256;++offset)for(int value=0;value<4;++value){Base();scenario.counts[0]=1;scenario.records[0][0].offset=offset;scenario.records[0][0].data=values[value?8+value:0];scenario.subtype=value==3?4:0;Compare();}
 const int deltas[]={-10001,-10000,-9999,9999,10000,10001};for(int a=0;a<6;++a)for(int delta=0;delta<6;++delta)for(int previous=0;previous<2;++previous)for(int flip=0;flip<2;++flip){Base();scenario.filter=1;scenario.flip=flip;scenario.counts[0]=1;scenario.records[0][0].offset=a*4;scenario.records[0][0].data=32768+deltas[delta];if(previous)scenario.previous[a]=0x47000080;Compare();}
 const int floats[]={0,0x3f800000,0xbf800000,0x47800000,0x4f000000,0xcf000000,0x7f800000,0xff800000,0x7fc01234};for(int fp=0;fp<9;++fp)for(int scale=0;scale<9;++scale)for(int round2=0;round2<4;++round2){Base();scenario.filter=1;scenario.flip=fp&1;scenario.round=rounds[round2];for(int aa=0;aa<6;++aa){scenario.current[aa]=floats[fp];scenario.previous[aa]=floats[(fp+aa)%9];scenario.scales[aa]=floats[scale];scenario.ranges[aa]=floats[(scale+aa)%9];}Compare();}
 const int invalidOffsets[]={-1,0x80000000,0x7fffffff,-49,-48};for(int oo=0;oo<5;++oo){Base();scenario.counts[0]=1;scenario.records[0][0].offset=invalidOffsets[oo];Compare();if(observed.result!=1||Count(3)||Count(2)||Count(5))Fail(109);}
 for(int binding=-1;binding<=3;++binding)for(int alias=0;alias<2;++alias)for(int queue=0;queue<=128;queue+=32){Base();scenario.bindCount=binding;scenario.aliases=alias;scenario.queueCount=queue;Compare();}
 for(int path=0;path<4;++path){Base();if(path==1)scenario.subtype=4;if(path>=2)scenario.status[0]=0x8007001e;if(path==3)scenario.acquire=-1;Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<17;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=5;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)Fail(104);}
 printf("JoystickBuffered: %d complete records, axes, timestamps, queues, retry, FP and fault scenarios match\n",checks);return 0;
}
