// Private whole-worker comparison: no real thread or file is started here.
// The actual separately compiled operation runs against scripted dependencies.
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/Recorder.h"
#include "../../src/reconstructed/VCR.h"
#include "../../src/reconstructed/VCRfile.h"
unsigned __stdcall UnknownRecorderThread(void*);
static BYTE referenceSpace[0x800000]={1};
struct World {BYTE recorder[0xd8],rings[2][0x101c],files[2][0x248],buffers[4][0x400],headers[2][128];char names[2][512];};
static World world;
struct Event {int kind,who,args[12],held;unsigned size;BYTE bytes[1024];World state;};
struct Observation {int result,exception,faultKind,faultAddress,count,waits,held,ended,reads,writes,claims,writeChecks,roomChecks,peeks;Event events[150];World state;};
static Observation observed,expected;
struct Scenario {unsigned seed,size;int prefix,first,second,fail,eof,busy,writeMask,roomMask,position,peekAgain,handle,headerSize,nameLength,mutateAt,mutation,throwAt,exitReturns,invalid;};
static Scenario scenario;
static int checks,retailActive;
static VCRInterface* Vcr(){return (VCRInterface*)world.recorder;}
static void Fail(int code){printf("Fixture invariant%d check%d ref%d events%d last%d first%d second%d fail%x mutation%d/%d\n",code,checks,retailActive,observed.count,observed.count?observed.events[observed.count-1].kind:-1,scenario.first,scenario.second,scenario.fail,scenario.mutateAt,scenario.mutation);fflush(stdout);ExitProcess(code);}
static int Which(const void* p,const void* a,const void* b){if(p==a)return 0;if(p==b)return 1;if(!p)return -1;Fail(80);return -1;}
static int Ring(const void* p){int n=Which(p,world.rings[0],world.rings[1]);if(n<0){volatile int ignored=*(volatile int*)p;(void)ignored;}return n;}
static int File(const void* p){int n=Which(p,world.files[0],world.files[1]);if(n<0){volatile int ignored=*(volatile int*)p;(void)ignored;}return n;}
static int Buffer(const void* p){if(!p)return -1;for(int n=0;n<4;++n)if(p==world.buffers[n])return n;for(n=0;n<2;++n)if(p==world.headers[n])return 10+n;return 20;}
static void Finish(){
    if(scenario.mutateAt==observed.count){VCRInterface* v=Vcr();switch(scenario.mutation){
    case 0:v->isBusy=!v->isBusy;v->fileReady=!v->fileReady;v->isStopped^=17;v->workerAcknowledged^=7;v->recordTimeMs^=0x345678;break;
    case 1:v->frameRing=(UnknownVcr*)world.rings[1];v->vcrFile=(UnknownRecorderOwner*)world.files[1];break;
    case 2:v->threadParameters.header=(int)world.headers[1];v->threadParameters.headerSize=33;v->threadParameters.fileName=(int)world.names[1];break;
    case 3:v->writeEvent=(HANDLE)0x2100;v->readForwardEvent=(HANDLE)0x2103;v->readReverseEvent=(HANDLE)0x2105;break;
    }}
    if(scenario.throwAt==observed.count)throw 0x741;
}
static Event& Begin(int kind,int who=0,int a=0,int b=0,int c=0,int d=0){
    if(observed.count>=150)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=who;e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;e.held=observed.held;e.state=world;return e;
}
static void __stdcall Enter(LPCRITICAL_SECTION lock){if(lock!=&Vcr()->lock)Fail(82);Begin(1);Finish();++observed.held;}
static void __stdcall Leave(LPCRITICAL_SECTION lock){if(lock!=&Vcr()->lock || observed.held!=1)Fail(83);Begin(2);Finish();--observed.held;}
static DWORD WaitValue(int value){if(value==12)return WAIT_FAILED;if(value==13)return WAIT_ABANDONED_0;if(value==14)return 0x12345678;return value;}
static DWORD __stdcall Wait(DWORD count,const HANDLE* events,BOOL all,DWORD timeout){
    if(count!=11 || all || timeout!=INFINITE || observed.held)Fail(84);
    Event& e=Begin(3);for(int n=0;n<11;++n){if(events[n]!=(HANDLE)(0x1100+n))Fail(85);e.args[n]=(int)events[n];}
    int step=observed.waits++;DWORD result;
    if(scenario.prefix && !step)result=scenario.prefix;
    else{step-=scenario.prefix!=0;result=WaitValue(step==0?scenario.first:step==1?scenario.second:10);}
    e.args[11]=result;Finish();return result;
}
static BOOL __stdcall Set(HANDLE event){Begin(4,0,(int)event);Finish();return scenario.seed&1?0:1;}
static BOOL __stdcall Reset(HANDLE event){Begin(5,0,(int)event);Finish();return scenario.seed&1?0:1;}
static void __cdecl End(unsigned code){if(code || observed.held)Fail(86);Begin(6);Finish();++observed.ended;if(!scenario.exitReturns)RaiseException(0xe0410001,0,0,0);}
int UnknownVcr::HasSlotToWrite(){int n=Ring(this);int result=(scenario.writeMask>>(observed.writeChecks++&1))&1;Begin(10,n,result);Finish();return result;}
void UnknownVcr::TakeSlotToWrite(void** data,int* size,int* index){int n=Ring(this);if(!data || !size || !index || *data)Fail(87);Begin(11,n);Finish();*data=world.buffers[n];*size=scenario.size<=1024?scenario.size:17;*index=27+n;}
void UnknownVcr::ReleaseSlot(int index){int n=Ring(this);Begin(12,n,index);Finish();slots[index&255].state=0;}
int UnknownVcr::HasRoomToRead(){int n=Ring(this);int result=(scenario.roomMask>>(observed.roomChecks++&1))&1;Begin(13,n,result);Finish();return result;}
int UnknownVcr::ClaimReadSlot(unsigned size,int* index,void** data){int n=Ring(this);if(!index || !data || *data)Fail(88);Begin(14,n,size);Finish();int which=observed.claims++&3;if(scenario.fail&32)return 0;*index=43+which;*data=world.buffers[which];return scenario.seed&1?-7:1;}
void UnknownVcr::CommitReadSlot(int position,int index){int n=Ring(this);Begin(15,n,position,index);Finish();slots[index&255].state=1;slots[index&255].filePosition=position;}
void UnknownVcr::MarkForwardEnd(){int n=Ring(this);Begin(16,n);Finish();slots[0].state=2;}
void UnknownVcr::MarkReverseEnd(){int n=Ring(this);Begin(17,n);Finish();slots[0].state=3;}
int UnknownVcr::StepPositionBack(int* extra){int n=Ring(this);if(!extra)Fail(89);Begin(18,n,scenario.position);Finish();*extra=0x82345678;return scenario.position;}
int UnknownVcr::PeekPosition(int* extra){int n=Ring(this);if(!extra)Fail(89);int position=observed.peeks++?scenario.peekAgain:scenario.position;Begin(19,n,position);Finish();*extra=0x72345678;return position;}
void UnknownVcr::SetFirstPosition(int position){int n=Ring(this);Begin(20,n,position);Finish();firstPosition=position;}
int UnknownVcr::GetFirstPosition(){int n=Ring(this);Begin(21,n,firstPosition);Finish();return firstPosition;}
void UnknownVcr::ResetSlots(){int n=Ring(this);Begin(22,n);Finish();writeCursor=playCursor=fillCursor=0;slots[0].state=0;}
int UnknownVcrFile::Open(char* name,char* mode){int n=File(this);int which=Which(name,world.names[0],world.names[1]);int kind=!strcmp(mode,"wb")?0:!strcmp(mode,"rb+")?1:!strcmp(mode,"rb")?2:-1;if(kind<0)Fail(90);Event& e=Begin(30,n,which,kind);if(name){e.size=strlen(name)+1;if(e.size>512)Fail(91);memcpy(e.bytes,name,e.size);}Finish();return scenario.fail&1?-1:scenario.handle;}
void UnknownVcrFile::Close(int handle){int n=File(this);Begin(31,n,handle);Finish();entries[0].field_0x00=0;}
int UnknownVcrFile::Read(void* buffer,int size,int count,int handle){int n=File(this);int role=Buffer(buffer);if(count!=1 || size<0 || size>1024)Fail(92);Begin(32,n,role,size,count,handle);Finish();++observed.reads;
    if(!size)return 0;
    if((role==20 && (scenario.fail&4)) || (role>=10 && role<12 && (scenario.fail&16)) || ((role<4 || role==-1) && (scenario.fail&8)))return 0;
    if(role==20){if(size!=4)Fail(93);*(unsigned*)buffer=scenario.size;}
    else{for(int i=0;i<size;++i)((BYTE*)buffer)[i]=(BYTE)(scenario.seed+i*17+observed.reads*31);}
    entries[0].position+=size;return scenario.seed&1?-7:1;
}
int UnknownVcrFile::Write(const void* buffer,int size,int count,int handle){int n=File(this);int role=Buffer(buffer);if(role==20 || size<0 || size>1024 || count!=1)Fail(94);Event& e=Begin(33,n,role,size,count,handle);e.size=size;if(size)memcpy(e.bytes,buffer,size);Finish();++observed.writes;if(scenario.fail&2)return 0;entries[0].position+=size;return scenario.seed&1?-7:1;}
int UnknownVcrFile::Seek(int handle,int offset,int origin){int n=File(this);Begin(34,n,handle,offset,origin);if(origin)Fail(95);Finish();entries[0].position=offset;return scenario.seed&1?-1:0;}
int UnknownVcrFile::Tell(int handle){int n=File(this);Begin(35,n,handle,entries[0].position);Finish();return entries[0].position;}
int UnknownVcrFile::IsEndOfFile(int handle){int n=File(this);Begin(36,n,handle,scenario.eof);Finish();return scenario.eof;}
template<class T>static DWORD Address(T method){union{T method;DWORD address;}u;u.method=method;return u.address;}
static DWORD Hook(DWORD address){switch(address){
case 0x524350:return Address(&UnknownVcr::HasSlotToWrite);case 0x524300:return Address(&UnknownVcr::TakeSlotToWrite);
case 0x524540:return Address(&UnknownVcr::ReleaseSlot);case 0x524560:return Address(&UnknownVcr::HasRoomToRead);
case 0x524390:return Address(&UnknownVcr::ClaimReadSlot);case 0x524440:return Address(&UnknownVcr::CommitReadSlot);
case 0x524940:return Address(&UnknownVcr::MarkForwardEnd);case 0x524970:return Address(&UnknownVcr::MarkReverseEnd);
case 0x5246c0:return Address(&UnknownVcr::StepPositionBack);case 0x5247c0:return Address(&UnknownVcr::PeekPosition);
case 0x5248f0:return Address(&UnknownVcr::SetFirstPosition);case 0x524900:return Address(&UnknownVcr::GetFirstPosition);
case 0x524910:return Address(&UnknownVcr::ResetSlots);case 0x524dd0:return Address(&UnknownVcrFile::Open);
case 0x525000:return Address(&UnknownVcrFile::Close);case 0x525070:return Address(&UnknownVcrFile::Read);
case 0x5251f0:return Address(&UnknownVcrFile::Write);case 0x5253d0:return Address(&UnknownVcrFile::Seek);
case 0x525440:return Address(&UnknownVcrFile::Tell);case 0x5254a0:return Address(&UnknownVcrFile::IsEndOfFile);
case 0x537c9f:return (DWORD)End;
}Fail(96);return 0;}
static DWORD imports[]={(DWORD)Wait,(DWORD)Enter,(DWORD)Leave,(DWORD)Set,(DWORD)Reset};
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)){printf("map coverage %p end%p\n",referenceSpace,referenceSpace+sizeof(referenceSpace));return 0;}
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 || dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE || pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
       pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 ||
       pe->OptionalHeader.SizeOfHeaders>(DWORD)size || pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD sectionsOffset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if(sectionsOffset>(DWORD)size || pe->FileHeader.NumberOfSections>((DWORD)size-sectionsOffset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(section[i].PointerToRawData>(DWORD)size || section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData ||
           section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage || section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    *(DWORD*)0x5500d8=(DWORD)Wait;*(DWORD*)0x55025c=(DWORD)Enter;*(DWORD*)0x550284=(DWORD)Leave;*(DWORD*)0x5500e8=(DWORD)Set;*(DWORD*)0x5500e0=(DWORD)Reset;
    BYTE* candidate=(BYTE*)UnknownRecorderThread;if(!VirtualProtect(candidate,2148,PAGE_EXECUTE_READWRITE,&old))return 0;
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){++records;
        if(entry[0]==0){BYTE* site=(BYTE*)entry[1];if(site<(BYTE*)0x4e6f80 || site+5>(BYTE*)0x4e77b5 || *site!=0xe8 || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[2])return 0;*(DWORD*)(site+1)=Hook(entry[2])-(DWORD)(site+5);}
        else if(entry[0]==1){if(entry[1]+4>2148 || entry[2]>4)return 0;*(DWORD*)(candidate+entry[1])=(DWORD)&imports[entry[2]];}
        else if(entry[0]==2){if(entry[1]+4>2148 || candidate[entry[1]-1]!=0xe8 || (DWORD)(candidate+entry[1]+4)+*(DWORD*)(candidate+entry[1])!=(DWORD)_endthreadex)return 0;*(DWORD*)(candidate+entry[1])=(DWORD)End-(DWORD)(candidate+entry[1]+4);}
        else return 0;
    }fclose(f);return records==103 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION && code!=0xe0410001)return EXCEPTION_CONTINUE_SEARCH;observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){unsigned(__stdcall* call)(void*)=retailActive?(unsigned(__stdcall*)(void*))0x4e6f80:UnknownRecorderThread;__try{observed.result=call(scenario.invalid==1?0:&Vcr()->threadParameters);}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){
    retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-501;memset(&world,0x53+(scenario.seed%19),sizeof(world));VCRInterface* v=Vcr();
    HANDLE* handles[]={&v->writeEvent,&v->createFileEvent,&v->reopenFileEvent,&v->readForwardEvent,&v->openPlaybackEvent,&v->readReverseEvent,&v->seekEvent,&v->rewindEvent,&v->stopReadingEvent,&v->closeFileEvent,&v->stopEvent};
    for(int n=0;n<11;++n)*handles[n]=(HANDLE)(0x1100+n);
    v->frameRing=(UnknownVcr*)world.rings[0];v->vcrFile=(UnknownRecorderOwner*)world.files[0];v->isBusy=scenario.busy;v->isStopped=-7;v->fileReady=0;v->workerAcknowledged=0;v->readsReset=0;v->recordTimeMs=-19;
    v->threadParameters.owner=scenario.invalid==2?0:v;v->threadParameters.header=(int)world.headers[0];v->threadParameters.headerSize=scenario.headerSize;v->threadParameters.fileName=scenario.invalid==3?0:(int)world.names[0];
    if(scenario.invalid==4)v->frameRing=0;
    for(n=0;n<2;++n){((UnknownVcr*)world.rings[n])->firstPosition=83+n*17;((UnknownVcrFile*)world.files[n])->entries[0].position=123+n*19;for(int i=0;i<128;++i)world.headers[n][i]=(BYTE)(scenario.seed+i*13+n*37);for(i=0;i<scenario.nameLength;++i)world.names[n][i]='A'+(i+n)%26;world.names[n][scenario.nameLength]=0;}
    for(n=0;n<4;++n)for(int i=0;i<1024;++i)world.buffers[n][i]=(BYTE)(scenario.seed+i*11+n*43);
    try{Invoke();}catch(int code){observed.exception=code;}observed.state=world;
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&observed,&expected,sizeof(observed))){BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int off=0;while(a[off]==b[off])++off;printf("Mismatch%d offset%d %02x/%02x seed%u prefix%d events%d/%d flags%x eof%d busy%d masks%d/%d pos%d size%u throw%d mutation%d/%d calls%d/%d\n",checks,off,a[off],b[off],scenario.seed,scenario.prefix,scenario.first,scenario.second,scenario.fail,scenario.eof,scenario.busy,scenario.writeMask,scenario.roomMask,scenario.position,scenario.size,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}
    if(observed.ended && (observed.held || (!scenario.mutateAt && Vcr()->isStopped) || observed.ended!=1 || (scenario.exitReturns?observed.result!=0:observed.exception!=0xe0410001)))Fail(97);
    if(observed.exception==0x741 && (!observed.count || observed.held!=observed.events[observed.count-1].held))Fail(98);
}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.prefix=1;scenario.first=0;scenario.second=10;scenario.size=17;scenario.headerSize=16;scenario.nameLength=31;scenario.position=123;scenario.peekAgain=71;scenario.handle=1;scenario.writeMask=3;scenario.roomMask=3;scenario.exitReturns=1;}
static int Count(int kind){int count=0;for(int n=0;n<observed.count;++n)if(observed.events[n].kind==kind)++count;return count;}
int main(int argc,char** argv){if(argc!=3 || !MapReference(argv[1],argv[2]))return 3;if(sizeof(VCRInterface)!=0xd8 || sizeof(UnknownVcr)!=0x101c || sizeof(UnknownVcrFile)!=0x248)return 4;
    Base();Compare();if(observed.result || observed.writes!=2 || Count(12)!=1 || Count(31)!=1 || !Vcr()->workerAcknowledged || !Vcr()->fileReady || Count(4)!=1)return 5;
    Base();scenario.prefix=4;scenario.first=3;Compare();if(observed.reads!=3 || Count(15)!=1 || Count(20)!=1 || Count(4)!=2 || !Vcr()->workerAcknowledged)return 6;
    Base();scenario.prefix=0;scenario.first=12;Compare();if(observed.waits!=1 || Count(31) || !observed.ended)return 7;
    Base();scenario.prefix=0;scenario.first=10;Compare();if(observed.waits!=1 || Count(31) || !observed.ended)return 8;
    Base();scenario.fail=1;Compare();if(Count(30)!=1 || Count(31) || observed.writes || Vcr()->workerAcknowledged || !Vcr()->fileReady || !observed.ended)return 10;
    Base();scenario.fail=2;Compare();if(observed.writes!=1 || Count(31)!=1 || Vcr()->workerAcknowledged || !observed.ended)return 11;
    Base();scenario.prefix=4;scenario.first=3;scenario.fail=4;scenario.eof=1;Compare();if(Count(16)!=1 || Count(15) || observed.reads!=2 || !observed.ended)return 12;
    Base();scenario.prefix=4;scenario.first=5;scenario.position=-1;Compare();if(Count(14)!=1 || Count(15)!=1 || world.buffers[0][0]!=0xff || Vcr()->recordTimeMs!=(int)0x82345678)return 13;
    const int prefix[]={0,1,2,4};const unsigned sizes[]={0,1,1023,1024,1025,0xffffffff};const int positions[]={0,-1,123,-7};
    for(int p=0;p<4;++p)for(int first=0;first<15;++first)for(int failure=0;failure<64;++failure)for(int size=0;size<6;++size){
        Base();scenario.seed=checks+1;scenario.prefix=prefix[p];scenario.first=first;scenario.second=(first+7)%15;scenario.fail=failure;scenario.size=sizes[size];scenario.position=positions[(failure+size)&3];scenario.peekAgain=(failure&1)?-1:71;scenario.eof=(failure>>1)&1;scenario.busy=(failure>>2)&1;scenario.writeMask=failure&3;scenario.roomMask=(failure>>2)&3;scenario.exitReturns=failure&1;Compare();
    }
    for(int first=0;first<11;++first){Base();scenario.prefix=4;scenario.first=first;scenario.second=10;Compare();int count=observed.count;
        for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<4;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
    for(int invalid=1;invalid<=4;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 9;}
    const int handles[]={-2147483647-1,-2,-1,0,1,7,2147483647};const int lengths[]={0,1,255,511};const int headers[]={0,1,16,127};
    for(int h=0;h<7;++h)for(int n=0;n<4;++n)for(int z=0;z<4;++z){Base();scenario.handle=handles[h];scenario.nameLength=lengths[n];scenario.headerSize=headers[z];scenario.second=9;Compare();}
    printf("RecorderWorker: %d complete event, data, lock, failure and termination scenarios match\n",checks);return 0;
}
