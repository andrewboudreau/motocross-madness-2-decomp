// Private retail comparison; production candidate compiled as a separate object.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/GUIManager.h"
static unsigned char referenceSpace[0x800000];
struct World { unsigned char user[2][sizeof(GUIUser)]; unsigned char device[2][sizeof(GUIInputDevice)];
    unsigned char tip[sizeof(ToolTip)]; unsigned char control[sizeof(UnknownGuiControl)];
    unsigned char dialog[sizeof(UIDialog)]; char text[32]; };
static World world;
struct Observation { int result,calls,exception,faultKind,faultAddress; DWORD receiver,text; int x,y; float time; World boundary,after; };
static Observation actual,expected;
static int checks,callbackMode;
static GUIUser* User(int i){return (GUIUser*)world.user[i];}
static GUIInputDevice* Device(int i){return (GUIInputDevice*)world.device[i];}
static ToolTip* Tip(){return (ToolTip*)world.tip;}
static UnknownGuiControl* Control(){return (UnknownGuiControl*)world.control;}
static UIDialog* Dialog(){return (UIDialog*)world.dialog;}
void ToolTip::ShowText(const char* text,int* pos,float time){
    ++actual.calls; actual.receiver=(DWORD)this;actual.text=(DWORD)text;
    actual.x=pos[0];actual.y=pos[1];actual.time=time;actual.boundary=world;
    if(callbackMode==1){enabled^=0x12345678;world.text[3]^=0x5a;User(0)->pointerDevice=Device(1);pos[0]=123;pos[1]=-321;}
    if(callbackMode==2)throw 73;
}
static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) { printf("Bad reference reservation %p\n",referenceSpace);return 0; }
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

