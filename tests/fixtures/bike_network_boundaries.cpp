// Actual Send/QueueRecord/FindRaceMode source bodies and production VCR ring.
// Transport and debug allocation are bounded test dependencies. Recorder locks
// and the write event use the real Win32 APIs, with no worker thread running.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/Recorder.h"
#include "../../src/reconstructed/VCR.h"

#define CHECK(n,x) typedef char n[(x)?1:-1]
CHECK(game_network,offsetof(TrackGame,network)==8);
CHECK(game_session,offsetof(TrackGame,eventManager)==0x570);
CHECK(owner_racer,offsetof(TrackGameViewOwner,followedRacer)==0xa8);
CHECK(owner_ghost,offsetof(TrackGameViewOwner,ball)==0xdc);
CHECK(recorder_size,sizeof(VCRInterface)==0xd8);
CHECK(recorder_lock,offsetof(VCRInterface,lock)==0x60);
CHECK(recorder_ring,offsetof(VCRInterface,frameRing)==0xb0);
CHECK(recorder_stop,offsetof(VCRInterface,isStopped)==0xb4);
CHECK(recorder_event,offsetof(VCRInterface,writeEvent)==0x30);
extern "C" int BikeRecorderPrimary();

static unsigned checks;
static BYTE packet[128],packetBefore[128];
struct SendCall { NetworkInterface* receiver; int kind,type,size,from,to,extraSize,extra; short sequence; void* data; BYTE bytes[128]; };
static SendCall sent;
static int sendCalls, sendResult;
void NetworkInterface::SendGuaranteed(int type,void* data,int size,int from) {
    sendCalls++;sent.receiver=this;sent.kind=1;sent.type=type;sent.size=size;sent.from=from;sent.data=data;
    if(size<0 || size>128) ExitProcess(90);memcpy(sent.bytes,data,size);
}
int NetworkInterface::SendRaw(int type,void* data,int size,short sequence,int from,int to,void* extra,int extraSize) {
    sendCalls++;sent.receiver=this;sent.kind=2;sent.type=type;sent.size=size;sent.sequence=sequence;
    sent.from=from;sent.to=to;sent.extraSize=extraSize;sent.data=data;
    if(size<0 || size>128 || (extraSize && extraSize!=4)) ExitProcess(91);
    sent.extra=extraSize?*(int*)extra:0;memcpy(sent.bytes,data,size);
    return sendResult;
}
static void TestSend() {
    DWORD words[(sizeof(NetworkInterface)+7)/4],before[(sizeof(NetworkInterface)+7)/4];
    NetworkInterface* network=(NetworkInterface*)words;
    const int types[]={1,10,13,0x87,0x80,-1},froms[]={0,3,-7},sizes[]={0,1,12,32};
    for(int t=0;t<6;t++) for(int f=0;f<3;f++) for(int to=0;to<2;to++)
    for(int mode=0;mode<2;mode++) for(int host=0;host<2;host++) for(int result=0;result<3;result++) for(int sz=0;sz<4;sz++) {
        memset(words,0xa5,sizeof(words));network->localPlayer=11;network->connectionMode=mode?3:0;
        network->isHost=host;network->serverPlayer=99;memcpy(before,words,sizeof(words));
        for(int i=0;i<128;i++) packet[i]=(BYTE)(i*37+t*13+f);memcpy(packetBefore,packet,128);
        memset(&sent,0,sizeof(sent));sendCalls=0;sendResult=result-1;
        int target=to?5:0,from=froms[f]?froms[f]:11;
        int answer=network->Send(types[t],packet,sizes[sz],froms[f],target);
        int guaranteed=(types[t]&0x80)!=0,relay=!guaranteed && mode && !host;
        if(answer!=(guaranteed || sendResult!=0) || sendCalls!=1 || sent.receiver!=network ||
           sent.kind!=(guaranteed?1:2) || sent.type!=types[t] || sent.size!=sizes[sz] || sent.from!=from ||
           sent.data!=packet || memcmp(sent.bytes,packet,sizes[sz]) || memcmp(words,before,sizeof(words)) || memcmp(packet,packetBefore,128)) ExitProcess(1);
        if(!guaranteed && (sent.to!=(relay?99:target) || sent.sequence!=0 || sent.extraSize!=(relay?4:0) || sent.extra!=(relay?target:0))) ExitProcess(2);
        checks++;
    }
}

