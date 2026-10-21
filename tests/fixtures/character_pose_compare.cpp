// Private native tests of complete, separately compiled pose helper bodies.
// Retail execution and dependency bridges are validation-only.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "motion/Motnctrl.h"
struct TreeMatrix4;
extern "C" double __cdecl _CIacos();
static unsigned char referenceSpace[0x800000];
int AdvanceClamped(Motion*,MotionPoseList**,MotionPoseList**,MotionPoseList**,float*,float*) { ExitProcess(91); return 0; }
void Character::BlendToMotion(Motion*) { ExitProcess(91); }
void MatrixMultiply(Matrix4* out,Matrix4 a,Matrix4 b) {
    ((void(__cdecl*)(Matrix4*,Matrix4,Matrix4))0x42a1a0)(out,a,b);
}
void AxisAngleMatrix(TreeMatrix4* out,float x,float y,float z,float a) {
    ((void(__cdecl*)(TreeMatrix4*,float,float,float,float))0x42de90)(out,x,y,z,a);
}
Vec3 CrossProductCall(const Vec3& a,const Vec3& b) {
    return ((Vec3(__cdecl*)(const Vec3&,const Vec3&))0x515600)(a,b);
}
Vec3 Vec3Normalize(const Vec3& v) {
    return ((Vec3(__cdecl*)(const Vec3&))0x5087b0)(v);
}
float FastInvSqrt(float v) { return ((float(__cdecl*)(float))0x460c00)(v); }

// The standalone TU also contains LoadVue, validated by a separate fixture.
// Any accidental use of that unrelated path fails this playback test.
UnknownResourceManager* g_ResourceManager;
void* DebugMalloc(unsigned int,const char*,int) { ExitProcess(90); return 0; }
void* DebugRealloc(void*,unsigned int,const char*,int) { ExitProcess(90); return 0; }
void* operator new(unsigned int,const char*,int) { ExitProcess(90); return 0; }
void operator delete(void*,const char*,int) { ExitProcess(90); }
UnknownTextureStream::UnknownTextureStream(UnknownResourceManager*) { ExitProcess(90); }
UnknownTextureStream::~UnknownTextureStream() { ExitProcess(90); }
int UnknownTextureStream::Seek(int,int,int) { ExitProcess(90); return 0; }
int UnknownTextureStream::Open(const char*,const char*,int) { ExitProcess(90); return 0; }
int UnknownTextureStream::ReadLine(char*,int) { ExitProcess(90); return 0; }
unsigned char Character::FindNode(const char*) { ExitProcess(90); return 0; }

