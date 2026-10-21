// Private execution of the unchanged standalone candidate and hash-pinned retail.
// Only direct dependencies are scripted. COM dispatch and target destruction
// use the actual ABI. This is not a DirectDraw-driver or complete-game test.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <new>
#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/DebugAlloc.h"
extern "C" void __cdecl __CxxFrameHandler();
extern "C" const UnknownGuid IID_IDirect3DHALDevice =
    {0x84e63de0,0x46aa,0x11cf,{0x81,0x6f,0,0,0xc0,0x20,0x15,0x6e}};
static unsigned char referenceSpace[0x800000];
struct World {
    unsigned char display[sizeof(UnknownDisplay)], targets[2][sizeof(PCRenderTarget)];
    UnknownDisplayMode modes[5]; DWORD drawObjects[2][4], surfaceObjects[2][4]; int value;
};
static World world;
static UnknownDisplay* Display(){return (UnknownDisplay*)world.display;}
static PCRenderTarget* Target(int i){return (PCRenderTarget*)world.targets[i];}
static DWORD drawTable[30],surfaceTable[50],targetVptr;
struct Scenario {int seed,supplied,modes,fail,caps,alias,mutation,throwAt;};
static Scenario scenario;
struct EventRecord {int op,args[6]; unsigned char data[124]; World state; int targetAlive[2],surfaceAlive[2];};
struct Observation {
    int result; DWORD exception,faultKind,faultAddress; int events,targetAlive[2],surfaceAlive[2];
    EventRecord trace[24]; World state;
};
static Observation observed,referenceResult;
static int* valuePointer;
static int silent;
static void Fail(int code){printf("Fixture invariant %d\n",code);ExitProcess(code);}
template<class T> static DWORD Address(T p){
    if(sizeof(p)!=4)Fail(82);union{T method;DWORD address;}u;u.method=p;return u.address;
}
static EventRecord& Event(int op,int a=0,int b=0,int c=0,int d=0,int e=0,int f=0){
    static EventRecord ignored;
    if(silent)return ignored;
    if(observed.events>=24)Fail(81);
    EventRecord& r=observed.trace[observed.events++];r.op=op;
    r.args[0]=a;r.args[1]=b;r.args[2]=c;r.args[3]=d;r.args[4]=e;r.args[5]=f;
    r.state=world;memcpy(r.targetAlive,observed.targetAlive,sizeof(r.targetAlive));
    memcpy(r.surfaceAlive,observed.surfaceAlive,sizeof(r.surfaceAlive));return r;
}
static void Callback(int op){
    if(silent)return;
    if(scenario.mutation==1 && valuePointer)*valuePointer=scenario.seed*100+op;
    if(scenario.mutation==2 && op==5)Display()->directDraw=(UnknownDirectDrawInterface*)world.drawObjects[1];
    if(scenario.mutation==3 && op==1){
        Display()->backBuffer=(UnknownSurfaceInterface*)world.surfaceObjects[1];Display()->frameBufferCount=-scenario.seed;
    }
    if(scenario.mutation==4 && op==7){if(valuePointer)*valuePointer=0x76543210;Target(0)->surfacePixelFormat^=0x314;}
    if(scenario.mutation==5 && op==8){if(valuePointer)*valuePointer=0x12345678;Target(0)->deviceCaps=17;}
    if(scenario.mutation==6 && op==4){Display()->currentDisplayMode=93;Display()->frameBufferCount=81;}
    if(scenario.mutation==7 && op==9 && valuePointer)*valuePointer=-71;
    if(scenario.throwAt==op)throw 70+op;
}
static int SourceFile(const char* file){
    const char* name=strrchr(file,'\\'),*slash=strrchr(file,'/');
    if(!name ||(slash&&slash>name))name=slash;name=name?name+1:file;
    if(strcmp(name,"PCVideoCard.cpp")&&strcmp(name,"NonLocalTextureMemory.cpp"))Fail(83);return 1;
}
int UnknownDisplay::SetFullscreenDisplayMode(int index,int flag){
    if(this!=Display() ||index<0||index>=5)Fail(84);
    Event(1,index,flag);Callback(1);currentDisplayMode=index;return scenario.fail==1?0:1;
}
void* operator new(unsigned int size,const char* file,int line){
    Event(2,size,SourceFile(file),line);Callback(2);
    if(size!=sizeof(PCRenderTarget)||line!=1185)Fail(85);
    if(scenario.fail==2 ||scenario.fail==12)return 0;
    if(observed.targetAlive[0])Fail(87);observed.targetAlive[0]=1;return Target(0);
}
void operator delete(void* p){
    if(!p)return;
    for(int i=0;i<2;++i)if(p==Target(i)){
        Event(10,i);if(!observed.targetAlive[i])Fail(88);observed.targetAlive[i]=0;return;
    }
    free(p);
}
void operator delete(void* p,const char* file,int line){
    Event(11,(int)p,SourceFile(file),line);operator delete(p);
}
RenderTarget::RenderTarget(){}
RenderTarget::~RenderTarget(){}
void RenderTarget::SetAlphaTest(int){Fail(86);}
void RenderTarget::DisableAlphaTest(){Fail(86);}
PCRenderTarget::PCRenderTarget(){
    Event(3,this==(void*)Target(0));Callback(3);surfacePixelFormat=0x22b+scenario.seed;deviceCaps=0x125;
}
PCRenderTarget::~PCRenderTarget(){
    Event(9,this==(void*)Target(0));Callback(9);
}
// Unused virtual slots fail immediately; they are needed solely to materialize
// the genuine C++ constructor/deleting-destructor ABI of the scripted target.
int PCRenderTarget::BeginScene(){Fail(86);return 0;}
int PCRenderTarget::EndScene(){Fail(86);return 0;}
int PCRenderTarget::Blit(void*,void*,void*,int){Fail(86);return 0;}
void* PCRenderTarget::LockSurface(void*,long*,int){Fail(86);return 0;}
int PCRenderTarget::UnlockSurface(void*){Fail(86);return 0;}
long PCRenderTarget::GetTextureStageState(int,int,int*){Fail(86);return 0;}
long PCRenderTarget::SetTextureStageState(int,int,int){Fail(86);return 0;}
void PCRenderTarget::SetRenderState(int,int,int){Fail(86);}
long PCRenderTarget::GetRenderState(int,int*){Fail(86);return 0;}
void PCRenderTarget::SetTextureBlend(int,int){Fail(86);}
long PCRenderTarget::ClearTexture(int){Fail(86);return 0;}
int PCRenderTarget::Clear(const CameraRect*,int){Fail(86);return 0;}
int PCRenderTarget::SupportsTextureFormat(int){Fail(86);return 0;}
int PCRenderTarget::SetViewport(void*){Fail(86);return 0;}
int PCRenderTarget::DrawIndexedPrimitive(int,int,int,int,int,int,int){Fail(86);return 0;}
int PCRenderTarget::DrawPrimitive(int,int,int,int,int){Fail(86);return 0;}
int PCRenderTarget::DrawIndexedPrimitiveVB(int,int,int,int,int,int,int){Fail(86);return 0;}
void PCRenderTarget::SetAlphaTest(int){Fail(86);}
void PCRenderTarget::DisableAlphaTest(){Fail(86);}

