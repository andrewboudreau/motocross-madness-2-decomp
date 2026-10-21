// Private complete-body comparison; production candidate is a separate object.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TrackGameGlobal.h"

static BYTE referenceSpace[0x800000];
struct World {
    BYTE camera[0x384],target[0x20],subject[0xbf0],games[2][0x300],controls[2][0x20];
    float values[8][5]; BYTE heap[6][32];
};
static World world;
TrackGame* g_TrackGame;
static DWORD cameraTable[76],controlTable[4];
static FollowCamera* CameraObject(){return (FollowCamera*)world.camera;}
static float& F(int offset){return *(float*)(world.camera+offset);}
static int& I(int offset){return *(int*)(world.camera+offset);}
static BYTE& B(int offset){return world.camera[offset];}
static Vector3& V(int offset){return *(Vector3*)(world.camera+offset);}
struct Event { int kind,a,b,c;float args[16];World state;int game; };
struct Observation {int result,exception,faultKind,faultAddress,count,allocations,baseTicks;
    unsigned fpStatus;Event events[80];World state;int game;};
static Observation observed,expected;
struct Scenario {unsigned seed;int state,current,flags,returns,keys,allocate,failAllocation,mutateAt,throwAt,invalid,round;
    float dt,yaw,blend,rate,fov,distance;};
