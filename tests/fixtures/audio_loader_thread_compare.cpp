// Private differential harness. Production is compiled as a separate object.
// Only dependency calls/import operands are redirected; the complete loader
// body, inline queue operations, flags and ownership transfers execute intact.
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/PCAudio.h"

static unsigned char referenceSpace[0x800000];
struct World {
    unsigned char audio[sizeof(UnknownPCAudioObject)];
    unsigned char sounds[5][sizeof(Sound)];
    Sound* queue[8];
    Sound* loaded[8];
};
static World world;
struct Event { int kind,receiver,arg,extra; int held[6]; World state; };
struct Observation {
    int result,exception,faultKind,faultAddress,events,waits,ended;
    int held[6],creates[5],fills[5],plays[5],recorded[5],capturedBytes;
    Event log[180]; World state;
};
static Observation observed,expected;
struct Scenario { unsigned seed; int count,waitMode,failMask,readyMask,streamMask,stoppedMask;
    int duplicate,invalid,mutateAt,throwAt,exitReturns,bytesCase,queueMutation; };
static Scenario scenario;
static int checks;
static UnknownPCAudioObject* Audio(){return (UnknownPCAudioObject*)world.audio;}
static Sound* Item(int n){return (Sound*)world.sounds[n];}
static int SoundIndex(const Sound* sound){
    for(int n=0;n<5;++n)if(sound==Item(n))return n;
    ExitProcess(81);return -1;
}
static int LockIndex(LPCRITICAL_SECTION lock){
    if(lock==&Audio()->queueLock)return 0;
    for(int n=0;n<5;++n)if(lock==&Item(n)->bufferLock)return n+1;
    ExitProcess(82);return -1;
}
static void EventAt(int kind,int receiver,int arg,int extra){
    if(observed.events>=180)ExitProcess(83);
    Event& e=observed.log[observed.events++];
    e.kind=kind;e.receiver=receiver;e.arg=arg;e.extra=extra;
    memcpy(e.held,observed.held,sizeof(e.held));e.state=world;
    if(scenario.mutateAt==observed.events){
        if(scenario.queueMutation){
            // Emulate another participant's completed queue operation during
            // an unlocked dependency call. This is deterministic interleaving,
            // not a simulation of the Windows scheduler or its memory model.
            if(observed.held[0])ExitProcess(96);
            if(scenario.queueMutation==1){
                if(Audio()->queuedSounds.m_count>=8)ExitProcess(97);
                world.queue[Audio()->queuedSounds.m_count++]=Item(4);
                Item(4)->soundFlags|=4;Item(4)->bufferReady=0;
            }else if(Audio()->queuedSounds.m_count>0){
                Audio()->queuedSounds.m_count--;
            }
        }else{
        // Scripted changes at a dependency boundary expose cached values and
        // required reloads. All pointers remain distinct valid test identities.
        int n=receiver>0?receiver-1:0;Sound* sound=Item(n);
        sound->bufferReady=!sound->bufferReady;sound->isStopped=!sound->isStopped;
        sound->bufferBytes^=0x81fedcba;sound->soundFlags^=4;
        sound->pendingBuffer=(UnknownSoundBuffer*)(0x12340000+n*16);
        sound->pendingBuffer3D=(UnknownSound3DBuffer*)(0x23450000+n*16);
        Audio()->queuedEvent=(HANDLE)0x76543210;Audio()->stopEvent=(HANDLE)0x76543211;
        }
    }
    if(scenario.throwAt==observed.events)throw 0x351;
}
static void __stdcall Enter(LPCRITICAL_SECTION lock){
    int n=LockIndex(lock);EventAt(2,n,0,0);++observed.held[n];
}
static void __stdcall Leave(LPCRITICAL_SECTION lock){
    int n=LockIndex(lock);EventAt(3,n,0,0);
    if(observed.held[n]!=1)ExitProcess(84);--observed.held[n];
}
static DWORD __stdcall Wait(DWORD count,const HANDLE* events,BOOL all,DWORD timeout){
    if(count!=2 || all || timeout!=INFINITE || events[0]!=(HANDLE)0x11111111 ||
       events[1]!=(HANDLE)0x22222222)ExitProcess(85);
    EventAt(1,0,(int)events[0],(int)events[1]);
    int wait=observed.waits++;
    if(scenario.waitMode==1)return WAIT_FAILED;
    if(scenario.waitMode==2)return 1;
    if(scenario.waitMode>=3 && wait==0){
        if(scenario.waitMode==3)return WAIT_TIMEOUT;
        if(scenario.waitMode==4)return WAIT_ABANDONED_0;
        return 0x81234567;
    }
    if(wait==(scenario.waitMode>=3?1:0))return 0;
    if(scenario.waitMode==6 && wait==2){
        Audio()->queuedSounds.m_count=2;world.queue[0]=Item(3);world.queue[1]=Item(4);
        Item(3)->bufferReady=0;Item(4)->bufferReady=0;return 0;
    }
    return scenario.seed&1?WAIT_FAILED:1;
}
static void __cdecl End(unsigned code){
    EventAt(9,0,code,0);observed.ended++;
    if(!scenario.exitReturns)RaiseException(0xe0310001,0,0,0);
}
void UnknownPCAudioObject::MakeRoomForSound(int bytes){
    if(this!=Audio() || observed.held[0]!=1)ExitProcess(86);
    EventAt(4,0,bytes,0);observed.capturedBytes=bytes;loadedBytes^=(unsigned)bytes;
}
int Sound::CreatePendingBuffer(){
    int n=SoundIndex(this);if(observed.held[n+1]!=1)ExitProcess(87);
    EventAt(5,n+1,0,0);++observed.creates[n];
    pendingBuffer=(UnknownSoundBuffer*)(0x34560000+n*16);
    pendingBuffer3D=(UnknownSound3DBuffer*)(0x45670000+n*16);
    return scenario.failMask&(1<<n)?0:(scenario.seed&1?-3:1);
}
int Sound::FillBufferFromFile(UnknownSoundBuffer** buffer){
    int n=SoundIndex(this);if(buffer!=&pendingBuffer || observed.held[n+1]!=1)ExitProcess(88);
    EventAt(6,n+1,(int)*buffer,(int)pendingBuffer3D);++observed.fills[n];
    pendingBuffer=(UnknownSoundBuffer*)(0x56780000+n*16);
    pendingBuffer3D=(UnknownSound3DBuffer*)(0x67890000+n*16);
    return scenario.seed&2?0:-1; // The loader ignores this result, even failure.
}
int Sound::Play(){
    int n=SoundIndex(this);if(observed.held[n+1]!=1)ExitProcess(89);
    EventAt(7,n+1,0,0);++observed.plays[n];currentVolume^=0x10203040;
    return scenario.seed&4?0:-7; // Also ignored by the loader.
}
void UnknownPCAudioObject::RecordLoadedSound(Sound* sound,int bytes){
    int n=SoundIndex(sound);if(this!=Audio() || observed.held[0]!=1)ExitProcess(90);
    if(bytes!=observed.capturedBytes)ExitProcess(95);
    EventAt(8,n+1,bytes,0);++observed.recorded[n];loadedBytes+=(unsigned)bytes;
    loadedSounds.m_count++;world.loaded[n]=sound;
}
template<class T> static DWORD Address(T method){union {T method;DWORD address;}u;u.method=method;return u.address;}
static DWORD Hook(DWORD target){
    switch(target){
    case 0x4be130:return Address(&UnknownPCAudioObject::MakeRoomForSound);
    case 0x4bd4b0:return Address(&Sound::CreatePendingBuffer);
    case 0x4bd0c0:return Address(&Sound::FillBufferFromFile);
    case 0x4bbcd0:return Address(&Sound::Play);
    case 0x4be0a0:return Address(&UnknownPCAudioObject::RecordLoadedSound);
    case 0x537c9f:return (DWORD)End;
    }ExitProcess(91);return 0;
}
static DWORD imports[3]={(DWORD)Wait,(DWORD)Enter,(DWORD)Leave};
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    *(DWORD*)0x5500d8=(DWORD)Wait;*(DWORD*)0x55025c=(DWORD)Enter;*(DWORD*)0x550284=(DWORD)Leave;
    BYTE* candidate=(BYTE*)UnknownPCAudioObject::UnknownThreadProc;
    if(!VirtualProtect(candidate,478,PAGE_EXECUTE_READWRITE,&old))return 0;
    // Validated Python records identify only complete-function dependency
    // relocations. Import operands point to private cells; shared CRT imports
    // remain untouched. The candidate object itself is never rewritten.
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){
        ++records;
        if(entry[0]==0){
            BYTE* site=(BYTE*)entry[1];
            if(site<(BYTE*)0x4bdc00 || site+5>(BYTE*)0x4bdde3 || *site!=0xe8 ||
               (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[2])return 0;
            *(DWORD*)(site+1)=Hook(entry[2])-(DWORD)(site+5);
        }else if(entry[0]==1){
            if(entry[1]+4>478 || entry[2]>2)return 0;
            *(DWORD*)(candidate+entry[1])=(DWORD)&imports[entry[2]];
        }else if(entry[0]==2){
            if(entry[1]+4>478 || candidate[entry[1]-1]!=0xe8 ||
               (DWORD)(candidate+entry[1]+4)+*(DWORD*)(candidate+entry[1])!=(DWORD)_endthreadex)return 0;
            *(DWORD*)(candidate+entry[1])=(DWORD)End-(DWORD)(candidate+entry[1]+4);
        }else return 0;
    }fclose(f);return records==21 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){
    DWORD code=e->ExceptionRecord->ExceptionCode;
    if(code!=EXCEPTION_ACCESS_VIOLATION && code!=0xe0310001)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=code;
    if(code==EXCEPTION_ACCESS_VIOLATION){observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Invoke(int retail){
    unsigned(__stdcall* call)(void*)=UnknownPCAudioObject::UnknownThreadProc;
    if(retail)call=(unsigned(__stdcall*)(void*))0x4bdc00;
    __try{observed.result=call(scenario.invalid==3?0:Audio());}
    __except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){
    memset(&observed,0,sizeof(observed));observed.result=-501;
    unsigned random=scenario.seed;
    for(unsigned i=0;i<sizeof(world);++i){random=random*1664525u+1013904223u;((BYTE*)&world)[i]=(BYTE)(random>>24);}
    Audio()->queuedEvent=(HANDLE)0x11111111;Audio()->stopEvent=(HANDLE)0x22222222;
    Audio()->queuedSounds.m_count=scenario.count;Audio()->queuedSounds.m_data=world.queue;
    Audio()->queuedSounds.m_capacity=8;Audio()->loadedSounds.m_count=0;Audio()->loadedSounds.m_data=world.loaded;
    for(int n=0;n<5;++n){
        Item(n)->soundFlags=(Item(n)->soundFlags&~4u)|((scenario.streamMask&(1<<n))?4:0);
        Item(n)->bufferReady=(scenario.readyMask>>n)&1;Item(n)->isStopped=(scenario.stoppedMask>>n)&1;
        world.queue[n]=Item(scenario.duplicate?n%2:n);
        if(scenario.bytesCase){
            const unsigned values[]={0,1,0x7fffffff,0x80000000,0xffffffff};
            Item(n)->bufferBytes=values[(scenario.bytesCase+n-1)%5];
        }
    }
    if(scenario.invalid==1)Audio()->queuedSounds.m_data=0;
    if(scenario.invalid==2)world.queue[0]=0;
    try{Invoke(retail);}catch(int e){observed.exception=e;}
    observed.state=world;
}
static void Compare(){
    Run(1);expected=observed;Run(0);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){
        BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;
        printf("Mismatch%d at%d %02x/%02x seed%u count%d wait%d masks%x/%x/%x/%x duplicate%d invalid%d mutate%d throw%d exit%d events%d/%d\n",
            checks,offset,a[offset],b[offset],scenario.seed,scenario.count,scenario.waitMode,scenario.failMask,scenario.readyMask,
            scenario.streamMask,scenario.stoppedMask,scenario.duplicate,scenario.invalid,scenario.mutateAt,scenario.throwAt,
            scenario.exitReturns,expected.events,observed.events);ExitProcess(2);
    }
    if(observed.ended){
        for(int n=0;n<6;++n)if(observed.held[n])ExitProcess(92);
        if(observed.ended!=1 || (scenario.exitReturns?observed.result!=0:observed.exception!=0xe0310001))ExitProcess(93);
    }
}
int main(int argc,char** argv){
    if(argc!=3 || !MapReference(argv[1],argv[2]))return 3;
    if(sizeof(UnknownPCAudioObject)!=0x54 || sizeof(Sound)!=0x1f8 || offsetof(UnknownPCAudioObject,queuedSounds)!=0x2c ||
       offsetof(Sound,bufferLock)!=0x48 || offsetof(Sound,bufferBytes)!=0x198)return 4;
    // Independent expected outcomes protect against an inert dispatcher or
    // two identically miswired paths agreeing without executing the operation.
    for(int fail=0;fail<2;++fail){
        memset(&scenario,0,sizeof(scenario));scenario.seed=91;scenario.count=3;
        scenario.streamMask=31;scenario.failMask=fail?7:0;Compare();
        if(Audio()->queuedSounds.m_count || observed.waits!=2 || observed.ended!=1)ExitProcess(98);
        for(int n=0;n<3;++n){
            if(observed.creates[n]!=1 || observed.fills[n]!=!fail || observed.plays[n]!=!fail || observed.recorded[n]!=!fail)ExitProcess(99);
            if(!fail && (!Item(n)->bufferReady || Item(n)->field_0x1f4_bit7 || Item(n)->pendingBuffer || Item(n)->pendingBuffer3D ||
                (DWORD)Item(n)->dsBuffer!=0x56780000+n*16 || (DWORD)Item(n)->dsBuffer3D!=0x67890000+n*16))ExitProcess(100);
            if(fail && (Item(n)->bufferReady || (DWORD)Item(n)->pendingBuffer!=0x34560000+n*16))ExitProcess(101);
        }
    }
    // Every five-item flag mask, failure mask and stopped state, varying seed,
    // byte values, nonboolean return values and both thread-exit observations.
    for(int stream=0;stream<32;++stream)for(int ready=0;ready<32;++ready)
    for(int fail=0;fail<32;++fail){
        memset(&scenario,0,sizeof(scenario));scenario.seed=stream*1024+ready*32+fail;
        scenario.count=5;scenario.streamMask=stream;scenario.readyMask=ready;scenario.failMask=fail;
        scenario.stoppedMask=(stream^ready^fail);scenario.exitReturns=scenario.seed&1;Compare();
    }
    for(int count=-2;count<=5;++count)for(int wait=0;wait<7;++wait)for(int duplicate=0;duplicate<2;++duplicate)
    for(int stopped=0;stopped<32;++stopped){
        memset(&scenario,0,sizeof(scenario));scenario.seed=stopped+count*111;scenario.count=count;scenario.waitMode=wait;
        scenario.duplicate=duplicate;scenario.streamMask=31;scenario.stoppedMask=stopped;Compare();
    }
    // Mutation and C++ propagation at every reachable boundary, with each
    // sound's full bytes and every held-lock state observed before and after.
    for(int duplicate=0;duplicate<2;++duplicate)for(int stopped=0;stopped<2;++stopped){
        memset(&scenario,0,sizeof(scenario));scenario.seed=0x81234567;scenario.count=5;scenario.streamMask=31;
        scenario.stoppedMask=stopped?31:0;scenario.duplicate=duplicate;scenario.waitMode=6;Compare();
        int events=observed.events;
        for(int event=1;event<=events;++event){scenario.throwAt=event;Compare();scenario.throwAt=0;scenario.mutateAt=event;Compare();scenario.mutateAt=0;}
    }
    for(int invalid=1;invalid<=3;++invalid){
        memset(&scenario,0,sizeof(scenario));scenario.seed=9;scenario.count=2;scenario.streamMask=31;scenario.invalid=invalid;Compare();
        if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)ExitProcess(94);
        const int faults[]={0,0,0x1e8,8};
        if(observed.faultKind || observed.faultAddress!=faults[invalid] || observed.held[0]!=(invalid==1))ExitProcess(102);
    }
    for(int bytes=1;bytes<=5;++bytes)for(int stopped=0;stopped<32;++stopped){
        memset(&scenario,0,sizeof(scenario));scenario.seed=stopped;scenario.count=5;
        scenario.streamMask=31;scenario.stoppedMask=stopped;scenario.bytesCase=bytes;Compare();
    }
    for(int mutation=1;mutation<=2;++mutation){
        memset(&scenario,0,sizeof(scenario));scenario.seed=117;scenario.count=3;scenario.streamMask=31;Compare();
        int events=observed.events,sites[180],count=0;
        for(int n=0;n<events;++n){
            int kind=observed.log[n].kind;
            if((kind==5 || kind==6 || kind==7) && !observed.log[n].held[0])sites[count++]=n+1;
        }
        if(count!=9)ExitProcess(103);
        for(n=0;n<count;++n){scenario.queueMutation=mutation;scenario.mutateAt=sites[n];Compare();}
    }
    printf("AudioLoaderThread: %d complete-body state, queue, lock and callback scenarios match\n",checks);return 0;
}
