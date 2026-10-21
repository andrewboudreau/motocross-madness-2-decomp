// Actual extracted strict Scene restart and Character motion providers.
// Raw prepared objects; only virtual pose application / procedural Tick are scripted.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stddef.h>
#include "SceneManager.h"
#include "motion/Motnctrl.h"
extern "C" long __cdecl _ftol();
static BYTE referenceSpace[0x800000] = {1};
typedef char SceneLayout[(sizeof(Scene)==0x8cc && offsetof(Scene,animationTable)==0xb4)?1:-1];
typedef char EntryLayout[(sizeof(UnknownSceneEntry)==0x44 && offsetof(UnknownSceneEntry,character)==4 &&
    offsetof(UnknownSceneEntry,motions)==0x20 && offsetof(UnknownSceneEntry,motionFrame)==0x18)?1:-1];
typedef char MotionLayout[(sizeof(Motion)==0xcc && offsetof(Motion,frames)==0xa4 &&
    offsetof(Motion,frameCount)==0xa8 && sizeof(MotionPoseList)==12)?1:-1];
typedef char CharacterLayout[(offsetof(Character,motionStopped)==0xc &&
    offsetof(Character,chr_field_0x10)==0x10 && offsetof(Character,currentMotion)==0x18)?1:-1];

struct World {
    BYTE scene[0x8cc+32], actor[0x294+32], car[0x22c+32];
    UnknownSceneTable table;
    UnknownSceneEntry entries[3];
    Motion motions[3];
    void* motionPointers[3][3];
    char sequences[3][3];
    MotionPoseList frames[3][5];
    CharacterPose poses[3][5][3];
};
static World state, initial;
struct Event { int kind, mask; DWORD receiver, pointer, time; World state; };
struct Result {
    int result, count; DWORD exception, kind, address, fp, control, faultFp;
    World state; Event events[4];
};
static Result observed, expected;
static DWORD characterTable[11], carTable[27];
static int checks, callbackMode;
static Scene* SceneObject() { return (Scene*)(state.scene+16); }
static Character* Actor() { return (Character*)(state.actor+16); }
static UnknownSceneAnimatedObject* Car() { return (UnknownSceneAnimatedObject*)(state.car+16); }