RenderTarget* PCRenderTarget::InitializeRenderTarget(UnknownDisplay* display,const UnknownGuid* id,
    UnknownSurfaceInterface* surface,int wantZ,int frames){
    if(display!=Display() ||(this &&this!=Target(0)))Fail(89);
    EventRecord& r=Event(4,this==(void*)Target(0),this==0,(int)surface,wantZ,frames);
    memcpy(r.data,id,sizeof(*id));Callback(4);
    if(scenario.fail==12){surfacePixelFormat=7;return this;} // Real null-this write fault.
    if(!this)return 0;
    targetDisplay=display;
    if(scenario.fail==3){delete this;return 0;}
    if(scenario.fail==9)return 0; // Deliberately retains the allocation, as this dependency may.
    if(scenario.fail==8){delete this;observed.targetAlive[1]=1;return Target(1);}
    return this;
}
struct ConstructorCall{PCRenderTarget* Create(){return ::new(this)PCRenderTarget;}};
void FillPixelFormat(int format,void* pixelFormat){
    EventRecord& r=Event(5,format);
    // The whole descriptor is initialized before this boundary, including its
    // size; checking it here catches early-field stores as well as final values.
    memcpy(r.data,(char*)pixelFormat-0x48,sizeof(UnknownSurfaceDesc));Callback(5);
    for(int i=0;i<32;++i)((BYTE*)pixelFormat)[i]=(BYTE)(format*7+i+scenario.seed);
}
static int DrawIndex(void* p){for(int i=0;i<2;++i)if(p==world.drawObjects[i])return i;Fail(90);return -1;}
static int SurfaceIndex(void* p){for(int i=0;i<2;++i)if(p==world.surfaceObjects[i])return i;Fail(91);return -1;}
static long __stdcall CreateSurface(void* self,UnknownSurfaceDesc* desc,UnknownSurfaceInterface** out,void* outer){
    int index=DrawIndex(self);EventRecord& r=Event(6,index,(int)outer);memcpy(r.data,desc,sizeof(*desc));Callback(6);
    if(scenario.fail==4)return -1;
    if(scenario.fail==11){*out=(UnknownSurfaceInterface*)world.surfaceObjects[index];observed.surfaceAlive[index]=1;return 1;}
    if(scenario.fail==6){*out=0;return 0;}
    *out=(UnknownSurfaceInterface*)world.surfaceObjects[index];observed.surfaceAlive[index]=1;return 0;
}
static long __stdcall GetCaps(void* self,UnknownSurfaceCaps* caps){
    int index=SurfaceIndex(self);EventRecord& r=Event(7,index);Callback(7);
    caps->caps=(scenario.seed*0x124421)&~0x20000000;
    if(scenario.caps&1)caps->caps|=0x20000000;
    caps->caps2=scenario.caps;caps->caps3=0x20000000;caps->caps4=0xffffffff;
    memcpy(r.data,caps,sizeof(*caps));return scenario.fail==5?-1:scenario.fail==10?1:0;
}
static long __stdcall Release(void* self){
    int index=SurfaceIndex(self);Event(8,index);Callback(8);
    if(!observed.surfaceAlive[index])Fail(92);observed.surfaceAlive[index]=0;return scenario.seed;
}
static DWORD Replacement(DWORD a){
    switch(a){
    case 0x4ca900:return Address(&UnknownDisplay::SetFullscreenDisplayMode);
    case 0x4a3010:return (DWORD)(void*(__cdecl*)(unsigned int,const char*,int))operator new;
    case 0x4c4ee0:return Address(&ConstructorCall::Create);
    case 0x4c4f80:return Address(&PCRenderTarget::InitializeRenderTarget);
    case 0x5119c0:return (DWORD)FillPixelFormat;
    case 0x4a3060:return (DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;
    case 0x53471a:return (DWORD)__CxxFrameHandler;
    }return 0;
}
static int MapReference(const char* path,const char* patches) {
    BYTE* mapped=(BYTE*)0x400000; DWORD old;
    if ((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
        !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* f=fopen(path,"rb"); if (!f) return 0;
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    BYTE* raw=(BYTE*)malloc(size); if (!raw || fread(raw,1,size,f)!=(size_t)size) return 0;
    fclose(f); IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if (size<sizeof(*dos) || dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 ||
        dos->e_lfanew>size-sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if (pe->Signature!=IMAGE_NT_SIGNATURE || pe->FileHeader.Machine!=0x14c ||
        pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000) return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for (int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if (section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
            section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage) return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw); f=fopen(patches,"rb"); if (!f) return 0;
    DWORD p[3];
    while (fread(p,sizeof(p),1,f)==1) {
        DWORD target=Replacement(p[1]);
        int inside=(p[0]>=0x4ca5a0 && p[0]+5<=0x4ca786) || (p[0]>=0x54cda0 && p[0]+5<=0x54cdc4);
        if (!inside || !target || (p[2]!=0xe8 && p[2]!=0xe9) || *(BYTE*)p[0]!=p[2] ||
            p[0]+5+*(DWORD*)(p[0]+1)!=p[1]) return 0;
        *(DWORD*)(p[0]+1)=target-p[0]-5;
    }
    fclose(f);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

static int Filter(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode==0xe06d7363)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=e->ExceptionRecord->ExceptionCode;
    if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)Fail(93);
    observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];
    observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static int Invoke(DWORD address){
    typedef int(UnknownDisplay::*Fn)(int*,RenderTarget*);Fn method;memcpy(&method,&address,4);
    __try{return (Display()->*method)(valuePointer,scenario.supplied?Target(1):0);}
    __except(Filter(GetExceptionInformation())){return -500;}
}
static void Run(int retail){
    memset(&observed,0,sizeof(observed));memset(&world,0x63+scenario.seed%19,sizeof(world));
    for(int i=0;i<2;++i){*(DWORD*)Target(i)=targetVptr;world.drawObjects[i][0]=(DWORD)drawTable;
        world.surfaceObjects[i][0]=(DWORD)surfaceTable;Target(i)->surfacePixelFormat=0x22b8+i+scenario.seed;}
    Display()->directDraw=scenario.fail==7?0:(UnknownDirectDrawInterface*)world.drawObjects[0];
    Display()->backBuffer=(UnknownSurfaceInterface*)world.surfaceObjects[0];Display()->frameBufferCount=2+scenario.seed;
    Display()->displayModes=world.modes;Display()->displayModeCount=scenario.modes<2?scenario.modes-1:5;
    for(i=0;i<5;++i){world.modes[i].width=640;world.modes[i].height=480;world.modes[i].bitDepth=16;}
    // 0: negative count; 1: zero; 2: all nonmatching; 3: first; 4: middle; 5: last.
    if(scenario.modes==2)for(i=0;i<5;++i){
        if(i%3==0)world.modes[i].width=641;else if(i%3==1)world.modes[i].height=479;else world.modes[i].bitDepth=32;
    }
    if(scenario.modes>=4)for(i=0;i<(scenario.modes==4?2:4);++i)world.modes[i].width=800;
    valuePointer=&world.value;
    switch(scenario.alias){
    case 1:valuePointer=&Display()->displayModeCount;break;
    case 2:valuePointer=&world.modes[0].width;break;
    case 3:valuePointer=&world.modes[1].bitDepth;break;
    case 4:valuePointer=&Target(scenario.supplied?1:0)->surfacePixelFormat;break;
    case 5:valuePointer=&Display()->frameBufferCount;break;
    case 6:valuePointer=&Display()->currentDisplayMode;break;
    case 7:valuePointer=&Display()->textureCacheLimit;break;
    case 8:valuePointer=0;break;
    }
    DWORD address=retail?0x4ca5a0:Address(&UnknownDisplay::ProbeNonLocalTextureMemory);
    observed.result=-501;
    try{observed.result=Invoke(address);}catch(int code){observed.exception=code;}
    observed.state=world;
}
static int checks,coverage[12],faults,returns[2],throws[10];
static void Compare(){
    Run(1);referenceResult=observed;Run(0);++checks;
    if(memcmp(&referenceResult,&observed,sizeof(observed))){
        const BYTE* r=(BYTE*)&referenceResult;const BYTE* c=(BYTE*)&observed;
        int first=0;while(first<sizeof(observed)&&r[first]==c[first])++first;
        printf("Mismatch%d at%d %02x/%02x seed%d supplied%d modes%d fail%d caps%d alias%d mutation%d throw%d result%d/%d exception%x/%x events%d/%d\n",
            checks,first,r[first],c[first],scenario.seed,scenario.supplied,scenario.modes,scenario.fail,scenario.caps,
            scenario.alias,scenario.mutation,scenario.throwAt,referenceResult.result,observed.result,
            referenceResult.exception,observed.exception,referenceResult.events,observed.events);ExitProcess(2);
    }
    for(int i=0;i<observed.events;++i){
        EventRecord& r=observed.trace[i];coverage[r.op]++;
        if(r.op==5){
            UnknownSurfaceDesc expected;memset(&expected,0,sizeof(expected));expected.size=sizeof(expected);
            if(memcmp(r.data,&expected,sizeof(expected)))Fail(104);
        }
        if(r.op==6){
            const UnknownSurfaceDesc* desc=(const UnknownSurfaceDesc*)r.data;
            if(desc->size!=0x7c||desc->width!=128||desc->height!=128||desc->flags!=0x1007||
               desc->caps[0]!=0x20005000||desc->caps[1]||desc->caps[2]||desc->caps[3]||desc->textureStage)Fail(105);
        }
    }
    if(scenario.alias==8&&(observed.events||observed.exception!=EXCEPTION_ACCESS_VIOLATION||
       observed.faultKind!=1||observed.faultAddress))Fail(106);
    if(observed.exception==EXCEPTION_ACCESS_VIOLATION)faults++;
    else if(observed.exception){if(observed.exception<71||observed.exception>79)Fail(94);throws[observed.exception-70]++;}
    else{if(observed.result<0||observed.result>1)Fail(95);returns[observed.result]++;}
    // Independent outcomes for ordinary, unmutated scenarios. No compatible
    // temporary mode faults; a successful caps query tests only caps bit29.
    if(!scenario.alias&&!scenario.mutation&&!scenario.throwAt&&!scenario.fail){
        if(!scenario.supplied&&scenario.modes<3){
            if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultAddress!=0x28)Fail(96);
        }else{
            if(observed.exception||observed.result!=(scenario.caps&1)||world.value!=0)Fail(97);
            if(observed.targetAlive[0]||observed.targetAlive[1]||observed.surfaceAlive[0]||observed.surfaceAlive[1])Fail(98);
        }
    }
    if(!scenario.supplied&&scenario.modes==3&&!scenario.alias&&!scenario.mutation&&!scenario.throwAt&&scenario.fail==1){
        if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultAddress!=0x28||observed.events!=1)Fail(107);
    }
    if(!scenario.supplied && scenario.modes==3&&!scenario.alias&&!scenario.mutation&&scenario.throwAt==3){
        if(observed.targetAlive[0]||observed.trace[observed.events-2].op!=11||observed.trace[observed.events-1].op!=10)Fail(99);
    }
}
int main(int argc,char**argv){
    if(argc!=3||!MapReference(argv[1],argv[2]))return 3;
    if(sizeof(UnknownDisplay)!=0xb78||sizeof(PCRenderTarget)!=0xbc4||sizeof(UnknownDisplayMode)!=0x24||
       sizeof(UnknownSurfaceDesc)!=0x7c||sizeof(UnknownSurfaceCaps)!=0x10||
       offsetof(RenderTarget,surfacePixelFormat)!=0x28||offsetof(UnknownDisplay,directDraw)!=0x190||
       offsetof(UnknownDisplay,backBuffer)!=0x1a0||offsetof(UnknownDisplay,frameBufferCount)!=0x78||
       offsetof(UnknownDisplay,displayModeCount)!=8||offsetof(UnknownDisplay,displayModes)!=0x10)Fail(100);
    for(int i=0;i<30;++i)drawTable[i]=(DWORD)abort;drawTable[6]=(DWORD)CreateSurface;
    for(i=0;i<50;++i)surfaceTable[i]=(DWORD)abort;surfaceTable[2]=(DWORD)Release;surfaceTable[14]=(DWORD)GetCaps;
    silent=1;::new(Target(1))PCRenderTarget;targetVptr=*(DWORD*)Target(1);silent=0;
    // Mode-search branches, positive/negative HRESULTs, ownership variants,
    // caller aliases and mutating external dependencies.
    int supplied,modes,alias;
    for(int seed=0;seed<8;++seed)for(supplied=0;supplied<2;++supplied)
    for(modes=0;modes<6;++modes)for(int fail=0;fail<13;++fail)
    for(alias=0;alias<9;++alias){
        memset(&scenario,0,sizeof(scenario));scenario.seed=seed;scenario.supplied=supplied;
        scenario.modes=modes;scenario.fail=fail;scenario.alias=alias;
        scenario.caps=(seed+alias)&3;scenario.mutation=(seed+fail)&7;Compare();
    }
    for(supplied=0;supplied<2;++supplied)for(modes=0;modes<6;++modes)
    for(int caps=0;caps<4;++caps)for(int mutation=0;mutation<8;++mutation){
        memset(&scenario,0,sizeof(scenario));scenario.supplied=supplied;scenario.modes=modes;
        scenario.caps=caps;scenario.mutation=mutation;scenario.seed=17;Compare();
    }
    for(supplied=0;supplied<2;++supplied)for(int op=1;op<=9;++op)
    for(alias=0;alias<9;++alias)for(int mutation=0;mutation<8;++mutation){
        memset(&scenario,0,sizeof(scenario));scenario.supplied=supplied;scenario.modes=3;
        scenario.throwAt=op;scenario.alias=alias;scenario.mutation=mutation;scenario.caps=1;Compare();
    }
    for(i=1;i<=11;++i)if(!coverage[i])Fail(101);
    for(i=1;i<=9;++i)if(!throws[i])Fail(102);
    if(!faults||!returns[0]||!returns[1])Fail(103);
    printf("NonLocalTextureMemory: %d scenarios match full state, ordered calls, aliases, faults and ownership\n",checks);return 0;
}
