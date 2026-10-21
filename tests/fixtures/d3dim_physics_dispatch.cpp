// Validation-only dispatch probes. These do not implement any game provider.
#if defined(PHYSICS_VIEW) || defined(RENDER_VIEW)
#ifdef PHYSICS_VIEW
#include "soultree/SoultreePhysicsObject.h"
#define DISPATCH PhysicsDispatch
#else
#include "D3DIMSoulTree.h"
#define DISPATCH RenderDispatch
#endif
extern "C" void DISPATCH(void* owner, int slot, void* first, void* second)
{
#ifdef PHYSICS_VIEW
    D3DIMSoultreeObject* object = static_cast<SoultreePhysicsObject*>(owner);
#else
    D3DIMSoultreeObject* object = static_cast<D3DIMSoultreeObject*>(owner);
#endif
    switch (slot) {
    case 2: object->LoadHierarchy(static_cast<UnknownTextureStream*>(first)); break;
    case 3: object->LoadHierarchyFromParameters(); break;
    case 4: object->SoultreeVirtualSlot4(static_cast<SoultreeObject**>(first)); break;
    case 5: object->SoultreeVirtualSlot5(); break;
    case 6: object->ResetLocalMatrices(); break;
    case 7: object->CloneFrom(static_cast<SoultreeObject*>(first)); break;
    case 8: object->CopySubtree(static_cast<SoultreeObject*>(first),static_cast<SoultreeObject*>(second)); break;
    }
}
#else
#include <windows.h>
#include <stdio.h>
#include <string.h>
extern "C" void PhysicsDispatch(void*, int, void*, void*);
extern "C" void RenderDispatch(void*, int, void*, void*);
struct Observation { void* receiver; void* first; void* second; int slot, calls; };
static Observation observed;
struct Probe {
    void Capture(int slot, void* first=0, void* second=0) {
        observed.receiver=this; observed.slot=slot; observed.first=first;
        observed.second=second; ++observed.calls;
    }
    void Slot2(void* first) { Capture(2,first); }
    void Slot3() { Capture(3); }
    void Slot4(void* first) { Capture(4,first); }
    void Slot5() { Capture(5); }
    void Slot6() { Capture(6); }
    void Slot7(void* first) { Capture(7,first); }
    void Slot8(void* first, void* second) { Capture(8,first,second); }
};
template<class T> static DWORD Address(T member) {
    DWORD address=0;
    if (sizeof(member)!=sizeof(address)) ExitProcess(10);
    memcpy(&address,&member,sizeof(address));
    return address;
}
int main() {
    DWORD table[10]={0,0,Address(&Probe::Slot2),Address(&Probe::Slot3),
        Address(&Probe::Slot4),Address(&Probe::Slot5),Address(&Probe::Slot6),
        Address(&Probe::Slot7),Address(&Probe::Slot8),0};
    DWORD values[]={0,1,0x12345678,0x7fffffff,0x80000000,0xfffffff8,0xffffffff,0x00400000};
    DWORD storage[0x600/4],before[0x600/4];
    int cases=0;
    for (int which=0;which<2;++which)
    for (int slot=2;slot<=8;++slot)
    for (int a=0;a<8;++a)
    for (int b=0;b<8;++b) {
        memset(storage,0xa5,sizeof(storage));
        void* primary=(BYTE*)storage+0x21c;
        *(DWORD**)primary=table;
        memcpy(before,storage,sizeof(before));
        memset(&observed,0,sizeof(observed));
        if (which) RenderDispatch(primary,slot,(void*)values[a],(void*)values[b]);
        else PhysicsDispatch(storage,slot,(void*)values[a],(void*)values[b]);
        void* expectedFirst=(slot==2||slot==4||slot==7||slot==8)?(void*)values[a]:0;
        void* expectedSecond=slot==8?(void*)values[b]:0;
        if (observed.receiver!=primary || observed.first!=expectedFirst ||
            observed.second!=expectedSecond || observed.slot!=slot || observed.calls!=1 ||
            memcmp(storage,before,sizeof(storage))) return 2;
        ++cases;
    }
    printf("%d cross-view dispatch cases preserve primary receiver and argument bits\n",cases);
    return 0;
}
#endif
