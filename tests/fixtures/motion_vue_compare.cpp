// Private differential execution of the actual standalone candidate object.
// Streams, allocation and node lookup are scripted; text parsing uses the same
// native CRT for both runs. Production never maps or invokes retail code.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "motion/Motnctrl.h"
#include "core/DebugAlloc.h"

extern "C" void __cdecl __CxxFrameHandler();
static unsigned char referenceSpace[0x800000];
UnknownResourceManager* g_ResourceManager = (UnknownResourceManager*)0x12345000;
// VC6 requires the other functions in the full sample object to resolve too.
// Those functions are outside this test. Any accidental traversal fails, rather
// than substituting an arithmetic or animation result and concealing coverage.
struct TreeMatrix4;
void MatrixMultiply(Matrix4*,Matrix4,Matrix4) { ExitProcess(111); }
void AxisAngleMatrix(TreeMatrix4*,float,float,float,float) { ExitProcess(112); }
Vec3 CrossProductCall(const Vec3&,const Vec3&) { ExitProcess(113);return Vec3(); }
Vec3 Vec3Normalize(const Vec3&) { ExitProcess(114);return Vec3(); }
float FastInvSqrt(float) { ExitProcess(115);return 0; }
void Character::BlendToMotion(Motion*) { ExitProcess(116); }
int AdvanceClamped(Motion*,MotionPoseList**,MotionPoseList**,MotionPoseList**,float*,float*) {
    ExitProcess(117);return 0;
}
Vec3* Vec3SubtractCall(Vec3*,const Vec3*,const Vec3*) { ExitProcess(118);return 0; }
Vec3* Vec3ScaleCall(Vec3*,const Vec3*,float) { ExitProcess(119);return 0; }
Vec3* Vec3AddCall(Vec3*,const Vec3*,const Vec3*) { ExitProcess(120);return 0; }
struct Scenario { int frames, transforms, requested, mask, format, mode, truncate; };
struct Block { int size, alive; unsigned char bytes[4096]; };
struct Observation {
    int result, exception, events, blocks;
    int trace[256][6];
    unsigned char motion[sizeof(Motion)], character[sizeof(Character)], manager[sizeof(MotionManager)];
    Block heap[16];
};
static Scenario scenario;
static Observation observed, expected;
static Block heap[16];
static unsigned char characterBytes[sizeof(Character)], motionBytes[sizeof(Motion)], managerBytes[sizeof(MotionManager)];
static Character* character = (Character*)characterBytes;
static Motion* motion = (Motion*)motionBytes;
static MotionManager* manager = (MotionManager*)managerBytes;
static char lines[32][256];
static int lineCount, cursor, blocks, reads, finds, allocations;
static void Event(int op,int a=0,int b=0,int c=0,int d=0,int e=0) {
    if (observed.events>=256) ExitProcess(90);
    int* p=observed.trace[observed.events++];p[0]=op;p[1]=a;p[2]=b;p[3]=c;p[4]=d;p[5]=e;
}
static int Identity(const void* p) {
    if (!p) return 0;
    if (p==character) return -1;
    if (p==manager) return -2;
    if (p==g_ResourceManager) return -3;
    for (int i=0;i<blocks;++i)
        if ((DWORD)p>=(DWORD)heap[i].bytes && (DWORD)p<(DWORD)heap[i].bytes+4096)
            return (i+1)*8192+(DWORD)p-(DWORD)heap[i].bytes;
    return -4;
}
static unsigned int Hash(const char* p) {
    if (!p) return 0;
    unsigned int h=2166136261u;while (*p) h=(h^(unsigned char)*p++)*16777619u;return h;
}
static int Source(const char* file) {
    const char* p=strrchr(file,'\\');const char* q=strrchr(file,'/');
    if (!p || (q && q>p)) p=q;
    if (strcmp(p?p+1:file,"Motnctrl.cpp")) ExitProcess(91);
    return 1; // Only the validated diagnostic path is normalized.
}
static void* Allocate(unsigned int size) {
    if (size>4096 || blocks==16) ExitProcess(92);
    Block& b=heap[blocks++];b.size=size;b.alive=1;memset(b.bytes,0xa5,sizeof(b.bytes));return b.bytes;
}
static void Retire(void* p) {
    int id=Identity(p);if (id<8192 || id%8192) ExitProcess(93);
    Block& b=heap[id/8192-1];if (!b.alive) ExitProcess(94);b.alive=0;
}
void* operator new(unsigned int size,const char* file,int line) {
    Event(1,size,line,Source(file));if (scenario.mode==1) throw 71;
    void* p=Allocate(size);Event(2,Identity(p));return p;
}
void operator delete(void* p) {
    if (!p) return;
    if (Identity(p)<0) { free(p);return; }
    Event(3,Identity(p));Retire(p);
}
void operator delete(void* p,const char* file,int line) {
    Event(4,Identity(p),line,Source(file));Retire(p);
}
void* DebugMalloc(unsigned int size,const char* file,int line) {
    Event(5,size,line,++allocations,Source(file));
    if (scenario.mode==4 && allocations==1) throw 74;
    if (scenario.mode==5 && allocations==2) throw 75;
    if ((scenario.mode==11 && allocations==1) || (scenario.mode==12 && allocations==2)) {
        Event(6,0);return 0;
    }
    void* p=Allocate(size);Event(6,Identity(p));return p;
}
void* DebugRealloc(void* p,unsigned int size,const char* file,int line) {
    Event(7,Identity(p),size,line,Source(file));
    if (scenario.mode==8) return 0;
    if (scenario.mode==9) throw 79;
    void* out=Allocate(size);
    if (p) {
        int old=Identity(p)/8192-1;
        memcpy(out,p,heap[old].size<(int)size?heap[old].size:size);Retire(p);
    }
    Event(8,Identity(out));return out;
}
UnknownTextureStream::UnknownTextureStream(UnknownResourceManager* owner) {
    Event(9,Identity(this),Identity(owner));if (scenario.mode==2) throw 72;
    memset(this,0,sizeof(*this));archives=owner;sliceOffset=37;
}
UnknownTextureStream::~UnknownTextureStream() { Event(10,Identity(this)); }
int UnknownTextureStream::Open(const char* path,const char* mode,int flags) {
    Event(11,Identity(this),Hash(path),Hash(mode),flags);return scenario.mode!=3;
}
int UnknownTextureStream::Seek(int offset,int origin,int flag) {
    Event(12,Identity(this),offset,origin,flag);cursor=0;return scenario.mode==10?-1:0;
}
int UnknownTextureStream::ReadLine(char* out,int size) {
    Event(13,Identity(this),size,cursor,++reads);
    if (scenario.mode==6 && reads==3) throw 76;
    if (cursor>=lineCount) { Event(14,0);return 0; }
    strncpy(out,lines[cursor++],size);out[size]=0;
    Event(14,1,Hash(out));return 1;
}
unsigned char Character::FindNode(const char* name) {
    Event(15,Identity(this),Hash(name),++finds);
    if (scenario.mode==7 && finds==2) throw 77;
    if (!name) return 0xff; // Scripted unknown lookup, including incomplete line tokens.
    int node=name[0]=='N'||name[0]=='n'?atoi(name+1):-1;
    return node>=0 && node<4 && (scenario.mask&(1<<node))?(unsigned char)(node*3+2):0xff;
}
struct ConstructorCall { UnknownTextureStream* Stream(UnknownResourceManager* owner) {
    return ::new(this) UnknownTextureStream(owner);
} };
struct DestructorCall { void Stream() { ((UnknownTextureStream*)this)->~UnknownTextureStream(); } };
struct NodeLookupCall { unsigned char Find(const char* name) {
    return ((Character*)this)->FindNode(name);
} };
template<class T> static DWORD Address(T method) {
    if (sizeof(method)!=4) ExitProcess(95);DWORD out;memcpy(&out,&method,4);return out;
}
static DWORD Replacement(DWORD target) {
    switch (target) {
    case 0x4a3010:return (DWORD)(void*(__cdecl*)(unsigned int,const char*,int))operator new;
    case 0x4a3060:return (DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;
    case 0x4a30c0:return (DWORD)(void(__cdecl*)(void*))operator delete;
    case 0x4a2e20:return (DWORD)DebugMalloc;
    case 0x4a2ec0:return (DWORD)DebugRealloc;
    case 0x460d10:return Address(&ConstructorCall::Stream);
    case 0x460d60:return Address(&DestructorCall::Stream);
    case 0x460f50:return Address(&UnknownTextureStream::Open);
    case 0x461aa0:return Address(&UnknownTextureStream::ReadLine);
    case 0x461340:return Address(&UnknownTextureStream::Seek);
    case 0x4a6b50:return Address(&NodeLookupCall::Find);
    case 0x534aaf:return (DWORD)sprintf;
    case 0x535d3d:return (DWORD)_strupr;
    case 0x536397:return (DWORD)strtok;
    case 0x536070:return (DWORD)_stricmp;
    case 0x536340:return (DWORD)atof;
    case 0x53471a:return (DWORD)__CxxFrameHandler;
    }return 0;
}
static int MapReference(const char* path,const char* patches) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if ((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
        !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* f=fopen(path,"rb");if (!f) return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if (!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if (pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    *(UnknownResourceManager**)0x572b44=g_ResourceManager;
    f=fopen(patches,"rb");if(!f)return 0;DWORD patch[3];
    while(fread(patch,sizeof(patch),1,f)==1) {
        DWORD at=patch[0],dest=Replacement(patch[1]);
        if(!dest || !((at>=0x4a5e40 && at+5<=0x4a6282)||(at>=0x54c450 && at+5<=0x54c474)) ||
           *(BYTE*)at!=patch[2] || at+5+*(DWORD*)(at+1)!=patch[1])return 0;
        *(DWORD*)(at+1)=dest-at-5;
    }fclose(f);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static void BuildLines() {
    lineCount=0;
    for(int frame=0;frame<scenario.frames;++frame) {
        sprintf(lines[lineCount++],scenario.format?"frame %d\n":"FRAME %d\n",frame);
        for(int node=0;node<scenario.transforms;++node) {
            int a=frame*19+node*3;
            sprintf(lines[lineCount++],scenario.format?
                "transform \"n%d\" 1 0 0 %d.25 -2.5 3e-2 4 -%d.75 6.125 7 8 -9.5\n":
                "TRANSFORM \"N%d\" 1 0 0 %d.25 -2.5 3e-2 4 -%d.75 6.125 7 8 -9.5\n",node,a,a+1);
        }
    }
    if(scenario.truncate && lineCount) --lineCount;
}
static int InvokeSEH(DWORD address) {
    typedef int(MotionManager::*Fn)(Character*,const char*,Motion*);Fn fn;memcpy(&fn,&address,4);
    __try {return (manager->*fn)(character,"test.vue",motion);}
    __except(GetExceptionCode()==0xe06d7363?EXCEPTION_CONTINUE_SEARCH:EXCEPTION_EXECUTE_HANDLER) {
        observed.exception=GetExceptionCode();return -1;
    }
}
static void Run(int reference) {
    memset(&observed,0,sizeof(observed));memset(heap,0,sizeof(heap));
    memset(characterBytes,0x59,sizeof(characterBytes));memset(managerBytes,0x65,sizeof(managerBytes));
    memset(motionBytes,0x73,sizeof(motionBytes));motion->frames=0;motion->frameCount=scenario.requested;
    blocks=cursor=reads=finds=allocations=0;BuildLines();
    try {observed.result=InvokeSEH(reference?0x4a5e40:Address(&MotionManager::LoadVue));}
    catch(int code){observed.exception=code;}
    observed.blocks=blocks;memcpy(observed.motion,motionBytes,sizeof(motionBytes));
    memcpy(observed.character,characterBytes,sizeof(characterBytes));memcpy(observed.manager,managerBytes,sizeof(managerBytes));
    memcpy(observed.heap,heap,sizeof(heap));
}
int main(int argc,char** argv) {
    if(argc!=3 || !MapReference(argv[1],argv[2]))return 3;
    int checks=0;
    for(int frames=0;frames<4;++frames)for(int transforms=0;transforms<4;++transforms)
    for(int requested=0;requested<5;++requested)for(int mask=0;mask<8;++mask)
    for(int format=0;format<2;++format)for(int mode=0;mode<13;++mode)
    for(int truncate=0;truncate<2;++truncate) {
        scenario.frames=frames;scenario.transforms=transforms;scenario.requested=requested;
        scenario.mask=mask;scenario.format=format;scenario.mode=mode;scenario.truncate=truncate;
        Run(1);expected=observed;Run(0);++checks;
        if(memcmp(&expected,&observed,sizeof(observed))) {
            int first=0;while(first<sizeof(observed) && ((BYTE*)&expected)[first]==((BYTE*)&observed)[first])++first;
            printf("Mismatch %d byte%d frames%d transforms%d requested%d mask%d format%d mode%d truncate%d exceptions%x/%x\n",
                   checks,first,frames,transforms,requested,mask,format,mode,truncate,expected.exception,observed.exception);return 2;
        }
        if (mode==1 && (observed.exception!=71 || observed.blocks!=0)) return 4;
        if (mode==2 && (observed.exception!=72 || observed.blocks!=1 || observed.heap[0].alive)) return 5;
        // Preserve the retail early-return leak instead of letting the fixture
        // silently clean it up. This test does not endorse that ownership bug.
        if (mode==3 && (observed.result || observed.exception || observed.blocks!=1 || !observed.heap[0].alive)) return 6;
    }
    printf("LoadVue: %d scenarios match complete state, ownership and calls\n",checks);return 0;
}
