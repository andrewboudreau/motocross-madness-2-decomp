#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <malloc.h>
#include "../../src/reconstructed/Wrecker.h"
#include "../../src/krusty2/constraint/ConstraintMethodCollisionModel.h"
#include "../../src/reconstructed/MemTag.h"

extern int g_UnclaimedBytes;
typedef char Layout[(sizeof(Wrecker)==0x53c && sizeof(ConstraintMethodCollisionModel)==0x10c &&
    sizeof(UnknownWreckerCollisionModel)==0x10c && sizeof(ConstraintProbe)==32 &&
    sizeof(Vector3)==12 && sizeof(ConVec3)==12 &&
    offsetof(Wrecker,collisionModel)==0x38 && offsetof(Wrecker,probePositions)==0x104 &&
    offsetof(Wrecker,probeHits)==0x108 && offsetof(ConstraintMethodCollisionModel,body)==0xc4 &&
    offsetof(ConstraintMethodCollisionModel,probeCount)==0x100 &&
    offsetof(ConstraintMethodCollisionModel,probes)==0x104 && offsetof(ConBody,node)==0x234 &&
    offsetof(UnknownWreckerRigidBody,initialVelocity)==0x13c)?1:-1];

static void Check(int value,const char* label) { if(!value){printf("FAIL %s\n",label);ExitProcess(7);} }
struct Block { void* p; unsigned size,actual; int alive; };
static Block blocks[12];
static unsigned requests[4];
static int active,blockCount,allocationCount,freeCount,failAt;
extern "C" void* __cdecl FixtureMalloc(unsigned size) {
    if(!active)return malloc(size);
    int index=allocationCount++;
    Check(index<4,"allocation count");requests[index]=size;
    if(index==failAt)return 0;
    void* p=malloc(size);Check(p!=0,"real malloc");
    Check(blockCount<12,"block count");Block& b=blocks[blockCount++];
    b.p=p;b.size=size;b.actual=_msize(p);b.alive=1;memset(p,0xcd,size);return p;
}
extern "C" void __cdecl FixtureFree(void* p) {
    if(active&&p){int found=0;for(int i=0;i<blockCount;i++)if(blocks[i].p==p&&blocks[i].alive){blocks[i].alive=0;found=1;break;}
        Check(found,"free ownership");++freeCount;}
    free(p);
}
extern "C" unsigned __cdecl FixtureMsize(void* p){return _msize(p);}

