// Private comparison of standalone motion playback and pose helper objects.
// Frame advancement and restart execute real retail dependencies. Only pose
// application is observed through a synthetic primary vtable; no game is run.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "motion/Motnctrl.h"
struct TreeMatrix4;

extern "C" double __cdecl _CIacos();
extern "C" long __cdecl _ftol();
static unsigned char referenceSpace[0x800000];
static unsigned char characterStorage[sizeof(Character)];
static Motion motions[2];
static MotionPoseList frames[2][4];
static CharacterPose poses[2][4][3];
static NodeNameEntry nodes[3];
static DWORD primaryTable[11];
struct Call {
    int slot, mask; CharacterPose pose;
    unsigned char state[sizeof(Character)];
};
static Call calls[12];
static int callCount;
static Character* Actor() { return (Character*)characterStorage; }
struct DirectCall {
    int Tick(float,int,int);
    void Blend(Motion*);
    void Apply4(int p,int mask) {
        Apply(4,p,mask);
    }
    void Apply5(int p,int mask) {
        Apply(5,p,mask);
    }
    void Apply(int slot,int p,int mask) {
        if ((void*)this!=Actor() || callCount>=12) ExitProcess(81);
        Call& c=calls[callCount++]; c.slot=slot; c.mask=mask;
        c.pose=*(CharacterPose*)p;
        // Bytes +1..+3 are structure padding, never initialized by the body.
        memset((BYTE*)&c.pose+1,0,3);
        memcpy(c.state,characterStorage,sizeof(characterStorage));
    }
};
template<class T> static DWORD Address(T p) {
    if (sizeof(p)!=4) ExitProcess(82);
    DWORD a; memcpy(&a,&p,4); return a;
}
int AdvanceClamped(Motion* m,MotionPoseList** c,MotionPoseList** a,MotionPoseList** b,float* t,float* w) {
    return ((int(__cdecl*)(Motion*,MotionPoseList**,MotionPoseList**,MotionPoseList**,float*,float*))0x4a5750)(m,c,a,b,t,w);
}
void Character::BlendToMotion(Motion* m) {
    typedef void(DirectCall::*Fn)(Motion*); DWORD a=0x4a8b60; Fn fn; memcpy(&fn,&a,4);
    (((DirectCall*)this)->*fn)(m);
}
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
        DWORD target=pair[1]==0x535240?(DWORD)_CIacos:pair[1]==0x534a88?(DWORD)_ftol:0;
        if (!target || *(BYTE*)pair[0]!=0xe8) return 0;
        *(DWORD*)(pair[0]+1)=target-pair[0]-5; ++count;
    }
    fclose(f); if (count!=4) return 0;
    ((void(__cdecl*)())0x460bb0)();
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

