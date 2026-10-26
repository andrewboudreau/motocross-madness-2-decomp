// Actual strict SetDetailLevel and all three source helpers, independent state oracle.
#include "../../src/reconstructed/BikeRace.h"
#include "../../src/reconstructed/D3DIMSoulTree.h"
#include <windows.h>
#include <stddef.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do { if (!(x)) { printf("failed %d case %d\n",__LINE__,cases); exit(1); } } while(0)
#define LAYOUT(n,e) typedef char n[(e)?1:-1]
LAYOUT(race_local,offsetof(BikeRace,localRacer)==0x38);
LAYOUT(race_net,offsetof(BikeRace,racerSlots)==0x3c);
LAYOUT(race_ai,offsetof(BikeRace,aiRacers)==0x40);
LAYOUT(racer_model,offsetof(UnknownBikeRaceRacer,modelNode)==0x3bc);
LAYOUT(racer_rider,offsetof(UnknownBikeRaceRacer,riderCharacter)==0x5c4);
LAYOUT(rider_model,offsetof(UnknownBikeRaceRider,modelNode)==0x1a0);
LAYOUT(model_size,sizeof(D3DIMSoultreeObject)==0x2d8);
LAYOUT(model_threshold,offsetof(D3DIMSoultreeObject,inverseLodThreshold)==0x284);
LAYOUT(model_dirty,offsetof(D3DIMSoultreeObject,subtreeDirty)==0x18c);
LAYOUT(model_light,offsetof(D3DIMSoultreeObject,useLighting)==0x1a4);
LAYOUT(model_texture,offsetof(D3DIMSoultreeObject,texturePerspective)==0x1b4);
LAYOUT(model_shade,offsetof(D3DIMSoultreeObject,gouraudShading)==0x1b8);
LAYOUT(setting_size,sizeof(SoultreeLodSetting)==16);
template<class T> struct Raw { DWORD words[(sizeof(T)+3)/4+8]; T* ptr(){return (T*)(words+4);} };
struct State {
    Raw<BikeRace> race;
    Raw<TrackGame> game;
    Raw<UnknownBikeRaceRacer> racers[5];
    Raw<UnknownBikeRaceRider> riders[5];
    Raw<D3DIMSoultreeObject> models[10];
    DWORD arrayGuard0[4]; UnknownBikeRaceRacer* ai[2]; DWORD arrayGuard1[4];
    UnknownBikeRaceRacer* net[2]; DWORD arrayGuard2[4];
};
static State state, expected;
TrackGame* g_TrackGame;
static BYTE* pages;
static BYTE pageExpected[4096];
static int cases, faults;
static DWORD exceptionCode, exceptionAddress, exceptionAccess;
static const DWORD reciprocals[2][10] = {
 {0x3e3a2e8c,0x3e4ccccd,0x3e638e39,0x3e800000,0x3e924925,0x3eaaaaab,0x3f000000,0x3f2aaaab,0x3f800000,0x3f800000},
 {0x3e124925,0x3e124925,0x3e2aaaab,0x3e2aaaab,0x3e4ccccd,0x3e800000,0x3e800000,0x3eaaaaab,0x3f000000,0x3f800000}};