static BYTE receiver[0x53c+64],model[0x10c+64],receiverBefore[0x53c],modelBefore[0x10c];
static BYTE* page;
static DWORD exceptionKind,exceptionAccess,exceptionAddress;
static int Filter(EXCEPTION_POINTERS* e){
    exceptionKind=e->ExceptionRecord->ExceptionCode;
    Check(exceptionKind==EXCEPTION_ACCESS_VIOLATION,"only expected AV");
    exceptionAccess=e->ExceptionRecord->ExceptionInformation[0];
    exceptionAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Invoke(Wrecker* w,const Vector3& point,DWORD node){
    exceptionKind=exceptionAccess=exceptionAddress=0;
    __try { w->AddProbe(point,(int)node); } __except(Filter(GetExceptionInformation())) {}
}
static DWORD Next(DWORD& seed){seed=seed*1664525+1013904223;return seed;}
static void Begin(){
    memset(blocks,0,sizeof(blocks));memset(requests,0,sizeof(requests));
    blockCount=allocationCount=freeCount=0;failAt=-1;active=1;
    memset(receiver,0xa5,sizeof(receiver));memset(model,0xa5,sizeof(model));
}
static void Finish(){
    active=0;for(int i=0;i<blockCount;i++)if(blocks[i].alive)free(blocks[i].p);
}
static void* Initial(unsigned bytes,unsigned char value){
    if(!bytes)return 0;void* p=malloc(bytes);Check(p!=0,"initial malloc");
    Block& b=blocks[blockCount++];b.p=p;b.size=bytes;b.actual=_msize(p);b.alive=1;memset(p,value,bytes);return p;
}
static void ExpectedRecord(ConstraintProbe* p,const DWORD* xyz,DWORD node){
    memset(p,0xcd,32);memcpy(&p->localPoint,xyz,12);p->node=(ConNode*)node;p->hit=0;
}
static void Normal(int count,DWORD seed,int fromProbe,int tagged){
    Begin();Wrecker* w=(Wrecker*)(receiver+32);ConstraintMethodCollisionModel* m=(ConstraintMethodCollisionModel*)(model+32);
    w->collisionModel=(UnknownWreckerCollisionModel*)m;m->probeCount=count;
    m->probes=(ConstraintProbe*)Initial(count*32,0x61);w->probePositions=(Vector3*)Initial(count*12,0x72);w->probeHits=(int*)Initial(count*4,0x83);
    DWORD xyz[3]={Next(seed),Next(seed),Next(seed)},node=Next(seed);Vector3 local;memcpy(&local,xyz,12);
    if(fromProbe&&count)memcpy(&m->probes[count-1].localPoint,xyz,12);
    BYTE oldProbe[32*31],oldPosition[12*31],oldHit[4*31];
    if(count){memcpy(oldProbe,m->probes,count*32);memcpy(oldPosition,w->probePositions,count*12);memcpy(oldHit,w->probeHits,count*4);}
    memcpy(receiverBefore,w,sizeof(Wrecker));memcpy(modelBefore,m,sizeof(*m));
    unsigned oldBytes=0;for(int i=0;i<blockCount;i++)oldBytes+=blocks[i].actual;
    int totals[3]={11,22,100000};BYTE tagBytes[sizeof(MemTagStack)];memset(tagBytes,0,sizeof(tagBytes));
    MemTagStack* tag=(MemTagStack*)tagBytes;tag->current=2;tag->ours=totals;
    g_MemTagStack=tagged?tag:0;g_UnclaimedBytes=100000;
    const Vector3& argument=(fromProbe&&count)?*(Vector3*)&m->probes[count-1].localPoint:local;
    Invoke(w,argument,node);Check(!exceptionKind,"ordinary success");
    Check(allocationCount==3&&freeCount==(count?3:0),"exact allocation/free events");
    Check(requests[0]==unsigned((count+1)*32)&&requests[1]==unsigned((count+1)*12)&&requests[2]==unsigned((count+1)*4),"allocation sizes");
    int expected=100000-oldBytes+(count+1)*48;
    Check((tagged?totals[2]:g_UnclaimedBytes)==expected,"real allocator accounting");
    Check(totals[0]==11&&totals[1]==22&&(tagged?g_UnclaimedBytes==100000:totals[2]==100000),"other accounting untouched");
    if(count){Check(!memcmp(m->probes,oldProbe,count*32),"old probes");Check(!memcmp(w->probePositions,oldPosition,count*12),"old positions");Check(!memcmp(w->probeHits,oldHit,count*4),"old hits");}
    ConstraintProbe expectedProbe;ExpectedRecord(&expectedProbe,xyz,node);
    Check(!memcmp(&m->probes[count],&expectedProbe,32),"complete appended record");
    DWORD zero[3]={0,0,0};Check(!memcmp(&w->probePositions[count],zero,12)&&w->probeHits[count]==0,"new caller arrays");
    ((Wrecker*)receiverBefore)->probePositions=w->probePositions;((Wrecker*)receiverBefore)->probeHits=w->probeHits;
    ((ConstraintMethodCollisionModel*)modelBefore)->probes=m->probes;((ConstraintMethodCollisionModel*)modelBefore)->probeCount=count+1;
    Check(!memcmp(w,receiverBefore,sizeof(Wrecker))&&!memcmp(m,modelBefore,sizeof(*m)),"full receiver state");
    for(i=0;i<32;i++)Check(receiver[i]==0xa5&&receiver[32+sizeof(Wrecker)+i]==0xa5&&model[i]==0xa5&&model[32+sizeof(*m)+i]==0xa5,"object guards");
    Finish();
}

// All failed allocations start with count zero: no fallback into an undersized
// old allocation is exercised. Guard cases stop at each independently known
// first read; successful earlier appends and real allocation ownership remain
// observable. Receiver storage is prepared bytes, not a constructed lifetime.
static void Fault(int mode){
    Begin();Wrecker* w=(Wrecker*)(receiver+32);ConstraintMethodCollisionModel* m=(ConstraintMethodCollisionModel*)(model+32);
    unsigned wbytes=sizeof(Wrecker),mbytes=sizeof(*m);int completed=0;DWORD address=0,access=0;
    if(mode<3){wbytes=mode==0?0x38:mode==1?0x104:0x108;w=(Wrecker*)(page+4096-wbytes);memset(w,0xa5,wbytes);completed=mode;address=(DWORD)(page+4096);}
    else if(mode<5){mbytes=mode==3?0x100:0x104;m=(ConstraintMethodCollisionModel*)(page+4096-mbytes);memset(m,0xa5,mbytes);address=(DWORD)(page+4096);}
    else {failAt=mode-5;completed=failAt;address=failAt==0?12:0;access=1;}
    if(wbytes>=0x3c)w->collisionModel=(UnknownWreckerCollisionModel*)m;
    if(wbytes>=0x108)w->probePositions=0;
    if(wbytes>=0x10c)w->probeHits=0;
    if(mbytes>=0x104)m->probeCount=0;
    if(mbytes>=0x108)m->probes=0;
    memcpy(receiverBefore,w,wbytes);memcpy(modelBefore,m,mbytes);
    g_MemTagStack=0;g_UnclaimedBytes=0;
    Vector3 point;DWORD xyz[3]={0x80000000,0x7fc12345,0x3f800000};memcpy(&point,xyz,12);
    Invoke(w,point,0xffffffff);Check(exceptionKind==EXCEPTION_ACCESS_VIOLATION&&exceptionAddress==address&&exceptionAccess==access,"first fault kind/address/access");
    int allocations=completed+(mode>=5?1:0);Check(allocationCount==allocations&&freeCount==0,"partial allocation events");
    Check(blockCount==completed,"successful allocation count");
    unsigned sizes[3]={32,12,4};for(int i=0;i<allocations;i++)Check(requests[i]==sizes[i],"partial requests");
    if(completed>=1){
        ((ConstraintMethodCollisionModel*)modelBefore)->probes=(ConstraintProbe*)blocks[0].p;
        ((ConstraintMethodCollisionModel*)modelBefore)->probeCount=1;
        ConstraintProbe expected;ExpectedRecord(&expected,xyz,0xffffffff);Check(!memcmp(blocks[0].p,&expected,32),"partial completed probe");
    }
    if(completed>=2){((Wrecker*)receiverBefore)->probePositions=(Vector3*)blocks[1].p;DWORD zero[3]={0,0,0};Check(!memcmp(blocks[1].p,zero,12),"partial completed position");}
    Check(!memcmp(w,receiverBefore,wbytes)&&!memcmp(m,modelBefore,mbytes),"partial full accessible state");
    Check(g_UnclaimedBytes==(completed==2?44:completed==1?32:0),"partial accounting");Finish();
}
int main(){
    MemTagStack* original=g_MemTagStack;int oldUnclaimed=g_UnclaimedBytes;
    page=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);Check(page!=0,"page");DWORD old;Check(VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old),"guard");
    int counts[5]={0,1,7,15,31};int cases=0;
    for(int c=0;c<5;c++)for(int seed=0;seed<128;seed++)for(int alias=0;alias<2;alias++)for(int tag=0;tag<2;tag++){Normal(counts[c],0x9e3779b9u+seed,alias,tag);++cases;}
    for(int mode=0;mode<8;mode++){Fault(mode);++cases;}
    g_MemTagStack=original;g_UnclaimedBytes=oldUnclaimed;VirtualFree(page,0,MEM_RELEASE);
    printf("Wrecker probes: %d cases, 8 independently modeled access faults\n",cases);return 0;
}