static Scenario scenario;
static int checks;
static int GameIndex(){return g_TrackGame==(TrackGame*)world.games[1];}
static void VerifyThis(const void* self){if(self!=CameraObject())ExitProcess(81);}
static Event& Begin(int kind,int a=0,int b=0,int c=0){
    if(observed.count>=80)ExitProcess(82);
    Event& e=observed.events[observed.count++];e.kind=kind;e.a=a;e.b=b;e.c=c;
    e.state=world;e.game=GameIndex();return e;
}
static void Finish(){
    if(observed.count==scenario.mutateAt){
        // Keep storage valid while changing values whose reloads are observable.
        F(0x234)=-F(0x234);F(0x258)+=1.25f;F(0x220)+=2.0f;
        B(0x274)=!B(0x274);B(0x276)=!B(0x276);I(0x268)=!I(0x268);
        I(0x244)=(I(0x244)+3)&7;
        world.values[0][0]+=7.0f;world.values[6][0]-=3.0f;
        *(DWORD*)(world.target+8)=scenario.current?0:(DWORD)CameraObject();
        g_TrackGame=(TrackGame*)world.games[1-GameIndex()];*(TrackGame**)0x56e26c=g_TrackGame;
    }
    if(observed.count==scenario.throwAt)throw 0x457;
}
static void VectorArgs(Event& e,const Vector3& v,int index=0){memcpy(e.args+index,&v,12);}
static Vector3 ReturnedVector(int kind){
    float seed=(float)(int)(scenario.seed%71)-35.0f;
    return Vector3(seed*.125f+kind,seed*-.0625f+2.0f,kind*-.5f+seed*.25f);
}
class Hooks {
public:
    Vector3 Heading(){VerifyThis(this);Begin(41);Finish();return ReturnedVector(41);}
    Vector3 Target(float dt){VerifyThis(this);Event& e=Begin(57);e.args[0]=dt;Finish();return ReturnedVector(57);}
    Vector3 Blend(){VerifyThis(this);Begin(33);Finish();return ReturnedVector(33);}
    bool Moving(bool current,float dt){VerifyThis(this);Event& e=Begin(45,current);e.args[0]=dt;Finish();return (scenario.returns&1)!=0;}
    void Turn(bool wide){VerifyThis(this);Begin(42,wide);Finish();F(0x308)+=.25f;}
    bool Following(){VerifyThis(this);Begin(75);Finish();return (scenario.returns&2)!=0;}
    bool Crash(){VerifyThis(this);Begin(74);Finish();return (scenario.returns&4)!=0;}
    Vector3 Aim(int wide,bool following,float dt){VerifyThis(this);Event& e=Begin(48,wide,following);e.args[0]=dt;Finish();return ReturnedVector(48);}
    Vector3 Position(bool following,const Vector3& base,float dt){VerifyThis(this);Event& e=Begin(49,following);VectorArgs(e,base);e.args[3]=dt;Finish();return ReturnedVector(49);}
    void SetTarget(const Vector3& v){VerifyThis(this);Event& e=Begin(43);VectorArgs(e,v);Finish();}
    void SetPosition(const Vector3& v){VerifyThis(this);Event& e=Begin(44);VectorArgs(e,v);Finish();}
    bool Settled(const Vector3& v,bool wide,bool moved){VerifyThis(this);Event& e=Begin(36,wide,moved);VectorArgs(e,v);Finish();return (scenario.returns&8)!=0;}
    void Touch(){VerifyThis(this);Begin(59);Finish();}
    void Adjust(Vector3* v){VerifyThis(this);Event& e=Begin(52);VectorArgs(e,*v);Finish();v->y+=.125f;}
    float Roll(){VerifyThis(this);Begin(51);Finish();return -.375f;}
    void Mode53(){VerifyThis(this);Begin(53);Finish();}
    void Mode54(){VerifyThis(this);Begin(54);Finish();}
    bool Mode56(){VerifyThis(this);Begin(56);Finish();return (scenario.returns&16)!=0;}
    unsigned char Mode55(){VerifyThis(this);Begin(55);Finish();return scenario.returns&32?0x80:0;}
    void Preset67(){VerifyThis(this);Begin(67);Finish();F(0x258)=37.0f;F(0x220)=12.0f;}
    void Preset68(){VerifyThis(this);Begin(68);Finish();F(0x258)=61.0f;F(0x22c)=.75f;}
    int Look(Vector3 v){VerifyThis(this);Event& e=Begin(29);VectorArgs(e,v);Finish();V(0x17c)=v;return -11;}
    int Key(int key,int kind,int modifiers,int device){
        int who=(BYTE*)this==world.controls[1];if((BYTE*)this!=world.controls[who])ExitProcess(83);
        if(kind || modifiers!=(int)0x80000000 || device || (key!=0xc7&&key!=0xcf))ExitProcess(84);
        Begin(100,key,who,modifiers);Finish();return (scenario.keys&(key==0xc7?1:2))?-7:0;
    }
};
void UnknownFollowCameraSubject::QueryGround(Vector3* point,int a,int b,int c){
    if((BYTE*)this!=world.subject || a || b || c)ExitProcess(85);
    Event& e=Begin(101);VectorArgs(e,*point);Finish();point->y+=1.25f;
}
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result,Vector3* value,Vector3* axis,float angle){
    Event& e=Begin(102,result==value,result==axis,value==axis);VectorArgs(e,*value);VectorArgs(e,*axis,3);e.args[6]=angle;Finish();
    // Deterministic scripted dependency, not a substitute Direct3DRM rotation.
    Vector3 v=*value;result->x=v.x+axis->x*angle;result->y=v.y+axis->y*angle;result->z=v.z+axis->z*angle;return result;
}
void Camera::SetFrame(const Vector3* position,const Vector3* forward,const Vector3* up,const float* r,const float* fov){
    VerifyThis(this);if(!position || forward || up || fov)ExitProcess(86);
    Event& e=Begin(103,r!=0);VectorArgs(e,*position);if(r)e.args[3]=*r;Finish();eyePosition=*position;if(r)roll=*r;
}
void Camera::SetFieldOfView(float value){VerifyThis(this);Event& e=Begin(104);e.args[0]=value;Finish();fieldOfView=value;}
int Camera::Tick(float dt){VerifyThis(this);Event& e=Begin(105);e.args[0]=dt;Finish();observed.baseTicks++;previousEyePosition=eyePosition;return -1;}
void* operator new(size_t size,const char* file,int line){
    if(size!=20 || line<0x4b0 || line>0x4b6 || line==0x4b3)ExitProcess(87);
    const char* suffix=strrchr(file,'\\');if(!suffix)suffix=strrchr(file,'/');suffix=suffix?suffix+1:file;
    if(strcmp(file,"D:\\aardvark\\VC\\krusty2\\FollowCam.cpp") && strcmp(suffix,"FollowCameraTick.cpp") && strcmp(suffix,"FollowCameraNearMisses.cpp"))ExitProcess(88);
    Begin(106,size,line);Finish();int n=observed.allocations++;if(n>=6)ExitProcess(89);
    if(scenario.failAllocation==n+1)return 0;return world.heap[n];
}
template<class T> static DWORD Address(T method){union {T method;DWORD address;}u;u.method=method;return u.address;}
void InitFastSqrtTable();
static DWORD Hook(DWORD address){
    switch(address){
    case 0x507c10:return Address(&UnknownFollowCameraSubject::QueryGround);
    case 0x53304c:return (DWORD)D3DRMVectorRotate;
    case 0x42e9b0:return Address(&Camera::SetFrame);
    case 0x460b50:return (DWORD)FastSqrt;
    case 0x4a3010:{typedef void*(__cdecl* Alloc)(size_t,const char*,int);Alloc a=operator new;return (DWORD)a;}
    case 0x42e930:return Address(&Camera::SetFieldOfView);
    // Virtual &Camera::Tick is a dispatch thunk, so obtain this test's direct
    // provider from a nonvirtual call through the independent member view.
    }return 0;
}
// Match the real nonvirtual direct call to Camera::Tick, without a virtual
// member-pointer thunk. This tiny fixture wrapper invokes the scripted helper.
class BaseTickHook {public:int Tick(float dt){return ((Camera*)this)->Camera::Tick(dt);}};
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[2];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){
        BYTE* site=(BYTE*)entry[0];DWORD hook=entry[1]==0x42e690?Address(&BaseTickHook::Tick):Hook(entry[1]);
        if(!hook || site<(BYTE*)0x465c20 || site+5>(BYTE*)0x466a78 || *site!=0xe8 ||
           (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1])return 0;
        *(DWORD*)(site+1)=hook-(DWORD)(site+5);++records;
    }fclose(f);
    if(records!=16 || !FlushInstructionCache(GetCurrentProcess(),0,0))return 0;
    // Independently checked retail initializer bodies supply the reference axes.
    ((void(__cdecl*)())0x466f70)();((void(__cdecl*)())0x467000)();
    InitFastSqrtTable();return 1;
}
static int Filter(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];
    observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Invoke(int retail){
    union {int(FollowCamera::*method)(float);DWORD address;}u;u.address=0x465c20;
    __try{observed.result=retail?(CameraObject()->*u.method)(scenario.dt):CameraObject()->FollowCamera::Tick(scenario.dt);}
    __except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){
    memset(&observed,0,sizeof(observed));observed.result=-501;
    memset(&world,0x53,sizeof(world));*(DWORD*)world.camera=(DWORD)cameraTable;
    *(DWORD*)(world.camera+0x18)=(DWORD)world.target;*(DWORD*)(world.target+8)=scenario.current?(DWORD)CameraObject():0;
    for(int g=0;g<2;++g){*(DWORD*)world.controls[g]=(DWORD)controlTable;*(DWORD*)(world.games[g]+0x14)=(DWORD)world.controls[g];*(int*)(world.games[g]+0x1c4)=((scenario.flags>>7)&1)^g;}
    g_TrackGame=(TrackGame*)world.games[0];*(TrackGame**)0x56e26c=g_TrackGame;
    // Finite initialized storage; unused bytes remain canaries in every snapshot.
    for(int offset=0x16c;offset<0x220;offset+=4)F(offset)=.25f;
    for(offset=0x220;offset<0x384;offset+=4)F(offset)=.25f;
    I(0x240)=(int)world.subject;I(0x244)=scenario.state;I(0x248)=scenario.flags&1?6:2;
    I(0x250)=scenario.flags&2?7:4;I(0x268)=scenario.flags&4?1:0;
    B(0x274)=(scenario.flags>>3)&1;B(0x275)=(scenario.flags>>4)&1;B(0x276)=(scenario.flags>>5)&1;
    B(0x278)=(scenario.flags>>6)&1;B(0x279)=(scenario.flags>>8)&1;
    F(0x220)=scenario.distance;F(0x224)=7.5f;F(0x228)=27.0f;F(0x22c)=.4f;F(0x230)=-.5f;F(0x234)=scenario.yaw;F(0x238)=1.25f;
    F(0x24c)=43.0f;F(0x254)=56.0f;F(0x258)=scenario.fov;F(0x16c)=77.0f;F(0x1dc)=109.0f;F(0x1e0)=10.0f;
    V(0x170)=ReturnedVector(20);V(0x25c)=ReturnedVector(13);V(0x2a8)=ReturnedVector(7);
    F(0x2c4)=18.0f;F(0x2c8)=-.2f;F(0x2cc)=scenario.yaw;F(0x2f4)=10.0f;F(0x2f8)=280.0f;F(0x2fc)=70.0f;F(0x300)=20.0f;F(0x304)=scenario.blend;
    for(int n=0;n<8;++n){
        world.values[n][0]=(float)n-.25f;world.values[n][1]=scenario.rate;world.values[n][2]=FLT_MAX;world.values[n][3]=-FLT_MAX;world.values[n][4]=1;
        I(0x27c+n*4)=(int)world.values[n];
    }
    if(scenario.allocate)I(0x288)=0;
    if(scenario.invalid==1)I(0x18)=0;
    if(scenario.invalid==2)I(0x294)=0;
    if(scenario.invalid==3)I(0x27c)=0;
    if(scenario.invalid==4){g_TrackGame=0;*(TrackGame**)0x56e26c=0;}
    _fpreset();_controlfp(scenario.round,_MCW_RC);
    try{Invoke(retail);}catch(int e){observed.exception=e;}
    observed.fpStatus=_statusfp();observed.state=world;observed.game=GameIndex();
    _fpreset();
}
static void Compare(){
    Run(1);expected=observed;Run(0);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){
        BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;
        printf("Mismatch%d offset%d %02x/%02x state%d current%d flags%x returns%x seed%u dt%g yaw%g allocate%d fail%d mutate%d throw%d invalid%d events%d/%d fp%x/%x\n",
            checks,offset,a[offset],b[offset],scenario.state,scenario.current,scenario.flags,scenario.returns,scenario.seed,scenario.dt,scenario.yaw,
            scenario.allocate,scenario.failAllocation,scenario.mutateAt,scenario.throwAt,scenario.invalid,expected.count,observed.count,expected.fpStatus,observed.fpStatus);ExitProcess(2);
    }
    if(!observed.exception && (observed.result!=1 || observed.baseTicks!=scenario.current))ExitProcess(90);
}
static void Default(){memset(&scenario,0,sizeof(scenario));scenario.dt=.016f;scenario.yaw=2.25f;scenario.blend=.125f;scenario.rate=.3f;scenario.fov=55;scenario.distance=20;}
static unsigned randomState=0x7313abcd;
static unsigned Random(){randomState=randomState*1664525u+1013904223u;return randomState;}
static float Finite(){union {unsigned bits;float value;}u;unsigned r=Random();u.bits=(r&0x807fffff)|(((r>>24)%40+107)<<23);return u.value;}
int main(int argc,char**argv){
    if(argc!=3 || !MapReference(argv[1],argv[2]))return 3;
    if(sizeof(FollowCamera)!=0x384 || sizeof(UnknownFollowCameraValue)!=20)return 4;
    for(int n=0;n<76;++n)cameraTable[n]=(DWORD)abort;for(n=0;n<4;++n)controlTable[n]=(DWORD)abort;
    cameraTable[41]=Address(&Hooks::Heading);cameraTable[57]=Address(&Hooks::Target);cameraTable[33]=Address(&Hooks::Blend);
    cameraTable[45]=Address(&Hooks::Moving);cameraTable[42]=Address(&Hooks::Turn);cameraTable[75]=Address(&Hooks::Following);cameraTable[74]=Address(&Hooks::Crash);
    cameraTable[48]=Address(&Hooks::Aim);cameraTable[49]=Address(&Hooks::Position);cameraTable[43]=Address(&Hooks::SetTarget);cameraTable[44]=Address(&Hooks::SetPosition);
    cameraTable[36]=Address(&Hooks::Settled);cameraTable[59]=Address(&Hooks::Touch);cameraTable[52]=Address(&Hooks::Adjust);cameraTable[51]=Address(&Hooks::Roll);
    cameraTable[53]=Address(&Hooks::Mode53);cameraTable[54]=Address(&Hooks::Mode54);cameraTable[56]=Address(&Hooks::Mode56);cameraTable[55]=Address(&Hooks::Mode55);
    cameraTable[67]=Address(&Hooks::Preset67);cameraTable[68]=Address(&Hooks::Preset68);cameraTable[29]=Address(&Hooks::Look);controlTable[3]=Address(&Hooks::Key);
    // Independent ordered outcomes: an already placed camera blends and emits
    // the base tick only while current; a first tick allocates all six distinct
    // easing objects in the observed Z/X/Y then X/Y/Z ownership order.
    for(int current=0;current<2;++current){
        Default();scenario.current=current;scenario.flags=0x10;Compare();
        const int sequence[]={41,57,45,42,75,48,49,74,33,52,54,103,29,56,55,100,100,104,105};
        int count=current?19:11;
        if(observed.count!=count || observed.allocations || !B(0x278) || I(0x244)!=0)ExitProcess(93);
        for(int event=0;event<count;++event)if(observed.events[event].kind!=sequence[event])ExitProcess(94);
    }
    Default();scenario.current=1;scenario.allocate=1;Compare();
    const int fields[]={0x284,0x27c,0x280,0x288,0x28c,0x290};
    if(observed.allocations!=6 || !B(0x275))ExitProcess(95);
    for(n=0;n<6;++n){if(I(fields[n])!=(int)world.heap[n])ExitProcess(96);for(int pad=20;pad<32;++pad)if(world.heap[n][pad]!=0x53)ExitProcess(97);}
    for(int state=-1;state<=8;++state)for(int current=0;current<2;++current)
    for(int flags=0;flags<512;++flags)for(int returns=0;returns<64;++returns){
        Default();scenario.state=state;scenario.current=current;scenario.flags=flags;scenario.returns=returns;
        scenario.seed=flags+returns*7;scenario.keys=returns&3;scenario.allocate=(flags>>8)&1;Compare();
    }
    // Every reachable callback in representative branch traces can mutate or
    // throw, including all six allocations and both input/global reloads.
    for(state=0;state<8;++state)for(int returns=0;returns<64;returns+=7){
        Default();scenario.state=state;scenario.current=1;scenario.flags=0x100;scenario.allocate=1;
        scenario.returns=returns;scenario.keys=3;Compare();int count=observed.count;
        for(int event=1;event<=count;++event){scenario.throwAt=event;Compare();scenario.throwAt=0;scenario.mutateAt=event;Compare();scenario.mutateAt=0;}
    }
    const float edges[]={-1.0f,-0.0f,0.0f,1.0e-30f,.016f,.3f,.99999994f,1.0f,1.00000012f,2.0f,2.00000024f,1000.0f,1.0e20f};
    const int rounds[]={_RC_NEAR,_RC_UP,_RC_DOWN,_RC_CHOP};
    for(state=0;state<8;++state)for(int edge=0;edge<13;++edge)for(int rate=0;rate<13;++rate)
    for(int round=0;round<4;++round){
        Default();scenario.state=state;scenario.current=1;scenario.flags=0x110;scenario.returns=(edge+rate)&63;
        scenario.dt=edges[edge];scenario.rate=edges[rate];scenario.yaw=edges[rate];scenario.blend=edges[edge];
        scenario.fov=edges[edge];scenario.distance=edges[rate];scenario.keys=3;scenario.round=rounds[round];Compare();
    }
    for(int sample=0;sample<8192;++sample){
        Default();scenario.seed=Random();scenario.current=Random()&1;scenario.state=(Random()>>16)&7;
        scenario.flags=(Random()>>12)&511;scenario.returns=(Random()>>11)&63;scenario.keys=Random()&3;
        scenario.dt=Finite();scenario.rate=Finite();scenario.yaw=Finite();scenario.blend=Finite();scenario.fov=Finite();scenario.distance=Finite();Compare();
    }
    for(int fail=1;fail<=6;++fail){Default();scenario.current=1;scenario.allocate=1;scenario.failAllocation=fail;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.allocations!=6 || I(fields[fail-1]))ExitProcess(91);}
    for(int invalid=1;invalid<=4;++invalid){Default();scenario.current=1;scenario.flags=0x10;scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)ExitProcess(92);}
    printf("FollowCameraTick: %d full-state, callback, allocation and floating-point scenarios match\n",checks);return 0;
}
