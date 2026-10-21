#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stddef.h>
#include "BikeRaceCore.h"
#include "TrackGame.h"
#include "vehicle/KrustyBikeCore.h"
#include "collision/CollisionObject.h"
#include "Wrecker.h"
#include "MemTag.h"
TrackGame* g_TrackGame;
MemTagStack* g_MemTagStack=0;
int g_UnclaimedBytes=0;
typedef char BikeSize[(sizeof(KrustyBike)==0x1638)?1:-1];
typedef char Boundary[(offsetof(KrustyBike,bikeCollision)==0x1418&&offsetof(KrustyBike,bikeAndRiderCollision)==0x1414&&offsetof(KrustyBike,frontWheel)==0x5f0&&offsetof(KrustyBike,rearWheel)==0x5f4&&offsetof(KrustyBike,wrecker)==0x604&&offsetof(Wrecker,collisionModel)==0x38&&offsetof(CollisionObject,ignoreList)==0x78&&offsetof(CollisionObject,ignoreCount)==0x7c)?1:-1];
struct World {
 DWORD guard0[4]; BYTE bike[4][sizeof(KrustyBike)]; BYTE wrecker[4][sizeof(Wrecker)];
 BYTE collider[20][sizeof(CollisionObject)]; void* lists[20][32];
 BYTE race[sizeof(BikeRace)]; BYTE game[sizeof(TrackGame)]; BYTE owner[sizeof(TrackGameViewOwner)];
 UnknownBikeRaceRacer* racers[8]; DWORD guard1[4];
};
static World world,expected;
static int checks,faults,events;
static DWORD actualException,actualKind,actualAddress,expectedException,expectedKind,expectedAddress;
static KrustyBike* Bike(int i){return (KrustyBike*)world.bike[i];}
static CollisionObject* Collider(int i){return (CollisionObject*)world.collider[i];}
static void Fail(const char*msg){printf("%s at %d event%d expected%x/%u/%x actual%x/%u/%x\n",msg,checks,events,expectedException,expectedKind,expectedAddress,actualException,actualKind,actualAddress);ExitProcess(2);}
static void Prepare(int seed,int aliases,int filled){
 memset(&world,0xa5,sizeof(world));
 for(int b=0;b<4;++b){
  KrustyBike*k=Bike(b);int base=aliases==1?0:b*5;
  k->bikeCollision=Collider(base);k->bikeAndRiderCollision=Collider(aliases==2?base:base+1);
  k->frontWheel=(BikeWheel*)Collider(base+2);k->rearWheel=(BikeWheel*)Collider(aliases==3?base+2:base+3);
  k->wrecker=(Wrecker*)world.wrecker[b];k->wrecker->collisionModel=(UnknownWreckerCollisionModel*)Collider(base+4);
 }
 for(int n=0;n<20;++n){
  Collider(n)->ignoreCount=32;Collider(n)->ignoreList=(int)world.lists[aliases==4?n%5:n];
  memset(world.lists[n],0,sizeof(world.lists[n]));
  for(int j=0;j<filled;++j)world.lists[n][j*2]=(void*)Collider((seed+j+n)%20);
 }
 g_TrackGame=(TrackGame*)world.game;
 actualException=actualKind=actualAddress=expectedException=expectedKind=expectedAddress=0;events=0;
}
static void ExpectedFault(DWORD address,DWORD kind=0){expectedException=EXCEPTION_ACCESS_VIOLATION;expectedKind=kind;expectedAddress=address;}
static BYTE* ExpectedPointer(void*p){DWORD offset=(BYTE*)p-(BYTE*)&world;if(offset>=sizeof(world))Fail("unexpected model pointer");return (BYTE*)&expected+offset;}
static int AddExpected(CollisionObject*target,void*owner){
 ++events;if(!target){ExpectedFault(0x7c);return 0;}
 CollisionObject*t=(CollisionObject*)ExpectedPointer(target);
 if(!t->ignoreList){ExpectedFault(0);return 0;}
 void**list=(void**)ExpectedPointer((void*)t->ignoreList);
 for(int i=0;i<t->ignoreCount;++i)if(list[i]==owner)return 1;
 for(i=0;i<t->ignoreCount;++i)if(list[i]==0){list[i]=owner;return 1;}
 Fail("unexpected allocator path in prepared lists");return 0;
}
static CollisionObject* Part(KrustyBike*k,int part){
 if(part==0)return k->bikeCollision;if(part==1)return k->bikeAndRiderCollision;
 if(part==2)return (CollisionObject*)k->frontWheel;if(part==3)return (CollisionObject*)k->rearWheel;
 if(!k->wrecker){ExpectedFault(0x38);return 0;}return (CollisionObject*)k->wrecker->collisionModel;
}
static int NotifyExpected(KrustyBike*a,KrustyBike*b){
 if(!a){ExpectedFault(0x1418);return 0;}if(!a->bikeCollision)return 1;
 if(!b){ExpectedFault(0x1418);return 0;}
 for(int pass=0;pass<2;++pass){KrustyBike*source=pass?b:a;KrustyBike*destination=pass?a:b;
  for(int target=0;target<5;++target)for(int owner=0;owner<5;++owner){
   CollisionObject*value=Part(source,owner);if(expectedException)return 0;
   CollisionObject*dest=Part(destination,target);if(expectedException)return 0;
   if(!AddExpected(dest,value))return 0;
  }
 }return 1;
}
static int Filter(EXCEPTION_POINTERS*e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 actualException=e->ExceptionRecord->ExceptionCode;actualKind=e->ExceptionRecord->ExceptionInformation[0];actualAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Finish(){++checks;
 if(actualException!=expectedException||actualKind!=expectedKind||actualAddress!=expectedAddress)Fail("exception");
 if(memcmp(&world,&expected,sizeof(world)))Fail("whole state");
 if(actualException)++faults;if(g_UnclaimedBytes!=0)Fail("unexpected allocation");
}
static void ProviderCases(){
 for(int seed=0;seed<16;++seed)for(int alias=0;alias<5;++alias)for(int filled=0;filled<5;++filled)
 for(int self=0;self<2;++self)for(int role=0;role<18;++role){
  Prepare(seed,alias,filled);KrustyBike*a=Bike(0),*b=self?a:Bike(1);
  if(role==1)a=0;if(role==2)b=0;if(role==3){a->bikeCollision=0;b=0;}
  if(role>=4&&role<=7){int part=role-3;if(part==1)b->bikeAndRiderCollision=0;if(part==2)b->frontWheel=0;if(part==3)b->rearWheel=0;if(part==4)b->wrecker->collisionModel=0;}
  if(role==8)a->wrecker=0;if(role==9)b->wrecker=0;
  if(role>=10){int index=role-10;Collider(index)->ignoreList=0;}
  expected=world;NotifyExpected(a,b);
  __try{a->Fn_00496F90(b);}__except(Filter(GetExceptionInformation())){}
  Finish();
 }
}
static void AnnounceCases(){
 for(int seed=0;seed<4;++seed)for(int alias=0;alias<5;++alias)for(int mode=0;mode<3;++mode)
 for(int blocked=0;blocked<2;++blocked)for(int newIndex=-1;newIndex<4;++newIndex)for(int oldIndex=-1;oldIndex<4;++oldIndex)for(int hole=0;hole<2;++hole){
  Prepare(seed,alias,seed);BikeRace*r=(BikeRace*)world.race;TrackGameViewOwner*v=(TrackGameViewOwner*)world.owner;
  for(int i=0;i<8;++i)world.racers[i]=(UnknownBikeRaceRacer*)Bike(i%4);
  if(hole)world.racers[2]=0;
  r->localRacer=(UnknownBikeRaceRacer*)Bike(0);r->aiRacers=world.racers+1;r->racerSlots=world.racers;
  v->raceView=(UnknownKrustyBikeView*)r;g_TrackGame->playerCount=mode==2?2:1;g_TrackGame->uiInteractionBlocked=blocked;
  g_TrackGame->mode.raceSettings.raceMode=mode==1?4:0;g_TrackGame->mode.raceSettings.aiRacerCount=3;g_TrackGame->mode.raceSettings.multiplayerAiRacerCount=2;
  KrustyBike*a=newIndex<0?0:Bike(newIndex),*old=oldIndex<0?0:Bike(oldIndex);expected=world;
  int limit=mode==1?2:4;
  if(!blocked)for(i=0;i<limit;++i){KrustyBike*other=(KrustyBike*)world.racers[i];if(!other)break;if(other==a)continue;
   if(a)NotifyExpected(a,other);if(old&&old!=other)NotifyExpected(other,old);if(a)NotifyExpected(other,a);
  }
  __try{v->AnnounceFollowedRacer((UnknownEventRacer*)a,(UnknownEventRacer*)old);}__except(Filter(GetExceptionInformation())){}
  Finish();
 }
}
static void AllocationCases(){
 for(int count=0;count<16;++count)for(int duplicate=0;duplicate<2;++duplicate){
  BYTE bytes[sizeof(CollisionObject)+32];memset(bytes,0xa5,sizeof(bytes));CollisionObject*c=(CollisionObject*)(bytes+16);
  void**old=count?(void**)malloc(count*4):0;for(int i=0;i<count;++i)old[i]=(void*)(0x1000+i*4);
  c->ignoreList=(int)old;c->ignoreCount=count;void*owner=duplicate&&count?old[count/2]:(void*)0x23456;
  // The genuine allocator accounts the CRT's full old allocation size.
  g_UnclaimedBytes=old?_msize(old):0;
  BYTE before[sizeof(bytes)];memcpy(before,bytes,sizeof(bytes));
  int oldAccounting=g_UnclaimedBytes;
  c->AddIgnoredOwner(owner);void**now=(void**)c->ignoreList;
  int expectedCount=count+(duplicate&&count?0:1);
  if(c->ignoreCount!=expectedCount)Fail("allocation count");
  if(g_UnclaimedBytes!=(expectedCount>count?expectedCount*4:oldAccounting))Fail("allocation accounting");
  CollisionObject*prior=(CollisionObject*)(before+16);prior->ignoreList=c->ignoreList;prior->ignoreCount=c->ignoreCount;
  if(memcmp(before,bytes,sizeof(bytes)))Fail("allocation receiver guards");
  for(i=0;i<count;++i)if(now[i]!=(void*)(0x1000+i*4))Fail("allocation prefix");
  if(expectedCount>count&&now[count]!=owner)Fail("allocation append");
  for(i=0;i<16;++i)if(bytes[i]!=0xa5||bytes[16+sizeof(CollisionObject)+i]!=0xa5)Fail("allocation guards");
  free(now);g_UnclaimedBytes=0;++checks;
 }
}
int main(){ProviderCases();AnnounceCases();AllocationCases();printf("%d actual notification cases, %d independently modeled AVs\n",checks,faults);return 0;}
