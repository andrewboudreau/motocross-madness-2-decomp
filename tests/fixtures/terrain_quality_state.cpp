#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "../../src/krusty2/broadphase/Terrain.h"

extern TerrainQualityEntry g_terrainQualityHardware[10];
extern TerrainQualityEntry g_terrainQualitySoftware[10];
typedef char Layout[(sizeof(TerrainQualityEntry)==16 && sizeof(Terrain)==0xcc4 &&
    offsetof(Terrain,drawDistance)==0xbec && offsetof(Terrain,qualityIndex)==0xbf0 &&
    offsetof(Terrain,drawDistanceDirty)==0xbf4 && offsetof(Terrain,qualityParamA)==0xca4 &&
    offsetof(Terrain,qualityParamB)==0xca8 && offsetof(Terrain,lowestQualityOverride)==0xcb0)?1:-1];
static BYTE objectBytes[sizeof(Terrain)+64], expected[sizeof(Terrain)], beforePage[4096];
static BYTE* page;
static DWORD fault,accessKind,address;
static int cases;
static void Check(int x,const char* label){if(!x){printf("FAIL %s case %d\n",label,cases);ExitProcess(7);}}
static int Filter(EXCEPTION_POINTERS* e){fault=e->ExceptionRecord->ExceptionCode;accessKind=e->ExceptionRecord->ExceptionInformation[0];address=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(Terrain* t,int index){fault=accessKind=address=0;__try{t->SelectQuality(index);}__except(Filter(GetExceptionInformation())){}}
static DWORD Next(DWORD& seed){seed=seed*1664525+1013904223;return seed;}
static void Normal(int which,int index,int overrideFlag,int initial,int dirty,int seedIndex){
    TerrainQualityEntry saved[10];
    TerrainQualityEntry* table=which?g_terrainQualitySoftware:g_terrainQualityHardware;
    memcpy(saved,table,sizeof(saved));
    // Every other seed deliberately mutates the real exported writable cells.
    // These are fixture mutations, not a claim that retail normally writes them.
    DWORD seed=0x9e3779b9u+seedIndex;
    if(seedIndex&1){for(int row=0;row<10;row++){
        table[row].drawDistanceSetting=(int)(Next(seed)%33)-16;
        table[row].closeUpDetailLimit=Next(seed);table[row].perspectiveLimit=Next(seed);table[row].hasDetailPass=Next(seed);
    }}
    memset(objectBytes,0xa5,sizeof(objectBytes));Terrain* t=(Terrain*)(objectBytes+32);
    t->drawDistance=initial==0?-1:initial==1?0:initial==2?table[index].drawDistanceSetting:100;
    t->drawDistanceDirty=dirty;t->lowestQualityOverride=overrideFlag;
    memcpy(expected,t,sizeof(Terrain));Terrain* e=(Terrain*)expected;
    int distance=table[index].drawDistanceSetting;if(distance<0)distance=0;
    e->qualityIndex=index;
    if(e->drawDistance!=distance){e->drawDistance=distance;e->drawDistanceDirty=1;}
    e->qualityParamA=table[overrideFlag?9:index].closeUpDetailLimit;
    e->qualityParamB=table[index].perspectiveLimit;
    int detail=table[index].hasDetailPass;
    g_pTerrainQualityTable=table;g_terrainQualityValue=0x12345678;
    Invoke(t,index);Check(!fault,"normal completion");
    Check(!memcmp(t,expected,sizeof(Terrain)),"complete receiver oracle");
    Check(g_terrainQualityValue==detail&&g_pTerrainQualityTable==table,"shared globals");
    for(int i=0;i<32;i++)Check(objectBytes[i]==0xa5&&objectBytes[32+sizeof(Terrain)+i]==0xa5,"object guards");
    memcpy(table,saved,sizeof(saved));++cases;
}
static void Fault(int mode){
    memset(objectBytes,0xa5,sizeof(objectBytes));Terrain* t=(Terrain*)(objectBytes+32);
    memset(page,0xa5,4096);g_pTerrainQualityTable=g_terrainQualityHardware;
    g_terrainQualityValue=0x12345678;
    unsigned receiverBytes=sizeof(Terrain),tableBytes=160;
    if(mode<4){unsigned sizes[4]={0xbf0,0xbf4,0xcb0,0xcb4};receiverBytes=sizes[mode];t=(Terrain*)(page+4096-receiverBytes);}
    else {unsigned sizes[6]={0,4,8,12,16,160};tableBytes=sizes[mode-4];g_pTerrainQualityTable=(TerrainQualityEntry*)(page+4096-tableBytes);if(tableBytes)memcpy(g_pTerrainQualityTable,g_terrainQualityHardware,tableBytes);}
    if(receiverBytes>=0xbf0)t->drawDistance=99;
    if(receiverBytes>=0xcb4)t->lowestQualityOverride=mode==8?1:0;
    memcpy(expected,t,receiverBytes);memcpy(beforePage,page,4096);
    Terrain* e=(Terrain*)expected;
    DWORD wantedAddress=0,wantedAccess=0;int wantedDetail=0x12345678;
    if(mode==0){wantedAddress=(DWORD)(page+4096);wantedAccess=1;}
    else {
        e->qualityIndex=0;
        if(mode>=4&&mode<=7)wantedAddress=(DWORD)(page+4096-tableBytes+12);
        else {
            wantedDetail=0;e->drawDistance=7;
            if(mode==1){wantedAddress=(DWORD)(page+4096);wantedAccess=1;}
            else {
                e->drawDistanceDirty=1;
                if(mode==2)wantedAddress=(DWORD)(page+4096);
                else if(mode==8)wantedAddress=(DWORD)(page+4096-tableBytes+148);
                else {e->qualityParamA=4096;e->qualityParamB=512;}
            }
        }
    }
    Invoke(t,0);
    if(wantedAddress)Check(fault==EXCEPTION_ACCESS_VIOLATION&&address==wantedAddress&&accessKind==wantedAccess,"independent first fault");
    else Check(!fault,"guarded success");
    Check(!memcmp(t,expected,receiverBytes),"partial receiver oracle");
    Check(g_terrainQualityValue==wantedDetail,"partial global oracle");
    if(mode>=4)Check(!memcmp(page,beforePage,4096),"read-only table page");
    else Check(!memcmp(page,beforePage,4096-receiverBytes),"receiver prefix guard");
    ++cases;
}
int main(){
    page=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);Check(page!=0,"allocation");DWORD old;Check(VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old),"guard page");
    for(int table=0;table<2;table++)for(int row=0;row<10;row++)for(int over=0;over<2;over++)for(int init=0;init<4;init++)for(int dirty=0;dirty<2;dirty++)for(int seed=0;seed<16;seed++)Normal(table,row,over,init,dirty,seed);
    for(int mode=0;mode<10;mode++)Fault(mode);
    VirtualFree(page,0,MEM_RELEASE);printf("Terrain quality: %d cases, 8 independently modeled access faults\n",cases);return 0;
}
