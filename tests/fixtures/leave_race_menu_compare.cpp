// Private complete race-to-menu comparison. External GUI/display/audio and
// allocation boundaries are scripted; no real window or device is changed.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <new>
#include "../../src/reconstructed/GUIManagerCore.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/DebugAlloc.h"
extern "C" void __cdecl __CxxFrameHandler();
static BYTE referenceSpace[0x800000]={1};
struct World {
    BYTE games[2][sizeof(TrackGame)],uis[2][sizeof(KrustyUI)],guis[2][sizeof(GUIManager)];
    BYTE users[2][sizeof(GUIUser)],tips[2][sizeof(UnknownKrustyUIToolTipView)];
    BYTE displays[2][sizeof(UnknownDisplay)],sounds[2][sizeof(PCSoundInterface)];
    BYTE dialogs[2][sizeof(UIDialog)],transition[sizeof(TransDlg)];
    UnknownDisplayMode modes[2][3];
};
static World world;
TrackGame* g_TrackGame;
static DWORD gameTables[2][20],sourceTable;
struct State {World world;int game;};
struct Event {int kind,who,a,b,c,d,live;char text[32];State state;};
struct Observation {int completed,exception,faultKind,faultAddress,count,live,allocations,deletes;Event events[48];State state;};
static Observation observed,expected;
struct Scenario {unsigned seed;int mask,mode,index,menu,findMode,callbackReturn,failAlloc,invalid,throwAt,mutateAt,mutation;};
static Scenario scenario;
static int retailActive,checks;
static void Fail(int code){printf("Invariant%d check%d ref%d mask%x mode%d index%d menu%d fail%d invalid%d throw%d mutation%d/%d count%d last%d\n",code,checks,retailActive,scenario.mask,scenario.mode,scenario.index,scenario.menu,scenario.failAlloc,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,observed.count,observed.count?observed.events[observed.count-1].kind:-1);fflush(stdout);ExitProcess(code);}
static TrackGame* GameAt(int n){return (TrackGame*)world.games[n];}
static KrustyUI* UI(int n){return (KrustyUI*)world.uis[n];}
static GUIManager* Gui(int n){return (GUIManager*)world.guis[n];}
static GUIUser* User(int n){return (GUIUser*)world.users[n];}
static int Which(const void* p,const void* a,const void* b){if(p==a)return 0;if(p==b)return 1;if(!p)return -1;Fail(81);return -1;}
static int GameIndex(){return Which(g_TrackGame,GameAt(0),GameAt(1));}
static int GuiIndex(const void* p){return Which(p,Gui(0),Gui(1));}
static void Snapshot(State& s){s.world=world;s.game=GameIndex();DWORD vptr=*(DWORD*)s.world.transition;if(vptr==sourceTable || vptr==0x552624)*(DWORD*)s.world.transition=0x552624;}
static void EventAt(int kind,int who=0,int a=0,int b=0,int c=0,int d=0,const char* text=0){
    if(observed.count>=48)Fail(82);Event& e=observed.events[observed.count++];e.kind=kind;e.who=who;e.a=a;e.b=b;e.c=c;e.d=d;e.live=observed.live;
    if(text){if(strlen(text)>31)Fail(83);strcpy(e.text,text);}Snapshot(e.state);
    if(scenario.mutateAt==observed.count){switch(scenario.mutation){
    case 0:g_TrackGame=GameAt(1-GameIndex());*(TrackGame**)0x56e26c=g_TrackGame;break;
    case 1:g_TrackGame->mode.toolTips^=0x1234;g_TrackGame->mode.field_0xa20=!g_TrackGame->mode.field_0xa20;g_TrackGame->playingReplay=!g_TrackGame->playingReplay;g_TrackGame->ui->playingDemo=!g_TrackGame->ui->playingDemo;g_TrackGame->mode.raceSettings.raceMode=(g_TrackGame->mode.raceSettings.raceMode+1)%6;g_TrackGame->ui->allowIme=!g_TrackGame->ui->allowIme;break;
    case 2:g_TrackGame->ui=UI(1);g_TrackGame->display=(UnknownDisplay*)world.displays[1];g_TrackGame->soundInterface=(SoundInterface*)world.sounds[1];break;
    case 3:g_TrackGame->ui->gui=Gui(1);User(0)->userToolTip=(ToolTip*)world.tips[1];g_TrackGame->ui->savedInputContext=(void*)0x23456789;g_TrackGame->windowHandle=(void*)0x34567891;break;
    }}
    if(scenario.throwAt==observed.count)throw 0x751;
}
static int SourceFile(const char* path){const char* name=strrchr(path,'\\');const char* slash=strrchr(path,'/');if(!name || (slash && slash>name))name=slash;name=name?name+1:path;if(strcmp(name,"EventManager.cpp") && strcmp(name,"LeaveRaceForMenu.cpp"))Fail(84);return 1;}
void* operator new(unsigned size,const char* file,int line){if(size!=sizeof(TransDlg) || line!=1064)Fail(85);EventAt(12,0,size,SourceFile(file),line);++observed.allocations;if(scenario.failAlloc)return 0;observed.live=1;return world.transition;}
void operator delete(void* p,const char* file,int line){if(p!=world.transition || line!=1064 || !observed.live)Fail(86);EventAt(17,0,SourceFile(file),line);observed.live=0;++observed.deletes;}
void operator delete(void* p){if(p)Fail(87);}
BaseObject::BaseObject(){refCount=1;}
BaseObject::~BaseObject(){}
int BaseObject::AddRef(){Fail(88);return 0;}
int BaseObject::Release(){Fail(88);return 0;}
int BaseObject::GetRefCount(){Fail(88);return 0;}
GameObject::GameObject(int flags):BaseObject(){}
GameObject::~GameObject(){}
int GameObject::Release(){Fail(88);return 0;}
GameObject* GameObject::SetRenderContext(void* target){Fail(88);return this;}
int GameObject::AppendSibling(GameObject* child,int value){Fail(88);return 0;}
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
UIDialog::UIDialog(int flags,const char* resource):GameObject(flags){if((void*)this!=world.transition)Fail(90);EventAt(13,0,flags,0,0,0,resource);strcpy(resourceName,resource);}
UIDialog::~UIDialog(){}
FAIL_INT(UIDialog,Release,()) FAIL_INT(UIDialog,Tick,(float)) FAIL_INT(UIDialog,RenderPre3D,())
FAIL_INT(UIDialog,UnknownVirtualSlot14,()) FAIL_INT(UIDialog,RenderPost3D,())
FAIL_INT(UIDialog,OnControlRelease,(UnknownControlEvent*,UnknownInputEntry*))
FAIL_INT(UIDialog,OnControlPress,(UnknownControlEvent*,UnknownInputEntry*))
UIDialog* UIDialog::UnknownVirtualSlot27(void*,CameraRect*,int,int,SoundGroup*,void*,const char*,UIDialog*,BackgroundImage*,const char*,int,GUIManager*,GUIUser*,const char*){Fail(88);return 0;}
FAIL_VOID(UIDialog,UnknownVirtualSlot28,(int)) FAIL_VOID(UIDialog,HandleEvent,(UnknownDialogEvent*))
FAIL_VOID(UIDialog,UnknownVirtualSlot30,()) FAIL_VOID(UIDialog,SyncValues,(int))
FAIL_VOID(TransDlg,HandleEvent,(UnknownDialogEvent*)) FAIL_INT(TransDlg,Tick,(float))
struct Constructors {UIDialog* Dialog(int flags,const char* name){return ::new(this) UIDialog(flags,name);}};
UIDialog* GUIManager::FindInputDialog(){int n=GuiIndex(this);EventAt(1,n);return scenario.mask&2?(UIDialog*)world.dialogs[n]:0;}
void UIDialog::EndDialog(int value){int n=Which(this,world.dialogs[0],world.dialogs[1]);if(value)Fail(89);EventAt(2,n,value);isClosing=1;}
extern "C" HIMC __stdcall ImmAssociateContext(HWND window,HIMC context){EventAt(3,0,(int)window,(int)context);return (HIMC)scenario.callbackReturn;}
int GUIManager::EnableWindowClipper(int value){if(value!=1)Fail(89);EventAt(4,GuiIndex(this),value);return scenario.callbackReturn;}
void TrackGame::StopCDAudio(){EventAt(5,Which(this,GameAt(0),GameAt(1)));}
int VideoCard::FindDisplayMode(int width,int height,int bits,int a,int b){if(width!=640 || height!=480 || bits!=16 || a || b)Fail(89);EventAt(6,Which(this,world.displays[0],world.displays[1]),width,height,bits,scenario.findMode);return scenario.findMode;}
struct VirtualHooks {
    int Switch0(int mode){EventAt(7,Which(this,GameAt(0),GameAt(1)),mode,0);return scenario.callbackReturn;}
    int Switch1(int mode){EventAt(7,Which(this,GameAt(0),GameAt(1)),mode,1);return scenario.callbackReturn;}
};
void GUIManager::CreateBackground(){EventAt(8,GuiIndex(this));ownsBackground^=0x81;}
GUIUser* GUIManager::GetUser(int index){int n=GuiIndex(this);if(index)Fail(89);EventAt(9,n,index);return scenario.invalid==1?0:User(n);}
void KrustyUI::RecordRaceResults(){EventAt(10,Which(this,UI(0),UI(1)));}
void GUIManager::ShowCursors(int value){if(value!=1)Fail(89);EventAt(11,GuiIndex(this),value);}
UIDialog* GUIManager::ShowDialog(UIDialog* dialog,int a,int flags,int b,UIDialog* parent,int c,int d,int wait){
    if(a || flags!=2 || b || parent || c || d || wait!=1 || (dialog && (void*)dialog!=world.transition))Fail(90);
    if(dialog && *(DWORD*)dialog!=(retailActive?0x552624:sourceTable))Fail(91);
    EventAt(14,GuiIndex(this),dialog!=0,flags,wait);return scenario.callbackReturn?dialog:0;
}
void TransDlg::SetNextMenu(int menu){if(this && (void*)this!=world.transition)Fail(92);EventAt(15,this?0:-1,menu);nextMenu=menu;}
int PCSoundInterface::SetPrimaryVolume(long value){if(value)Fail(93);EventAt(16,Which(this,world.sounds[0],world.sounds[1]),value);return scenario.callbackReturn;}
template<class T>static DWORD Address(T method){union{T method;DWORD address;}u;u.method=method;return u.address;}
static DWORD Replacement(DWORD target){switch(target){
case 0x485df0:return Address(&GUIManager::FindInputDialog);case 0x46ff30:return Address(&UIDialog::EndDialog);
case 0x53305e:return (DWORD)ImmAssociateContext;case 0x4868b0:return Address(&GUIManager::EnableWindowClipper);
case 0x521a40:return Address(&TrackGame::StopCDAudio);case 0x52d250:return Address(&VideoCard::FindDisplayMode);
case 0x485ef0:return Address(&GUIManager::CreateBackground);case 0x486540:return Address(&GUIManager::GetUser);
case 0x49bbb0:return Address(&KrustyUI::RecordRaceResults);case 0x486630:return Address(&GUIManager::ShowCursors);
case 0x4a3010:return (DWORD)(void*(__cdecl*)(unsigned,const char*,int))operator new;
case 0x4a3060:return (DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;
case 0x469db0:return Address(&Constructors::Dialog);case 0x485a70:return Address(&GUIManager::ShowDialog);
case 0x455c40:return Address(&TransDlg::SetNextMenu);case 0x4be9b0:return Address(&PCSoundInterface::SetPrimaryVolume);
case 0x53471a:return (DWORD)__CxxFrameHandler;
}Fail(94);return 0;}
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
        if(!((entry[0]>=0x45e710 && entry[0]+5<=0x45e923)||(entry[0]>=0x54ab30 && entry[0]+5<=0x54ab51)) || *site!=entry[2] || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1]){printf("patch failure %x %x %x\n",entry[0],entry[1],entry[2]);return 0;}
        *(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);
    }fclose(f);return records==17 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{void(EventManager::*method)(int);DWORD address;}call;call.method=&EventManager::LeaveRaceForMenu;if(retailActive)call.address=0x45e710;
    // ECX is unused by both complete bodies; a real EventManager is not needed.
    __try{(((EventManager*)0x12345678)->*call.method)(scenario.menu);observed.completed=1;}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));memset(&world,0x53+scenario.seed%19,sizeof(world));g_TrackGame=GameAt(0);*(TrackGame**)0x56e26c=g_TrackGame;
    for(int n=0;n<2;++n){TrackGame* g=GameAt(n);*(DWORD*)g=(DWORD)gameTables[n];g->ui=scenario.mask&1?UI(n):0;g->display=(UnknownDisplay*)world.displays[n];g->soundInterface=(SoundInterface*)world.sounds[n];g->windowHandle=(void*)(0x12345670+n*4);
        g->display->displayModes=world.modes[n];g->mode.displayModeIndex=scenario.index;g->mode.field_0xa20=scenario.mask&16;g->mode.toolTips=(scenario.seed&1)?-7:0;g->mode.raceSettings.raceMode=scenario.mode;g->playingReplay=scenario.mask&32;
        UI(n)->gui=Gui(n);UI(n)->savedInputContext=scenario.mask&4?(void*)(0x23456780+n*4):0;UI(n)->allowIme=scenario.mask&8;UI(n)->playingDemo=scenario.mask&64;User(n)->userToolTip=scenario.invalid==2?0:(ToolTip*)world.tips[n];
        for(int k=0;k<3;++k){world.modes[n][k].width=scenario.mask&128?800:640;world.modes[n][k].height=scenario.mask&256?600:480;world.modes[n][k].bitDepth=scenario.mask&512?32:16;}
    }
    if(scenario.invalid==3){g_TrackGame=0;*(TrackGame**)0x56e26c=0;}if(scenario.invalid==4)GameAt(0)->display=0;
    try{Invoke();}catch(int code){observed.exception=code;}Snapshot(observed.state);
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int off=0;while(a[off]==b[off])++off;printf("Mismatch%d offset%d %02x/%02x mask%x mode%d index%d menu%d return%d find%d fail%d invalid%d throw%d mutation%d/%d events%d/%d\n",checks,off,a[off],b[off],scenario.mask,scenario.mode,scenario.index,scenario.menu,scenario.callbackReturn,scenario.findMode,scenario.failAlloc,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.mask=1|2|4|8|128;scenario.mode=1;scenario.menu=3001;scenario.findMode=2;scenario.callbackReturn=-7;}
static int Count(int kind){int count=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++count;return count;}
int main(int argc,char** argv){if(argc!=4)return 3;sourceTable=strtoul(argv[3],0,16);if(!MapReference(argv[1],argv[2]))return 4;
    gameTables[0][19]=Address(&VirtualHooks::Switch0);gameTables[1][19]=Address(&VirtualHooks::Switch1);
    if(sizeof(TransDlg)!=0x7f64 || sizeof(KrustyUI)!=0x1364 || sizeof(GUIManager)!=0x3e0)return 5;
    Base();Compare();if(!observed.completed || !observed.live || observed.allocations!=1 || observed.deletes || Count(7)!=1 || Count(10)!=1 || Count(16)!=1 || ((TransDlg*)world.transition)->nextMenu!=3001 || GameAt(0)->uiInteractionBlocked || GameAt(0)->podiumFinished || GameAt(0)->uiBlockedSeconds || ((UnknownKrustyUIToolTipView*)world.tips[0])->enabled!=-7)return 6;
    Base();scenario.mask=0;Compare();if(!observed.completed || observed.count || observed.allocations)return 7;
    Base();scenario.mask=1;scenario.mode=0;Compare();if(!observed.completed || Count(6) || Count(7) || Count(10) || Count(3) || Count(4) || Count(2) || Count(5)!=1)return 12;
    Base();Compare();int findAt=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==6)findAt=i+1;
    Base();scenario.mutateAt=findAt;scenario.mutation=0;Compare();int switchAt=0;for(i=0;i<observed.count;++i)if(observed.events[i].kind==7)switchAt=i;
    if(!findAt || !switchAt || observed.events[switchAt].who!=1 || observed.events[switchAt].b!=0)return 13;
    const int modes[]={-2147483647-1,-1,0,1,2,3,4,5,2147483647};
    for(int mask=0;mask<1024;++mask)for(int mode=0;mode<9;++mode){Base();scenario.seed=checks+1;scenario.mask=mask;scenario.mode=modes[mode];scenario.index=mask%3;scenario.menu=modes[(mode+3)%9];scenario.callbackReturn=modes[(mode+5)%9];scenario.findMode=modes[(mode+7)%9];Compare();}
    for(int path=0;path<3;++path){Base();if(path==1)scenario.mask=1;if(path==2)scenario.mask=1023;Compare();int count=observed.count;
        for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<4;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
    Base();scenario.failAlloc=1;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.faultKind!=1 || observed.faultAddress!=0x7f60 || observed.live || Count(14)!=1 || Count(16))return 8;
    Base();Compare();int ctor=0,show=0;for(i=0;i<observed.count;++i){if(observed.events[i].kind==13)ctor=i+1;if(observed.events[i].kind==14)show=i+1;}
    Base();scenario.throwAt=ctor;Compare();if(!ctor || observed.exception!=0x751 || observed.live || observed.deletes!=1 || observed.events[observed.count-1].kind!=17)return 9;
    Base();scenario.throwAt=show;Compare();if(!show || observed.exception!=0x751 || !observed.live || observed.deletes)return 10;
    for(int invalid=1;invalid<=4;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 11;}
    printf("LeaveRaceForMenu: %d complete transition, state, callback, allocation and unwind scenarios match\n",checks);return 0;
}