static int MapReference(const char* path,const char* patches) {
    BYTE* mapped=(BYTE*)0x400000; DWORD old;
    if ((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
        !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* f=fopen(path,"rb"); if (!f) return 0;
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    BYTE* raw=(BYTE*)malloc(size); if (!raw || fread(raw,1,size,f)!=(size_t)size) return 0;
    fclose(f); IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if (pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000) return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for (int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if (section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
            section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage) return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw); f=fopen(patches,"rb"); if (!f) return 0;
    DWORD pair[2]; int count=0;
    while (fread(pair,4,2,f)==2) {
        DWORD target=pair[1]==0x535240?(DWORD)_CIacos:0;
        if (!target || *(BYTE*)pair[0]!=0xe8) return 0;
        *(DWORD*)(pair[0]+1)=target-pair[0]-5; ++count;
    }
    fclose(f); if (count!=3) return 0;
    ((void(__cdecl*)())0x460bb0)();
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

struct State {
    BYTE before[16];
    CharacterPose pose[3];
    Vec3 axis;
    float angle;
    BYTE after[16];
};
static State state;
static unsigned int randomState;
static unsigned int Next() { randomState=randomState*1664525u+1013904223u;return randomState; }
static float Coordinate() { return ((int)(Next()%4097)-2048)/997.0f; }
static void Reset(int seed,int shape,int scale) {
    memset(&state,0x5a,sizeof(state));
    randomState=seed+1;
    const float scales[]={0,0.0009765625f,0.25f,1,3,1024};
    for(int i=0;i<3;++i) {
        CharacterPose& p=state.pose[i];p.nodeIndex=(unsigned char)(seed+i);p.hasPose=seed*3-i;
        p.axisZ=Vec3(Coordinate(),Coordinate(),Coordinate());
        p.axisY=Vec3(Coordinate(),Coordinate(),Coordinate());
        p.position=Vec3(Coordinate(),Coordinate(),Coordinate());
        p.axisZ=p.axisZ*scales[scale];p.axisY=p.axisY*scales[(scale+i)%6];
    }
    if(shape==1) state.pose[1].axisY=state.pose[0].axisY;
    if(shape==2) {
        state.pose[1].axisY=state.pose[0].axisY;
        state.pose[1].axisZ=state.pose[0].axisZ;
    }
    if(shape==3) {
        state.pose[0].axisY=Vec3(0,1,0);state.pose[1].axisY=Vec3(0,-1,0);
        state.pose[0].axisZ=Vec3(0,0,1);state.pose[1].axisZ=Vec3(0,0,-1);
    }
    state.axis=Vec3(Coordinate(),Coordinate(),Coordinate());state.angle=123.25f;
}
static void Invoke(int function,DWORD address,int alias,float parameter) {
    CharacterPose* a=&state.pose[0];CharacterPose* b=&state.pose[1];CharacterPose* out=&state.pose[2];
    if(function==0) {
        if(alias) out=a;
        ((void(__cdecl*)(CharacterPose*,CharacterPose*,Vec3,float))address)(a,out,state.axis,parameter);
    } else if(function==1) {
        Vec3* axis=&state.axis;float* angle=&state.angle;
        if(alias==1) axis=&a->axisZ;
        if(alias==2) axis=&b->axisY;
        if(alias==3) angle=&a->position.x;
        if(alias==4) {axis=&a->axisY;angle=&axis->x;}
        if(alias==5) b=a;
        ((void(__cdecl*)(CharacterPose*,CharacterPose*,Vec3*,float*))address)(a,b,axis,angle);
    } else {
        if(alias==1) out=a;
        if(alias==2) out=b;
        if(alias==3) b=a;
        if(alias==4) b=out=a;
        ((void(__cdecl*)(CharacterPose*,CharacterPose*,CharacterPose*,float))address)(a,b,out,parameter);
    }
}
int main(int argc,char** argv) {
    if(argc!=6 || !MapReference(argv[1],argv[2])) return 3;
    DWORD candidate[3]={strtoul(argv[3],0,16),strtoul(argv[4],0,16),strtoul(argv[5],0,16)};
    const DWORD retail[3]={0x4a7dc0,0x4a7fd0,0x4a8470};
    const int aliases[3]={2,6,5};
    const float parameters[]={-6.2831855f,-1,-0.0000001f,-0.0f,0,0.0000001f,
        0.25f,0.5f,0.99999994f,1,1.00000012f,1.5707964f,3.1415927f,6.2831855f};
    static State expected;int checks[3]={0,0,0};
    for(int fn=0;fn<3;++fn) for(int seed=0;seed<64;++seed)
    for(int shape=0;shape<4;++shape) for(int scale=0;scale<6;++scale)
    for(int alias=0;alias<aliases[fn];++alias)
    for(int param=0;param<(fn==1?1:14);++param) {
        Reset(seed,shape,scale);Invoke(fn,retail[fn],alias,parameters[param]);
        memcpy(&expected,&state,sizeof(state));
        Reset(seed,shape,scale);Invoke(fn,candidate[fn],alias,parameters[param]);
        if(memcmp(&expected,&state,sizeof(state))) {
            printf("Pose mismatch function%d seed%d shape%d scale%d alias%d parameter%d\n",fn,seed,shape,scale,alias,param);
            for(int k=0;k<sizeof(state);++k) if(((BYTE*)&expected)[k]!=((BYTE*)&state)[k]) {
                printf("byte%d %02x/%02x\n",k,((BYTE*)&expected)[k],((BYTE*)&state)[k]);break;
            }
            return 2;
        }
        ++checks[fn];
    }
    printf("Pose helpers: RotatePose %d, PoseRotation %d, InterpolatePose %d complete-state cases match\n",checks[0],checks[1],checks[2]);
    return 0;
}
