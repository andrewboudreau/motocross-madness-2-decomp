// Private differential execution of the actual standalone UIDialog tick object.
// Dependencies are scripted; callbacks observe only initialized event fields.
#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/DialogProc.h"
#include "../../src/reconstructed/DialogEventKind.h"
#include "../../src/reconstructed/GUIManager.h"
#include "../../src/reconstructed/InputDevice.h"
#include "../../src/reconstructed/UIDialog.h"

extern "C" long __cdecl _ftol();
typedef char dialog_size[sizeof(UIDialog)==0x7f58?1:-1];
typedef char timer_size[sizeof(UITimer)==0x18?1:-1];
typedef char timer_elapsed[offsetof(UITimer,elapsedTime)==0x0c?1:-1];
typedef char dialog_time[offsetof(UIDialog,lastFrameTime)==0x9c0?1:-1];
typedef char dialog_timers[offsetof(UIDialog,timerList)==0x7f24?1:-1];
typedef char control_cursor[offsetof(UIControl,mouseAnim)==0x180?1:-1];
typedef char user_hover[offsetof(GUIUser,hoverControl)==0x1d8?1:-1];
typedef char device_pointer[offsetof(GUIInputDevice,inputDevice)==0xac?1:-1];
typedef char device_position[offsetof(GUIInputDevice,pointerPosition)==0xa4?1:-1];
static unsigned char referenceSpace[0x800000];
struct World {
    unsigned char dialog[sizeof(UIDialog)];
    unsigned char users[2][sizeof(GUIUser)];
    unsigned char controls[3][sizeof(UIControl)];
    unsigned char devices[4][sizeof(GUIInputDevice)];
    unsigned char inputs[4][sizeof(InputDevice)];
    unsigned char timers[4][sizeof(UITimer)];
    unsigned char managers[2][sizeof(GUIManager)];
    UITimer* timerPointers[2][4];
    GUIInputDevice* devicePointers[2][4];
};
static union { double alignment; World value; } space;
static World& world=space.value;
static DWORD dialogVtable[32], controlVtable[65];
static unsigned char cursorTokens[8];
struct Scenario { int seed,count,state,focus,callback; };
static Scenario scenario;
struct Event {
    int operation,receiver,args[8];
    World state;
};
struct Observation { int result,count; Event events[24]; World state; };
static Observation observed, expected;
static int timerMutated;

static UIDialog* Dialog() { return (UIDialog*)world.dialog; }
static GUIUser* User(int i) { return (GUIUser*)world.users[i]; }
static UIControl* Control(int i) { return (UIControl*)world.controls[i]; }
static GUIInputDevice* Device(int i) { return (GUIInputDevice*)world.devices[i]; }
static UITimer* Timer(int i) { return (UITimer*)world.timers[i]; }
static GUIManager* Manager(int i) { return (GUIManager*)world.managers[i]; }
static int UserIndex(const GUIUser* p) {
    if (p==User(0)) return 0; if (p==User(1)) return 1; ExitProcess(81);return 0;
}
static int ControlIndex(const UIControl* p) {
    for (int i=0;i<3;++i) if (p==Control(i)) return i;
    ExitProcess(82);return 0;
}
static Event& Record(int operation,const void* receiver) {
    if (observed.count>=24) ExitProcess(83);
    Event& e=observed.events[observed.count++];
    e.operation=operation;e.receiver=(int)receiver;memcpy(&e.state,&world,sizeof(world));
    return e;
}
static void Mutate(int operation) {
    UIDialog* d=Dialog();
    if (scenario.callback==4) d->guiUser=d->guiUser==User(0)?User(1):User(0);
    if (scenario.callback==5) {
        User(0)->hoverControl=(UnknownGuiControl*)Control(2);
        User(1)->hoverControl=(UnknownGuiControl*)Control(0);
        Control(0)->mouseAnim=(int)(cursorTokens+7);
        world.dialog[0x25]^=4;d->isClosing^=1;
    }
    if (scenario.callback==6) {
        d->lastFrameTime=0.75f;
        d->guiManager=d->guiManager==Manager(0)?Manager(1):Manager(0);
    }
    if ((operation==2 || operation==3) && !timerMutated) {
        timerMutated=1;
        if (scenario.callback==2) { d->timerList.m_count=1;world.timerPointers[0][1]=0; }
        if (scenario.callback==3) { d->timerList.m_data=world.timerPointers[1];d->timerList.m_count=4; }
        if (scenario.callback==7) {
            d->timerList.m_count=3;
            world.timerPointers[0][1]=Timer(3);world.timerPointers[0][2]=Timer(0);
            Timer(3)->targetControl=Control(2);Timer(3)->periodTime=0;
            User(0)->hoverControl=(UnknownGuiControl*)Control(2);
        }
    }
}
struct DirectCall {
    int Tick(float);
    int BaseTick(float time) { return ((GameObject*)this)->GameObject::Tick(time); }
    void EventCall(UnknownDialogEvent* event) {
        if ((void*)this!=Dialog()) ExitProcess(84);
        int timer=event->kind==kDialogTimer;
        if (!timer && event->kind!=kDialogFrame) ExitProcess(85);
        Event& e=Record(timer?2:1,this);
        e.args[0]=event->kind;e.args[1]=event->handled;e.args[2]=(int)event->controlName;
        e.args[3]=(int)event->dialog;e.args[4]=(int)event->gui;e.args[5]=(int)event->control;
        // code is uninitialized for frame events; field_0x18/key are always
        // uninitialized here. Neither their bits nor the event's stack address
        // is observable through this scripted callback.
        if (timer) e.args[6]=event->code;
        if (scenario.callback==1) { event->handled=1;Dialog()->sendFrameEvent=1; }
        Mutate(timer?2:1);
    }
    void TimerCall(int timer) {
        ControlIndex((UIControl*)this);
        Event& e=Record(3,this);e.args[0]=timer;
        bool found=false;for(int i=0;i<4;++i)if(timer==(int)Timer(i))found=true;
        if(!found)ExitProcess(86);
        Mutate(3);
    }
};
template<class T> static DWORD Address(T method) {
    if (sizeof(method)!=4) ExitProcess(87);DWORD value;memcpy(&value,&method,4);return value;
}