struct Boundary {
    int Restart(int,float,int,int);
    void Apply(int pose,int mask) {
        if ((void*)this!=Actor() || observed.count>=4 || mask!=0) ExitProcess(70);
        DWORD offset=(DWORD)pose-(DWORD)&state.poses;
        if (offset>sizeof(state.poses)-sizeof(CharacterPose) || offset%sizeof(CharacterPose)) ExitProcess(71);
        Event& event=observed.events[observed.count++];
        event.kind=4;event.mask=mask;event.receiver=(DWORD)this;event.pointer=pose;
        memcpy(&event.state,&state,sizeof(state));
        // Safe, bounded callback effects: preserve frame/count/pointer topology.
        if(callbackMode==1)Actor()->chr_field_0x38=observed.count;
        if(callbackMode==2)RaiseException(0xe0410001,0,0,0);
    }
    int Tick(float time) {
        if ((void*)this!=Car() || observed.count>=4)ExitProcess(72);
        Event& event=observed.events[observed.count++];
        event.kind=10;event.receiver=(DWORD)this;memcpy(&event.time,&time,4);
        memcpy(&event.state,&state,sizeof(state));
        if(callbackMode==1)Car()->skipSmoothing=7;
        if(callbackMode==2)RaiseException(0xe0410001,0,0,0);
        return 123;
    }
};
template<class T> static DWORD Address(T value) {
    if(sizeof(value)!=4)ExitProcess(73);DWORD result;memcpy(&result,&value,4);return result;
}
static void Setup(int seed,int index,int sequenceLength,int scenario) {
    memset(&state,(seed*37+19)&255,sizeof(state));
    Scene* scene=SceneObject();Character* actor=Actor();
    scene->animationTable=scenario==0?0:&state.table;
    state.table.entries=state.entries;state.table.entryCount=3;
    *(DWORD*)actor=(DWORD)characterTable;*(DWORD*)Car()=(DWORD)carTable;
    actor->motionStopped=scenario==2;actor->blendActive=1;
    actor->chr_field_0x10=123.0f;actor->chr_field_0x38=0;
    actor->lastFrame=&state.frames[0][0];actor->lastFrameTime=-1;
    actor->currentMotion=&state.motions[0];actor->currentFrame=&state.frames[0][0];
    for(int entry=0;entry<3;entry++) {
        UnknownSceneEntry& e=state.entries[entry];
        e.keyFramed=scenario!=3;e.character=scenario==4?0:(UnknownSceneObject*)actor;
        e.proceduralObject=Car();e.motions=state.motionPointers[entry];e.motionCount=3;
        e.motionSequence=state.sequences[entry];e.sequenceLength=(signed char)sequenceLength;
        e.sequenceIndex=0;e.currentMotion=0;e.motionFrame=-9;
        for(int m=0;m<3;m++) {
            state.motionPointers[entry][m]=scenario==5?0:&state.motions[m];
            state.sequences[entry][m]=(char)((m+entry)%3+1);
        }
    }
    for(int m=0;m<3;m++) {
        Motion& motion=state.motions[m];motion.frames=state.frames[m];
        motion.frameCount=(seed%3)*2+1;motion.posesPerFrame=seed%4;
        motion.fps=(seed%3+1)*5;motion.frameTime=1.0f/motion.fps;motion.looping=seed&1;
        for(int f=0;f<5;f++) {
            MotionPoseList& frame=state.frames[m][f];frame.frameIndex=f;
            frame.poses=state.poses[m][f];frame.count=motion.posesPerFrame;
            for(int p=0;p<3;p++) {
                CharacterPose& pose=state.poses[m][f][p];pose.nodeIndex=(unsigned char)p;
                pose.hasPose=1;pose.axisZ=Vec3(0,0,1);pose.axisY=Vec3(0,1,0);
                pose.position=Vec3((float)m,(float)f,(float)p);
            }
        }
    }
    memcpy(&initial,&state,sizeof(state));
}
static int Filter(EXCEPTION_POINTERS* info) {
    DWORD code=info->ExceptionRecord->ExceptionCode;
    if(code!=EXCEPTION_ACCESS_VIOLATION && code!=0xe0410001)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=code;
    if(code==EXCEPTION_ACCESS_VIOLATION) {
        observed.kind=info->ExceptionRecord->ExceptionInformation[0];
        observed.address=info->ExceptionRecord->ExceptionInformation[1];
    }
    observed.faultFp=info->ContextRecord->FloatSave.StatusWord&63;
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int reference,int index,float time,int force,int motion,unsigned rounding,unsigned precision) {
    memcpy(&state,&initial,sizeof(state));memset(&observed,0,sizeof(observed));observed.result=-99;
    _fpreset();_controlfp(rounding|precision,_MCW_RC|_MCW_PC);
    __try {
        if(reference) {
            int(Boundary::*method)(int,float,int,int);DWORD address=0x4eb040;
            memcpy(&method,&address,4);
            observed.result=(((Boundary*)SceneObject())->*method)(index,time,force,motion);
        } else observed.result=SceneObject()->RestartEntry(index,time,force,motion);
    } __except(Filter(GetExceptionInformation())) {}
    observed.fp=_statusfp();observed.control=_controlfp(0,0);
    memcpy(&observed.state,&state,sizeof(state));_fpreset();
}
static int Check(int index,float time,int force,int motion,unsigned rounding,unsigned precision) {
    Run(1,index,time,force,motion,rounding,precision);memcpy(&expected,&observed,sizeof(expected));
    Run(0,index,time,force,motion,rounding,precision);
    if(memcmp(&observed,&expected,sizeof(expected))) {
        BYTE* a=(BYTE*)&observed;BYTE* b=(BYTE*)&expected;unsigned i=0;
        while(i<sizeof(expected)&&a[i]==b[i])i++;
        printf("difference case=%d byte=%u result=%d/%d exception=%08x/%08x\n",checks,i,
            observed.result,expected.result,observed.exception,expected.exception);return 0;
    }
    ++checks;return 1;
}
static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++) {
        if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);
}

