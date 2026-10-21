// Native execution of the complete production GridNode corner sampler.
// No sampler dependency is scripted; only its unused segment virtual fails fast.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "../../src/reconstructed/Griddraw.h"
#include "../../src/krusty2/broadphase/Terrain.h"
#include "../../src/reconstructed/BoundingBoxTreeBuild.h"
#include "../../src/reconstructed/MatrixUtil.h"
#include "../../src/krusty2/math/Vec3.h"
#include "../../src/reconstructed/VectorScaleAssign.h"

typedef char GridLayout[sizeof(GridNode)==0x34 && offsetof(GridNode,level)==0x28 ? 1:-1];
typedef char GridBaseLayout[sizeof(GridBaseBlock)==0xd50 && sizeof(GridBaseCell)==8 ? 1:-1];
typedef char Views[sizeof(TerrainVec3)==12 && sizeof(TreeVec3)==12 && sizeof(Vector3)==12 && sizeof(Vec3)==12 && sizeof(GridVec3)==12 ? 1:-1];
typedef char VectorOffsets[offsetof(TerrainVec3,z)==8 && offsetof(TreeVec3,z)==8 && offsetof(Vector3,z)==8 && offsetof(Vec3,z)==8 && offsetof(GridVec3,z)==8 ? 1:-1];

static BYTE referenceSpace[0x800000]={1};
struct Storage {
    DWORD nodes[4][sizeof(GridNode)/4];
    GridNode* children[2][256];
    DWORD blocks[2][sizeof(GridBaseBlock)/4];
    DWORD output[32];
};
static Storage state,initial;
struct Observation {int result,exception,access,address;unsigned status,control;Storage state;};
static Observation observed,expected;
static unsigned checks;
static unsigned scaleChecks;
static GridNode* Node(int index){return (GridNode*)state.nodes[index];}
static GridBaseBlock* Block(int index){return (GridBaseBlock*)state.blocks[index];}
int GridNode::UnknownVirtualSlot1(const GridVec3*,const GridVec3*,GridVec3*,GridNode**,int*,int*){ExitProcess(81);return 0;}

