#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "../../src/krusty2/vehicle/BikeHandlebarView.h"
#include "../../src/krusty2/vehicle/VehicleSpeedState.h"
#include "../../src/krusty2/vehicle/BikeRaceHandlerView.h"
typedef char Layout[(sizeof(BikeA60C)==0x4c && offsetof(BikeA60C,node)==0x40 &&
    offsetof(BikeA60C,owner)==0x44 && sizeof(VehicleSpeedState)==0x1e4 &&
    offsetof(VehicleSpeedState,gearTimer)==0xc && sizeof(KbRaceHandler)==1 &&
    sizeof(void(KbRaceHandler::*)(void*,float,int))==4)?1:-1];
static unsigned char before[4096];
static int Invoke(int kind,void* receiver,void* who,float value,int code) {
    __try {
        if(kind==0)((BikeA60C*)receiver)->~BikeA60C();
        else if(kind==1)((VehicleSpeedState*)receiver)->~VehicleSpeedState();
        else ((KbRaceHandler*)receiver)->Fn_004DE580(who,value,code);
    } __except(EXCEPTION_EXECUTE_HANDLER) { return 0; }
    return 1;
}
int main() {
    BYTE* page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    DWORD old;if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 1;
    void* receivers[4]={page+16,page+23,page+4096,0};
    int count=0;
    for(int seed=0;seed<256;++seed) {
        for(unsigned i=0;i<4096;++i)page[i]=(BYTE)(seed+37*i);
        memcpy(before,page,4096);
        for(int kind=0;kind<3;++kind)for(int r=0;r<4;++r) {
            // Both pointer arguments may be inaccessible; the full bodies
            // never dereference them. This is an x86 ABI probe, not lifetime.
            if(!Invoke(kind,receivers[r],page+4096,(seed-128)*0.25f,seed*0x10001))return 2;
            if(memcmp(page,before,4096))return 3;
            ++count;
        }
    }
    if(count!=3072)return 4;
    printf("Gameplay defaults: 3072 unchanged-page and inaccessible-pointer calls\n");
    return 0;
}
