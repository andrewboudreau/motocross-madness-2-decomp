// Private complete startup-operation comparison. GUI/profile/resource and
// allocation boundaries are scripted; the unchanged standalone sample runs.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include <new>
#include "../../src/reconstructed/GUIManagerCore.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/DebugAlloc.h"
extern "C" void __cdecl __CxxFrameHandler();
// Initialized storage reserves the mapped reference interval before the large
// BSS snapshot arrays; this contains no game instructions or copied data.
static BYTE referenceSpace[0x800000]={1};
struct World {
    BYTE games[2][sizeof(TrackGame)],ui[sizeof(KrustyUI)],guis[2][sizeof(GUIManager)],intro[sizeof(Intro1Dlg)];
    BYTE users[2][sizeof(GUIUser)],tips[2][sizeof(UnknownKrustyUIToolTipView)];
    BYTE displays[2][0x70],controls[2][0x20],networks[2][sizeof(NetworkInterface)],joysticks[2][sizeof(PCJoystickDevice)],managers[2][sizeof(UnknownResourceManager)];
};
static World world;
TrackGame* g_TrackGame;
UnknownResourceManager* g_ResourceManager;
char g_ResourceNames[36][16];
static DWORD uiTable[27],gameTable[30],sourceIntroTable;
struct State {World world;char names[36][16];int game,manager;};
struct Event {int kind,who,a,b,c,d,live[2];char text[128];State state;};
struct Observation {int result,exception,faultKind,faultAddress,count,live[2],allocations,releases;unsigned fp;Event events[100];State state;};
static Observation observed,expected;
struct Scenario {unsigned seed;int mask,length,font,scale,bold,volume,filter,showIntro,network,failAlloc,appendFail,throwAt,mutateAt,mutation,round;};
static Scenario scenario;
static int retailActive,checks;
static void Fail(int code){printf("Fixture invariant %d check%d active%d mask%x fail%d append%d throw%d mutate%d/%d events%d last%d\n",code,checks,retailActive,scenario.mask,scenario.failAlloc,scenario.appendFail,scenario.throwAt,scenario.mutateAt,scenario.mutation,observed.count,observed.count?observed.events[observed.count-1].kind:-1);fflush(stdout);ExitProcess(code);}
static KrustyUI* UI(){return (KrustyUI*)world.ui;}
static TrackGame* GameAt(int n){return (TrackGame*)world.games[n];}
static GUIManager* Gui(int n){return (GUIManager*)world.guis[n];}
static GUIUser* User(int n){return (GUIUser*)world.users[n];}
static int Which(const void* p,const void* a,const void* b){if(p==a)return 0;if(p==b)return 1;printf("Which %p vs %p/%p\n",p,a,b);Fail(81);return -1;}
static int GameIndex(){return Which(g_TrackGame,GameAt(0),GameAt(1));}
static void Snapshot(State& s){s.world=world;memcpy(s.names,retailActive?(void*)0x68a498:g_ResourceNames,sizeof(s.names));s.game=GameIndex();s.manager=Which(g_ResourceManager,world.managers[0],world.managers[1]);
    DWORD vptr=*(DWORD*)s.world.intro;if(vptr==sourceIntroTable || vptr==0x5548fc)*(DWORD*)s.world.intro=0x5548fc;
}
static void EventAt(int kind,int who=0,int a=0,int b=0,int c=0,int d=0,const char* text=0){
    if(observed.count>=100)Fail(82);Event& event=observed.events[observed.count++];event.kind=kind;event.who=who;event.a=a;event.b=b;event.c=c;event.d=d;memcpy(event.live,observed.live,sizeof(event.live));
    if(text){if(strlen(text)>127)Fail(83);strcpy(event.text,text);}Snapshot(event.state);
    if(scenario.mutateAt==observed.count){switch(scenario.mutation){
    case 0:g_TrackGame=GameAt(1-GameIndex());*(TrackGame**)0x56e26c=g_TrackGame;break;
    case 1:g_TrackGame->mode.uiSounds=!g_TrackGame->mode.uiSounds;g_TrackGame->mode.uiVolume^=123;g_TrackGame->mode.toolTips=!g_TrackGame->mode.toolTips;g_TrackGame->mode.uiAnimations^=15;break;
    case 2:UI()->gui=Gui(1);break;
    case 3:g_ResourceManager=(UnknownResourceManager*)world.managers[1];*(UnknownResourceManager**)0x572b44=g_ResourceManager;g_TrackGame->network=0;g_TrackGame->controlInterface->activeJoystick=0;break;
    }}
    if(scenario.throwAt==observed.count)throw 0x721;
}
static int SourceFile(const char* path){const char* name=strrchr(path,'\\');const char* slash=strrchr(path,'/');if(!name || (slash && slash>name))name=slash;name=name?name+1:path;if(strcmp(name,"krustyui.cpp") && strcmp(name,"KrustyUINearMisses.cpp"))Fail(84);return 1;}
void* operator new(unsigned size,const char* file,int line){int n=size==sizeof(GUIManager)?0:1;if((n==0 && line!=121)||(n==1 && (size!=sizeof(Intro1Dlg)||line!=201)))Fail(85);EventAt(2,n,size,SourceFile(file),line);++observed.allocations;if(scenario.failAlloc==n+1)return 0;observed.live[n]=1;return n?(void*)world.intro:(void*)Gui(0);}
void operator delete(void* p,const char* file,int line){int n=Which(p,Gui(0),world.intro);EventAt(3,n,SourceFile(file),line);if(!observed.live[n])Fail(86);observed.live[n]=0;}
void operator delete(void* p){if(p)Fail(87);}
// These C++ constructors deliberately model only external dependency behavior.
// Real generated ctor/vptr/unwind ABI is used, including fail-fast unused slots.
BaseObject::BaseObject(){refCount=1;}
BaseObject::~BaseObject(){}
int BaseObject::AddRef(){Fail(88);return 0;}
int BaseObject::Release(){Fail(88);return 0;}
int BaseObject::GetRefCount(){Fail(88);return 0;}
GameObject::GameObject(int flags):BaseObject(){}
GameObject::~GameObject(){}
int GameObject::Release(){Fail(88);return 0;}
GameObject* GameObject::SetRenderContext(void* target){if(this!=UI())Fail(89);EventAt(1,0,(int)target);renderContext=target;return this;}
int GameObject::AppendSibling(GameObject* child,int value){if(this!=UI() || value!=-1)Fail(89);EventAt(5,0,(int)child,value);return !scenario.appendFail;}
#define FAIL_INT(cls,name,args) int cls::name args {Fail(88);return 0;}
#define FAIL_VOID(cls,name,args) void cls::name args {Fail(88);}
FAIL_VOID(GameObject,Disable,()) FAIL_VOID(GameObject,Enable,())
FAIL_VOID(GameObject,OnDisable,()) FAIL_VOID(GameObject,OnEnable,())
FAIL_INT(GameObject,UnknownVirtualSlot9,(int)) FAIL_INT(GameObject,Tick,(float))
FAIL_INT(GameObject,UnknownVirtualSlot11,(float)) FAIL_INT(GameObject,PrepareGeometry,())
FAIL_INT(GameObject,RenderPre3D,()) FAIL_INT(GameObject,UnknownVirtualSlot14,()) FAIL_INT(GameObject,RenderPost3D,())
FAIL_INT(GameObject,SetPaused,(int)) FAIL_INT(GameObject,OnDeactivate,()) FAIL_INT(GameObject,OnActivate,())
FAIL_INT(GameObject,UnknownVirtualSlot19,(int)) FAIL_INT(GameObject,OnChar,(int)) FAIL_INT(GameObject,OnKeyDown,(int))
FAIL_INT(GameObject,OnControlRelease,(UnknownControlEvent*,UnknownInputEntry*))
FAIL_INT(GameObject,OnControlPress,(UnknownControlEvent*,UnknownInputEntry*))
FAIL_INT(GameObject,OnNetworkMessage,(int,void*,int,int,int)) FAIL_INT(GameObject,AttachRenderContext,(void*))
FAIL_VOID(GameObject,MarkPendingRelease,())
GUIManager::GUIManager(int flags):GameObject(flags){EventAt(4,Which(this,Gui(0),Gui(1)),flags);}
GUIManager::~GUIManager(){Fail(88);}
FAIL_INT(GUIManager,Tick,(float)) FAIL_INT(GUIManager,RenderPre3D,()) FAIL_INT(GUIManager,RenderPost3D,())
FAIL_INT(GUIManager,OnControlRelease,(UnknownControlEvent*,UnknownInputEntry*))
FAIL_INT(GUIManager,OnControlPress,(UnknownControlEvent*,UnknownInputEntry*))
UIDialog::UIDialog(int flags,const char* resource):GameObject(flags){if((void*)this!=world.intro)Fail(90);EventAt(21,0,flags,0,0,0,resource);strcpy(resourceName,resource);}
UIDialog::~UIDialog(){}
FAIL_INT(UIDialog,Release,()) FAIL_INT(UIDialog,Tick,(float)) FAIL_INT(UIDialog,RenderPre3D,())
FAIL_INT(UIDialog,UnknownVirtualSlot14,()) FAIL_INT(UIDialog,RenderPost3D,())
FAIL_INT(UIDialog,OnControlRelease,(UnknownControlEvent*,UnknownInputEntry*))
FAIL_INT(UIDialog,OnControlPress,(UnknownControlEvent*,UnknownInputEntry*))
UIDialog* UIDialog::UnknownVirtualSlot27(void*,CameraRect*,int,int,SoundGroup*,void*,const char*,UIDialog*,BackgroundImage*,const char*,int,GUIManager*,GUIUser*,const char*){Fail(88);return 0;}
FAIL_VOID(UIDialog,UnknownVirtualSlot28,(int)) FAIL_VOID(UIDialog,HandleEvent,(UnknownDialogEvent*))
FAIL_VOID(UIDialog,UnknownVirtualSlot30,()) FAIL_VOID(UIDialog,SyncValues,(int))
FAIL_VOID(Intro1Dlg,HandleEvent,(UnknownDialogEvent*)) FAIL_INT(Intro1Dlg,Tick,(float))
FAIL_INT(Intro1Dlg,OnControlPress,(UnknownControlEvent*,UnknownInputEntry*))
struct Constructors {GameObject* Context(void* p){return ((GameObject*)this)->GameObject::SetRenderContext(p);}GUIManager* Gui(int flags){return ::new(this) GUIManager(flags);}UIDialog* Dialog(int flags,const char* name){return ::new(this) UIDialog(flags,name);}};
void GuiProgressCallback(){Fail(91);}
static int GuiIndex(const void* p){return Which(p,Gui(0),Gui(1));}
GUIManager* GUIManager::SetUp(void* target,Palette8* palette,TextureMapManager* textures,BackgroundImage* background,int sound,SoundGroup* group,const char* font,short size,const char* cursor,int callback){
    if(background || sound || group || size!=15 || strcmp(font,"Arial") || strcmp(cursor,"ui\\cursor.tga") || (callback!=(int)GuiProgressCallback && callback!=0x49bda0))Fail(92);
    EventAt(6,this?GuiIndex(this):-1,(int)target,(int)palette,(int)textures,size,cursor);renderContext=target;return this;
}
void GUIManager::SetToolTipFont(const char* font){EventAt(7,GuiIndex(this),0,0,0,0,font);strcpy(toolTipFontFace,font);}
void GUIManager::CreateBackground(){EventAt(8,GuiIndex(this));ownsBackground^=0x81;}
void GUIManager::SetDialogDirectory(const char* path){EventAt(9,GuiIndex(this),0,0,0,0,path);strcpy(dialogDirectory,path);}
void GUIManager::SetWaitCursorImage(const char* path){EventAt(10,GuiIndex(this),0,0,0,0,path);strcpy(waitCursorImage,path);}
GUIUser* GUIManager::GetUser(int index){int n=GuiIndex(this);EventAt(11,n,index);if(index)Fail(93);return User(n);}
void GUIUser::ForgetDevices(){EventAt(12,Which(this,User(0),User(1)));acceptedDevices.m_count=0;}
int GUIUser::AcceptDevice(GUIInputDevice* device){EventAt(13,Which(this,User(0),User(1)),(int)device);acceptedDevices.m_count++;return -3;}
void GUIManager::ShowCursors(int value){EventAt(14,GuiIndex(this),value);}
UnknownTextureStream* UnknownResourceManager::AddArchive(const char* path,int flags){EventAt(15,Which(this,world.managers[0],world.managers[1]),flags,0,0,0,path);return 0;}
void TrackGameMode::LoadBonusTrackNames(){EventAt(16,Which(this,&GameAt(0)->mode,&GameAt(1)->mode));}
int TrackGameMode::LoadProfile(){EventAt(17,Which(this,&GameAt(0)->mode,&GameAt(1)->mode));return scenario.mask&1;}
void PCJoystickDevice::UnknownMethod4c3ae0(unsigned char value){EventAt(18,this?Which(this,world.joysticks[0],world.joysticks[1]):-1,value);field_0x5d5=value;}
void KrustyUI::LoadBikesAndRiders(){if(this!=UI() || bikes || riders || manufacturers)Fail(94);EventAt(19);bikes=(void*)0x12341111;riders=(void*)0x12342222;}
void KrustyUI::LoadGarageTables(){if(this!=UI())Fail(94);EventAt(20);manufacturers=(void*)0x12343333;}
UIDialog* GUIManager::ShowDialog(UIDialog* dialog,int a,int flags,int b,UIDialog* parent,int c,int d,int wait){
    if(a || flags!=2 || b || parent || c || d || wait!=1 || (dialog && (void*)dialog!=world.intro))Fail(95);
    if(dialog && *(DWORD*)dialog!=(retailActive?0x5548fc:sourceIntroTable))Fail(96);
    EventAt(22,GuiIndex(this),dialog!=0,flags,wait);return dialog;
}
int TrackGame::LoadResourceString(int id,char* buffer,int size){int n=Which(this,GameAt(0),GameAt(1));if(size!=128)Fail(97);EventAt(23,n,id,size);
    if(id>=5000 && id<5036){int length=(scenario.length+id-5000)%128;for(int i=0;i<length;++i)buffer[i]='A'+(id+i+n)%26;buffer[length]=0;return scenario.mask&1?length:0;}
    if(id==0xfed){const char* values[]={"0","-0","1.25","-2.5","1e38","1e-40","nan","bad"};strcpy(buffer,values[scenario.scale]);return scenario.mask&2;}
    if(id==0xff2){const char* values[]={"Arial","NONE","none suffix","","Courier"};if(scenario.font<5)strcpy(buffer,values[scenario.font]);else{memset(buffer,'F',127);buffer[127]=0;}return scenario.mask&4;}
    if(id==0x1469){sprintf(buffer,"%d",scenario.bold);return scenario.mask&8;}Fail(98);return 0;
}
struct VirtualHooks {
    int Release(){if((void*)this!=UI())Fail(99);EventAt(24);++observed.releases;return 0;}
    int RegistryFlag(const char* key,int defaultValue){EventAt(25,Which(this,GameAt(0),GameAt(1)),defaultValue,0,0,0,key);if(strcmp(key,"JoystickFilter"))Fail(99);return scenario.filter;}
    int RegistryString(const char* key,const char* fallback,char* buffer,unsigned long* size){int n=Which(this,GameAt(0),GameAt(1));if(buffer!=GameAt(n)->mode.profileName || strcmp(fallback,"") || *size!=16)Fail(99);EventAt(26,n,*size,0,0,0,key);strcpy(buffer,"fixture-profile");*size=15;return -7;}
};
template<class T>static DWORD Address(T method){union{T method;DWORD address;}u;u.method=method;return u.address;}
static DWORD Replacement(DWORD target){switch(target){
case 0x4692f0:return Address(&Constructors::Context);case 0x521970:return Address(&TrackGame::LoadResourceString);
case 0x534b10:return (DWORD)strncpy;case 0x536340:return (DWORD)atof;case 0x5476f0:return (DWORD)_strnicmp;case 0x536057:return (DWORD)atoi;
case 0x4a3010:return (DWORD)(void*(__cdecl*)(unsigned,const char*,int))operator new;
case 0x4a3060:return (DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;
case 0x485190:return Address(&Constructors::Gui);case 0x469db0:return Address(&Constructors::Dialog);
case 0x4853b0:return Address(&GUIManager::SetUp);case 0x469130:return Address((int(GameObject::*)(GameObject*,int))&GameObject::AppendSibling);
case 0x4866c0:return Address(&GUIManager::SetToolTipFont);case 0x485ef0:return Address(&GUIManager::CreateBackground);
case 0x485d70:return Address(&GUIManager::SetDialogDirectory);case 0x486560:return Address(&GUIManager::SetWaitCursorImage);
case 0x486540:return Address(&GUIManager::GetUser);case 0x487d60:return Address(&GUIUser::ForgetDevices);case 0x487c30:return Address(&GUIUser::AcceptDevice);
case 0x486630:return Address(&GUIManager::ShowCursors);case 0x4e9030:return Address(&UnknownResourceManager::AddArchive);
case 0x523e50:return Address(&TrackGameMode::LoadBonusTrackNames);case 0x5231f0:return Address(&TrackGameMode::LoadProfile);
case 0x4c3ae0:return Address(&PCJoystickDevice::UnknownMethod4c3ae0);case 0x49a8b0:return Address(&KrustyUI::LoadBikesAndRiders);
case 0x49a540:return Address(&KrustyUI::LoadGarageTables);case 0x485a70:return Address(&GUIManager::ShowDialog);
case 0x53471a:return (DWORD)__CxxFrameHandler;
}Fail(100);return 0;}
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)){printf("map coverage %p end%p\n",referenceSpace,referenceSpace+sizeof(referenceSpace));return 0;}
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 || dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE || pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
       pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 ||
       pe->OptionalHeader.SizeOfHeaders>(DWORD)size || pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD sectionsOffset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if(sectionsOffset>(DWORD)size || pe->FileHeader.NumberOfSections>((DWORD)size-sectionsOffset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(section[i].PointerToRawData>(DWORD)size || section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData ||
           section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage || section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){++records;BYTE* site=(BYTE*)entry[0];
        if(!((entry[0]>=0x4988a0 && entry[0]+5<=0x498cf0)||(entry[0]>=0x54be40 && entry[0]+5<=0x54be7b)) || *site!=entry[2] || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1]){printf("patch failure %x %x %x\n",entry[0],entry[1],entry[2]);return 0;}
        *(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);
    }fclose(f);return records==36 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{GameObject*(KrustyUI::*method)(RenderTarget*,int);DWORD address;}call;call.method=&KrustyUI::Create;if(retailActive)call.address=0x4988a0;
    __try{observed.result=(int)(UI()->*call.method)((RenderTarget*)0x1234abcd,scenario.showIntro);}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){
    retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-501;memset(&world,0x53+(scenario.seed%17),sizeof(world));memset(g_ResourceNames,0x72,sizeof(g_ResourceNames));memcpy((void*)0x68a498,g_ResourceNames,sizeof(g_ResourceNames));
    *(DWORD*)UI()=(DWORD)uiTable;UI()->gui=Gui(0);g_TrackGame=GameAt(0);g_ResourceManager=(UnknownResourceManager*)world.managers[0];*(TrackGame**)0x56e26c=g_TrackGame;*(UnknownResourceManager**)0x572b44=g_ResourceManager;
    for(int n=0;n<2;++n){TrackGame* g=GameAt(n);*(DWORD*)g=(DWORD)gameTable;g->display=(UnknownDisplay*)world.displays[n];g->controlInterface=(ControlInterface*)world.controls[n];g->controlInterface->activeJoystick=scenario.mask&16?(JoystickDevice*)world.joysticks[n]:0;g->textureManager=(TextureMapManager*)(0x34560000+n*16);g->network=scenario.network?(NetworkInterface*)world.networks[n]:0;if(g->network)g->network->lobbyConnected=scenario.network==2;*(DWORD*)(world.displays[n]+0x64)=0x45670000+n*16;
        g->mode.uiAnimations=(scenario.mask&32)?-7:0;g->mode.toolTips=scenario.mask&64;g->mode.uiSounds=scenario.mask&128;g->mode.uiVolume=scenario.volume;
        Gui(n)->keyboardDevice=(GUIInputDevice*)(0x56780000+n*16);Gui(n)->toolTipBold=123;Gui(n)->fontSizeOffset=0x3e800000;User(n)->userToolTip=(ToolTip*)world.tips[n];
    }
    _control87(_MCW_EM|_PC_64|(scenario.round<<8),_MCW_EM|_MCW_PC|_MCW_RC);_clear87();try{Invoke();}catch(int e){observed.exception=e;}observed.fp=_status87();Snapshot(observed.state);
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int off=0;while(a[off]==b[off])++off;printf("Mismatch%d offset%d %02x/%02x seed%u mask%x font%d scale%d fail%d append%d throw%d mutate%d/%d events%d/%d\n",checks,off,a[off],b[off],scenario.seed,scenario.mask,scenario.font,scenario.scale,scenario.failAlloc,scenario.appendFail,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.mask=255;scenario.scale=2;scenario.font=4;scenario.volume=83;scenario.filter=0x123;scenario.bold=-7;scenario.showIntro=1;scenario.length=14;}
int main(int argc,char** argv){if(argc!=4)return 3;sourceIntroTable=strtoul(argv[3],0,16);if(!MapReference(argv[1],argv[2]))return 4;
    uiTable[2]=Address(&VirtualHooks::Release);gameTable[22]=Address(&VirtualHooks::RegistryFlag);gameTable[23]=Address(&VirtualHooks::RegistryString);
    if(sizeof(KrustyUI)!=0x1364 || sizeof(GUIManager)!=0x3e0 || sizeof(Intro1Dlg)!=0x7f5c)return 5;
    Base();Compare();if(observed.result!=(int)UI() || observed.allocations!=2 || observed.releases || !observed.live[0] || !observed.live[1] || Gui(0)->soundVolume!=-625 || Gui(0)->toolTipBold!=-7 || strcmp(Gui(0)->toolTipFontFace,"Courier"))return 6;
    Base();scenario.appendFail=1;Compare();if(observed.result || observed.releases!=1 || observed.allocations!=1)return 7;
    for(int mask=0;mask<256;++mask)for(int font=0;font<6;++font)for(int scale=0;scale<8;++scale){Base();scenario.seed=checks;scenario.mask=mask;scenario.font=font;scenario.scale=scale;scenario.showIntro=mask&1;scenario.network=(mask>>1)%3;scenario.length=(mask+font*13)%128;scenario.volume=mask-128;Compare();}
    for(mask=0;mask<4;++mask){Base();scenario.mask=mask?255:0;scenario.network=mask==2?2:0;scenario.appendFail=mask==3;Compare();int count=observed.count;
        for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int m=0;m<4;++m){scenario.mutateAt=at;scenario.mutation=m;Compare();}scenario.mutateAt=0;}}
    for(int fail=1;fail<=2;++fail){Base();scenario.failAlloc=fail;Compare();
        if(fail==1 && (observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.faultKind!=1 || observed.faultAddress!=0x18 || observed.live[0] || observed.live[1]))return 8;
        if(fail==2 && (observed.exception || observed.result!=(int)UI() || !observed.live[0] || observed.live[1] || observed.events[observed.count-1].kind!=22 || observed.events[observed.count-1].a))return 9;
    }
    Base();Compare();int constructorAt[2]={0,0};
    for(int event=0;event<observed.count;++event){if(observed.events[event].kind==4)constructorAt[0]=event+1;if(observed.events[event].kind==21)constructorAt[1]=event+1;}
    for(int ctor=0;ctor<2;++ctor){Base();scenario.throwAt=constructorAt[ctor];Compare();
        if(!scenario.throwAt || observed.exception!=0x721 || observed.live[ctor] || observed.allocations!=ctor+1 || observed.events[observed.count-1].kind!=3 || observed.events[observed.count-1].who!=ctor || (ctor && !observed.live[0]))return 10;
    }
    const int values[]={(-2147483647-1),2147483647,-1,0,100,101,256};
    for(int volume=0;volume<7;++volume)for(int filter=0;filter<7;++filter){Base();scenario.volume=values[volume];scenario.filter=values[filter];Compare();}
    for(int round=0;round<4;++round)for(int scale=0;scale<8;++scale){Base();scenario.round=round*4;scenario.scale=scale;Compare();}
    printf("KrustyUICreate: %d complete startup, allocation, profile and callback scenarios match\n",checks);return 0;
}
