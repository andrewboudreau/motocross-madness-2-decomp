// Execute the complete source/retail query up to its first cell request.
// The boundary throws before height/material/normal interpolation.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../../src/krusty2/broadphase/Terrain.h"
static BYTE referenceSpace[0x800000];
static BYTE terrainBytes[sizeof(Terrain)],fieldBytes[0x100];
static TerrainVec3 point,normal;
static unsigned char material;
struct Observation {int called,x,z,normal;unsigned fp,control;TerrainVec3 point,output;unsigned char material;};
static Observation observed,expected;
static unsigned checks;
void TerrainShutdownObject::GetCellCorners(int x,int z,float* heights,TerrainVec3* normals,unsigned char* kinds){
    if((void*)this!=fieldBytes || !heights || !kinds)ExitProcess(81);
    observed.called=1;observed.x=x;observed.z=z;observed.normal=normals!=0;
    throw 0x3910; // Stop at the first cell request, before interpolation.
}
float TerrainDot(const TerrainVec3&,const TerrainVec3&){ExitProcess(82);return 0;}
float FastInvSqrt(float){ExitProcess(83);return 0;}
void TerrainTriangleNormal(const TerrainVec3*,const TerrainVec3*,const TerrainVec3*,TerrainVec3*,float*){ExitProcess(84);}
template<class T>static DWORD Address(T method){union{T method;DWORD address;}u;u.method=method;return u.address;}
static int Map(const char* name){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 || !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(name,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);
    if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    if(size<sizeof(IMAGE_DOS_HEADER) || ((IMAGE_DOS_HEADER*)raw)->e_lfanew<0 || ((IMAGE_DOS_HEADER*)raw)->e_lfanew+sizeof(IMAGE_NT_HEADERS)>(unsigned)size)return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+((IMAGE_DOS_HEADER*)raw)->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 || pe->OptionalHeader.SizeOfHeaders>(unsigned)size)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(unsigned)size || section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw);BYTE* site=(BYTE*)0x507cf8;
    if(*site!=0xe8 || (DWORD)(site+5)+*(DWORD*)(site+1)!=0x483910)return 0;
    *(DWORD*)(site+1)=Address(&TerrainShutdownObject::GetCellCorners)-(DWORD)(site+5);
    return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static void Run(int retail,unsigned x,unsigned z,unsigned scale,int mode,int precision,int hasField,int wantsNormal){
    memset(&observed,0,sizeof(observed));memset(terrainBytes,0x71,sizeof(terrainBytes));
    Terrain* terrain=(Terrain*)terrainBytes;terrain->heightField=hasField?(TerrainShutdownObject*)fieldBytes:0;
    memcpy(&terrain->invGridCellSize,&scale,4);memcpy(&point.x,&x,4);point.y=7.25f;memcpy(&point.z,&z,4);
    normal.x=3;normal.y=4;normal.z=5;material=0x37;
    _control87(_MCW_EM|precision|mode,_MCW_EM|_MCW_PC|_MCW_RC);_clear87();
    union{void(Terrain::*method)(TerrainVec3*,TerrainVec3*,int,unsigned char*);DWORD address;}call;memset(&call,0,sizeof(call));call.address=0x507c10;
    try{if(retail)(terrain->*call.method)(&point,wantsNormal?&normal:0,0,&material);else terrain->QueryGround(&point,wantsNormal?&normal:0,0,&material);}
    catch(int n){if(n!=0x3910)ExitProcess(85);}
    observed.fp=_status87();observed.control=_control87(0,0);observed.point=point;observed.output=normal;observed.material=material;
}
static void Compare(unsigned x,unsigned z,unsigned scale,int mode,int precision,int hasField=1,int wantsNormal=0){
    Run(1,x,z,scale,mode,precision,hasField,wantsNormal);expected=observed;
    Run(0,x,z,scale,mode,precision,hasField,wantsNormal);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){printf("Mismatch %u x%08x z%08x scale%08x rc%x pc%x: cells%d %d,%d / %d %d,%d fp%x/%x control%x/%x\n",checks,x,z,scale,mode,precision,expected.called,expected.x,expected.z,observed.called,observed.x,observed.z,expected.fp,observed.fp,expected.control,observed.control);ExitProcess(2);}
}
int main(int argc,char** argv){
    if(argc!=2 || !Map(argv[1]))return 3;
    Compare(0x3fa00000,0x3fa00000,0x3f800000,_RC_NEAR,_PC_64);
    if(!observed.called || observed.x!=1 || observed.z!=1)return 5;
    Compare(0x3f800000,0x40000000,0x3f800000,_RC_NEAR,_PC_64);
    if(!observed.called || observed.x!=0 || observed.z!=2)return 6;
    Compare(0x7f800000,0x3fa00000,0x3f800000,_RC_NEAR,_PC_64);
    if(observed.called || !(observed.fp&_SW_INVALID))return 7;
    const unsigned values[]={0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x3effffff,0x3f000000,0x3f000001,0x3f7fffff,0x3f800000,0x3f800001,0x3fa00000,0x3fbfffff,0x3fc00000,0x3fc00001,0x40000000,0x40200000,0x403fffff,0x40400000,0x40400001,0xbf800000,0x4affffff,0x4b000000,0x4effffff,0x4f000000,0x4f800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000,0x7f800001};
    const unsigned scales[]={0x3f800000,0x3eaaaaab,0x40000000,0x3f000000,0,0xbf800000};
    const unsigned precision[]={_PC_24,_PC_53,_PC_64};
    for(int pc=0;pc<3;++pc)for(int mode=0;mode<4;++mode)for(unsigned scale=0;scale<6;++scale)
      for(unsigned x=0;x<sizeof(values)/4;++x)for(unsigned z=0;z<sizeof(values)/4;++z)
        Compare(values[x],values[z],scales[scale],mode*0x100,precision[pc],1,(x+z)&1);
    unsigned seed=19;
    for(int n=0;n<200000;++n){seed=seed*1664525+1013904223;unsigned x=seed;seed=seed*1664525+1013904223;unsigned z=seed;
        Compare(x,z,scales[n%6],(n%4)*0x100,precision[n%3],(n%17)!=0,n&1);}
    printf("Terrain cell boundary: %u cases match\n",checks);return 0;
}