static BYTE oldBlock[192],newBlock[192],expectedOld[192],expectedNew[192];
static unsigned allocations,frees,allocatedSize;
static int allocationFails;
void* DebugMalloc(unsigned int size,const char* file,int line) {
    if(!strstr(file,"VCR.cpp") || line!=83 || size>sizeof(newBlock)-16) ExitProcess(92);
    allocations++;allocatedSize=size;return allocationFails?0:newBlock+8;
}
void DebugFree(void* p,const char* file,int line) {
    if(p!=oldBlock+8 || !strstr(file,"VCR.cpp") || line!=82) ExitProcess(93);
    frees++;
}
static void TestQueue() {
    DWORD object[sizeof(VCRInterface)/4],ringWords[sizeof(UnknownVcr)/4],expectedRing[sizeof(UnknownVcr)/4];
    VCRInterface* recorder=(VCRInterface*)object;UnknownVcr* ring=(UnknownVcr*)ringWords;
    UnknownVcr* expected=(UnknownVcr*)expectedRing;
    memset(object,0,sizeof(object));InitializeCriticalSection(&recorder->lock);
    recorder->writeEvent=CreateEventA(0,FALSE,FALSE,0);if(!recorder->writeEvent) ExitProcess(94);
    const unsigned sizes[]={0,1,4,12,32,128};const int cursors[]={-1,0,254,255};
    for(int sizeIndex=0;sizeIndex<6;sizeIndex++) for(int cursor=0;cursor<4;cursor++)
    for(int state=0;state<4;state++) for(int stopped=0;stopped<2;stopped++) for(int present=0;present<2;present++)
    for(int capacity=0;capacity<4;capacity++) for(int failure=0;failure<2;failure++) {
        unsigned size=sizes[sizeIndex],needed=size+16;int next=cursors[cursor]+1;if(next==256)next=0;
        memset(ringWords,0x5a,sizeof(ringWords));ring->fillCursor=cursors[cursor];
        ring->slots[next].data=oldBlock+8;ring->slots[next].size=capacity==0?0:capacity==1?needed-1:capacity==2?needed:needed+17;
        ring->slots[next].state=(char)state;ring->slots[next].inUse=7;
        recorder->frameRing=present?ring:0;recorder->isStopped=stopped;
        memset(oldBlock,0xc5,sizeof(oldBlock));memset(newBlock,0xd7,sizeof(newBlock));
        for(int i=0;i<128;i++) packet[i]=(BYTE)(i*37+sizeIndex*13);memcpy(packetBefore,packet,128);
        memcpy(expectedOld,oldBlock,sizeof(oldBlock));memcpy(expectedNew,newBlock,sizeof(newBlock));memcpy(expectedRing,ringWords,sizeof(ringWords));
        allocations=frees=allocatedSize=0;allocationFails=failure;
        ResetEvent(recorder->writeEvent);
        bool reached=present && !stopped && state!=1;
        bool grow=capacity<2;
        bool success=reached && !(grow && failure);
        if(reached) {
            expected->fillCursor=next;
            if(success) {
                BYTE* output;
                if(grow) {
                    expected->slots[next].data=newBlock+8;expected->slots[next].size=size+12;output=expectedNew+8;
                } else output=expectedOld+8;
                *(unsigned*)output=size+12;((int*)output)[1]=0x87;((int*)output)[2]=-7;((unsigned*)output)[3]=size;
                memcpy(output+16,packet,size);expected->slots[next].inUse=0;expected->slots[next].state=1;
            }
        }
        int answer=recorder->QueueRecord(0x87,-7,packet,size);
        DWORD event=WaitForSingleObject(recorder->writeEvent,0);
        if(answer!=(int)success || event!=(success?WAIT_OBJECT_0:WAIT_TIMEOUT) ||
           allocations!=(unsigned)(reached && grow) || frees!=(unsigned)(reached && grow) ||
           (allocations && allocatedSize!=needed) || memcmp(ringWords,expectedRing,sizeof(ringWords)) ||
           memcmp(oldBlock,expectedOld,sizeof(oldBlock)) || memcmp(newBlock,expectedNew,sizeof(newBlock)) ||
           memcmp(packet,packetBefore,128) || recorder->lock.RecursionCount!=0 || recorder->lock.OwningThread!=0) {
            printf("queue mismatch %d %d %d %d %d %d %d\n",sizeIndex,cursor,state,stopped,present,capacity,failure);ExitProcess(3);
        }
        checks++;
    }
    CloseHandle(recorder->writeEvent);DeleteCriticalSection(&recorder->lock);
}

static DWORD gameWords[(sizeof(TrackGame)+3)/4],gameBefore[(sizeof(TrackGame)+3)/4];
TrackGame* g_TrackGame=(TrackGame*)gameWords;
static DWORD ownerWords[5][0xe0/4],ownersBefore[5][0xe0/4];
static void TestRaceMode() {
    DWORD managerWords[(sizeof(EventManager)+3)/4];memset(managerWords,0x39,sizeof(managerWords));
    EventManager* manager=(EventManager*)managerWords;
    for(int mask=0;mask<32;mask++) for(int seed=0;seed<3;seed++) {
        memset(gameWords,0x51,sizeof(gameWords));memset(ownerWords,0x37,sizeof(ownerWords));
        TrackGameViewOwner** fields[5]={&g_TrackGame->supercrossEvent,&g_TrackGame->quarryEvent,
            &g_TrackGame->bajaEnduroEvent,&g_TrackGame->nationalEvent,&g_TrackGame->tagEvent};
        TrackGameViewOwner* expected=0;
        for(int i=0;i<5;i++) {
            *fields[i]=(mask&(1<<i))?(TrackGameViewOwner*)ownerWords[i]:0;
            ownerWords[i][0xa8/4]=0x12340000+seed*0x100+i*4;
            ownerWords[i][0xdc/4]=0x45670000+seed*0x100+i*4;
            if(!expected && *fields[i]) expected=*fields[i];
        }
        memcpy(gameBefore,gameWords,sizeof(gameWords));memcpy(ownersBefore,ownerWords,sizeof(ownerWords));
        TrackGameViewOwner* answer=manager->FindRaceMode();
        if(answer!=expected || memcmp(gameWords,gameBefore,sizeof(gameWords)) || memcmp(ownerWords,ownersBefore,sizeof(ownerWords))) ExitProcess(4);
        if(answer && ((DWORD)answer->followedRacer!=((DWORD*)expected)[0xa8/4] || (DWORD)answer->ball!=((DWORD*)expected)[0xdc/4])) ExitProcess(5);
        checks++;
    }
}
int main() {
    if(BikeRecorderPrimary()!=0) return 6;
    TestSend();TestQueue();TestRaceMode();
    printf("Bike network: %u native send, recorder and race-owner cases passed\n",checks);
    return 0;
}