void GUIUser::SetCursorAnimation(UnknownCursorAnimation* animation) {
    UserIndex(this);Event& e=Record(4,this);e.args[0]=(int)animation;
    cursorAnimation=animation;Mutate(4);
}
GUIInputDevice* GUIUser::GetAcceptedDevice(int index) {
    UserIndex(this);if(index<0 || index>4)ExitProcess(88);
    GUIInputDevice* result=acceptedDevices.Get(index);
    Event& e=Record(5,this);e.args[0]=index;e.args[1]=(int)result;
    Mutate(5);return result;
}
UIControl* UIControl::PreviousFocusableControl() {
    int n=ControlIndex(this);UIControl* result=(scenario.seed&4)?0:Control((n+2)%3);
    Event& e=Record(6,this);e.args[0]=(int)result;Mutate(6);return result;
}
UIControl* UIControl::NextFocusableControl() {
    int n=ControlIndex(this);UIControl* result=(scenario.seed&8)?0:Control((n+1)%3);
    Event& e=Record(7,this);e.args[0]=(int)result;Mutate(7);return result;
}
int UIDialog::RequestHover(UIControl* control,int a,int b) {
    if(this!=Dialog())ExitProcess(89);ControlIndex(control);
    Event& e=Record(8,this);e.args[0]=(int)control;e.args[1]=a;e.args[2]=b;
    guiUser->hoverControl=(UnknownGuiControl*)control;Mutate(8);return -7;
}
void UIDialog::UpdatePointerHover() {
    if(this!=Dialog())ExitProcess(90);Record(9,this);Mutate(9);
}
int GameObject::Tick(float time) {
    if((void*)this!=Dialog())ExitProcess(91);
    Event& e=Record(10,this);memcpy(&e.args[0],&time,4);Mutate(10);return -9;
}

static DWORD Replacement(DWORD target) {
    switch(target) {
    case 0x534a88:return (DWORD)_ftol;
    case 0x487fb0:return Address(&GUIUser::SetCursorAnimation);
    case 0x488310:return Address(&GUIUser::GetAcceptedDevice);
    case 0x4727c0:return Address(&UIControl::PreviousFocusableControl);
    case 0x472810:return Address(&UIControl::NextFocusableControl);
    case 0x470000:return Address(&UIDialog::RequestHover);
    case 0x46f120:return Address(&UIDialog::UpdatePointerHover);
    case 0x4693d0:return Address(&DirectCall::BaseTick);
    }return 0;
}
static int MapReference(const char* path,const char* patches) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);f=fopen(patches,"rb");if(!f)return 0;DWORD patch[2];int count=0;
    while(fread(patch,sizeof(patch),1,f)==1) {
        DWORD at=patch[0],target=Replacement(patch[1]);
        if(!target || at<0x46ef00 || at+5>0x46f11f || *(BYTE*)at!=0xe8 ||
           at+5+*(DWORD*)(at+1)!=patch[1])return 0;
        *(DWORD*)(at+1)=target-at-5;++count;
    }fclose(f);return count==9 && FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