static int Filter(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    actual.exception=e->ExceptionRecord->ExceptionCode;actual.faultKind=e->ExceptionRecord->ExceptionInformation[0];
    actual.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static int InvokePointer(int retail,GUIInputDevice* device){
    union { int (GUIUser::*method)(GUIInputDevice*);DWORD address; } call;
    call.method=&GUIUser::SetPointerDevice;if(retail)call.address=0x488120;
    __try{return (User(0)->*call.method)(device);}__except(Filter(GetExceptionInformation())){return -5;}
}
static void InvokeTip(int retail,UnknownGuiControl* control){
    union { void (ToolTip::*method)(UnknownGuiControl*);DWORD address; } call;
    call.method=&ToolTip::ShowControlToolTip;if(retail)call.address=0x486b10;
    __try{(Tip()->*call.method)(control);}__except(Filter(GetExceptionInformation())){}
}
static void Initialize(unsigned int seed){
    unsigned char* p=(unsigned char*)&world;
    for(unsigned int i=0;i<sizeof(world);++i){seed=seed*1664525u+1013904223u;p[i]=(unsigned char)(seed>>24);}
    memcpy(world.text,"tooltip fixture",16);
}
static void Check(){
    ++checks;if(memcmp(&actual,&expected,sizeof(actual))){
        for(unsigned int i=0;i<sizeof(actual);++i)if(((BYTE*)&actual)[i]!=((BYTE*)&expected)[i]){printf("Mismatch %d at %u\n",checks,i);break;}
        ExitProcess(2);
    }
}
static void Pointer(int retail,int seed,int arg,int owner,int old){
    memset(&actual,0,sizeof(actual));Initialize(seed);
    User(0)->pointerDevice=old==0?0:Device(old-1);
    Device(0)->ownerUser=owner==0?0:User(owner-1);
    actual.boundary=world;
    actual.result=InvokePointer(retail,arg==0?0:(arg==1?Device(0):(GUIInputDevice*)1));
    actual.after=world;
    if(arg<2){
        int success=arg==0 || owner!=2;
        if(actual.result!=success || actual.exception || actual.calls)ExitProcess(81);
        if(arg==0 && User(0)->pointerDevice)ExitProcess(82);
        if(arg==1 && owner==0 && (User(0)->pointerDevice!=Device(0) || Device(0)->ownerUser!=User(0)))ExitProcess(83);
        if(arg==1 && owner!=0 && memcmp(&actual.boundary,&world,sizeof(world)))ExitProcess(84);
    }
}
static void Tooltip(int retail,int seed,int device,int fault,int left,int right,int bottom,int x,int y){
    memset(&actual,0,sizeof(actual));Initialize(seed);
    Control()->ownerDialog=Dialog();Dialog()->guiUser=User(0);User(0)->pointerDevice=device?Device(0):0;
    Control()->toolTipText=seed&1?world.text:0;Control()->area.left=left;Control()->area.right=right;Control()->area.bottom=bottom;
    Device(0)->pointerPosition.x=x;Device(0)->pointerPosition.y=y;
    if(fault==2)Control()->ownerDialog=0;
    if(fault==3)Dialog()->guiUser=0;
    if(fault==4)User(0)->pointerDevice=(GUIInputDevice*)1;
    try {InvokeTip(retail,fault==1?0:Control());}catch(int e){actual.exception=e;}
    actual.after=world;
    if(!fault){
        unsigned int delta=(unsigned int)right-(unsigned int)left;
        int half=(int)delta/2;
        int expectedX=device?x:(int)((unsigned int)left+(unsigned int)half);
        int expectedY=device?(int)((unsigned int)y+16u):bottom;
        if(actual.calls!=1 || actual.receiver!=(DWORD)Tip() || actual.text!=(DWORD)(seed&1?world.text:0) ||
           actual.x!=expectedX || actual.y!=expectedY || actual.time!=1.0f || actual.exception!=(callbackMode==2?73:0))ExitProcess(85);
    }else if(actual.calls || actual.exception!=EXCEPTION_ACCESS_VIOLATION)ExitProcess(86);
}
int main(int argc,char**argv){
    if(argc!=2 || !MapReference(argv[1]))return 3;
    if(offsetof(GUIUser,pointerDevice)!=0x2c || offsetof(GUIInputDevice,ownerUser)!=0xc0 ||
       offsetof(GUIInputDevice,pointerPosition)!=0xa4 || offsetof(UIDialog,guiUser)!=0x34 ||
       offsetof(UnknownGuiControl,ownerDialog)!=0xb8 || offsetof(UnknownGuiControl,toolTipText)!=0xf0)return 4;
    union{void(ToolTip::*method)(const char*,int*,float);DWORD address;} hook;hook.method=&ToolTip::ShowText;
    if(*(BYTE*)0x486b6e!=0xe8 || 0x486b73+*(DWORD*)0x486b6f!=0x486b80)return 5;
    *(DWORD*)0x486b6f=hook.address-0x486b73;FlushInstructionCache(GetCurrentProcess(),0,0);
    for(int seed=0;seed<256;++seed)for(int arg=0;arg<3;++arg)for(int owner=0;owner<3;++owner)for(int old=0;old<3;++old){
        Pointer(1,seed,arg,owner,old);expected=actual;Pointer(0,seed,arg,owner,old);Check();
    }
    int values[]={0,1,-1,2,-2,15,-15,0x7fffffff,(int)0x80000000};
    for(int l=0;l<9;++l)for(int r=0;r<9;++r)for(int b=0;b<9;++b)for(int dev=0;dev<2;++dev)for(callbackMode=0;callbackMode<3;++callbackMode){
        Tooltip(1,l+r+b,dev,0,values[l],values[r],values[b],values[r],values[l]);expected=actual;
        Tooltip(0,l+r+b,dev,0,values[l],values[r],values[b],values[r],values[l]);Check();
    }
    unsigned int random=987654321;
    for(int n=0;n<8192;++n){
        int v[5];for(int j=0;j<5;++j){random=random*1664525u+1013904223u;v[j]=(int)random;}
        callbackMode=n%3;
        Tooltip(1,n,n&1,0,v[0],v[1],v[2],v[3],v[4]);expected=actual;
        Tooltip(0,n,n&1,0,v[0],v[1],v[2],v[3],v[4]);Check();
    }
    for(int fault=1;fault<=4;++fault)for(int s=0;s<32;++s){
        Tooltip(1,s,s&1,fault,0,1,2,3,4);expected=actual;Tooltip(0,s,s&1,fault,0,1,2,3,4);Check();
    }
    printf("GuiPointerHelpers: %d ownership, coordinate, callback and fault scenarios match\n",checks);return 0;
}
