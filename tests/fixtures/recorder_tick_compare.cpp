// Private whole-method replay comparison. Only external dependency calls are
// scripted; production and reference execute all playback branches themselves.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "../../src/reconstructed/Recorder.h"
#include "../../src/reconstructed/VCR.h"
extern "C" void _ftol(); // Address only: execute the real linked VC6 CRT helper.
static BYTE referenceSpace[0x800000];
struct World {BYTE recorder[0xd8],buffers[2][0x400],rings[2][0x101c];};
static World world;
struct Event {int kind,a,b,c,d,e,held;World state;};
struct Observation {int result,exception,faultKind,faultAddress,count,takes,callbacks,held;unsigned fp;Event log[100];World state;};
static Observation observed,expected;
struct Scenario {unsigned seed;int mode,ready,reverse,reset,restarted,pending,count,firstResult,laterResult,callback,laterCallback,keep,recordType,mutateAt,mutation,throwAt,invalid,round;float time;};
static Scenario scenario;
static int checks;
static VCRInterface* Vcr(){return (VCRInterface*)world.recorder;}
static unsigned Random(unsigned& state){return state=state*1664525u+1013904223u;}
static void EventAt(int kind,int a=0,int b=0,int c=0,int d=0,int e=0){
    if(observed.count>=100)ExitProcess(81);
    Event& event=observed.log[observed.count++];event.kind=kind;event.a=a;event.b=b;event.c=c;event.d=d;event.e=e;event.held=observed.held;event.state=world;
    if(scenario.mutateAt==observed.count){
        VCRInterface* v=Vcr();
        switch(scenario.mutation){
        case 0:v->recordPending=!v->recordPending;v->readingReverse=!v->readingReverse;v->readsReset=!v->readsReset;v->reverseReadRestarted=!v->reverseReadRestarted;v->recordTimeMs^=0x12345678;v->recordId^=0x7654;v->recordType=-1;break;
        case 1:v->recordBuffer=world.buffers[1];v->frameRing=(UnknownVcr*)world.rings[1];break;
        case 2:((UnknownRecorderRecord*)v->recordBuffer)->count=2;((UnknownRecorderRecord*)v->recordBuffer)->time=-.0009999f;break;
        case 3:v->recordCallback=0;break;
        }
    }
    if(scenario.throwAt==observed.count)throw 0x641;
}
static void __stdcall Enter(LPCRITICAL_SECTION lock){if(lock!=&Vcr()->lock)ExitProcess(82);EventAt(1);++observed.held;}
static void __stdcall Leave(LPCRITICAL_SECTION lock){if(lock!=&Vcr()->lock || observed.held!=1)ExitProcess(83);EventAt(2);--observed.held;}
static int Buffer(void* p){if(!p)return -1;for(int n=0;n<2;++n)if(p==world.buffers[n])return n;ExitProcess(84);return 0;}
static int Ring(void* p){for(int n=0;n<2;++n)if(p==world.rings[n])return n;ExitProcess(85);return 0;}
int VCRInterface::TakeRecord(int* type,int* id,int* position,void* data,unsigned* size,int signal){
    if(this!=Vcr() || type!=&recordType || id!=&recordId || !position || !size)ExitProcess(86);
    int index=observed.takes++;int result=index==0?scenario.firstResult:scenario.laterResult;
    if(index>=5)result=4; // Finite source of records, including malformed negative counts.
    EventAt(3,Buffer(data),position==&recordPosition,signal,result,index);
    if(!result){
        *type=index==0?scenario.recordType:((scenario.seed&1)?-1:10);*id=0x170+index;*position=0x1280+index*31;*size=sizeof(UnknownRecorderRecord);
        UnknownRecorderRecord* r=(UnknownRecorderRecord*)data;r->time=scenario.time;r->field_0x04=0x416000+index;r->count=scenario.count;
    }
    return result;
}
void VCRInterface::StopReading(){if(this!=Vcr())ExitProcess(87);EventAt(4);readsReset=0;fileReady=0;}
void VCRInterface::RestartReverseRead(){if(this!=Vcr())ExitProcess(87);EventAt(5);isBusy^=0x83;}
void VCRInterface::Seek(unsigned time){if(this!=Vcr())ExitProcess(87);EventAt(6,time);sceneTimePending=1;pendingSceneTime=time*.001f;}
void VCRInterface::Rewind(){if(this!=Vcr())ExitProcess(87);EventAt(7);sceneTimePending=1;pendingSceneTime=0;fileReady=0;}
void UnknownVcr::AppendPosition(int position,int time){int n=Ring(this);EventAt(8,n,position,time);positionIndex++;firstPosition=position;}
void UnknownVcr::SeekPosition(int position){int n=Ring(this);EventAt(9,n,position);positionIndex=position;}
static int Callback(int type,void* data,int flag,int id,int* keep,int time,int* milliseconds){
    if(!keep || !milliseconds || keep==milliseconds)ExitProcess(88);
    int index=observed.callbacks++;
    int result=index==0?scenario.callback:scenario.laterCallback;
    EventAt(10,type,Buffer(data),flag,id,time);
    *keep=scenario.keep;*milliseconds=0x82345678u+index;return result;
}
template<class T>static DWORD Address(T method){union{T method;DWORD address;}u;u.method=method;return u.address;}
static DWORD Hook(DWORD target){switch(target){
case 0x4e8810:return Address(&VCRInterface::TakeRecord);
case 0x4e8990:return Address(&VCRInterface::StopReading);
case 0x4e89c0:return Address(&VCRInterface::RestartReverseRead);
case 0x4e8a20:return Address(&VCRInterface::Seek);
case 0x4e8a70:return Address(&VCRInterface::Rewind);
case 0x524590:return Address(&UnknownVcr::AppendPosition);
case 0x524870:return Address(&UnknownVcr::SeekPosition);
case 0x534a88:return (DWORD)_ftol;
}ExitProcess(89);return 0;}
static DWORD imports[]={(DWORD)Enter,(DWORD)Leave};
static int MapReference(const char* path,const char* patches,DWORD candidateAddress){
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
    *(DWORD*)0x55025c=(DWORD)Enter;*(DWORD*)0x550284=(DWORD)Leave;
    BYTE* candidate=(BYTE*)candidateAddress;
    if(!VirtualProtect(candidate,2448,PAGE_EXECUTE_READWRITE,&old))return 0;
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){++records;
        if(entry[0]==0){BYTE* site=(BYTE*)entry[1];if(site<(BYTE*)0x4e7d60 || site+5>(BYTE*)0x4e869c || *site!=0xe8 || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[2])return 0;*(DWORD*)(site+1)=Hook(entry[2])-(DWORD)(site+5);}
        else if(entry[0]==1){if(entry[1]+4>2448 || entry[2]>1)return 0;*(DWORD*)(candidate+entry[1])=(DWORD)&imports[entry[2]];}
        else return 0;
    }fclose(f);return records>30 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(int retail){union{int(VCRInterface::*method)(float);DWORD address;}call;call.address=0x4e7d60;
    __try{observed.result=retail?(Vcr()->*call.method)(scenario.time):Vcr()->VCRInterface::Tick(scenario.time);}
    __except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){
    memset(&observed,0,sizeof(observed));observed.result=-501;unsigned seed=scenario.seed;
    for(unsigned n=0;n<sizeof(world);++n)((BYTE*)&world)[n]=(BYTE)(Random(seed)>>24);
    VCRInterface* v=Vcr();v->recordMode=scenario.mode;v->fileReady=scenario.ready;v->readingReverse=scenario.reverse;v->readsReset=scenario.reset;v->reverseReadRestarted=scenario.restarted;v->recordPending=scenario.pending;
    v->recordBuffer=world.buffers[0];v->frameRing=(UnknownVcr*)world.rings[0];v->recordCallback=Callback;v->recordType=scenario.recordType;v->recordId=0x123;v->recordTimeMs=-234;v->recordPosition=0x8192;
    for(int i=0;i<2;++i){((UnknownRecorderRecord*)world.buffers[i])->time=scenario.time;((UnknownRecorderRecord*)world.buffers[i])->count=scenario.count;}
    if(scenario.invalid==1)v->recordCallback=0;
    if(scenario.invalid==2)v->recordBuffer=0;
    _control87(_MCW_EM|_PC_64|(scenario.round<<8),_MCW_EM|_MCW_PC|_MCW_RC);_clear87();
    try{Invoke(retail);}catch(int e){observed.exception=e;}
    observed.fp=_status87();observed.state=world;
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;
    if(memcmp(&observed,&expected,sizeof(observed))){BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int off=0;while(a[off]==b[off])++off;printf("Mismatch %d offset%d %02x/%02x seed%u mode%d reverse%d pending%d result%d/%d cb%d/%d count%d mutation%d/%d throw%d events%d/%d\n",checks,off,a[off],b[off],scenario.seed,scenario.mode,scenario.reverse,scenario.pending,scenario.firstResult,scenario.laterResult,scenario.callback,scenario.laterCallback,scenario.count,scenario.mutateAt,scenario.mutation,scenario.throwAt,expected.count,observed.count);ExitProcess(2);}
    if(!observed.exception && (observed.result!=1 || observed.held))ExitProcess(90);
    if(observed.exception==0x641 && (!observed.count || observed.held!=observed.log[observed.count-1].held))ExitProcess(91);
    if(observed.exception==EXCEPTION_ACCESS_VIOLATION && observed.held)ExitProcess(92);
}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.mode=1;scenario.ready=1;scenario.restarted=1;scenario.recordType=-1;scenario.time=1.2345f;scenario.count=2;scenario.callback=1;scenario.laterCallback=1;scenario.keep=1;}
int main(int argc,char** argv){
    if(argc!=4 || !MapReference(argv[1],argv[2],strtoul(argv[3],0,16)))return 3;
    if(sizeof(VCRInterface)!=0xd8 || offsetof(VCRInterface,lock)!=0x60 || offsetof(VCRInterface,recordTimeMs)!=0xd4)return 4;
    Base();scenario.ready=0;Compare();if(observed.result!=1 || observed.count!=2 || observed.takes || observed.callbacks || observed.held) return 5;
    Base();scenario.reverse=1;scenario.reset=1;Compare();if(observed.result!=1 || !Vcr()->reverseReadRestarted || Vcr()->readsReset || observed.log[observed.count-1].kind!=5)return 6;
    Base();scenario.callback=5;Compare();if(observed.result!=1 || Vcr()->recordPending!=1 || observed.takes!=1 || observed.callbacks!=1)return 7;
    Base();scenario.callback=12;Compare();if(observed.result!=1 || observed.log[observed.count-1].kind!=7 || !Vcr()->sceneTimePending)return 8;
    Base();Compare();
    if(observed.takes!=3 || observed.callbacks!=3 || Vcr()->recordTimeMs!=1234 || Vcr()->recordPending)return 10;
    int positions=0;for(int event=0;event<observed.count;++event)if(observed.log[event].kind==8){Event& e=observed.log[event];if(e.c!=(positions?1234:-234))return 11;++positions;}
    if(positions!=4 || observed.log[observed.count-1].kind!=8 || observed.log[observed.count-1].b!=-1)return 12;
    const int counts[]={-1,0,1,3};
    for(int mode=0;mode<7;++mode)for(int result=0;result<5;++result)for(int later=0;later<5;++later)
    for(int cb=0;cb<14;++cb)for(int next=0;next<14;++next)for(int c=0;c<4;++c){
        Base();scenario.seed=checks+1;scenario.firstResult=result;scenario.laterResult=later;scenario.callback=cb-1;scenario.laterCallback=next-1;scenario.count=counts[c];scenario.keep=checks&1?-3:0;
        scenario.pending=mode==1;scenario.reverse=mode>=2 && mode<=4;scenario.reset=mode==2;scenario.restarted=mode!=3;scenario.ready=mode!=5;scenario.mode=mode==6?2:1;Compare();
    }
    for(mode=0;mode<4;++mode)for(int cb=0;cb<14;++cb){
        Base();scenario.pending=mode==1;scenario.reverse=mode==2;scenario.firstResult=mode==3?1:0;scenario.callback=cb-1;scenario.laterCallback=cb-1;Compare();int events=observed.count;
        for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<4;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}
    }
    const unsigned floats[]={0,0x80000000,1,0x007fffff,0x00800000,0x3a83126f,0x3a83126e,0xba83126f,0x3f7fffff,0x4a03126e,0xca03126e,0x7f7fffff,0xff7fffff};
    for(int f=0;f<13;++f)for(int round=0;round<4;++round)for(int pending=0;pending<2;++pending){Base();scenario.time=*(float*)&floats[f];scenario.round=round*4;scenario.pending=pending;Compare();}
    for(int invalid=1;invalid<=2;++invalid){Base();scenario.invalid=invalid;scenario.pending=1;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 9;}
    unsigned random=0x12345;
    for(int n=0;n<8192;++n){Base();scenario.seed=Random(random);scenario.pending=(Random(random)>>16)&1;scenario.reverse=(Random(random)>>16)&1;scenario.reset=(Random(random)>>16)&1;scenario.restarted=(Random(random)>>16)&1;scenario.firstResult=(Random(random)>>16)%5;scenario.laterResult=(Random(random)>>16)%5;scenario.callback=(Random(random)>>16)%14-1;scenario.laterCallback=(Random(random)>>16)%14-1;scenario.count=(Random(random)>>16)%5-1;scenario.keep=(Random(random)>>16)&1;scenario.recordType=(Random(random)>>16)&1?-1:10;unsigned bits=Random(random)&0xfeffffff;scenario.time=*(float*)&bits;scenario.round=((Random(random)>>16)&3)*4;Compare();}
    printf("RecorderTick: %d complete playback, callback, lock and floating-point scenarios match\n",checks);return 0;
}
