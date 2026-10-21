// Private whole-function comparison. Every dependency is scripted; no retail
// dispatch or fixture implementation is linked into the source-built game.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/DebugAlloc.h"
extern "C" void __cdecl __CxxFrameHandler();
extern "C" const UnknownGuid IID_IDirect3DHALDevice =
    {0x84e63de0,0x46aa,0x11cf,{0x81,0x6f,0,0,0xc0,0x20,0x15,0x6e}};
static unsigned char referenceSpace[0x800000];
static unsigned char gameStorage[sizeof(TrackGame)], displayStorage[sizeof(UnknownDisplay)];
static unsigned char providedStorage[sizeof(PCRenderTarget)];
TrackGame* g_TrackGame=(TrackGame*)gameStorage;
static UnknownDisplay* Display(){return (UnknownDisplay*)displayStorage;}
static PCRenderTarget* Provided(){return (PCRenderTarget*)providedStorage;}
struct Block { int size,alive; unsigned char bytes[sizeof(PCRenderTarget)]; };
static Block heap[8];
static UnknownTgaFile file;
static unsigned char pixels[256];
static UnknownDisplayMode modes[4];
struct Scenario {
    int provided,modeCount,matchMask,modeFailMask,initFailMask,fileExists,bpp,width,height;
    int failSurface,failTexture,fullResult,tileFail,tileResult;
    int failAlloc,throwAlloc,throwCtor,throwOperation;
};
static Scenario scenario;
struct EventRecord {
    int op,args[20]; UnknownRect rect;
    void* gameDisplay; void* gameTarget;
};
struct Observation {
    int exception,fault,faultOperation,faultAddress,events,blocks,fileAlive;
    EventRecord trace[80];Block heap[8];
    unsigned char display[sizeof(UnknownDisplay)],game[sizeof(TrackGame)],provided[sizeof(PCRenderTarget)];
    UnknownTgaFile file;UnknownDisplayMode modes[4];
};
static Observation observed,referenceResult;
static int blocks,allocations,constructors,modeIndex,textureCalls,passes;
static void Fail(int code){printf("Fixture invariant %d\n",code);ExitProcess(code);}
static int Identity(const void* p) {
    if (!p)return 0;if(p==Display())return -1;if(p==Provided())return -2;
    if(p==&file)return -3;if(p==pixels)return -4;if(p==g_TrackGame)return -5;
    for(int i=0;i<blocks;++i)if(p==heap[i].bytes)return i+1;
    return (int)p;
}
static EventRecord& Event(int op,int a=0,int b=0,int c=0,int d=0,int e=0) {
    if(observed.events>=80)Fail(80);
    EventRecord& r=observed.trace[observed.events++];r.op=op;
    r.args[0]=a;r.args[1]=b;r.args[2]=c;r.args[3]=d;r.args[4]=e;
    r.gameDisplay=g_TrackGame->display;r.gameTarget=g_TrackGame->renderTarget;
    return r;
}
template<class T>static DWORD Address(T p) {
    if(sizeof(p)!=4)Fail(81);union {T p;DWORD a;} u;u.p=p;return u.a;
}
static int SourceFile(const char* p) {
    const char* n=strrchr(p,'\\'),*s=strrchr(p,'/');if(!n||(s&&s>n))n=s;n=n?n+1:p;
    if(strcmp(n,"PCVideoCard.cpp")&&strcmp(n,"PartialTextureUploads.cpp"))Fail(82);return 1;
}
void* operator new(unsigned int size,const char* path,int line) {
    Event(1,size,SourceFile(path),line,++allocations);
    if(allocations==scenario.throwAlloc)throw 71;
    if(allocations==scenario.failAlloc)return 0;
    if(blocks>=8||size>sizeof(heap[0].bytes))Fail(83);
    Block& b=heap[blocks++];b.size=size;b.alive=1;memset(b.bytes,0xa5,sizeof(b.bytes));return b.bytes;
}
void operator delete(void* p) {
    if(!p)return;
    int n=Identity(p);if(n<1||n>blocks){free(p);return;}
    Event(2,n);if(!heap[n-1].alive)Fail(84);heap[n-1].alive=0;
}
void operator delete(void* p,const char* path,int line) {
    Event(3,Identity(p),SourceFile(path),line);operator delete(p);
}
RenderTarget::RenderTarget(){}
RenderTarget::~RenderTarget(){}
void RenderTarget::SetAlphaTest(int){Fail(86);}
void RenderTarget::DisableAlphaTest(){Fail(86);}
PCRenderTarget::PCRenderTarget(){
    Event(4,Identity(this),++constructors);if(constructors==scenario.throwCtor)throw 72;
    targetDisplay=Display();surfacePixelFormat=565;deviceCaps=0x1234;
}
PCRenderTarget::~PCRenderTarget(){Event(5,Identity(this));}
RenderTarget* PCRenderTarget::InitializeRenderTarget(UnknownDisplay* d,const UnknownGuid* id,
        UnknownSurfaceInterface* surface,int wantZ,int frames) {
    EventRecord& r=Event(6,Identity(this),Identity(d),(int)surface,wantZ,frames);
    memcpy(r.args+5,id,sizeof(*id));
    if(scenario.throwOperation==1)throw 73;
    if(!this)return 0; // Scripted null receiver boundary, not a claim about the real initializer.
    if(scenario.initFailMask&(1<<modeIndex)){delete this;return 0;}return this;
}
BaseObject::BaseObject(){refCount=1;}
BaseObject::~BaseObject(){}
int BaseObject::AddRef(){return ++refCount;}
int BaseObject::GetRefCount(){return refCount;}
int BaseObject::Release(){
    Event(7,Identity(this),refCount);if(scenario.throwOperation==5)throw 75;
    if(!--refCount){delete this;return 0;}return refCount;
}
TextureMap::TextureMap(TextureMapManager*,int){}
TextureMap::~TextureMap(){}
PCTextureMap::PCTextureMap(TextureMapManager* manager,int value):TextureMap(manager,value){
    Event(8,Identity(this),(int)manager,value,++constructors);
    if(constructors==scenario.throwCtor)throw 72;
}
PCTextureMap::~PCTextureMap(){Event(9,Identity(this));}
int PCTextureMap::CreateSurface(void* bits,int w,int h,int stride,int minimum,int source,int format,
    UnknownTexturePalette* palette,int flags,void* surfacePalette,int check,int unused,int au,int av,
    UnknownTextureFormatChoice* choice,int alpha,unsigned int key){
    EventRecord& r=Event(10,Identity(this),Identity(bits),w,h,stride);
    int a[]={minimum,source,format,(int)palette,flags,(int)surfacePalette,check,unused,au,av,(int)choice,alpha,(int)key};
    memcpy(r.args+5,a,sizeof(a));++textureCalls;
    textureWidth=w;textureHeight=h;mapFormat=format;averageColor=textureCalls*99;
    if(scenario.throwOperation==2)throw 74;
    return textureCalls==scenario.failSurface?0:1;
}
int PCTextureMap::CreateTextureSurface(int a,int b,int c){
    Event(11,Identity(this),a,b,c);if(scenario.throwOperation==3)throw 74;
    return !scenario.failTexture;
}
UnknownTgaFile* LoadFile(const char* name,UnknownTgaFile* output,int flags){
    if(strcmp(name,"partblt.tga"))Fail(87);Event(12,Identity(output),flags);
    observed.fileAlive=scenario.fileExists;return scenario.fileExists?&file:0;
}
void FreeFile(UnknownTgaFile* p){
    Event(13,Identity(p));if(p!=&file||!observed.fileAlive)Fail(88);observed.fileAlive=0;
}
int UnknownDisplay::SetFullscreenDisplayMode(int mode,int windowed){
    modeIndex=mode;Event(14,Identity(this),mode,windowed);currentDisplayMode=mode;
    return !(scenario.modeFailMask&(1<<mode));
}
int TestPartialTextureUpload(PCRenderTarget* target,UnknownRect* area,PCTextureMap* image,
    PCTextureMap* expected,PCTextureMap* rendered,UnknownDisplay* display,int present){
    EventRecord& r=Event(15,Identity(target),Identity(image),Identity(expected),Identity(rendered),Identity(display));
    r.args[5]=present;r.args[6]=passes;r.rect=*area;
    expected->field_0x40+=13;rendered->field_0x40+=passes+29;
    if(scenario.throwOperation==4)throw 74;
    int index=passes++;return index==0?scenario.fullResult:index-1==scenario.tileFail?scenario.tileResult:1;
}
struct ConstructorCalls {
    PCRenderTarget* Target(){return ::new(this)PCRenderTarget;}
    PCTextureMap* Texture(TextureMapManager* m,int value){return ::new(this)PCTextureMap(m,value);}
};
static DWORD Replacement(DWORD a){
    switch(a){
    case 0x5125c0:return (DWORD)LoadFile;
    case 0x4ca900:return Address(&UnknownDisplay::SetFullscreenDisplayMode);
    case 0x4a3010:return (DWORD)(void*(__cdecl*)(unsigned int,const char*,int))operator new;
    case 0x4c4ee0:return Address(&ConstructorCalls::Target);
    case 0x4c4f80:return Address(&PCRenderTarget::InitializeRenderTarget);
    case 0x4c5f00:return Address(&ConstructorCalls::Texture);
    case 0x512dd0:return (DWORD)FreeFile;
    case 0x4caf70:return (DWORD)TestPartialTextureUpload;
    case 0x4a3060:return (DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;
    case 0x53471a:return (DWORD)__CxxFrameHandler;
    }return 0;
}
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(size<sizeof(*dos)||dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->FileHeader.Machine!=0x14c||pe->OptionalHeader.ImageBase!=0x400000||
       pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* s=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(s[i].PointerToRawData+s[i].SizeOfRawData>(DWORD)size||s[i].VirtualAddress+s[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+s[i].VirtualAddress,raw+s[i].PointerToRawData,s[i].SizeOfRawData);
    }
    free(raw);f=fopen(patches,"rb");if(!f)return 0;DWORD p[3];
    while(fread(p,sizeof(p),1,f)==1){
        DWORD target=Replacement(p[1]);
        int inside=(p[0]>=0x4cab00&&p[0]+5<=0x4caf69)||(p[0]>=0x54cdd0&&p[0]+5<=0x54ce36);
        if(!inside||!target||(p[2]!=0xe8&&p[2]!=0xe9)||*(BYTE*)p[0]!=p[2]||p[0]+5+*(DWORD*)(p[0]+1)!=p[1])return 0;
        *(DWORD*)(p[0]+1)=target-p[0]-5;
    }
    fclose(f);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static void Invoke(int retail){
    union{void(UnknownDisplay::*method)(RenderTarget*);DWORD address;}call;
    call.method=&UnknownDisplay::ProbePartialTextureUploads;if(retail)call.address=0x4cab00;
    try{(Display()->*call.method)(scenario.provided?Provided():0);}catch(int e){observed.exception=e;}
}
static int FaultFilter(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.fault=e->ExceptionRecord->ExceptionCode;
    observed.faultOperation=e->ExceptionRecord->ExceptionInformation[0];
    observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
static void GuardedInvoke(int retail){
    __try{Invoke(retail);}__except(FaultFilter(GetExceptionInformation())){}
}
static void Run(int retail){
    memset(&observed,0,sizeof(observed));memset(heap,0,sizeof(heap));memset(gameStorage,0x53,sizeof(gameStorage));
    memset(displayStorage,0x64,sizeof(displayStorage));memset(providedStorage,0x75,sizeof(providedStorage));
    memset(&file,0x36,sizeof(file));memset(modes,0,sizeof(modes));memset(pixels,0xa7,sizeof(pixels));
    blocks=allocations=constructors=modeIndex=textureCalls=passes=0;
    g_TrackGame=(TrackGame*)gameStorage;*(TrackGame**)0x56e26c=g_TrackGame;
    g_TrackGame->display=(UnknownDisplay*)0x12345678;g_TrackGame->renderTarget=(RenderTarget*)0x23456789;
    Display()->displayModeCount=scenario.modeCount;Display()->displayModes=modes;Display()->field_0x198=(void*)0x89abcdef;
    Display()->backBuffer=(UnknownSurfaceInterface*)0x13572468;Display()->frameBufferCount=3;
    for(int i=0;i<4;++i){modes[i].width=(scenario.matchMask&(1<<i))?640:800;modes[i].height=480;modes[i].bitDepth=16;}
    // Existing target is not constructed or owned by this method. Give it the
    // scripted class vtable without adding fixture events to the scenario.
    int saved=scenario.throwCtor;scenario.throwCtor=0;::new(providedStorage)PCRenderTarget;scenario.throwCtor=saved;
    observed.events=0;constructors=0;
    file.width=(short)scenario.width;file.height=(short)scenario.height;file.bitsPerPixel=(unsigned char)scenario.bpp;
    file.bits=pixels;file.bitsSize=sizeof(pixels);
    GuardedInvoke(retail);observed.blocks=blocks;
    memcpy(observed.heap,heap,sizeof(heap));memcpy(observed.display,displayStorage,sizeof(displayStorage));
    memcpy(observed.game,gameStorage,sizeof(gameStorage));memcpy(observed.provided,providedStorage,sizeof(providedStorage));
    observed.file=file;memcpy(observed.modes,modes,sizeof(modes));
}
static int checks,exceptions,faults;
static void Compare(){
    Run(1);referenceResult=observed;Run(0);++checks;
    if(memcmp(&referenceResult,&observed,sizeof(observed))){
        BYTE* a=(BYTE*)&referenceResult,*b=(BYTE*)&observed;int at=0;while(at<sizeof(observed)&&a[at]==b[at])++at;
        printf("Mismatch %d byte%d %02x/%02x provided%d modes%d mask%x modefail%x initfail%x file%d bpp%d dim%d/%d surface%d tex%d full%d tile%d/%d alloc%d throw%d/%d/%d\n",
            checks,at,a[at],b[at],scenario.provided,scenario.modeCount,scenario.matchMask,scenario.modeFailMask,scenario.initFailMask,
            scenario.fileExists,scenario.bpp,scenario.width,scenario.height,scenario.failSurface,scenario.failTexture,
            scenario.fullResult,scenario.tileFail,scenario.tileResult,scenario.failAlloc,scenario.throwAlloc,scenario.throwCtor,scenario.throwOperation);
        ExitProcess(2);
    }
    if(observed.exception)++exceptions;if(observed.fault)++faults;
}
static void Defaults(){
    memset(&scenario,0,sizeof(scenario));scenario.modeCount=3;scenario.matchMask=7;scenario.fileExists=1;
    scenario.bpp=16;scenario.width=scenario.height=256;scenario.fullResult=1;scenario.tileFail=16;
}
int main(int argc,char**argv){
    if(argc!=3||!MapReference(argv[1],argv[2]))return 3;
    for(int provided=0;provided<2;++provided)for(int count=0;count<=4;++count)
    for(int mask=0;mask<16;++mask)for(int modeFail=0;modeFail<16;++modeFail)for(int initFail=0;initFail<4;++initFail){
        Defaults();scenario.provided=provided;scenario.modeCount=count;scenario.matchMask=mask;
        scenario.modeFailMask=modeFail;scenario.initFailMask=initFail;Compare();
    }
    const int dimensions[]={-7,0,1,256,32767};const int bpps[]={0,16,24,32};const int results[]={-3,0,1,2};
    for(provided=0;provided<2;++provided)for(int dim=0;dim<5;++dim)for(int bpp=0;bpp<4;++bpp)
    for(int fail=0;fail<6;++fail)for(int full=0;full<4;++full)for(int tile=0;tile<=16;++tile){
        Defaults();scenario.provided=provided;scenario.width=dimensions[dim];scenario.height=dimensions[4-dim];
        scenario.bpp=bpps[bpp];scenario.fileExists=fail!=5;scenario.failSurface=fail<4?fail:0;
        scenario.failTexture=fail==4;scenario.fullResult=results[full];scenario.tileFail=tile;scenario.tileResult=results[dim%4];Compare();
    }
    for(provided=0;provided<2;++provided)for(int kind=0;kind<4;++kind)for(int at=1;at<=5;++at){
        Defaults();scenario.provided=provided;
        if(kind==0)scenario.failAlloc=at;if(kind==1)scenario.throwAlloc=at;
        if(kind==2)scenario.throwCtor=at;if(kind==3)scenario.throwOperation=at;Compare();
    }
    printf("PartialTextureUploads: %d scenarios match state, ownership, calls, exceptions and faults\n",checks);
    printf("%d C++ exceptions, %d access violations\n",exceptions,faults);return 0;
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

int PCTextureMap::LoadSurface(UnknownTextureStream*,int,int,int,int,int,int,UnknownTexturePalette*,int,void*,int,int,UnknownTextureFormatChoice*,int,unsigned int){Fail(86);return 0;}
TextureMap* PCTextureMap::Clone(){Fail(86);return 0;}
int PCTextureMap::IsResident(){Fail(86);return 0;}
int PCTextureMap::Upload(UnknownRect*,int){Fail(86);return 0;}
void PCTextureMap::ReleaseTextureSurface(){Fail(86);}
int PCTextureMap::SetAsTexture(){Fail(86);return 0;}
void PCTextureMap::RecreateIfLost(){Fail(86);}
void* PCTextureMap::LockSurface(void*,long*,int){Fail(86);return 0;}
int PCTextureMap::UnlockSurface(void*){Fail(86);return 0;}
int PCTextureMap::BuildMipLevels(int){Fail(86);return 0;}
void* PCTextureMap::LockLevel(int){Fail(86);return 0;}
int PCTextureMap::UnlockLevel(int){Fail(86);return 0;}
int PCTextureMap::ApplyColorKey(unsigned int){Fail(86);return 0;}
void PCTextureMap::Bind(){Fail(86);}
int PCTextureMap::DumpAllLevels(){Fail(86);return 0;}
