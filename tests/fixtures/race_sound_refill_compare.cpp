// Private complete-function differential. The three Sound methods execute real
// source; only COM and rand are scripted. No production retail dispatch.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "../../src/reconstructed/RaceSound.h"
#include "../../src/reconstructed/TrackGameGlobal.h"
static unsigned char referenceSpace[0x800000];
TrackGame* g_TrackGame;
enum { BUFFER=49152, GUARD=4096, EVENTS=8 };
struct World {
    DWORD before[4]; BYTE race[sizeof(RaceSound)]; DWORD middle[4];
    BYTE sound[2][sizeof(Sound)],audio[sizeof(PCSoundInterface)],game[64];
    DWORD com[2][4]; BYTE source[3][BUFFER],buffer[2][BUFFER],device[2][BUFFER]; DWORD after[4];
};
static World* worlds;
#define world worlds[0]
#define initial worlds[1]
struct Event { int kind,receiver,args[7],held[2]; World state; BYTE guard[GUARD]; };
struct Observation { DWORD exception,faultKind,faultAddress;int events,randoms,held[2],unlockedUnheld;unsigned control,status;
    Event log[EVENTS]; World state; BYTE guard[GUARD]; };
static Observation* observations;
#define observed observations[0]
#define expected observations[1]
struct Scenario { unsigned seed;int width,half,kind,avail,count,engine,cursor,randValue,status,alias,invalid;
    int mutateAt,action,throwAt,guardTarget,validBytes; };
static Scenario scenario;
static BYTE* guard;
static DWORD vtable[21];
static int checks;
static RaceSound* Race(){return (RaceSound*)world.race;}
static Sound* SoundAt(int n){return (Sound*)world.sound[n];}
static UnknownRaceSoundChannel* Channel(int n){return &Race()->engineChannels[n];}
static int Receiver(void* p){for(int n=0;n<2;++n)if(p==world.com[n])return n;ExitProcess(80);return -1;}
static void Boundary(int kind,int receiver,int a=0,int b=0,int c=0,int d=0,int e=0,int f=0,int g=0){
    if(observed.events>=EVENTS)ExitProcess(81);
    Event& event=observed.log[observed.events++];event.kind=kind;event.receiver=receiver;
    event.args[0]=a;event.args[1]=b;event.args[2]=c;event.args[3]=d;event.args[4]=e;event.args[5]=f;event.args[6]=g;
    memcpy(event.held,observed.held,sizeof(event.held));event.state=world;memcpy(event.guard,guard,GUARD);
    if(scenario.mutateAt==observed.events){
        UnknownRaceSoundChannel* channel=Race()->streamChannel;
        switch(scenario.action){
        case 1:Race()->streamChannel=Channel(1);break;
        case 2:Race()->bytesPerSample=3-Race()->bytesPerSample;break;
        case 3:channel->nextSampleKind=channel->nextSampleKind==3?4:3;break;
        case 4:channel->sampleOffset+=3;break;
        case 5:channel->channelSound=SoundAt(1);break;
        case 6:channel->engine=(channel->engine+1)%3;Race()->idleCount[channel->engine]=1;break;
        case 7:channel->channelSound->dsBuffer=0;break;
        case 8:channel->sampleBuffer=world.buffer[1];break;
        case 9:channel->streamSample=world.source[2];break;
        }
    }
    if(scenario.throwAt==observed.events)RaiseException(0xe0420001,0,0,0);
}
static long __stdcall GetPosition(void* self,unsigned long* play,unsigned long* write){
    if(!play||!write||play==write)ExitProcess(82);int n=Receiver(self);Boundary(1,n);
    *play=(unsigned)(scenario.seed*7);*write=(unsigned)scenario.cursor;return scenario.status;
}
static long __stdcall Lock(void* self,unsigned long offset,unsigned long bytes,void** first,unsigned long* firstBytes,
                           void** second,unsigned long* secondBytes,unsigned long flags){
    int n=Receiver(self);if(!first||!firstBytes||!second||!secondBytes)ExitProcess(83);
    Boundary(2,n,offset,bytes,flags);if(scenario.status>=0)++observed.held[n];
    *first=world.device[n];*second=world.device[n]+40000;*firstBytes=44100;*secondBytes=37;
    if(scenario.alias==1)*first=Race()->streamChannel->sampleBuffer;
    if(scenario.alias==2)*first=(BYTE*)Race()->streamChannel->sampleBuffer+4;
    if(scenario.alias==3)*first=world.source[1];
    if(scenario.guardTarget==3)*first=guard+GUARD-scenario.validBytes;
    if(scenario.invalid==6)*first=0;
    return scenario.status;
}
static long __stdcall Unlock(void* self,void* first,unsigned long firstBytes,void* second,unsigned long secondBytes){
    int n=Receiver(self);Boundary(3,n,(int)first,firstBytes,(int)second,secondBytes);
    if(observed.held[n])--observed.held[n];else ++observed.unlockedUnheld;
    return scenario.status;
}
static int __cdecl Random(){Boundary(4,-1);++observed.randoms;return scenario.randValue;}
template<class T>static DWORD Address(T m){union{T method;DWORD value;}u;u.method=m;return u.value;}
static DWORD Dependency(DWORD target){switch(target){
 case 0x4bcd40:return Address(&Sound::GetBufferPosition);case 0x4bd020:return Address(&Sound::LockBuffer);
 case 0x4bd080:return Address(&Sound::UnlockBuffer);case 0x534c1b:return (DWORD)Random;default:ExitProcess(84);return 0;}}