static const int texture[2][10]={{1,1,1,1,1,1,1,1,1,1},{0,0,0,0,0,1,1,1,1,1}};
static const int lighting[2][10]={{1,1,1,1,1,1,1,1,1,1},{0,0,0,1,1,1,1,1,1,1}};
static const int shading[2][10]={{1,1,1,1,1,1,1,1,1,1},{0,0,0,0,0,0,1,1,1,1}};
static int Filter(EXCEPTION_POINTERS* info) {
 exceptionCode=info->ExceptionRecord->ExceptionCode;
 if(exceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 exceptionAccess=info->ExceptionRecord->ExceptionInformation[0];
 exceptionAddress=info->ExceptionRecord->ExceptionInformation[1];
 return EXCEPTION_EXECUTE_HANDLER;
}
static void Invoke(int level){
 exceptionCode=exceptionAddress=exceptionAccess=0;
 __try {state.race.ptr()->SetDetailLevel(level);}
 __except(Filter(GetExceptionInformation())){}
}
static void Fill(void* target,unsigned int size,unsigned int seed){
 BYTE* p=(BYTE*)target;
 for(unsigned int i=0;i<size;i++){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;p[i]=(BYTE)seed;}
}
static void ModelOracle(int index,int blade,int level){
 BYTE* p=(BYTE*)expected.models[index].ptr();
 *(DWORD*)(p+0x284)=reciprocals[blade][level];
 *(DWORD*)(p+0x18c)=1;
 *(DWORD*)(p+0x1a4)=lighting[blade][level];
 *(DWORD*)(p+0x1b4)=texture[blade][level];
 *(DWORD*)(p+0x1b8)=shading[blade][level];
}
static void Run(int level,int blade,int mode,int aiCount,int netCount,int alias,int seed,int failAt){
 // A write AV can retain an x87 temporary; each isolated scenario starts clean.
 _fpreset(); _controlfp(_MCW_EM|_RC_NEAR|_PC_64,_MCW_EM|_MCW_RC|_MCW_PC);
 Fill(&state,sizeof(state),seed);
 g_TrackGame=state.game.ptr();
 g_TrackGame->bladeRenderer=blade? -7:0;
 g_TrackGame->mode.raceSettings.raceMode=mode;
 g_TrackGame->mode.raceSettings.aiRacerCount=aiCount;
 g_TrackGame->mode.raceSettings.networkPlayerCount=(unsigned char)netCount;
 state.race.ptr()->localRacer=state.racers[0].ptr();
 state.race.ptr()->aiRacers=state.ai;state.race.ptr()->racerSlots=state.net;
 for(int i=0;i<2;i++){state.ai[i]=state.racers[i+1].ptr();state.net[i]=state.racers[i+3].ptr();}
 for(i=0;i<5;i++){
  state.racers[i].ptr()->riderCharacter=state.riders[i].ptr();
  state.racers[i].ptr()->modelNode=(UnknownBikeRaceModel*)state.models[alias?i%2:2*i].ptr();
  state.riders[i].ptr()->modelNode=(UnknownBikeRaceModel*)state.models[alias?i%2:2*i+1].ptr();
 }
 if(failAt>=0){
  REQUIRE(!alias && aiCount==2 && netCount==2 && mode!=4);
  UnknownBikeRaceModel* bad=(UnknownBikeRaceModel*)(pages+4096-0x284);
  if(failAt&1)state.riders[failAt/2].ptr()->modelNode=bad;
  else state.racers[failAt/2].ptr()->modelNode=bad;
 }
 Fill(pages,4096,seed+19);memcpy(pageExpected,pages,4096);
 memcpy(&expected,&state,sizeof(state));
 // Build independent visit order from bounded input counts, not source pointers.
 int order[10], count=0;order[count++]=0;order[count++]=1;
 if(mode!=4)for(i=0;i<aiCount;i++){order[count++]=2+2*i;order[count++]=3+2*i;}
 for(i=0;i<netCount;i++){order[count++]=6+2*i;order[count++]=7+2*i;}
 for(i=0;i<count;i++){
  if(order[i]==failAt)break;
  ModelOracle(alias?(order[i]/2)%2:order[i],blade,level);
 }
 g_ActiveLodSettings=(SoultreeLodSetting*)0x12345678;
 unsigned int control=_controlfp(0,0);Invoke(level);
 REQUIRE(_controlfp(0,0)==control);
 REQUIRE(g_TrackGame==state.game.ptr());
 REQUIRE(g_ActiveLodSettings==(blade?g_BladeLodSettings:g_NonBladeLodSettings));
 REQUIRE(memcmp(&state,&expected,sizeof(state))==0);
 REQUIRE(memcmp(pages,pageExpected,4096)==0);
 if(failAt<0)REQUIRE(exceptionCode==0);
 else {REQUIRE(exceptionCode==EXCEPTION_ACCESS_VIOLATION);REQUIRE(exceptionAccess==1);REQUIRE(exceptionAddress==(DWORD)(pages+4096));faults++;}
 cases++;
}
int main(){
 REQUIRE(g_ActiveLodSettings==0);
 pages=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);REQUIRE(pages!=0);
 DWORD old;REQUIRE(VirtualProtect(pages+4096,4096,PAGE_NOACCESS,&old));
 unsigned int original=_controlfp(0,0);_controlfp(_MCW_EM|_RC_NEAR|_PC_64,_MCW_EM|_MCW_RC|_MCW_PC);
 SoultreeLodSetting tablesBefore[20];memcpy(tablesBefore,g_NonBladeLodSettings,160);memcpy(tablesBefore+10,g_BladeLodSettings,160);
 const int counts[]={-2,0,1,2};const int modes[]={0,4,7};
 for(int level=0;level<10;level++)for(int blade=0;blade<2;blade++)for(int mode=0;mode<3;mode++)
  for(int a=0;a<4;a++)for(int n=0;n<3;n++)for(int alias=0;alias<2;alias++)for(int seed=1;seed<=3;seed++)
   Run(level,blade,modes[mode],counts[a],n,alias,seed,-1);
 for(level=0;level<10;level++)for(int faultBlade=0;faultBlade<2;faultBlade++)for(int fault=0;fault<10;fault++)Run(level,faultBlade,0,2,2,0,17,fault);
 REQUIRE(memcmp(tablesBefore,g_NonBladeLodSettings,160)==0);REQUIRE(memcmp(tablesBefore+10,g_BladeLodSettings,160)==0);
 _controlfp(original,_MCW_EM|_MCW_RC|_MCW_PC);VirtualFree(pages,0,MEM_RELEASE);
 printf("Race detail: %d cases, %d independently modeled write faults\n",cases,faults);return 0;
}