int main(int argc,char** argv) {
    if(argc!=2 || !MapReference(argv[1]))return 1;
    // Only the reference CRT conversion boundary is redirected. The four game
    // bodies and their direct calls remain the actual mapped retail code.
    BYTE* call=(BYTE*)0x4a56ed;
    if(call[0]!=0xe8 || 0x4a56f2+*(LONG*)(call+1)!=0x534a88)return 2;
    *(DWORD*)(call+1)=(DWORD)&_ftol-0x4a56f2;
    if(!FlushInstructionCache(GetCurrentProcess(),0,0))return 3;
    memset(characterTable,0,sizeof(characterTable));memset(carTable,0,sizeof(carTable));
    characterTable[4]=Address(&Boundary::Apply);carTable[10]=Address(&Boundary::Tick);
    const float times[]={-1.0f,0.0f,0.0001f,0.05f,0.25f,4.0f};
    const unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
    const unsigned pc[]={_PC_64,_PC_53,_PC_24};
    for(int seed=0;seed<8;seed++)for(int index=0;index<3;index++)
    for(int sequence=1;sequence<=3;sequence++)for(int scenario=0;scenario<6;scenario++)
    for(int force=0;force<2;force++)for(int motion=0;motion<4;motion++)
    for(int t=0;t<6;t++)for(callbackMode=0;callbackMode<3;callbackMode++) {
        Setup(seed,index,sequence,scenario);
        if(!Check(index,times[t],force,motion,rc[seed%4],pc[seed%3]))return 4;
    }
    // Independent expected outcomes supplement the differential comparison.
    callbackMode=0;Setup(1,0,3,0);
    if(!Check(0,0.25f,1,0,_RC_NEAR,_PC_64) || observed.result || observed.count ||
       observed.exception || memcmp(&state,&initial,sizeof(state)))return 10;
    Setup(1,0,3,2);
    if(!Check(0,0.05f,1,0,_RC_NEAR,_PC_64) || observed.result!=1 || observed.count!=1 ||
       observed.exception || Actor()->currentMotion!=&state.motions[0] ||
       Actor()->currentFrame!=&state.frames[0][0] || Actor()->blendActive ||
       Actor()->chr_field_0x10!=0.05f || state.entries[0].motionFrame!=0)return 11;
    Setup(0,0,3,2);
    if(!Check(0,4,1,0,_RC_NEAR,_PC_64) || observed.result!=1 || observed.count ||
       observed.exception || Actor()->motionStopped!=1 || Actor()->chr_field_0x10!=0 ||
       state.entries[0].motionFrame!=0)return 12;
    Setup(1,0,3,3);
    if(!Check(0,0,1,0,_RC_NEAR,_PC_64) || observed.result || observed.count ||
       observed.exception || Car()->skipSmoothing || Car()->pathTime!=0.0001f)return 13;
    callbackMode=1;Setup(1,0,3,3);
    if(!Check(0,0.25f,1,0,_RC_NEAR,_PC_64) || observed.result || observed.count!=1 ||
       observed.exception || Car()->skipSmoothing!=7 || Car()->pathTime!=0.25f)return 14;
    callbackMode=0;Setup(1,0,3,5);
    if(!Check(0,0.05f,1,0,_RC_NEAR,_PC_64) || observed.exception!=EXCEPTION_ACCESS_VIOLATION ||
       observed.kind || observed.address!=0xa4 || Actor()->currentMotion || Actor()->motionStopped ||
       Actor()->blendActive!=1 || Actor()->currentFrame!=&state.frames[0][0])return 15;
    Setup(1,1,2,2);
    if(!Check(1,0.05f,1,3,_RC_NEAR,_PC_64) || observed.exception || observed.result!=1 ||
       Actor()->currentMotion!=&state.motions[1] || state.entries[1].sequenceIndex!=0)return 16;
    printf("Scene motion: %d state, frame, callback and fault cases passed\n",checks);
    return 0;
}