static int MapReference(const char* path,const char* patchPath){
    DWORD old;BYTE* mapped=(BYTE*)0x400000;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)){printf("map fail %d\n",__LINE__);return 0;}
    FILE* file=fopen(path,"rb");if(!file){printf("map fail %d\n",__LINE__);return 0;}fseek(file,0,SEEK_END);long length=ftell(file);rewind(file);
    if(length<(long)sizeof(IMAGE_DOS_HEADER)){printf("map fail %d\n",__LINE__);return 0;}BYTE* raw=(BYTE*)malloc(length);
    if(!raw||fread(raw,1,length,file)!=(size_t)length){printf("map fail %d\n",__LINE__);return 0;}fclose(file);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||(DWORD)dos->e_lfanew>(DWORD)length-sizeof(IMAGE_NT_HEADERS)){printf("map fail %d\n",__LINE__);return 0;}
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||
       pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||
       pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)length||
       pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage){printf("map fail %d\n",__LINE__);return 0;}
    IMAGE_SECTION_HEADER* sections=IMAGE_FIRST_SECTION(pe);
    if((BYTE*)(sections+pe->FileHeader.NumberOfSections)>raw+length){printf("map fail %d\n",__LINE__);return 0;}
    memset(mapped,0,pe->OptionalHeader.SizeOfImage);memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    for(int n=0;n<pe->FileHeader.NumberOfSections;++n){
        IMAGE_SECTION_HEADER& s=sections[n];
        if(s.PointerToRawData>(DWORD)length||s.SizeOfRawData>(DWORD)length-s.PointerToRawData||
           s.VirtualAddress>pe->OptionalHeader.SizeOfImage||s.SizeOfRawData>pe->OptionalHeader.SizeOfImage-s.VirtualAddress){printf("map fail %d\n",__LINE__);return 0;}
        memcpy(mapped+s.VirtualAddress,raw+s.PointerToRawData,s.SizeOfRawData);
    }free(raw);
    BYTE* candidate=(BYTE*)Address(&RaceSound::RefillStream);
    if(!VirtualProtect(candidate,2038,PAGE_EXECUTE_READWRITE,&old)){printf("map fail %d\n",__LINE__);return 0;}
    file=fopen(patchPath,"rb");if(!file){printf("map fail %d\n",__LINE__);return 0;}DWORD record[3];int count=0;
    while(fread(record,sizeof(record),1,file)==1){++count;
        if(record[0]==0){BYTE* site=(BYTE*)record[1];
            if(site<(BYTE*)0x4e4d10||site+5>(BYTE*)0x4e5500||*site!=0xe8||
               (DWORD)(site+5)+*(DWORD*)(site+1)!=record[2]){printf("map fail %d\n",__LINE__);return 0;}
            *(DWORD*)(site+1)=Dependency(record[2])-(DWORD)(site+5);
        }else if(record[0]==1){
            if(record[1]<1||record[1]+4>2038||candidate[record[1]-1]!=0xe8||
               (DWORD)(candidate+record[1]+4)+*(DWORD*)(candidate+record[1])!=(DWORD)rand){printf("map fail %d\n",__LINE__);return 0;}
            *(DWORD*)(candidate+record[1])=(DWORD)Random-(DWORD)(candidate+record[1]+4);
        }else {printf("map fail %d\n",__LINE__);return 0;}
    }fclose(file);return count==17&&FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){
    DWORD code=e->ExceptionRecord->ExceptionCode;
    if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=EXCEPTION_INT_DIVIDE_BY_ZERO&&code!=0xe0420001)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Invoke(int retail){union{void(RaceSound::*method)();DWORD address;}u;u.method=&RaceSound::RefillStream;if(retail)u.address=0x4e4d10;
    __try{((scenario.invalid==3?(RaceSound*)0:Race())->*u.method)();}__except(Filter(GetExceptionInformation())){}
}
static void Prepare(){
    unsigned state=scenario.seed+1;for(unsigned i=0;i<sizeof(world);++i){state=state*1664525u+1013904223u;((BYTE*)&world)[i]=(BYTE)(state>>24);}
    for(i=0;i<GUARD;++i)guard[i]=(BYTE)(i*7+scenario.seed);
    g_TrackGame=(TrackGame*)world.game;*(SoundInterface**)(world.game+4)=(PCSoundInterface*)world.audio;
    ((PCSoundInterface*)world.audio)->audioStarted=1;
    for(int n=0;n<2;++n){world.com[n][0]=(DWORD)vtable;SoundAt(n)->dsBuffer=(UnknownSoundBuffer*)world.com[n];}
    Race()->bytesPerSample=scenario.width;Race()->streamChannel=Channel(0);
    for(n=0;n<3;++n){Race()->idleCount[n]=Race()->powerBandCount[n]=Race()->sputterCount[n]=scenario.count;
        for(int j=0;j<10;++j){Race()->idleSamples[n][j]=world.source[(n+j)%3];Race()->idleSampleBytes[n][j]=40000+j;
            Race()->powerBandSamples[n][j]=world.source[(n+j+1)%3];Race()->powerBandSampleBytes[n][j]=41000+j;
            Race()->sputterSamples[n][j]=world.source[(n+j+2)%3];Race()->sputterSampleBytes[n][j]=42000+j;}}
    int half=11025*scenario.width;int available[]={0,1,half-1,half,half+1,half*2,half/2};
    for(n=0;n<2;++n){UnknownRaceSoundChannel* c=Channel(n);c->channelSound=SoundAt(n);c->engine=scenario.engine;c->sampleKind=6;
        c->sampleOffset=17+scenario.seed%31;c->sampleBytes=c->sampleOffset+available[scenario.avail];
        c->fillHalf=scenario.half;c->nextSampleKind=scenario.kind;c->streamSample=world.source[n];c->sampleBuffer=world.buffer[n];}
    if(scenario.alias==4)Channel(0)->streamSample=world.buffer[0];
    if(scenario.alias==5)Channel(0)->streamSample=world.buffer[0]+4;
    if(scenario.alias==6)Channel(0)->sampleBuffer=world.source[0]+4;
    if(scenario.invalid==1)Channel(0)->channelSound=0;
    if(scenario.invalid==2)Race()->streamChannel=0;
    if(scenario.invalid==4)Channel(0)->streamSample=0;
    if(scenario.invalid==5)Channel(0)->sampleBuffer=0;
    if(scenario.invalid==7)SoundAt(0)->dsBuffer=0;
    if(scenario.guardTarget==1)Channel(0)->streamSample=guard+GUARD-scenario.validBytes-Channel(0)->sampleOffset;
    if(scenario.guardTarget==2)Channel(0)->sampleBuffer=guard+GUARD-scenario.validBytes;
    initial=world;
}
static void Run(int retail){memset(&observed,0,sizeof(observed));Prepare();_fpreset();Invoke(retail);
    observed.control=_controlfp(0,0);observed.status=_statusfp();observed.state=world;memcpy(observed.guard,guard,GUARD);
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){unsigned off=0;BYTE*a=(BYTE*)&expected,*b=(BYTE*)&observed;while(a[off]==b[off])++off;
        printf("Mismatch%d at%u %02x/%02x seed%u width%d half%d kind%d avail%d count%d cursor%d rand%d status%d alias%d invalid%d mutate%d/%d throw%d guard%d/%d events%d/%d exception%x/%x\n",checks,off,a[off],b[off],scenario.seed,scenario.width,scenario.half,scenario.kind,scenario.avail,scenario.count,scenario.cursor,scenario.randValue,scenario.status,scenario.alias,scenario.invalid,scenario.mutateAt,scenario.action,scenario.throwAt,scenario.guardTarget,scenario.validBytes,expected.events,observed.events,expected.exception,observed.exception);ExitProcess(2);}
}
static void Defaults(){memset(&scenario,0,sizeof(scenario));scenario.seed=7;scenario.width=2;scenario.half=1;scenario.avail=4;scenario.count=3;scenario.cursor=40000;scenario.randValue=32767;}
static void Oracle(){
    // Independent byte oracle for ordinary nonwrapping and wrapping reads.
    for(int which=0;which<4;++which){Defaults();scenario.half=which/2+1;scenario.cursor=scenario.half==1?40000:16000;scenario.avail=which&1?6:4;Compare();
        if(observed.exception||observed.events!=3||observed.randoms||observed.held[0]||observed.unlockedUnheld)ExitProcess(90);
        int half=22050,begin=scenario.half==1?0:half,offset=17+scenario.seed%31,available=(which&1)?half/2:half+1;
        int expectedOffset=available<half?half-available:offset+half;
        if(Channel(0)->sampleOffset!=expectedOffset||Channel(0)->fillHalf!=(scenario.half==1?2:1))ExitProcess(91);
        for(int i=0;i<BUFFER;++i){BYTE value=initial.buffer[0][i];if(i>=begin&&i<begin+half){int at=i-begin;value=at<available?initial.source[0][offset+at]:initial.source[0][at-available];}
            if(world.buffer[0][i]!=value)ExitProcess(92);if(world.device[0][i]!=(i<44100?value:initial.device[0][i]))ExitProcess(93);}
        if(observed.log[2].kind!=3||observed.log[2].args[1]!=(scenario.half==1?22050:44100)||observed.log[2].args[3])ExitProcess(94);
    }
}
static void SelectionOracle(){
    const int kinds[]={1,3,4};
    for(int width=1;width<=2;++width)for(int halfIndex=1;halfIndex<=2;++halfIndex)
    for(int k=0;k<3;++k)for(int engine=0;engine<3;++engine)for(int failed=0;failed<2;++failed){
        Defaults();scenario.width=width;scenario.half=halfIndex;scenario.cursor=halfIndex==1?20000*width:8000*width;
        scenario.kind=kinds[k];scenario.engine=engine;scenario.avail=6;scenario.status=failed?-1:0;Compare();
        if(observed.exception||observed.events!=4||observed.randoms!=1||observed.held[0]||
           observed.unlockedUnheld!=failed)ExitProcess(97);
        int half=11025*width,available=half/2,index=32767%3,source=(engine+index+k)%3,begin=halfIndex==1?0:half;
        int expectedKind=kinds[k],next=k==0?0:k==1?4:3;
        if(Channel(0)->sampleKind!=expectedKind||Channel(0)->nextSampleKind!=next||
           Channel(0)->sampleOffset!=half-available||Channel(0)->streamSample!=world.source[source]||
           Channel(0)->sampleBytes!=40000+k*1000+index||Channel(0)->fillHalf!=(halfIndex==1?2:1))ExitProcess(98);
        int offset=17+scenario.seed%31;
        for(int i=0;i<BUFFER;++i){BYTE value=initial.buffer[0][i];
            if(i>=begin&&i<begin+half){int at=i-begin;value=at<available?initial.source[0][offset+at]:initial.source[source][at-available];}
            if(world.buffer[0][i]!=value||world.device[0][i]!=(i<2*half?value:initial.device[0][i]))ExitProcess(99);}
        if(observed.log[3].kind!=3||observed.log[3].args[1]!=(halfIndex==1?half:2*half))ExitProcess(100);
    }
}

