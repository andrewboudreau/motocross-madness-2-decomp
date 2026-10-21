// Actual message/queue/finder/string providers. Only allocation and LoadStringA
// are scripted; the temporary object's release and allocation faults are observed.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/DebugAlloc.h"
#define CHECK(n,x) typedef char n[(x)?1:-1]
CHECK(message_size,sizeof(UnknownMessage)==0x8c);
CHECK(message_time,offsetof(UnknownMessage,showSeconds)==0x80);
CHECK(message_age,offsetof(UnknownMessage,shownSeconds)==0x84);
CHECK(message_next,offsetof(UnknownMessage,next)==0x88);
CHECK(queue_size,sizeof(TextQueueOverlay)==0x58);
CHECK(queue_head,offsetof(TextQueueOverlay,queueHead)==0x50);
CHECK(owner_queue,offsetof(TrackGameViewOwner,textQueue)==0x6c);
extern "C" int BikeTextLayouts();

static unsigned checks;
static DWORD gameWords[(sizeof(TrackGame)+3)/4],gameBefore[(sizeof(TrackGame)+3)/4];
TrackGame* g_TrackGame=(TrackGame*)gameWords;
static DWORD pool[43],expectedPool[43];
static int allocationFails,allocations,releases,lastLine,unwinds;
static void* released;
void* operator new(unsigned int size,const char* file,int line) {
    if(size!=0x8c || !strstr(file,"TrackOverlay") || (line!=1451 && line!=1453)) ExitProcess(90);
    allocations++;lastLine=line;return allocationFails?0:pool+4;
}
void operator delete(void* memory) { releases++;released=memory; }
void operator delete(void* memory,const char*,int) { unwinds++; }
static DWORD faultCode,faultAddress,faultKind;
static int Fault(EXCEPTION_POINTERS* p) {
    faultCode=p->ExceptionRecord->ExceptionCode;
    faultKind=p->ExceptionRecord->ExceptionInformation[0];
    faultAddress=p->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
static int Replace(TextQueueOverlay* queue,UnknownMessage* message) {
    __try {queue->ReplaceHeadMessage(message);return 1;}
    __except(Fault(GetExceptionInformation())) {return 0;}
}
static void MakeText(char* out,int length,int seed) {
    for(int i=0;i<length;i++)out[i]=(char)('A'+(i+seed)%26);
    out[length]=0;
}
static void TestConstructor() {
    const int lengths[]={0,1,15,127};const DWORD durations[]={0,0x40500000,0x80000000,0x7fc12345};
    char input[160],beforeInput[160];DWORD object[43],expected[43];
    for(int seed=0;seed<64;seed++) for(int l=0;l<4;l++) for(int duration=0;duration<4;duration++) {
        memset(input,0x61,sizeof(input));MakeText(input,lengths[l],seed);memcpy(beforeInput,input,sizeof(input));
        memset(object,seed,sizeof(object));memcpy(expected,object,sizeof(object));
        char* e=(char*)(expected+4);memcpy(e,input,lengths[l]+1);
        ((DWORD*)e)[0x80/4]=durations[duration];((DWORD*)e)[0x84/4]=0;((DWORD*)e)[0x88/4]=0;
        // Invoke the actual constructor through a fixture helper using placement
        // new, so it receives precisely this guarded storage and float bit pattern.
        extern UnknownMessage* ConstructMessage(void*,const char*,float);
        UnknownMessage* answer=ConstructMessage(object+4,input,*(float*)&durations[duration]);
        if(answer!=(UnknownMessage*)(object+4) || memcmp(object,expected,sizeof(object)) ||
           memcmp(input,beforeInput,sizeof(input))) ExitProcess(1);
        checks++;
    }
}
static void TestReplace() {
    const int lengths[]={0,1,15,127};
    DWORD queueWords[sizeof(TextQueueOverlay)/4],expectedQueue[sizeof(TextQueueOverlay)/4];
    DWORD inputWords[35],headWords[35],tailWords[35],beforeInput[35],beforeHead[35],beforeTail[35];
    TextQueueOverlay* queue=(TextQueueOverlay*)queueWords;
    UnknownMessage* input=(UnknownMessage*)inputWords;
    UnknownMessage* head=(UnknownMessage*)headWords;
    UnknownMessage* tail=(UnknownMessage*)tailWords;
    for(int seed=0;seed<64;seed++) for(int l=0;l<4;l++) for(int mode=0;mode<4;mode++) for(int fail=0;fail<2;fail++) {
        memset(queueWords,0x58,sizeof(queueWords));memset(pool,0xa6,sizeof(pool));
        memset(inputWords,0x21,sizeof(inputWords));memset(headWords,0x32,sizeof(headWords));memset(tailWords,0x43,sizeof(tailWords));
        MakeText(input->line,lengths[l],seed);MakeText(head->line,lengths[l],seed+1);MakeText(tail->line,lengths[l],seed+2);
        input->showSeconds=3.25f;head->showSeconds=-2.0f;tail->showSeconds=1.5f;
        input->shownSeconds=19;head->shownSeconds=29;tail->shownSeconds=39;
        input->next=tail;head->next=tail;tail->next=0;
        queue->queueHead=mode?head:0;
        UnknownMessage* source=mode==2?head:mode==3?tail:input;
        memcpy(expectedQueue,queueWords,sizeof(queueWords));memcpy(expectedPool,pool,sizeof(pool));
        memcpy(beforeInput,inputWords,sizeof(inputWords));memcpy(beforeHead,headWords,sizeof(headWords));memcpy(beforeTail,tailWords,sizeof(tailWords));
        allocations=releases=unwinds=0;released=0;allocationFails=fail;lastLine=0;faultCode=faultAddress=faultKind=0;
        if(!fail) {
            UnknownMessage* e=(UnknownMessage*)(expectedPool+4);
            strcpy(e->line,source->line);e->showSeconds=source->showSeconds;e->shownSeconds=0;e->next=mode?tail:0;
            ((TextQueueOverlay*)expectedQueue)->queueHead=(UnknownMessage*)(pool+4);
        }
        int answer=Replace(queue,source);
        if(answer!=(fail && mode?0:1) || allocations!=1 || lastLine!=(mode?1453:1451) || unwinds ||
           releases!=(!fail && mode?1:0) || (releases && released!=head) ||
           memcmp(queueWords,expectedQueue,sizeof(queueWords)) || memcmp(pool,expectedPool,sizeof(pool)) ||
           memcmp(inputWords,beforeInput,sizeof(inputWords)) || memcmp(headWords,beforeHead,sizeof(headWords)) ||
           memcmp(tailWords,beforeTail,sizeof(tailWords))) ExitProcess(2);
        if(fail && mode && (faultCode!=EXCEPTION_ACCESS_VIOLATION || faultKind!=1 || faultAddress!=0x88)) ExitProcess(3);
        if((!fail || !mode) && faultCode) ExitProcess(4);
        checks++;
    }
}
static DWORD owners[5][0x70/4],beforeOwners[5][0x70/4];
static void TestFinder() {
    DWORD manager[sizeof(EventManager)/4];memset(manager,0x91,sizeof(manager));
    for(int mask=0;mask<32;mask++) for(int seed=0;seed<3;seed++) {
        memset(gameWords,0x61,sizeof(gameWords));memset(owners,0x72,sizeof(owners));
        TrackGameViewOwner** fields[]={&g_TrackGame->supercrossEvent,&g_TrackGame->quarryEvent,&g_TrackGame->bajaEnduroEvent,&g_TrackGame->nationalEvent,&g_TrackGame->tagEvent};
        TextQueueOverlay* expected=0;int selected=0;
        for(int i=0;i<5;i++) {
            *fields[i]=(mask&(1<<i))?(TrackGameViewOwner*)owners[i]:0;
            ((TrackGameViewOwner*)owners[i])->textQueue=(TextQueueOverlay*)(seed==0?0:0x12340000+seed*0x100+i*4);
            if(!selected && *fields[i]) {selected=1;expected=(*fields[i])->textQueue;}
        }
        memcpy(gameBefore,gameWords,sizeof(gameWords));memcpy(beforeOwners,owners,sizeof(owners));
        if(((EventManager*)manager)->FindTextQueue()!=expected || memcmp(gameBefore,gameWords,sizeof(gameWords)) ||
           memcmp(beforeOwners,owners,sizeof(owners))) ExitProcess(5);
        checks++;
    }
}
static int loadCalls,loadMode,loadSize;static UINT loadId;static HINSTANCE loadInstance;static char* loadBuffer;
static char resourceText[128];
static int __stdcall ScriptLoadString(HINSTANCE instance,UINT id,char* buffer,int size) {
    loadCalls++;loadInstance=instance;loadId=id;loadBuffer=buffer;loadSize=size;
    if(!loadMode || size<=0)return 0;
    int n=strlen(resourceText);if(n>=size)n=size-1;
    memcpy(buffer,resourceText,n);buffer[n]=0;return n;
}
static void PatchLoadString() {
    BYTE* image=(BYTE*)GetModuleHandleA(0);IMAGE_NT_HEADERS* nt=(IMAGE_NT_HEADERS*)(image+((IMAGE_DOS_HEADER*)image)->e_lfanew);
    IMAGE_IMPORT_DESCRIPTOR* import=(IMAGE_IMPORT_DESCRIPTOR*)(image+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    int patched=0;
    for(;import->Name;import++) {
        DWORD* names=(DWORD*)(image+import->OriginalFirstThunk),*cells=(DWORD*)(image+import->FirstThunk);
        for(;*names;names++,cells++) if(!(*names&IMAGE_ORDINAL_FLAG)) {
            IMAGE_IMPORT_BY_NAME* name=(IMAGE_IMPORT_BY_NAME*)(image+*names);
            if(!strcmp((char*)name->Name,"LoadStringA")) {
                DWORD old;if(!VirtualProtect(cells,4,PAGE_READWRITE,&old))ExitProcess(91);
                *cells=(DWORD)ScriptLoadString;DWORD ignored;VirtualProtect(cells,4,old,&ignored);patched++;
            }
        }
    }
    if(patched!=1)ExitProcess(92);
}
static void TestStrings() {
    const int sizes[]={0,1,8,128},ids[]={0,0x14d7,-1};char storage[256],expected[256];
    for(int seed=0;seed<32;seed++) for(int id=0;id<3;id++) for(int instance=0;instance<2;instance++)
    for(int capacity=0;capacity<4;capacity++) for(int mode=0;mode<2;mode++) {
        memset(gameWords,0x63,sizeof(gameWords));g_TrackGame->resourceInstance=instance?(void*)0x12340000:0;
        memcpy(gameBefore,gameWords,sizeof(gameWords));memset(storage,0x54,sizeof(storage));memcpy(expected,storage,sizeof(storage));
        MakeText(resourceText,1+seed*3,seed);loadCalls=0;loadMode=mode;
        int n=strlen(resourceText);if(n>=sizes[capacity])n=sizes[capacity]-1;
        int success=instance && mode && n>0;
        if(success){memcpy(expected+16,resourceText,n);expected[16+n]=0;}
        else strcpy(expected+16,"Resource String Unavailable");
        int answer=g_TrackGame->LoadResourceString(ids[id],storage+16,sizes[capacity]);
        if(answer!=success || loadCalls!=instance || memcmp(storage,expected,sizeof(storage)) ||
           memcmp(gameBefore,gameWords,sizeof(gameWords)))ExitProcess(6);
        if(instance && (loadInstance!=(HINSTANCE)0x12340000 || loadId!=(UINT)ids[id] || loadBuffer!=storage+16 || loadSize!=sizes[capacity]))ExitProcess(7);
        checks++;
    }
}
int main() {
    if(BikeTextLayouts()!=0)return 8;
    PatchLoadString();TestConstructor();TestReplace();TestFinder();TestStrings();
    printf("Bike text: %u native message, queue, owner and resource cases passed\n",checks);
    return 0;
}
