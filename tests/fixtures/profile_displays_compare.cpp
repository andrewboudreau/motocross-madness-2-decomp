// Private comparison only: the complete standalone candidate is linked
// unchanged. Registry, allocator, display and COM dependencies are scripted;
// this fixture does not test real DirectDraw or run the complete game.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "../../src/reconstructed/PCGame.h"
#include "../../src/reconstructed/DebugAlloc.h"

extern "C" void __cdecl __CxxFrameHandler();
extern "C" const UnknownGuid IID_IDirect3DHALDevice =
    {0x84e63de0,0x46aa,0x11cf,{0x81,0x6f,0,0,0xc0,0x20,0x15,0x6e}};
UnknownDisplay* g_Displays[4];
int g_DisplayCount;
static unsigned char referenceSpace[0x800000];
static unsigned char gameStorage[sizeof(PCGame)], displayStorage[4][sizeof(UnknownDisplay)];
static unsigned char targetStorage[4][sizeof(PCRenderTarget)];
static UnknownDisplayMode modes[4][4];
static DWORD gameTable[38], displayTable[5], drawTable[30], surfaceTable[50];
static DWORD drawObjects[4], surfaceObjects[4][11];
static PCGame* Game() { return (PCGame*)gameStorage; }
static UnknownDisplay* Display(int i) { return (UnknownDisplay*)displayStorage[i]; }
static PCRenderTarget* Target(int i) { return (PCRenderTarget*)targetStorage[i]; }
struct Scenario {
    int count, loaded, disabled, fail, surfaceLimit, partial, platform, caps, deviceCaps;
    int modeCount, seed, throwKind;
};
static Scenario scenario;
struct EventRecord {
    int op, args[7]; char name[128]; unsigned char data[160];
    void* display; void* target;
};
struct Observation {
    int result, exception, events, targetAlive[4], surfaceAlive[4][11];
    EventRecord trace[320];
    unsigned char game[sizeof(PCGame)], displays[sizeof(displayStorage)], targets[sizeof(targetStorage)];
    UnknownDisplayMode modes[4][4];
};
static Observation observed, referenceResult;
static int activeDisplay, created[4];
static void Fail(int code) { printf("Fixture invariant %d\n",code); ExitProcess(code); }
static int Index(const void* p) {
    for (int i=0;i<4;++i) if (p==Display(i)) return i;
    Fail(80); return -1;
}
static EventRecord& Event(int op,int a=0,int b=0,int c=0,int d=0,int e=0,int f=0,int g=0) {
    if (observed.events>=320) Fail(81);
    EventRecord& r=observed.trace[observed.events++];
    r.op=op;r.args[0]=a;r.args[1]=b;r.args[2]=c;r.args[3]=d;r.args[4]=e;r.args[5]=f;r.args[6]=g;
    r.display=Game()->display;r.target=Game()->renderTarget;
    return r;
}
template<class T> static DWORD Address(T p) {
    if (sizeof(p)!=4) Fail(82);
    union { T method; DWORD address; } value;
    value.method=p; return value.address;
}
static int SourceFile(const char* file) {
    const char* name=strrchr(file,'\\'),*slash=strrchr(file,'/');
    if (!name || (slash && slash>name)) name=slash;
    name=name?name+1:file;
    if (strcmp(name,"PCGame.cpp") && strcmp(name,"PCGameNearMisses.cpp")) Fail(83);
    return 1;
}
void* operator new(unsigned int size,const char* file,int line) {
    Event(1,size,SourceFile(file),line,activeDisplay);
    if (size!=sizeof(PCRenderTarget) || line!=1423) Fail(84);
    if (scenario.throwKind==1) throw 71;
    if (scenario.fail==9) return 0;
    observed.targetAlive[activeDisplay]=1;
    return Target(activeDisplay);
}
void operator delete(void* p) {
    if (!p) return;
    for (int i=0;i<4;++i) if (p==Target(i)) {
        Event(2,i); if (!observed.targetAlive[i]) Fail(85);
        observed.targetAlive[i]=0; return;
    }
    free(p);
}
void operator delete(void* p,const char* file,int line) {
    Event(3,activeDisplay,SourceFile(file),line);
    operator delete(p);
}
RenderTarget::RenderTarget() {}
RenderTarget::~RenderTarget() {}
void RenderTarget::SetAlphaTest(int) { Fail(86); }
void RenderTarget::DisableAlphaTest() { Fail(86); }
PCRenderTarget::PCRenderTarget() {
    Event(4,activeDisplay);
    if (scenario.throwKind==2) throw 72;
    surfacePixelFormat=7+activeDisplay;
    deviceCaps=scenario.deviceCaps;
}
PCRenderTarget::~PCRenderTarget() { Event(5,activeDisplay); }
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
        UnknownSurfaceInterface* surface,int wantZ,int frames) {
    int n=Index(display);
    EventRecord& r=Event(6,n,this==(void*)Target(n),this==0,wantZ,frames,(int)surface);
    memcpy(r.data,id,sizeof(*id));
    if (scenario.throwKind==3) throw 73;
    if (scenario.fail==9) return 0; // Both callers pass null through to this dependency.
    targetDisplay=display;
    if (scenario.fail==8) { delete this; return 0; }
    return this;
}
struct ConstructorCall { PCRenderTarget* Create(){return ::new(this) PCRenderTarget;} };
int PCGame::LoadDisplayProfile(UnknownDisplay* d) {
    int n=Index(d); Event(7,n); d->textureCacheLimit=123+n;
    return (scenario.loaded>>n)&1;
}
int PCGame::LimitDisplayModes(UnknownDisplay* d,int width,int height) {
    int n=Index(d); Event(8,n,width,height);
    for (int i=0;i<d->displayModeCount;++i) d->displayModes[i].hardwareUsable=(i+width+height)&1;
    return 1;
}
int UnknownDisplay::SetWindowedCooperativeLevel(int x,int y,int w,int h) {
    int n=Index(this); Event(9,n,x,y,w,h); cooperativeLevel=w?8:16;
    return scenario.fail!=1;
}
int UnknownDisplay::SetFullscreenCooperativeLevel() {
    int n=Index(this); Event(10,n); cooperativeLevel=32; return scenario.fail!=2;
}
void UnknownDisplay::RefreshDriverCaps() {
    activeDisplay=Index(this); Event(11,activeDisplay); driverCaps=scenario.caps;
}
int UnknownDisplay::ProbeNonLocalTextureMemory(int* value,RenderTarget* target) {
    int n=Index(this); Event(12,n,(int)target); *value=321+n; return scenario.seed+n-1;
}
void UnknownDisplay::ProbePartialTextureUploads(RenderTarget* target) {
    int n=Index(this); Event(13,n,(int)target);
    if (Game()->display!=this || Game()->renderTarget!=target) Fail(87);
    partialTextureUploadResult=scenario.seed*17+n;
    if (scenario.throwKind==4) throw 74;
}
void FillPixelFormat(int format,void* pixelFormat) {
    Event(14,format); unsigned char* p=(unsigned char*)pixelFormat;
    for (int i=0;i<32;++i) p[i]=(unsigned char)(format*7+i);
}
struct Hooks {
    int Read(const char* name,int value,int flag) {
        EventRecord& r=Event(flag?15:16,value); strcpy(r.name,name);
        if (!flag) return scenario.partial;
        if (strstr(name,"DisabledWindowed")) return scenario.disabled&1;
        if (strstr(name,"DisabledFullScreen")) return scenario.disabled&2;
        if (strstr(name,"DisabledSoftware")) return scenario.disabled&4;
        if (strstr(name,"DisabledHardware")) return scenario.disabled&8;
        Fail(88); return 0;
    }
    int GetFlag(const char* name,int value){return Read(name,value,1);}
    int GetInt(const char* name,int value){return Read(name,value,0);}
    int SetInt(const char* name,int value) {
        EventRecord& r=Event(17,value); strcpy(r.name,name); return scenario.seed&1;
    }
    int SetFlag(const char* name,int value) {
        EventRecord& r=Event(18,value); strcpy(r.name,name); return scenario.seed&1;
    }
    int SetBinary(const char* name,const void* data,unsigned long size) {
        EventRecord& r=Event(19,size,(int)data); strcpy(r.name,name);
        if (size>sizeof(r.data)) Fail(89); memcpy(r.data,data,size); return scenario.seed&1;
    }
    int Filter(UnknownDisplay* d) {
        Event(20,Index(d));
        for (int i=0;i<d->displayModeCount;++i) d->displayModes[i].textureMemory+=23+i;
        return 1;
    }
    int SetMode(int w,int h,int depth,int a,int b,int hardware,int c) {
        activeDisplay=Index(this);
        EventRecord& r=Event(21,w,h,depth,a,b,hardware,c);
        memcpy(r.data,&activeDisplay,sizeof(activeDisplay));
        Display(activeDisplay)->currentDisplayMode=hardware;
        return scenario.fail!=(hardware?5:4);
    }
};
static int DrawIndex(void* self) {
    for (int i=0;i<4;++i) if (self==drawObjects+i) return i;
    Fail(90);return -1;
}
static long __stdcall SetMode(void* self,DWORD w,DWORD h,DWORD depth,DWORD rate,DWORD flags) {
    Event(22,DrawIndex(self),w,h,depth,rate,flags); return scenario.fail==3?-1:0;
}
static long __stdcall VidMem(void* self,UnknownSurfaceCaps* caps,DWORD* total,DWORD* available) {
    int n=DrawIndex(self); EventRecord& r=Event(23,n);
    memcpy(r.data,caps,sizeof(*caps)); *total=0x1000000+scenario.seed*256+n; *available=*total-4096;
    return scenario.seed&1?-1:0; // Writes are retained even if the HRESULT fails.
}
static long __stdcall CreateSurface(void* self,UnknownSurfaceDesc* desc,UnknownSurfaceInterface** out,void* outer) {
    int n=DrawIndex(self),at=created[n]++;
    EventRecord& r=Event(24,n,at,(int)outer); memcpy(r.data,desc,sizeof(*desc));
    if (at==scenario.surfaceLimit) return -1;
    if (at>=10) Fail(91);
    *out=(UnknownSurfaceInterface*)&surfaceObjects[n][at]; observed.surfaceAlive[n][at]=1;
    return 0;
}
static long __stdcall Release(void* self) {
    for (int n=0;n<4;++n) for (int i=0;i<10;++i) if (self==&surfaceObjects[n][i]) {
        Event(25,n,i); if (!observed.surfaceAlive[n][i]) Fail(92);
        observed.surfaceAlive[n][i]=0; return scenario.seed&1;
    }
    Fail(93);return 0;
}
static DWORD Replacement(DWORD a) {
    switch(a) {
    case 0x4c16f0:return Address(&PCGame::LoadDisplayProfile);
    case 0x534aaf:return (DWORD)sprintf;
    case 0x4c9d20:return Address(&UnknownDisplay::SetWindowedCooperativeLevel);
    case 0x4c9c90:return Address(&UnknownDisplay::SetFullscreenCooperativeLevel);
    case 0x4c9b50:return Address(&UnknownDisplay::RefreshDriverCaps);
    case 0x4a3010:return (DWORD)(void*(__cdecl*)(unsigned int,const char*,int))operator new;
    case 0x4c4ee0:return Address(&ConstructorCall::Create);
    case 0x4c4f80:return Address(&PCRenderTarget::InitializeRenderTarget);
    case 0x4c0760:return Address(&PCGame::LimitDisplayModes);
    case 0x4ca5a0:return Address(&UnknownDisplay::ProbeNonLocalTextureMemory);
    case 0x5119c0:return (DWORD)FillPixelFormat;
    case 0x4cab00:return Address(&UnknownDisplay::ProbePartialTextureUploads);
    case 0x4a3060:return (DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;
    case 0x53471a:return (DWORD)__CxxFrameHandler;
    } return 0;
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
        int inside=(p[0]>=0x4c0d10 && p[0]+5<=0x4c140e) || (p[0]>=0x54cbf0 && p[0]+5<=0x54cc14);
        if (!inside || !target || (p[2]!=0xe8 && p[2]!=0xe9) || *(BYTE*)p[0]!=p[2] ||
            p[0]+5+*(DWORD*)(p[0]+1)!=p[1]) return 0;
        *(DWORD*)(p[0]+1)=target-p[0]-5;
    }
    fclose(f);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static void Run(int retail) {
    memset(&observed,0,sizeof(observed));memset(created,0,sizeof(created));
    memset(gameStorage,0x53,sizeof(gameStorage));memset(displayStorage,0x61,sizeof(displayStorage));
    memset(targetStorage,0x74,sizeof(targetStorage));memset(modes,0,sizeof(modes));
    *(DWORD*)gameStorage=(DWORD)gameTable;Game()->osVersion.platformId=scenario.platform;
    Game()->display=(UnknownDisplay*)0x12345678;Game()->renderTarget=(RenderTarget*)0x23456789;
    g_DisplayCount=scenario.count;activeDisplay=0;
    for (int n=0;n<4;++n) {
        g_Displays[n]=Display(n);*(DWORD*)Display(n)=(DWORD)displayTable;
        Display(n)->directDraw=(UnknownDirectDrawInterface*)&drawObjects[n];
        Display(n)->backBuffer=(UnknownSurfaceInterface*)&surfaceObjects[n][10];
        Display(n)->frameBufferCount=1+n;Display(n)->displayModes=modes[n];
        Display(n)->displayModeCount=scenario.modeCount;Display(n)->driverCaps=0;
        sprintf(Display(n)->driverGuidText,"driver-%d-%d",scenario.seed,n);
        drawObjects[n]=(DWORD)drawTable;
        for (int s=0;s<11;++s) surfaceObjects[n][s]=(DWORD)surfaceTable;
        for (int m=0;m<4;++m) {
            int* words=(int*)&modes[n][m];for (int k=0;k<9;++k) words[k]=scenario.seed*111+n*27+m*9+k;
        }
    }
    memcpy((void*)0x68a754,g_Displays,sizeof(g_Displays));*(int*)0x68a764=g_DisplayCount;
    union { int(PCGame::*method)();DWORD address; } call;call.method=&PCGame::ProfileDisplays;
    if (retail) call.address=0x4c0d10;
    try { observed.result=(Game()->*call.method)(); } catch(int code) { observed.exception=code; }
    memcpy(observed.game,gameStorage,sizeof(gameStorage));memcpy(observed.displays,displayStorage,sizeof(displayStorage));
    memcpy(observed.targets,targetStorage,sizeof(targetStorage));memcpy(observed.modes,modes,sizeof(modes));
}
static int checks;
static void Compare() {
    Run(1);referenceResult=observed;Run(0);++checks;
    if (memcmp(&referenceResult,&observed,sizeof(observed))) {
        const BYTE* r=(BYTE*)&referenceResult;const BYTE* c=(BYTE*)&observed;
        int first=0;while(first<sizeof(observed) && r[first]==c[first])++first;
        printf("Mismatch %d offset%d %02x/%02x count%d load%d disabled%d fail%d surface%d partial%d platform%d caps%x dev%x modes%d seed%d throw%d\n",
            checks,first,r[first],c[first],scenario.count,scenario.loaded,scenario.disabled,scenario.fail,
            scenario.surfaceLimit,scenario.partial,scenario.platform,scenario.caps,scenario.deviceCaps,
            scenario.modeCount,scenario.seed,scenario.throwKind);ExitProcess(2);
    }
    // Normal completion must retire every constructed target and successful
    // texture. Exception paths intentionally preserve retail's ownership state.
    if (!observed.exception) {
        for (int n=0;n<4;++n) {
            if (observed.targetAlive[n]) Fail(94);
            for (int s=0;s<11;++s) if (observed.surfaceAlive[n][s]) Fail(95);
        }
        if (observed.result!=1) Fail(96);
    }
}
int main(int argc,char** argv) {
    if (argc!=3 || !MapReference(argv[1],argv[2])) return 3;
    for (int i=0;i<38;++i) gameTable[i]=(DWORD)abort;
    gameTable[20]=Address(&Hooks::GetInt);gameTable[22]=Address(&Hooks::GetFlag);
    gameTable[25]=Address(&Hooks::SetInt);gameTable[27]=Address(&Hooks::SetFlag);
    gameTable[29]=Address(&Hooks::SetBinary);gameTable[34]=Address(&Hooks::Filter);
    for (i=0;i<5;++i) displayTable[i]=(DWORD)abort;displayTable[2]=Address(&Hooks::SetMode);
    for (i=0;i<30;++i) drawTable[i]=(DWORD)abort;
    drawTable[6]=(DWORD)CreateSurface;drawTable[21]=(DWORD)SetMode;drawTable[23]=(DWORD)VidMem;
    for (i=0;i<50;++i) surfaceTable[i]=(DWORD)abort;surfaceTable[2]=(DWORD)Release;
    const int partials[]={-3,0,1,7};
    // Full combinations for display count/cache and every disable mask/failure.
    for (int count=0;count<=4;++count) for (int loaded=0;loaded<(1<<count);++loaded)
    for (int disabled=0;disabled<16;++disabled) for (int fail=0;fail<10;++fail) {
        memset(&scenario,0,sizeof(scenario));scenario.count=count;scenario.loaded=loaded;
        scenario.disabled=disabled;scenario.fail=fail;scenario.surfaceLimit=10;
        scenario.partial=1;scenario.caps=1;scenario.modeCount=3;scenario.seed=3;Compare();
    }
    // Surface failures at every boundary, modes, partial flags and capability branches.
    for (int limit=0;limit<=10;++limit) for (int p=0;p<4;++p)
    for (int cap=0;cap<4;++cap) for (int platform=1;platform<=2;++platform)
    for (int dev=0;dev<2;++dev) for (int modes=0;modes<=4;++modes) {
        memset(&scenario,0,sizeof(scenario));scenario.count=4;scenario.loaded=5;
        scenario.surfaceLimit=limit;scenario.partial=partials[p];scenario.caps=(cap&1)|((cap&2)?0x400:0);
        scenario.platform=platform;scenario.deviceCaps=dev?0x4000:0;scenario.modeCount=modes;
        scenario.seed=p;Compare();
    }
    for (int kind=1;kind<=4;++kind) {
        memset(&scenario,0,sizeof(scenario));scenario.count=2;scenario.surfaceLimit=10;
        scenario.partial=1;scenario.caps=1;scenario.modeCount=2;scenario.throwKind=kind;Compare();
    }
    for (int negative=-2;negative<0;++negative) {
        memset(&scenario,0,sizeof(scenario));scenario.count=negative;Compare();
    }
    printf("ProfileDisplays: %d scenarios match state, ordered calls, resources and unwind\n",checks);
    return 0;
}
