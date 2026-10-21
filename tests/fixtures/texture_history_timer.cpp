// Validation only: actual timer constructor and actual ReadClock/WinMM backend.
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <new>
#include "../../src/reconstructed/TextureMapManager.h"
typedef char Layout[(sizeof(ManagedTextureGroup)==0x258 && sizeof(UnknownTimer)==12 && offsetof(UnknownTimer,timerStart)==0 &&
    offsetof(UnknownTimer,field_0x04)==4 && offsetof(UnknownTimer,timerInterval)==8 &&
    offsetof(ManagedTextureGroup,texelsBlittedHistory)==0x1d4 &&
    offsetof(ManagedTextureGroup,blitTimeHistory)==0x1e0 &&
    offsetof(ManagedTextureGroup,texturesBlittedHistory)==0x1ec)?1:-1];
static BYTE* page;
static BYTE before[4096],expected[4096];
static DWORD complete,code,address,access,returned;
static int Filter(EXCEPTION_POINTERS* e) {
    code=e->ExceptionRecord->ExceptionCode;
    address=access=0;
    if(code==EXCEPTION_ACCESS_VIOLATION) {
        address=e->ExceptionRecord->ExceptionInformation[1];
        access=e->ExceptionRecord->ExceptionInformation[0];
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
static void* Construct(void* target,int interval) {
    return new(target) UnknownTimer(interval);
}
static void Run(void* target,int interval) {
    complete=code=address=access=returned=0;
    __try { returned=(DWORD)Construct(target,interval);complete=1; }
    __except(Filter(GetExceptionInformation())) {}
}
static int cases,faults;
static void Check(int offset,DWORD value,int mode) {
    memcpy(before,page,4096);memcpy(expected,page,4096);
    int interval;memcpy(&interval,&value,4);
    DWORD start=timeGetTime();
    void* receiver=mode==4?(void*)1:(void*)(page+offset);
    Run(receiver,interval);
    DWORD finish=timeGetTime(),stamp=0;
    if(mode==0 || mode==2) {
        memcpy(&stamp,page+offset,4);
        if(stamp-start>finish-start)ExitProcess(10);
        memcpy(expected+offset,&stamp,4);
        memset(expected+offset+4,0,4);
    }
    if(mode==0) {
        memcpy(expected+offset+8,&value,4);
        if(!complete||code||returned!=(DWORD)receiver)ExitProcess(11);
    } else {
        DWORD wanted=mode==4?5:(DWORD)(page+offset+(mode==2?8:4));
        if(complete||returned||code!=EXCEPTION_ACCESS_VIOLATION||access!=1||address!=wanted)ExitProcess(12);
        ++faults;
    }
    if(memcmp(page,expected,4096))ExitProcess(13);
    ++cases;
}
// Controlled platform-only boundary: the actual strict ReadClock body remains intact.
static int throwStage,phase,scriptCalls,scriptThrows;
static DWORD scriptTime,oldTime,oldInterval;
static UnknownTimer* active;
static void Boundary(int stage) {
    if(stage!=phase+1 || active->field_0x04!=0 ||
       (DWORD)active->timerStart!=oldTime || (DWORD)active->timerInterval!=oldInterval)ExitProcess(20);
    phase=stage;
    if(throwStage==stage)RaiseException(0xe1234567,0,0,0);
}
static MMRESULT WINAPI Begin(UINT resolution) { if(resolution!=1)ExitProcess(21);Boundary(1);return 1; }
static DWORD WINAPI Time() { Boundary(2);return scriptTime; }
static MMRESULT WINAPI End(UINT resolution) { if(resolution!=1)ExitProcess(22);Boundary(3);return 1; }
static DWORD* ImportSlot(const char* wanted) {
    BYTE* base=(BYTE*)GetModuleHandle(0);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* nt=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);
    IMAGE_IMPORT_DESCRIPTOR* imp=(IMAGE_IMPORT_DESCRIPTOR*)(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for(;imp->Name;++imp) {
        IMAGE_THUNK_DATA* names=(IMAGE_THUNK_DATA*)(base+imp->OriginalFirstThunk);
        IMAGE_THUNK_DATA* slots=(IMAGE_THUNK_DATA*)(base+imp->FirstThunk);
        for(;names->u1.AddressOfData;++names,++slots) {
            if(!(names->u1.Ordinal&IMAGE_ORDINAL_FLAG)) {
                IMAGE_IMPORT_BY_NAME* name=(IMAGE_IMPORT_BY_NAME*)(base+(DWORD)names->u1.AddressOfData);
                if(!strcmp((char*)name->Name,wanted))return (DWORD*)&slots->u1.Function;
            }
        }
    }
    return 0;
}
static void Scripted() {
    const char* names[]={"timeBeginPeriod","timeGetTime","timeEndPeriod"};
    DWORD replacement[]={(DWORD)Begin,(DWORD)Time,(DWORD)End},saved[3],protect[3];
    DWORD* slots[3];
    for(int i=0;i<3;++i) {
        slots[i]=ImportSlot(names[i]);if(!slots[i])ExitProcess(23);
        saved[i]=*slots[i];if(!VirtualProtect(slots[i],4,PAGE_READWRITE,&protect[i]))ExitProcess(24);
        *slots[i]=replacement[i];
    }
    for(int seed=0;seed<32;++seed)for(throwStage=0;throwStage<4;++throwStage) {
        for(int n=0;n<4096;++n)page[n]=(BYTE)(seed+37*n);
        memcpy(expected,page,4096);active=(UnknownTimer*)(page+32);
        oldTime=(DWORD)active->timerStart;oldInterval=(DWORD)active->timerInterval;
        scriptTime=0xfffffff0u+seed;phase=0;
        DWORD interval=0x80000000u+seed;int arg;memcpy(&arg,&interval,4);
        Run(active,arg);memset(expected+36,0,4);
        if(throwStage) {
            if(complete||returned||code!=0xe1234567||phase!=throwStage)ExitProcess(25);
            ++scriptThrows;
        } else {
            if(!complete||code||returned!=(DWORD)active||phase!=3)ExitProcess(26);
            memcpy(expected+32,&scriptTime,4);memcpy(expected+40,&interval,4);
        }
        if(memcmp(page,expected,4096))ExitProcess(27);
        ++scriptCalls;
    }
    for(i=0;i<3;++i)*slots[i]=saved[i];
    for(i=2;i>=0;--i) { DWORD ignored;VirtualProtect(slots[i],4,protect[i],&ignored); }
}
int main() {
    DWORD old;
    page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 1;
    DWORD values[]={0,1,5000,0x7fffffff,0x80000000,0xffffffff,0x55555555,0xaaaaaaaa};
    for(int seed=0;seed<32;++seed)for(int v=0;v<8;++v) {
        for(int i=0;i<4096;++i)page[i]=(BYTE)(seed+i*37);
        for(int o=16;o<24;++o)Check(o,values[v],0);
        Check(4092,values[v],1);
        Check(4088,values[v],2);
        Check(4096,values[v],3);
        Check(0,values[v],4);
    }
    Scripted();
    if(cases!=3072||faults!=1024||scriptCalls!=128||scriptThrows!=96)return 2;
    printf("Texture timer: %d actual clock cases, %d exact write AV; %d platform boundary cases, %d throws\n",cases,faults,scriptCalls,scriptThrows);
    return 0;
}
