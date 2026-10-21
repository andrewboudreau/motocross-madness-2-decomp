// Private complete control-handler comparison. Only virtual input dependencies
// are scripted; both whole bodies execute their own vector and scalar math.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/TrackGameGlobal.h"
static BYTE referenceSpace[0x800000];
struct World {BYTE camera[0x384],games[2][0x20],controls[2][0x20],joysticks[2][0x20];};
static World world;
TrackGame* g_TrackGame;
static DWORD controlsTable[4],joystickTable[5];
static float& F(int offset){return *(float*)(world.camera+offset);}
static int& I(int offset){return *(int*)(world.camera+offset);}
struct Event {int kind,receiver,code,argument,result;float x,y;int game;World state;};
struct Observation {int result,exception,faultKind,faultAddress,count,game;unsigned fpStatus;Event events[32];World state;};
static Observation observed,expected;
struct Scenario {unsigned seed;int state,mode,mask,joystick,active,mutateAt,mutation,throwAt,invalid,round;
    float dt,distance,targetDistance,pitch,yaw,x,y;};
static Scenario scenario;
static int checks;
static int GameIndex(){return g_TrackGame==(TrackGame*)world.games[1];}
static int KeyIndex(int key){const int keys[]={0x48,0x50,0x4b,0x4d,0x4a,0x0c,0x4e,0x0d,2,3};for(int i=0;i<10;++i)if(keys[i]==key)return i;ExitProcess(81);return -1;}
static void EventAt(int kind,int receiver,int code,int argument,int result,float x=0,float y=0){
    if(observed.count>=32)ExitProcess(82);
    Event& e=observed.events[observed.count++];e.kind=kind;e.receiver=receiver;e.code=code;e.argument=argument;e.result=result;e.x=x;e.y=y;e.game=GameIndex();e.state=world;
    if(scenario.mutateAt==observed.count){
        switch(scenario.mutation){
        case 0:
            F(0x220)=scenario.distance<50?55.0f:17.0f;F(0x2c0)=-F(0x2c0);
            F(0x22c)=-F(0x22c);F(0x234)=-F(0x234);F(0xac)=-F(0xac);F(0x17c)=-F(0x17c);
            I(0x244)=(I(0x244)+3)&7;I(0x268)=!I(0x268);I(0x26c)=!I(0x26c);break;
        case 1:g_TrackGame=(TrackGame*)world.games[1-GameIndex()];*(TrackGame**)0x56e26c=g_TrackGame;break;
        case 2:*(DWORD*)(world.controls[0]+0xc)=(DWORD)world.joysticks[1];*(DWORD*)(world.controls[1]+0xc)=0;break;
        case 3:g_TrackGame=0;*(TrackGame**)0x56e26c=0;break;
        }
    }
    if(scenario.throwAt==observed.count)throw 0x48a;
}
class Hooks {
public:
    int Key(int key,int kind,int modifiers,int device){
        int who=(BYTE*)this==world.controls[1];if((BYTE*)this!=world.controls[who] || device)ExitProcess(83);
        int index=KeyIndex(key);if(kind!=(index<8?0:2) || modifiers!=(index<8?0x3f:0xc))ExitProcess(84);
        int result=scenario.mask&(1<<index)?(scenario.seed&1?-7:1):0;
        EventAt(1,who,key,modifiers,result);return result;
    }
    bool Axes(int index,float* x,float* y){
        int who=(BYTE*)this==world.joysticks[1];if((BYTE*)this!=world.joysticks[who] || index || !x || !y || x==y)ExitProcess(85);
        *x=scenario.x;*y=scenario.y;
        bool result=scenario.joystick>=2;EventAt(2,who,index,0,result,*x,*y);return result;
    }
};
template<class T> static DWORD Address(T method){union {T method;DWORD address;}u;u.method=method;return u.address;}
static int MapReference(const char* path){
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
    }free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static int Filter(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];
    observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Invoke(int retail){
    FollowCamera* camera=(FollowCamera*)world.camera;
    union {bool(FollowCamera::*method)(bool,float);DWORD address;}call;call.address=0x463a30;
    __try {observed.result=retail?(camera->*call.method)(scenario.active!=0,scenario.dt):
        camera->FollowCamera::UnknownVirtualSlot45(scenario.active!=0,scenario.dt);}
    __except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){
    memset(&observed,0,sizeof(observed));observed.result=-501;memset(&world,0x53,sizeof(world));
    for(int offset=0xac;offset<0xec;offset+=4)F(offset)=((int)(scenario.seed%29)-14+(offset-0xac)/4)*.125f;
    for(offset=0x170;offset<0x188;offset+=4)F(offset)=((int)(scenario.seed%23)-11+(offset-0x170)/4)*.25f;
    for(offset=0x2a8;offset<0x2b4;offset+=4)F(offset)=((int)(scenario.seed%19)-9+(offset-0x2a8)/4)*.5f;
    F(0x220)=scenario.distance;F(0x22c)=scenario.pitch;F(0x234)=scenario.yaw;F(0x2c0)=scenario.targetDistance;
    I(0x244)=scenario.state;I(0x268)=scenario.mode&1;I(0x26c)=scenario.mode&2;world.camera[0x274]=scenario.seed&1;
    for(int n=0;n<2;++n){
        *(DWORD*)world.controls[n]=(DWORD)controlsTable;*(DWORD*)(world.controls[n]+0xc)=scenario.joystick?(DWORD)world.joysticks[n]:0;
        *(DWORD*)world.joysticks[n]=(DWORD)joystickTable;*(DWORD*)(world.games[n]+0x14)=(DWORD)world.controls[n];
    }
    g_TrackGame=(TrackGame*)world.games[0];
    if(scenario.invalid==1)g_TrackGame=0;
    if(scenario.invalid==2)*(DWORD*)(world.games[0]+0x14)=0;
    if(scenario.invalid==3)*(DWORD*)world.controls[0]=0;
    if(scenario.invalid==4)*(DWORD*)world.joysticks[0]=0;
    *(TrackGame**)0x56e26c=g_TrackGame;
    _fpreset();_controlfp(scenario.round,_MCW_RC);
    try{Invoke(retail);}catch(int e){observed.exception=e;}
    observed.fpStatus=_statusfp();observed.game=GameIndex();observed.state=world;_fpreset();
}
static void Compare(){
    Run(1);expected=observed;Run(0);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){
        BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;
        printf("Mismatch%d offset%d %02x/%02x state%d mode%d keys%x joystick%d active%d seed%u dt%g d%g target%g pitch%g yaw%g xy%g/%g mutation%d/%d throw%d invalid%d round%x events%d/%d result%d/%d fp%x/%x\n",
            checks,offset,a[offset],b[offset],scenario.state,scenario.mode,scenario.mask,scenario.joystick,scenario.active,scenario.seed,scenario.dt,
            scenario.distance,scenario.targetDistance,scenario.pitch,scenario.yaw,scenario.x,scenario.y,scenario.mutation,scenario.mutateAt,
            scenario.throwAt,scenario.invalid,scenario.round,expected.count,observed.count,expected.result,observed.result,expected.fpStatus,observed.fpStatus);ExitProcess(2);
    }
}
static void Default(){memset(&scenario,0,sizeof(scenario));scenario.active=1;scenario.dt=.016f;scenario.distance=27;scenario.targetDistance=8;scenario.pitch=.5f;scenario.yaw=-.25f;}
static unsigned randomState=0x931718df;
static unsigned Random(){randomState=randomState*1664525u+1013904223u;return randomState;}
static float Finite(){union{unsigned bits;float value;}u;unsigned r=Random();u.bits=(r&0x807fffff)|(((r>>24)%40+107)<<23);return u.value;}
int main(int argc,char**argv){
    if(argc!=2 || !MapReference(argv[1]))return 3;
    if(sizeof(FollowCamera)!=0x384)return 4;
    for(int i=0;i<4;++i)controlsTable[i]=(DWORD)abort;for(i=0;i<5;++i)joystickTable[i]=(DWORD)abort;
    controlsTable[3]=Address(&Hooks::Key);joystickTable[4]=Address(&Hooks::Axes);
    // Explicit expectations guard against an inert dispatcher or two identically
    // miswired tests: inactive clears settled; excluded active states set it.
    for(int state=-1;state<=8;++state){
        Default();scenario.state=state;scenario.active=0;scenario.invalid=1;Compare();
        if(observed.result || observed.count || world.camera[0x274] || observed.exception)ExitProcess(86);
        if(state==3 || state==6 || state==7){scenario.active=1;Compare();if(observed.result || observed.count || !world.camera[0x274] || observed.exception)ExitProcess(87);}
    }
    Default();scenario.state=5;scenario.mask=1;scenario.dt=1;Compare();
    if(observed.result!=1 || F(0x174)!=297.5f)ExitProcess(88);
    Default();scenario.mode=3;scenario.mask=1;scenario.dt=1;Compare();
    if(observed.result || F(0x2ac)!=296.0f || world.camera[0x274])ExitProcess(89);
    for(state=-1;state<=8;++state)for(int mode=0;mode<4;++mode)
    for(int mask=0;mask<1024;++mask)for(int joystick=0;joystick<4;++joystick){
        Default();scenario.state=state;scenario.mode=mode;scenario.mask=mask;scenario.joystick=joystick;
        scenario.seed=mask+mode*67;scenario.x=joystick==3?.5f:0;scenario.y=joystick==3?-.25f:0;Compare();
    }
    // Callback changes and exceptions at every reachable boundary in traces
    // spanning all branches, including short-circuited alternate keys.
    const int masks[]={0,0x3ff,0x155,0x2aa};
    for(state=0;state<8;++state)for(int mode=0;mode<4;++mode)for(int mask=0;mask<4;++mask){
        Default();scenario.state=state;scenario.mode=mode;scenario.mask=masks[mask];scenario.joystick=2;Compare();
        int count=observed.count;
        for(int event=1;event<=count;++event){
            scenario.throwAt=event;Compare();scenario.throwAt=0;
            for(int mutation=0;mutation<4;++mutation){scenario.mutation=mutation;scenario.mutateAt=event;Compare();scenario.mutateAt=0;}
        }
    }
    const float distances[]={-100,0,6.4999995f,6.5f,6.5000005f,6.9999995f,7,7.0000005f,49.999996f,50,50.000004f,1e6f};
    const float angles[]={-3.141593f,-3.1415927f,-1.5500001f,-1.55f,-0.0f,0,1.55f,1.5500001f,3.1415927f,3.141593f};
    const float times[]={-1,0,1e-30f,.016f,1,1000};
    const int rounds[]={_RC_NEAR,_RC_UP,_RC_DOWN,_RC_CHOP};
    for(int distance=0;distance<12;++distance)for(int angle=0;angle<10;++angle)for(int dt=0;dt<6;++dt)
    for(int round=0;round<4;++round)for(int mode=0;mode<3;++mode){
        Default();scenario.mode=mode==0?3:0;scenario.state=mode==1?5:0;scenario.mask=0x3ff;scenario.joystick=3;
        scenario.distance=distances[distance];scenario.targetDistance=distances[distance];scenario.pitch=angles[angle];scenario.yaw=angles[9-angle];
        scenario.dt=times[dt];scenario.round=rounds[round];scenario.x=angles[angle];scenario.y=angles[9-angle];Compare();
    }
    for(int sample=0;sample<16384;++sample){
        Default();scenario.seed=Random();scenario.mode=(Random()>>16)&3;scenario.state=(Random()>>16)%10-1;
        scenario.mask=(Random()>>11)&1023;scenario.joystick=(Random()>>16)&3;
        scenario.dt=Finite();scenario.distance=Finite();scenario.targetDistance=Finite();scenario.pitch=Finite();scenario.yaw=Finite();scenario.x=Finite();scenario.y=Finite();Compare();
    }
    // Explicit finite bit-pattern extremes include subnormals and arithmetic
    // overflow under the normal masked floating-point environment.
    const unsigned patterns[]={0,0x80000000,1,0x80000001,0x007fffff,0x807fffff,0x00800000,0x80800000,
        0x3f7fffff,0x3f800001,0x7f7fffff,0xff7fffff};
    for(int a=0;a<12;++a)for(int b=0;b<12;++b)for(int mode=0;mode<3;++mode)for(int round=0;round<4;++round){
        union{unsigned bits;float value;}x,y;x.bits=patterns[a];y.bits=patterns[b];
        Default();scenario.mode=mode==0?3:0;scenario.state=mode==1?5:0;scenario.mask=0x3ff;scenario.joystick=3;
        scenario.dt=x.value;scenario.distance=y.value;scenario.targetDistance=y.value;scenario.pitch=x.value;scenario.yaw=y.value;
        scenario.x=x.value;scenario.y=y.value;scenario.round=rounds[round];Compare();
    }
    for(int invalid=1;invalid<=4;++invalid){
        Default();scenario.joystick=2;scenario.invalid=invalid;Compare();
        const int addresses[]={0,0x14,0,0xc,0x10};
        if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.faultKind || observed.faultAddress!=addresses[invalid])ExitProcess(90);
    }
    printf("FollowCameraControls: %d complete input, movement, callback and floating-point scenarios match\n",checks);return 0;
}