int main(int argc,char**argv){
    if(argc!=3||!MapReference(argv[1],argv[2]))return 3;
    worlds=(World*)malloc(2*sizeof(World));observations=(Observation*)malloc(2*sizeof(Observation));
    if(!worlds||!observations)return 6;
    guard=(BYTE*)VirtualAlloc(0,GUARD*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;
    if(!guard||!VirtualProtect(guard+GUARD,GUARD,PAGE_NOACCESS,&old))return 4;
    if(sizeof(RaceSound)!=0x12b0||sizeof(UnknownRaceSoundChannel)!=0x54||offsetof(RaceSound,streamChannel)!=0x434||
       offsetof(RaceSound,bytesPerSample)!=0x1298||offsetof(Sound,dsBuffer)!=0xc)return 5;
    for(int i=0;i<21;++i)vtable[i]=(DWORD)abort;vtable[4]=(DWORD)GetPosition;vtable[11]=(DWORD)Lock;vtable[19]=(DWORD)Unlock;
    int k;const int kinds[]={0,1,3,4,2,-1},halves[]={0,1,2,3,-1},returns[]={0,1,-1,(int)0x80000000},counts[]={1,3,10};
    for(int width=1;width<=2;++width)for(int h=0;h<5;++h)for(k=0;k<6;++k)for(int avail=0;avail<7;++avail)for(int cursor=0;cursor<9;++cursor){
        Defaults();scenario.width=width;scenario.half=halves[h];scenario.kind=kinds[k];scenario.avail=avail;scenario.seed=width*17+h*3+k+avail;
        const int times[]={5512*width-1,5512*width,5512*width+1,11025*width-1,11025*width,11025*width+1,16537*width-1,16537*width,16537*width+1};scenario.cursor=times[cursor];scenario.engine=(k+avail)%3;scenario.count=counts[cursor%3];scenario.randValue=(cursor%3==0?0:cursor%3==1?1:32767);Compare();}
    for(int half=1;half<=2;++half)for(k=0;k<4;++k)for(int status=0;status<4;++status)for(int alias=0;alias<7;++alias){Defaults();scenario.half=half;scenario.cursor=half==1?40000:16000;scenario.kind=kinds[k];scenario.avail=6;scenario.status=returns[status];scenario.alias=alias;Compare();}
    for(half=1;half<=2;++half)for(k=0;k<4;++k)for(int boundary=1;boundary<=4;++boundary){
        for(int action=1;action<=9;++action){Defaults();scenario.half=half;scenario.cursor=half==1?40000:16000;scenario.kind=kinds[k];scenario.avail=6;scenario.mutateAt=boundary;scenario.action=action;Compare();}
        Defaults();scenario.half=half;scenario.cursor=half==1?40000:16000;scenario.kind=kinds[k];scenario.avail=6;scenario.throwAt=boundary;Compare();}
    for(half=1;half<=2;++half)for(k=0;k<4;++k)for(int invalid=1;invalid<=7;++invalid){if(invalid==7)continue; /* GetBufferPosition would leave its cursor unwritten. */Defaults();scenario.half=half;scenario.cursor=half==1?40000:16000;scenario.kind=kinds[k];scenario.avail=6;scenario.invalid=invalid;Compare();}
    for(half=1;half<=2;++half)for(k=1;k<4;++k){Defaults();scenario.half=half;scenario.cursor=half==1?40000:16000;scenario.kind=kinds[k];scenario.avail=6;scenario.count=0;Compare();if(observed.exception!=EXCEPTION_INT_DIVIDE_BY_ZERO||observed.held[0]!=1)ExitProcess(95);}
    const int edges[]={0,1,2,3,4,7,8,15,16,31,63,127};
    for(half=1;half<=2;++half)for(int target=1;target<=3;++target)for(int edge=0;edge<12;++edge){Defaults();scenario.half=half;scenario.cursor=half==1?40000:16000;scenario.guardTarget=target;scenario.validBytes=edges[edge];Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.held[0]!=1)ExitProcess(96);}
    for(int variant=0;variant<256;++variant){Defaults();scenario.seed=variant*1664525u+1013904223u;
        scenario.width=1+(variant&1);scenario.half=1+((variant>>1)&1);scenario.cursor=(scenario.half==1?20000:8000)*scenario.width;
        scenario.kind=kinds[(variant>>2)%6];scenario.avail=(variant>>3)%7;scenario.count=1+(variant%10);scenario.randValue=(variant*31337)&32767;
        scenario.engine=(variant>>4)%3;scenario.status=returns[(variant>>5)%4];scenario.alias=(variant>>6)%7;Compare();}
    Oracle();SelectionOracle();printf("RaceSoundRefill: %d complete-state, sample-copy, COM and fault scenarios match\n",checks);return 0;
}
