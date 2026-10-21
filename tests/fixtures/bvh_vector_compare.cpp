// Private point-leaf integration: unchanged extracted CapsuleTreeQuery and real math.
// Triangle/interior-only dependencies terminate if unexpectedly reached.
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "bvh/BoundingBoxTreeQuery.h"
#include "collision/CollisionPoint.h"

void InitFastSqrtTable();

typedef char Vec3Size[(sizeof(Vec3)==12 && sizeof(CollisionVec3)==12)?1:-1];
typedef char VectorOffsets[(offsetof(Vec3,x)==0 && offsetof(Vec3,y)==4 && offsetof(Vec3,z)==8 &&
    offsetof(CollisionVec3,x)==0 && offsetof(CollisionVec3,y)==4 && offsetof(CollisionVec3,z)==8)?1:-1];
typedef char PointLeafSize[(sizeof(QueryPointLeaf)==16 && offsetof(QueryPointLeaf,point)==4)?1:-1];
static unsigned char referenceSpace[0x800000];
static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000; DWORD old;
    if ((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
        !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* f=fopen(path,"rb"); if (!f) return 0;
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    if (size<(long)sizeof(IMAGE_DOS_HEADER)) return 0;
    BYTE* raw=(BYTE*)malloc(size); if (!raw || fread(raw,1,size,f)!=(size_t)size) return 0;
    fclose(f); IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if (dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 ||
        dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if (pe->Signature!=IMAGE_NT_SIGNATURE || pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
        pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 ||
        pe->OptionalHeader.SizeOfHeaders>(DWORD)size ||
        pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage) return 0;
    DWORD sectionOffset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if (sectionOffset>(DWORD)size ||
        pe->FileHeader.NumberOfSections>((DWORD)size-sectionOffset)/sizeof(IMAGE_SECTION_HEADER)) return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for (int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if (section[i].PointerToRawData>(DWORD)size ||
            section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData ||
            section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage ||
            section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress) return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw); return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

int PointInTriangle(const Vec3*,const QueryTriangle*) { ExitProcess(61); return 0; }
float SegmentSegmentDistance(const Vec3*,const Vec3*,const Vec3*,const Vec3*) { ExitProcess(62); return 0; }
int BoxCapsuleOverlap(const Vec3*,const Vec3*,const Vec3*,float,float,const Matrix4*) { ExitProcess(63); return 0; }
void Vec3TransformNormalTranspose(Vec3*,Vec3,const Matrix4*) { ExitProcess(64); }

static void PatchCall(unsigned long at, unsigned long expected, void* target) {
    if (*(unsigned char*)at!=0xe8 || at+5+*(unsigned long*)(at+1)!=expected) ExitProcess(65);
    *(unsigned long*)(at+1)=(unsigned long)target-(at+5);
}
#include "bvh_vector_patches.h"

struct World {
    unsigned long before[8];
    Vec3 ends[2];
    QueryPointLeaf leaf;
    Matrix4 transform;
    unsigned long after[8];
};
static World world, initial, expected;
typedef int (__cdecl* Query)(const Vec3*,float,float,QueryTreeNode*,const Matrix4*,int);
static unsigned long savedControl;
static int checks;

static float Finite(unsigned long* state) {
    *state=*state*1664525u+1013904223u;
    return ((int)((*state>>8)&0xffff)-32768)/256.0f;
}
static void Prepare(int seed,int mode) {
    memset(&world,0,sizeof(world));
    for(int i=0;i<8;++i) { world.before[i]=0xc0010000+i; world.after[i]=0x1234abcd+i; }
    unsigned long random=seed+1;
    for(i=0;i<6;++i) ((float*)world.ends)[i]=Finite(&random);
    for(i=0;i<3;++i) ((float*)&world.leaf.point)[i]=Finite(&random);
    world.leaf.marker=-1.0f;
    world.transform._11=world.transform._22=world.transform._33=world.transform._44=1.0f;
    if(mode==1) world.ends[1]=world.ends[0];
    if(mode==2) world.leaf.point=world.ends[0];
    if(mode==3) world.leaf.point=world.ends[1];
    if(mode==4) {
        world.transform._41=Finite(&random); world.transform._42=Finite(&random); world.transform._43=Finite(&random);
    }
    if(mode==5) { world.transform._11=0; world.transform._12=1; world.transform._21=-1; world.transform._22=0; }
    if(mode==6) { world.transform._11=2; world.transform._22=0.5f; world.transform._33=-1; }
    if(mode==7) {
        world.ends[0]=Vec3(0,0,0); world.ends[1]=Vec3(2,0,0); world.leaf.point=Vec3(1,1,0);
    }
    memcpy(&initial,&world,sizeof(world));
}
static int Check(float radius,unsigned long fp) {
    memcpy(&world,&initial,sizeof(world)); _clearfp(); _control87(fp,_MCW_EM|_MCW_RC|_MCW_PC);
    int reference=((Query)0x4298c0)(world.ends,radius,radius*radius,(QueryTreeNode*)&world.leaf,&world.transform,0);
    unsigned long status=_statusfp()&0x3f, control=_control87(0,0);
    memcpy(&expected,&world,sizeof(world));
    memcpy(&world,&initial,sizeof(world)); _clearfp(); _control87(fp,_MCW_EM|_MCW_RC|_MCW_PC);
    int source=CapsuleTreeQuery(world.ends,radius,radius*radius,(QueryTreeNode*)&world.leaf,&world.transform,0);
    unsigned long sourceStatus=_statusfp()&0x3f, sourceControl=_control87(0,0);
    ++checks;
    return source==reference && sourceStatus==status && sourceControl==control &&
           !memcmp(&expected,&world,sizeof(world)) && !memcmp(&initial,&world,sizeof(world));
}
int main(int argc,char** argv) {
    if(argc!=2 || !MapReference(argv[1])) return 3;
    InstallPatches();
    FlushInstructionCache(GetCurrentProcess(),0,0);
    savedControl=_control87(0,0); InitFastSqrtTable();
    unsigned long precision[]={_PC_24,_PC_53,_PC_64};
    unsigned long rounding[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
    float radii[]={-1.0f,0.0f,0.5f,1.0f,1.25f,256.0f};
    for(int seed=0;seed<64;++seed) for(int mode=0;mode<8;++mode) {
        Prepare(seed,mode);
        for(int r=0;r<6;++r) for(int pc=0;pc<3;++pc) for(int rc=0;rc<4;++rc)
            if(!Check(radii[r],_MCW_EM|precision[pc]|rounding[rc])) {
                printf("Mismatch seed%d mode%d radius%d pc%d rc%d\n",seed,mode,r,pc,rc); return 2;
            }
    }
    Prepare(0,7); _clearfp(); _control87(_MCW_EM|_PC_53|_RC_NEAR,_MCW_EM|_MCW_PC|_MCW_RC);
    if(CapsuleTreeQuery(world.ends,1.0f,1.0f,(QueryTreeNode*)&world.leaf,&world.transform,0)!=0 ||
       CapsuleTreeQuery(world.ends,1.25f,1.5625f,(QueryTreeNode*)&world.leaf,&world.transform,0)!=1) return 4;
    checks+=2;
    _control87(savedControl,_MCW_EM|_MCW_RC|_MCW_PC);
    printf("BVH vectors: %d point-leaf and boundary cases passed\n",checks);
    return 0;
}