static void Reset() {
    memset(&world,0x53,sizeof(world));memset(&observed,0,sizeof(observed));timerMutated=0;
    UIDialog* d=Dialog();*(DWORD*)d=(DWORD)dialogVtable;
    world.dialog[0x25]=(scenario.state&1)?4:0;d->isClosing=(scenario.state>>1)&1;
    d->guiUser=User(0);d->guiManager=Manager(0);d->lastFrameTime=scenario.seed%3?0.125f:0.0f;
    d->sendFrameEvent=scenario.seed&1;d->joystickCentred=(scenario.seed>>1)&1;
    d->cursorAnimation=scenario.seed&4?(UnknownCursorAnimation*)(cursorTokens+1):0;
    const unsigned int times[]={0,1,15,1000,0x7fffffff,0x80000000,0xfffffffe,0xffffffff};
    for(int i=0;i<4;++i) {
        UITimer* t=Timer(i);t->timerId=100+i;t->elapsedTime=times[(scenario.seed+i)%8];
        t->periodTime=times[(scenario.seed+2*i)%8];t->targetControl=(scenario.seed+i)&1?Control(i%3):0;
        world.timerPointers[0][i]=t;world.timerPointers[1][i]=Timer((i+2)%4);
        GUIInputDevice* device=Device(i);device->inputDevice=(InputDevice*)world.inputs[i];
        *(int*)(world.inputs[i]+0x0c)=(scenario.seed+i)%3==0?1:2;
        const int directions[9][2]={{-1,0},{1,0},{0,-1},{0,1},{0,0},{-2,2},{2,-2},{1,1},{-1,-1}};
        int which=(scenario.seed+i)%9;device->pointerPosition.x=directions[which][0];
        device->pointerPosition.y=directions[which][1];device->ownerUser=User(i%2);
        world.devicePointers[0][i]=device;world.devicePointers[1][i]=Device(3-i);
    }
    const int counts[]={0,1,3,4};d->timerList.m_count=counts[scenario.count];
    d->timerList.m_data=scenario.seed%8==6?0:world.timerPointers[0];
    if(scenario.seed%8==7)d->timerList.m_count=-1;
    if(scenario.seed&16)world.timerPointers[0][1]=0;
    d->timerList.m_capacity=4;d->timerList.m_growBy=4;d->timerList.m_allocated=0;
    for(int u=0;u<2;++u) {
        GUIUser* user=User(u);user->hoverControl=scenario.focus?(UnknownGuiControl*)Control((scenario.focus-1+u)%3):0;
        user->acceptedDevices.m_count=(scenario.seed+u)%5;
        user->acceptedDevices.m_data=world.devicePointers[u];user->acceptedDevices.m_capacity=4;
        user->acceptedDevices.m_growBy=4;user->acceptedDevices.m_allocated=0;
        if(scenario.seed%8==5)world.devicePointers[u][1]=0;
    }
    for(int c=0;c<3;++c) {
        *(DWORD*)Control(c)=(DWORD)controlVtable;
        Control(c)->mouseAnim=c==0?(int)(cursorTokens+2):0;
    }
}
static void Run(DWORD address,float dt) {
    typedef int(DirectCall::*Fn)(float);Fn fn;memcpy(&fn,&address,4);
    observed.result=(((DirectCall*)Dialog())->*fn)(dt);
    memcpy(&observed.state,&world,sizeof(world));
}
int main(int argc,char** argv) {
    if(argc!=4 || !MapReference(argv[1],argv[2]))return 3;
    DWORD candidate=strtoul(argv[3],0,16);
    memset(dialogVtable,0,sizeof(dialogVtable));memset(controlVtable,0,sizeof(controlVtable));
    dialogVtable[29]=Address(&DirectCall::EventCall);controlVtable[60]=Address(&DirectCall::TimerCall);
    const float steps[]={0.0f,0.0009f,0.001f,0.0166667f,-0.001f,1.25f};int checks=0;
    unsigned int operations=0;int handledTimers=0,previousNull=0,previousFound=0,nextNull=0,nextFound=0;
    for(int seed=0;seed<32;++seed)for(int count=0;count<4;++count)for(int state=0;state<4;++state)
    for(int focus=0;focus<3;++focus)for(int callback=0;callback<8;++callback)for(int dt=0;dt<6;++dt) {
        scenario.seed=seed;scenario.count=count;scenario.state=state;scenario.focus=focus;scenario.callback=callback;
        Reset();Run(0x46ef00,steps[dt]);expected=observed;Reset();Run(candidate,steps[dt]);
        if(memcmp(&expected,&observed,sizeof(observed))) {
            int byte=0;while(byte<sizeof(observed) && ((BYTE*)&expected)[byte]==((BYTE*)&observed)[byte])++byte;
            printf("Mismatch seed%d count%d state%d focus%d callback%d dt%d byte%d events%d/%d\n",
                seed,count,state,focus,callback,dt,byte,expected.count,observed.count);return 2;
        }
        if(observed.result!=1)return 4;
        if(callback==0 && state!=0) {
            // A frame event still precedes the paused/closing gate, but no
            // timers or input dependencies run. Zero lastFrameTime uses .001
            // even when the caller supplies another frame duration.
            float time=seed%3?steps[dt]:0.001f;
            if(observed.count!=(seed&1) || memcmp(&Dialog()->lastFrameTime,&time,4) ||
               Dialog()->sendFrameEvent!=0)return 5;
        }
        for(int event=0;event<observed.count;++event) {
            Event& e=observed.events[event];operations|=1u<<e.operation;
            if(callback==1 && e.operation==2) {
                if(event!=observed.count-1)return 6;
                ++handledTimers;
            }
            if(e.operation==6) {if(e.args[0])++previousFound;else ++previousNull;}
            if(e.operation==7) {if(e.args[0])++nextFound;else ++nextNull;}
        }
        ++checks;
    }
    if(operations!=0x7fe || !handledTimers || !previousNull || !previousFound || !nextNull || !nextFound)return 7;
    printf("UIDialogTick: %d full-state and callback scenarios match\n",checks);return 0;
}
