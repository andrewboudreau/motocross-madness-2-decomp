// Private behavioral harness. Only the five initialized packet bytes are read
// until Send deliberately initializes all twelve bytes. No production fallback.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
extern "C" long __cdecl _ftol();
class TrackGame;
class UnknownBikeRaceRacer;
class TrackGameViewOwner {public:void PickFollowedRacer();};
class BikeRace {public:UnknownBikeRaceRacer* NextBike(int*);};
class ActualBikeRace {public:UnknownBikeRaceRacer* NextBike(int*);};
class NetworkInterface {public:int Send(int,void*,int,int,int);};
class VCRInterface {public:int QueueRecord(int,int,const void*,unsigned);};
static BYTE referenceSpace[0x800000]={1};
struct World {
 BYTE owner[0x140],races[2][0x400],games[2][0x3500],networks[2][0x80];
 BYTE racers[8][0x1200],recorders[2][0x80];DWORD slots[2][12],ai[2][12];
};
static World world;TrackGame* g_TrackGame;
struct Scenario {int seed,players,aiCount,mode,blocked,host,network,save,playing,recorder,flags[8],random,count,holes,aliases,nullAi,throwAt,mutateAt,mutation,invalid,sendResult,queueResult,writePacket,round;};
static Scenario scenario;
struct State {World world;int game,fp;};
struct Event {int kind,who,args[6],cursorAfter,result;BYTE payload[12];State state;};
struct Observation {int completed,exception,faultKind,faultAddress,count,fp;Event events[32];State state;};
static Observation observed,expected;static int checks,retailActive;static const void* packet;
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant %d case %d reference %d events %d\n",code,checks,retailActive,observed.count);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.owner)return 1;for(int i=0;i<2;++i){if(p==world.races[i])return 10+i;if(p==world.games[i])return 20+i;if(p==world.networks[i])return 30+i;if(p==world.recorders[i])return 40+i;if(p==world.slots[i])return 50+i;if(p==world.ai[i])return 60+i;}for(int j=0;j<8;++j)if(p==world.racers[j])return 100+j;return(int)p;}
static void FromReference(){if(retailActive)g_TrackGame=*(TrackGame**)0x56e26c;}
static void ToReference(){if(retailActive)*(TrackGame**)0x56e26c=g_TrackGame;}
static void Snapshot(State& state){FromReference();state.world=world;state.game=Identity(g_TrackGame);state.fp=_statusfp();}
static Event& Begin(int kind,const void* who=0,int a=0,int b=0,int c=0,int d=0,int e=0){FromReference();if(observed.count>=32)Fail(81);Event& event=observed.events[observed.count++];event.kind=kind;event.who=Identity(who);event.args[0]=a;event.args[1]=b;event.args[2]=c;event.args[3]=d;event.args[4]=e;Snapshot(event.state);return event;}
static void Boundary(){
 if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:At(world.owner,0x34)=(DWORD)world.races[1];break;
 case 1:g_TrackGame=(TrackGame*)world.games[1];break;
 case 2:At(world.owner,0xa8)=(DWORD)world.racers[6];At(world.racers[6],0x11bc)=0x76543210;break;
 case 3:At(world.races[0],0x3c)=(DWORD)world.slots[1];At(world.races[0],0x158)=1;break;
 case 4:At(world.games[0],0x3334)=0;At(world.games[0],0x3428)=1;break;
 case 5:At(world.races[0],0x1a0)=(DWORD)world.recorders[1];At(world.games[0],0x3334)=1;At(world.games[0],0x3428)=0;break;
 case 6:{for(int i=0;i<8;++i)At(world.racers[i],0x4a0)=1;break;}
 case 7:At(world.games[0],0x3430)=1;break;
 case 8:At(world.networks[0],0xc)=0xfedcba98;At(world.games[0],8)=(DWORD)world.networks[1];break;
 case 9:At(world.owner,0xa8)=0;break;
 case 10:At(world.races[0],0x3c)=0;break;
 case 11:At(world.owner,0x34)=0;break;
 }}ToReference();if(scenario.throwAt==observed.count)throw 0x781;
}
UnknownBikeRaceRacer* BikeRace::NextBike(int* cursor){Event& e=Begin(1,this,*cursor);Boundary();UnknownBikeRaceRacer* result=((ActualBikeRace*)this)->NextBike(cursor);e.cursorAfter=*cursor;e.result=Identity(result);return result;}
extern "C" int __cdecl rand(){Begin(2);Boundary();return scenario.random;}
int NetworkInterface::Send(int kind,void* data,int size,int player,int flags){Event& e=Begin(3,this,kind,size,player,flags);if(size!=12)Fail(82);packet=data;e.payload[4]=((BYTE*)data)[4];memcpy(e.payload+8,(BYTE*)data+8,4);
 if(scenario.writePacket)for(int i=0;i<12;++i)((BYTE*)data)[i]=(BYTE)(0x31+i*17);
 Boundary();return scenario.sendResult;
}
int VCRInterface::QueueRecord(int kind,int player,const void* data,unsigned count){Event& e=Begin(4,this,kind,player,count,data==packet);if(data!=packet)Fail(83);
 if(scenario.writePacket)memcpy(e.payload,data,12);else{e.payload[4]=((BYTE*)data)[4];memcpy(e.payload+8,(BYTE*)data+8,4);}Boundary();return scenario.queueResult;
}
static DWORD Replacement(DWORD va){switch(va){case 0x4204e0:return Address(&BikeRace::NextBike);case 0x534c1b:return(DWORD)rand;case 0x534a88:return(DWORD)_ftol;case 0x4ac830:return Address(&NetworkInterface::Send);case 0x4e8720:return Address(&VCRInterface::QueueRecord);}Fail(84);return 0;}
static int MapReference(const char* path,const char* patches){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int count=0;while(fread(entry,sizeof(entry),1,f)==1){BYTE* site=(BYTE*)entry[0];if(entry[0]<0x4a9d20||entry[0]+5>0x4a9e74||*site!=0xe8||entry[2]!=0xe8||(DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1])return 0;*(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);++count;}fclose(f);return count==6&&FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=code;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{void(TrackGameViewOwner::*method)();DWORD address;}call;call.method=&TrackGameViewOwner::PickFollowedRacer;if(retailActive)call.address=0x4a9d20;__try{(((TrackGameViewOwner*)world.owner)->*call.method)();observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));memset(&world,0x52+scenario.seed%37,sizeof(world));packet=0;
 for(int n=0;n<2;++n){At(world.games[n],8)=scenario.network?(DWORD)world.networks[n]:0;At(world.networks[n],0x10)=scenario.host;At(world.networks[n],0xc)=0x1234+n*9;At(world.games[n],0x18)=scenario.players;At(world.games[n],0x2d70)=scenario.mode;At(world.games[n],0x2d94)=scenario.aiCount;At(world.games[n],0x2d98)=scenario.aiCount;At(world.games[n],0x3334)=scenario.save;At(world.games[n],0x3428)=scenario.playing;At(world.games[n],0x3430)=scenario.blocked;
  At(world.races[n],0x38)=(DWORD)world.racers[n?7:0];At(world.races[n],0x3c)=(DWORD)world.slots[n];At(world.races[n],0x40)=scenario.nullAi?0:(DWORD)world.ai[n];At(world.races[n],0x158)=scenario.count;At(world.races[n],0x1a0)=scenario.recorder?(DWORD)world.recorders[n]:0;
  for(int k=0;k<12;++k){int index=(k+n*3)%8;if(scenario.aliases)index%=2;world.slots[n][k]=scenario.holes&&k==scenario.holes-1?0:(DWORD)world.racers[index];world.ai[n][k]=scenario.holes&&k==scenario.holes-1?0:(DWORD)world.racers[(index+1)%8];}}
 for(int r=0;r<8;++r){At(world.racers[r],0x4a0)=scenario.flags[r];At(world.racers[r],0x11bc)=1000+r*137;}
 At(world.owner,0x34)=(DWORD)world.races[0];At(world.owner,0xa8)=(DWORD)world.racers[7];g_TrackGame=(TrackGame*)world.games[0];
 switch(scenario.invalid){case 1:g_TrackGame=0;break;case 2:At(world.owner,0x34)=0;break;case 3:At(world.races[0],0x38)=0;break;case 4:At(world.races[0],0x3c)=0;break;case 5:At(world.games[0],8)=1;break;}
 ToReference();_fpreset();_controlfp(scenario.round,_MCW_RC);try{Invoke();}catch(int code){observed.exception=code;}observed.fp=_statusfp();Snapshot(observed.state);_fpreset();
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch %d offset %d %02x/%02x players%d ai%d blocked%d count%d flags%d,%d,%d random%d mutation%d/%d throw%d invalid%d events%d/%d fp%x/%x\n",checks,offset,a[offset],b[offset],scenario.players,scenario.aiCount,scenario.blocked,scenario.count,scenario.flags[0],scenario.flags[1],scenario.flags[2],scenario.random,scenario.mutateAt,scenario.mutation,scenario.throwAt,scenario.invalid,expected.count,observed.count,expected.fp,observed.fp);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.players=3;scenario.host=scenario.network=scenario.save=scenario.recorder=1;scenario.count=3;scenario.random=16384;scenario.sendResult=-1;scenario.queueResult=-7;}
static int Count(int kind){int count=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++count;return count;}
static Event& Find(int kind){for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)return observed.events[i];Fail(85);return observed.events[0];}
static void SelectionOracle(int selected,int randomCalls,int replay){if(!observed.completed||observed.exception||Identity((void*)At(world.owner,0xa8))!=100+selected||Count(2)!=randomCalls||Count(3)!=1||Count(4)!=replay)Fail(90);Event& send=Find(3);if(send.who!=30||send.args[0]!=0x87||send.args[1]!=12||send.args[2]!=0x1234||send.args[3]||send.payload[4]||*(int*)(send.payload+8)!=1000+selected*137)Fail(91);if(replay){Event& record=Find(4);if(record.who!=40||record.args[0]!=0x87||record.args[1]!=1000+selected*137||record.args[2]!=1||record.args[3]!=1)Fail(92);if(scenario.writePacket){for(int i=0;i<12;++i)if(record.payload[i]!=(BYTE)(0x31+i*17))Fail(93);}else if(memcmp(send.payload,record.payload,12))Fail(94);}}
int main(int argc,char** argv){if(argc!=3||!MapReference(argv[1],argv[2]))return 3;
 Base();Compare();SelectionOracle(1,1,1);Base();scenario.writePacket=1;Compare();SelectionOracle(1,1,1);
 Base();scenario.flags[0]=scenario.flags[1]=scenario.flags[2]=1;Compare();SelectionOracle(0,0,1);
 Base();scenario.flags[0]=scenario.flags[1]=1;Compare();SelectionOracle(2,0,1);
 Base();scenario.flags[1]=1;scenario.random=16384;Compare();SelectionOracle(2,1,1);
 Base();scenario.flags[2]=1;scenario.random=32767;Compare();SelectionOracle(0,1,1);
 Base();scenario.flags[1]=2;Compare();SelectionOracle(1,1,1);
 Base();scenario.flags[1]=-1;Compare();SelectionOracle(1,1,1);
 // Four iterator calls precede rand in the three-player path. Retail keeps
 // the view/count across rand but reloads the saved view's slots afterward.
 Base();scenario.mutateAt=5;scenario.mutation=0;Compare();if(Identity((void*)At(world.owner,0xa8))!=101||Find(4).who!=41)return 8;
 Base();scenario.mutateAt=5;scenario.mutation=3;Compare();if(Identity((void*)At(world.owner,0xa8))!=104||*(int*)(Find(3).payload+8)!=1548)return 9;
 Base();scenario.mutateAt=5;scenario.mutation=6;Compare();if(Identity((void*)At(world.owner,0xa8))!=101)return 10;
 Base();scenario.blocked=1;Compare();SelectionOracle(0,0,1);
 Base();scenario.host=0;Compare();if(!observed.completed||observed.count||Identity((void*)At(world.owner,0xa8))!=107)return 4;
 Base();Compare();int sendBoundary=0;for(int e=0;e<observed.count;++e)if(observed.events[e].kind==3)sendBoundary=e+1;
 Base();scenario.mutateAt=sendBoundary;scenario.mutation=2;scenario.writePacket=1;Compare();if(Find(4).args[1]!=0x76543210||*(int*)(Find(3).payload+8)!=1137||Identity((void*)At(world.owner,0xa8))!=106)return 5;
 Base();scenario.throwAt=sendBoundary;Compare();if(observed.exception!=0x781||Count(4)||Identity((void*)At(world.owner,0xa8))!=101)return 6;
 const int flagValues[]={0,1,2,-1};const int randomValues[]={0,1,10922,10923,16384,21845,32766,32767};const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 for(int a=0;a<4;++a)for(int b=0;b<4;++b)for(int c=0;c<4;++c)for(int random=0;random<8;++random)for(int round=0;round<4;++round){Base();scenario.flags[0]=flagValues[a];scenario.flags[1]=flagValues[b];scenario.flags[2]=flagValues[c];scenario.random=randomValues[random];scenario.round=rounds[round];scenario.seed=checks;Compare();}
 for(int players=0;players<=4;++players)for(int ai=0;ai<=3;++ai)for(int mode=0;mode<2;++mode)for(int blocked=0;blocked<2;++blocked)for(int holes=0;holes<4;++holes)for(int aliases=0;aliases<2;++aliases)for(int nullAi=0;nullAi<2;++nullAi){Base();scenario.players=players;scenario.aiCount=ai;scenario.mode=mode?4:0;scenario.blocked=blocked;scenario.holes=holes;scenario.aliases=aliases;scenario.nullAi=nullAi;scenario.count=players>1?players+ai:(mode?2:ai+1);scenario.flags[1]=1;scenario.flags[2]=2;scenario.flags[3]=-1;scenario.seed=checks;Compare();}
 const int gates[]={0,1,2,-1};for(int host=0;host<4;++host)for(int net=0;net<2;++net)for(int save=0;save<4;++save)for(int playing=0;playing<4;++playing)for(int recorder=0;recorder<2;++recorder)for(int payload=0;payload<2;++payload){Base();scenario.host=gates[host];scenario.network=net;scenario.save=gates[save];scenario.playing=gates[playing];scenario.recorder=recorder;scenario.writePacket=payload;Compare();if(!observed.completed||observed.exception||Count(3)!=(net&&host?1:0)||Count(4)!=(net&&host&&save&&!playing&&recorder?1:0))Fail(95);}
 for(int path=0;path<5;++path){Base();if(path==1)scenario.flags[0]=scenario.flags[1]=1;if(path==2)scenario.flags[0]=scenario.flags[1]=scenario.flags[2]=1;if(path==3)scenario.save=0;if(path==4)scenario.players=1;Compare();int boundaries=observed.count;for(int at=1;at<=boundaries;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<12;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;scenario.writePacket=mutation&1;Compare();}scenario.mutateAt=0;}}
 const int results[]={0,1,-1,(-2147483647-1),2147483647};for(int send=0;send<5;++send)for(int queue=0;queue<5;++queue){Base();scenario.sendResult=results[send];scenario.queueResult=results[queue];Compare();SelectionOracle(1,1,1);}
 for(int invalid=1;invalid<=5;++invalid){Base();scenario.invalid=invalid;if(invalid==3)scenario.blocked=1;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 7;}
 Base();scenario.count=0;Compare();Base();scenario.count=-1;scenario.random=0;Compare();
 printf("FollowedRacerPicker: %d complete selection, iterator, packet, callback, FP and fault scenarios match\n",checks);return 0;
}