static void Reset(int seed,int count,int mode,int stopped,int looping,int flagMask) {
    memset(characterStorage,0x53,sizeof(characterStorage)); memset(calls,0,sizeof(calls)); callCount=0;
    memset(motions,0,sizeof(motions)); memset(frames,0,sizeof(frames)); memset(poses,0,sizeof(poses)); memset(nodes,0,sizeof(nodes));
    *(DWORD*)characterStorage=(DWORD)primaryTable;
    for (int n=0;n<3;++n) nodes[n].applyTranslation=(flagMask>>n)&1;
    for (int m=0;m<2;++m) {
        motions[m].frames=frames[m]; motions[m].frameCount=4; motions[m].posesPerFrame=count;
        motions[m].fps=seed%3==0?1:seed%3==1?5:15;
        const int rates[]={7,24,30,60,120,16777215,16777217,16777219};
        if (seed>=16) motions[m].fps=rates[(seed-16)%8];
        motions[m].frameTime=1.0f/motions[m].fps; motions[m].looping=looping;
        motions[m].transitionTime=seed%2?0.25f:0;
        for (int f=0;f<4;++f) {
            frames[m][f].frameIndex=f; frames[m][f].poses=poses[m][f]; frames[m][f].count=count;
            for (int p=0;p<3;++p) {
                CharacterPose& pose=poses[m][f][p]; pose.nodeIndex=(unsigned char)p; pose.hasPose=(seed+p+f)%2;
                int rotation=(seed+m+f+p)%4;
                const float cs[4]={1,0,-1,0},sn[4]={0,1,0,-1};
                pose.axisZ=Vec3(sn[rotation],0,cs[rotation]); pose.axisY=Vec3(0,1,0);
                if (seed%4==1) pose.axisY=Vec3(0.25f,(float)(f+1),-0.5f);
                if (seed%4==2) pose.axisZ=Vec3(0,0,1);
                if (seed%4==3) pose.axisY=Vec3(0,0,0);
                pose.position=Vec3((float)(p+f)*0.25f,(float)(m-f)*0.5f,(float)(seed+p-m)*0.125f);
            }
        }
    }
    Character* c=Actor(); c->nodeNames=nodes; c->nodeCount=3;
    c->motionStopped=stopped; c->currentMotion=&motions[0]; c->currentFrame=&frames[0][seed%4];
    c->blendFromMotion=&motions[1]; c->lastFrame=&frames[1][(seed/4)%4];
    c->chr_field_0x10=0.025f*(seed%3); c->lastFrameTime=0.03f*(seed%4);
    c->blendDuration=0.5f; c->blendFromTime=mode==1?-0.75f:mode==2?0.0f:mode==3?0.5f:0.75f;
    c->blendActive=mode!=0;
}
struct Snapshot {
    unsigned char character[sizeof(Character)];
    Motion motion[2]; MotionPoseList frame[2][4]; CharacterPose pose[2][4][3];
    NodeNameEntry node[3]; Call trace[12]; int count,result;
};
static void Capture(Snapshot* s,int result) {
    memset(s,0,sizeof(*s)); memcpy(s->character,characterStorage,sizeof(characterStorage));
    memcpy(s->motion,motions,sizeof(motions)); memcpy(s->frame,frames,sizeof(frames));
    memcpy(s->pose,poses,sizeof(poses)); memcpy(s->node,nodes,sizeof(nodes));
    memcpy(s->trace,calls,sizeof(calls)); s->count=callCount; s->result=result;
}
static int Tick(DWORD address,float dt,int mirror,int mask) {
    typedef int(DirectCall::*Fn)(float,int,int); Fn fn; memcpy(&fn,&address,4);
    return (((DirectCall*)Actor())->*fn)(dt,mirror,mask);
}
static unsigned int randomState;
static unsigned int Next() { randomState=randomState*1664525u+1013904223u;return randomState; }
static float Coordinate() { return ((int)(Next()%4097)-2048)/997.0f; }
static void Vary(int seed) {
    randomState=seed+1;
    for (int m=0;m<2;++m) {
        for (int f=0;f<4;++f) for (int p=0;p<3;++p) {
            CharacterPose& pose=poses[m][f][p];
            pose.axisY=Vec3(Coordinate(),Coordinate(),Coordinate());
            pose.axisZ=Vec3(Coordinate(),Coordinate(),Coordinate());
            pose.position=Vec3(Coordinate(),Coordinate(),Coordinate());
        }
    }
    Actor()->blendDuration=(Next()%1000+1)/997.0f;
    Actor()->blendFromTime=Coordinate();
    Actor()->chr_field_0x10=(Next()%1000)/997.0f;
    Actor()->lastFrameTime=(Next()%1000)/997.0f;
}
int main(int argc,char** argv) {
    if (argc!=4 || !MapReference(argv[1],argv[2])) return 3;
    DWORD candidate=strtoul(argv[3],0,16);
    memset(primaryTable,0,sizeof(primaryTable)); primaryTable[4]=Address(&DirectCall::Apply4); primaryTable[5]=Address(&DirectCall::Apply5);
    static Snapshot expected,actual; int checks=0;
    const float steps[]={0.0f,0.01f,0.1f,0.3f,1.0f,2.5f,0.016666667f,0.06666667f,0.033333333f,0.1234567f};
    for (int seed=0;seed<64;++seed) for (int count=0;count<4;++count)
    for (int mode=0;mode<5;++mode) for (int stopped=0;stopped<2;++stopped)
    for (int looping=0;looping<2;++looping) for (int mirror=0;mirror<2;++mirror)
    for (int flags=0;flags<8;++flags) for (int dt=0;dt<10;++dt) {
        int mask=(seed+flags)%3==0?-1:seed*17+flags;
        Reset(seed,count,mode,stopped,looping,flags); Capture(&expected,Tick(0x4a70c0,steps[dt],mirror,mask));
        Reset(seed,count,mode,stopped,looping,flags); Capture(&actual,Tick(candidate,steps[dt],mirror,mask));
        if (memcmp(&expected,&actual,sizeof(actual))) {
            printf("Mismatch seed%d count%d mode%d stopped%d loop%d mirror%d flags%d dt%d calls%d/%d\n",seed,count,mode,stopped,looping,mirror,flags,dt,expected.count,actual.count);
            for (int b=0;b<sizeof(actual);++b) if (((BYTE*)&expected)[b]!=((BYTE*)&actual)[b]) {
                printf("first byte%d: %02x/%02x\n",b,((BYTE*)&expected)[b],((BYTE*)&actual)[b]);break;
            }
            return 2;
        }
        ++checks;
    }
    for (int trial=0;trial<16384;++trial) {
        int seed=trial%64,count=3,mode=trial%5,stopped=trial%7==0;
        int looping=trial%2,mirror=(trial/2)%2,flags=(trial/4)%8;
        float step=(trial%1997)/997.0f;
        Reset(seed,count,mode,stopped,looping,flags);Vary(trial);
        Capture(&expected,Tick(0x4a70c0,step,mirror,trial));
        Reset(seed,count,mode,stopped,looping,flags);Vary(trial);
        Capture(&actual,Tick(candidate,step,mirror,trial));
        if (memcmp(&expected,&actual,sizeof(actual))) {
            printf("Varied playback mismatch trial%d seed%d mode%d flags%d\n",trial,seed,mode,flags);
            for (int b=0;b<sizeof(actual);++b) if (((BYTE*)&expected)[b]!=((BYTE*)&actual)[b]) {
                printf("first byte%d: %02x/%02x\n",b,((BYTE*)&expected)[b],((BYTE*)&actual)[b]);break;
            }
            return 2;
        }
        ++checks;
    }
    printf("Character playback: %d full state and applied pose scenarios match\n",checks);
    return 0;
}