static int Map(const char* path){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 || !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    if(size<sizeof(IMAGE_DOS_HEADER) || ((IMAGE_DOS_HEADER*)raw)->e_lfanew<0 || ((IMAGE_DOS_HEADER*)raw)->e_lfanew+sizeof(IMAGE_NT_HEADERS)>(unsigned)size)return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+((IMAGE_DOS_HEADER*)raw)->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 || pe->OptionalHeader.SizeOfHeaders>(unsigned)size)return 0;
    IMAGE_SECTION_HEADER* sections=IMAGE_FIRST_SECTION(pe);
    if((BYTE*)(sections+pe->FileHeader.NumberOfSections)>raw+size)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(sections[i].PointerToRawData>(unsigned)size || sections[i].SizeOfRawData>(unsigned)size-sections[i].PointerToRawData || sections[i].VirtualAddress>pe->OptionalHeader.SizeOfImage || sections[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-sections[i].VirtualAddress)return 0;
        memcpy(mapped+sections[i].VirtualAddress,raw+sections[i].PointerToRawData,sections[i].SizeOfRawData);
    }
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* info){
    if(info->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=info->ExceptionRecord->ExceptionCode;observed.access=info->ExceptionRecord->ExceptionInformation[0];observed.address=info->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail,int x,int z,int flags,int mode,int precision){
    memset(&observed,0,sizeof(observed));observed.result=-13;state=initial;
    float* heights=(float*)&state.output[2];
    GridVec3* normals=flags&1?(GridVec3*)&state.output[8]:0;
    unsigned char* bytes=flags&2?(unsigned char*)&state.output[24]:0;
    union{int(GridNode::*method)(int,int,float*,GridVec3*,unsigned char*);DWORD address;}call;
    memset(&call,0,sizeof(call));call.method=&GridNode::GetCellCorners;if(retail)call.address=0x483910;
    _control87(_MCW_EM|mode|precision,_MCW_EM|_MCW_RC|_MCW_PC);_clear87();
    __try{observed.result=(Node(0)->*call.method)(x,z,heights,normals,bytes);}__except(Filter(GetExceptionInformation())){}
    observed.status=_status87();observed.control=_control87(0,0);observed.state=state;
}
static void Compare(int x,int z,int flags,int mode,int precision){
    Run(1,x,z,flags,mode,precision);expected=observed;Run(0,x,z,flags,mode,precision);++checks;
    if(memcmp(&observed,&expected,sizeof(observed))){printf("Grid mismatch %u x%d z%d flags%d rc%x pc%x\n",checks,x,z,flags,mode,precision);ExitProcess(2);}
}
static void Seed(unsigned seed,int shape){
    memset(&state,0xa5,sizeof(state));unsigned random=seed+1;
    const unsigned scales[]={0,0x80000000,0x3f800000,0xbf800000,0x3eaaaaab,0x00000001,0x7f7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345};
    for(int n=0;n<4;++n){GridNode* node=Node(n);node->level=0;node->children=0;node->childOffsets=0;node->block=Block(n&1);
        memcpy(&node->reliefScale,&scales[(seed+n)%11],4);memcpy(&node->heightScale,&scales[(seed*3+n)%11],4);
    }
    for(n=0;n<2;++n){GridBaseBlock* block=Block(n);block->baseHeight=(seed&1)?-32768:32767;
        for(int i=0;i<289;++i){random=random*1664525u+1013904223u;GridBaseCell* cell=&block->cells[i];cell->surface=(unsigned char)(i+seed);cell->height=(unsigned short)random;cell->normal[0]=(signed char)(random>>8);cell->normal[1]=(signed char)(random>>16);cell->normal[2]=(signed char)(random>>24);}
        for(i=0;i<256;++i)state.children[n][i]=(shape==3 && i%3==0)?0:Node(2+(i&1));
    }
    if(shape){Node(0)->level=shape==2?2:1;Node(0)->children=shape==4?0:state.children[0];}
    if(shape==2){Node(1)->level=1;Node(1)->children=state.children[1];for(int i=0;i<256;++i)state.children[0][i]=Node(1);}
    initial=state;
}
struct RetailScale {RetailScale& Apply(float);};
static void CheckScale(unsigned seed,int mode,int precision){
    unsigned values[16],saved[16],actual[16];unsigned random=seed+1;
    const unsigned edges[]={0,0x80000000,0x3f800000,0xbf800000,0x00000001,0x00800000,0x7f7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345,0xffc54321,0x3eaaaaab};
    for(int i=0;i<16;++i){random=random*1664525u+1013904223u;values[i]=seed<13?edges[(seed+i)%13]:random;}
    float factor;memcpy(&factor,values+11,4);
    union{RetailScale&(RetailScale::*method)(float);DWORD address;}call;memset(&call,0,sizeof(call));call.address=0x40ae00;
    memcpy(actual,values,sizeof(actual));_control87(_MCW_EM|mode|precision,_MCW_EM|_MCW_RC|_MCW_PC);_clear87();
    if(&(((RetailScale*)(actual+4))->*call.method)(factor)!=(RetailScale*)(actual+4))ExitProcess(82);
    unsigned status=_status87(),control=_control87(0,0);memcpy(saved,actual,sizeof(actual));
    for(int kind=0;kind<2;++kind){
        memcpy(actual,values,sizeof(actual));_control87(_MCW_EM|mode|precision,_MCW_EM|_MCW_RC|_MCW_PC);_clear87();
        void* result=kind?(void*)&(*(TerrainVec3*)(actual+4)*=factor):(void*)&(*(BikeVec3ScaleAssign*)(actual+4)*=factor);
        ++scaleChecks;
        if(result!=actual+4 || memcmp(saved,actual,sizeof(actual)) || status!=_status87() || control!=_control87(0,0)){printf("Scale mismatch %u seed%u kind%d rc%x pc%x\n",scaleChecks,seed,kind,mode,precision);ExitProcess(83);}
    }
}
int main(int argc,char** argv){
    if(argc!=2 || !Map(argv[1]))return 3;
    BikeVec3ScaleAssign bike;bike.x=1;bike.y=-2;bike.z=4;
    TerrainVec3 terrain(1,-2,4);
    if(&(bike*=0.5f)!=&bike || bike.x!=0.5f || bike.y!=-1 || bike.z!=2 ||
       &(terrain*=0.5f)!=&terrain || terrain.x!=0.5f || terrain.y!=-1 || terrain.z!=2)return 9;
    // Independent expected corner order, height scaling and surface values.
    Seed(0,0);GridNode* root=Node(0);root->reliefScale=root->heightScale=1;root->block->baseHeight=0;
    const int slots[]={0,1,17,18};for(int i=0;i<4;++i){root->block->cells[slots[i]].height=(unsigned short)(i+1);root->block->cells[slots[i]].surface=(unsigned char)(10+i);}
    initial=state;Compare(0,0,3,_RC_NEAR,_PC_64);
    if(observed.result!=1 || observed.exception)return 4;
    for(i=0;i<4;++i)if(((float*)&state.output[2])[i]!=(float)(i+1) || ((BYTE*)&state.output[24])[i]!=10+i)return 5;
    Seed(0,4);Compare(0,0,3,_RC_NEAR,_PC_64);if(observed.result || observed.exception)return 6;
    for(i=0;i<4;++i){GridVec3* normal=(GridVec3*)&state.output[8];if(((float*)&state.output[2])[i]!=0 || normal[i].x!=0 || normal[i].y!=1 || normal[i].z!=0 || ((BYTE*)&state.output[24])[i])return 7;}
    Seed(0,4);Compare(0,0,1,_RC_NEAR,_PC_64);if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.access!=1 || observed.address)return 8;
    const int modes[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};const int precisions[]={_PC_24,_PC_53,_PC_64};
    for(unsigned sample=0;sample<2033;++sample)for(int round=0;round<4;++round)for(int prec=0;prec<3;++prec)
        CheckScale(sample,modes[round],precisions[prec]);
    const int coords[]={0,1,14,15,16,255,256,511,4095,4096};
    for(unsigned seed=0;seed<33;++seed)for(int shape=0;shape<5;++shape){Seed(seed,shape);
        for(int xy=0;xy<(shape?10:4);++xy)for(int flags=0;flags<4;++flags)for(int mode=0;mode<4;++mode)for(int precision=0;precision<3;++precision)
            Compare(coords[xy],coords[(xy+1)%(shape?10:4)],flags,modes[mode],precisions[precision]);
    }
    printf("Grid corners: %u native leaf, descent, clamp, output and fault cases match; scale operators: %u raw-bit cases match\n",checks,scaleChecks);return 0;
}
