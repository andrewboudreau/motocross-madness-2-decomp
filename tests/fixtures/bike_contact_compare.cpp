// Actual production CollisionPoint object against private retail helpers.
// Only retail's debug allocation call is redirected; no method is copied or
// replaced in the production object. Constructors/vtables are not conflated
// with the provisional Bike views.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "collision/CollisionPoint.h"
#include "vehicle/Bike.h"
#include "soultree/SoultreePhysicsContact.h"

typedef char ContactSize[sizeof(CollisionPoint) == 0xc4 ? 1 : -1];
typedef char Vectors[sizeof(Vec3) == 12 && sizeof(CollisionVec3) == 12 &&
    offsetof(Vec3,x) == offsetof(CollisionVec3,x) && offsetof(Vec3,y) == offsetof(CollisionVec3,y) &&
    offsetof(Vec3,z) == offsetof(CollisionVec3,z) ? 1 : -1];
typedef char BikeFields[offsetof(BikeA5F8,point) == offsetof(CollisionPoint,worldPosition) &&
    offsetof(BikeA5F8,axis) == offsetof(CollisionPoint,surfaceNormal) &&
    offsetof(BikeA5F8,active) == offsetof(CollisionPoint,inContact) &&
    offsetof(BikeA5F8,state) == offsetof(CollisionPoint,field_0xb4) ? 1 : -1];
typedef char ListView[offsetof(SoultreeContact,ownerNode) == offsetof(CollisionPoint,ownerNode) &&
    offsetof(SoultreeContact,worldPosition) == offsetof(CollisionPoint,worldPosition) &&
    offsetof(SoultreeContact,contactNormal) == offsetof(CollisionPoint,surfaceNormal) &&
    offsetof(SoultreeContact,touching) == offsetof(CollisionPoint,inContact) ? 1 : -1];
typedef char WheelContact[offsetof(BikeWheel,contactPoint_0x0b8) == 0xb8 ? 1 : -1];

// Link-only guards for methods outside this fixture's two exercised bodies.
// Any accidental entry into those dependencies terminates the test.
CollisionVec3::CollisionVec3(float,float,float) { ExitProcess(90); }
CollisionVec3* CollisionDivide(CollisionVec3*,const CollisionVec3*,float) { ExitProcess(91); return 0; }
float CollisionLength(const CollisionVec3*) { ExitProcess(92); return 0; }
float FastSqrt(float) { ExitProcess(93); return 0; }
float FastInvSqrt(float) { ExitProcess(94); return 0; }
float Vec3DotCall(const Vec3*,const Vec3*) { ExitProcess(95); return 0; }
CollisionVec3* Vec3Normalize(CollisionVec3*,const CollisionVec3*) { ExitProcess(96); return 0; }
Vec3 SoultreeObject::LocalToWorldPoint(const Vec3&) { ExitProcess(97); return Vec3(); }
static BYTE referenceSpace[0x800000] = {1};
struct State {
    DWORD allocation[0xc4/4+4];
    CollisionPoint* list[10];
    int count;
    DWORD position[3];
};
static State state, initial;
struct Observation {
    State state;
    CollisionPoint* result;
    unsigned allocations, size, line;
    unsigned exception, access, address, status, control;
};
static Observation observed, expected;
static int allocationMode;
static unsigned checks;
static DWORD productionVtable;
static CollisionPoint* Allocated() { return (CollisionPoint*)&state.allocation[2]; }

void* operator new(unsigned int size, const char* file, int line) {
    if(size != 0xc4 || line != 27 || !strstr(file,"CollisionPoint.cpp")) ExitProcess(80);
    observed.allocations++; observed.size=size; observed.line=line;
    if(allocationMode == 1) return 0;
    if(allocationMode == 2) RaiseException(0xe000c022, 0, 0, 0);
    return Allocated();
}
void operator delete(void*, const char*, int) { ExitProcess(81); }

static int Map(const char* path) {
    BYTE* mapped=(BYTE*)0x400000; DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* f=fopen(path,"rb"); if(!f) return 0; fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    BYTE* raw=(BYTE*)malloc(size); if(!raw || fread(raw,1,size,f)!=(size_t)size) return 0; fclose(f);
    if(size<sizeof(IMAGE_DOS_HEADER) || ((IMAGE_DOS_HEADER*)raw)->e_lfanew<0 ||
       ((IMAGE_DOS_HEADER*)raw)->e_lfanew+sizeof(IMAGE_NT_HEADERS)>(unsigned)size) return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+((IMAGE_DOS_HEADER*)raw)->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 ||
       pe->OptionalHeader.SizeOfHeaders>(unsigned)size) return 0;
    IMAGE_SECTION_HEADER* sections=IMAGE_FIRST_SECTION(pe);
    if((BYTE*)(sections+pe->FileHeader.NumberOfSections)>raw+size) return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++) {
        if(sections[i].PointerToRawData>(unsigned)size || sections[i].SizeOfRawData>(unsigned)size-sections[i].PointerToRawData ||
           sections[i].VirtualAddress>pe->OptionalHeader.SizeOfImage ||
           sections[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-sections[i].VirtualAddress) return 0;
        memcpy(mapped+sections[i].VirtualAddress,raw+sections[i].PointerToRawData,sections[i].SizeOfRawData);
    }
    free(raw);
    // Only this verified allocation boundary is scripted. The local globals
    // have the values established by the production unit's vector initializers.
    BYTE* call=(BYTE*)0x43a350;
    if(*call!=0xe8 || (DWORD)(call+5)+*(DWORD*)(call+1)!=0x4a3010) return 0;
    void* (__cdecl *allocate)(unsigned int,const char*,int)=operator new;
    *(DWORD*)(call+1)=(DWORD)allocate-(DWORD)(call+5);
    memset((void*)0x5797f0,0,12);
    float axis[3]={0,1,0}; memcpy((void*)0x579810,axis,12);
    return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* info) {
    DWORD code=info->ExceptionRecord->ExceptionCode;
    if(code!=EXCEPTION_ACCESS_VIOLATION && code!=0xe000c022) return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=code;
    if(code==EXCEPTION_ACCESS_VIOLATION) {
        observed.access=info->ExceptionRecord->ExceptionInformation[0];
        observed.address=info->ExceptionRecord->ExceptionInformation[1];
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Add(int retail,int capacity,float threshold,float force,int owner,int track) {
    state=initial; memset(&observed,0,sizeof(observed));
    // A null-allocation AV can interrupt the body with live x87 values.
    // Reset stack/tag state as well as flags before each independent run.
    _fpreset(); _control87(_MCW_EM|_RC_NEAR|_PC_64,_MCW_EM|_MCW_RC|_MCW_PC); _clear87();
    typedef CollisionPoint* (__cdecl *Function)(int,CollisionPoint**,const CollisionVec3*,CollisionContactOwner*,float,int*,float,int);
    Function call=retail?(Function)0x43a330:AddCollisionPoint;
    __try {
        observed.result=call(capacity,state.list,(const CollisionVec3*)state.position,
            (CollisionContactOwner*)owner,threshold,&state.count,force,track);
    } __except(Filter(GetExceptionInformation())) {}
    observed.status=_status87(); observed.control=_control87(0,0); observed.state=state;
    if(observed.allocations && !allocationMode) {
        DWORD& vtable=observed.state.allocation[2];
        if(vtable!=(retail?0x5511d8:productionVtable)) ExitProcess(82);
        // The two independently linked vtable addresses are the sole stored
        // address normalized. Their class and complete constructor are proved.
        vtable=0x5511d8;
    }
}
static void CompareAdd(int capacity,float threshold,float force,int owner,int track) {
    Add(1,capacity,threshold,force,owner,track); expected=observed;
    Add(0,capacity,threshold,force,owner,track); checks++;
    if(memcmp(&observed,&expected,sizeof(observed))) {
        printf("Add mismatch %u cap%d count%d mode%d\n",checks,capacity,initial.count,allocationMode);
        for(unsigned i=0;i<sizeof(observed);i++) if(((BYTE*)&observed)[i]!=((BYTE*)&expected)[i])
            printf("offset %x: %02x != %02x\n",i,((BYTE*)&observed)[i],((BYTE*)&expected)[i]);
        fflush(stdout); ExitProcess(2);
    }
    if(initial.count>=capacity) {
        if(observed.result || observed.allocations || observed.exception || memcmp(&state,&initial,sizeof(state))) ExitProcess(83);
    } else if(allocationMode==1) {
        if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.access!=1 || observed.address!=8 ||
           state.count!=initial.count || state.list[initial.count]) ExitProcess(84);
    } else if(allocationMode==2) {
        if(observed.exception!=0xe000c022 || memcmp(&state,&initial,sizeof(state))) ExitProcess(85);
    } else if(observed.result!=Allocated() || state.count!=initial.count+1 || state.list[initial.count]!=Allocated()) ExitProcess(86);
}
static void CompareRemove(CollisionPoint* point) {
    typedef void (__cdecl *Function)(CollisionPoint**,CollisionPoint*,int*);
    State expectedState;
    state=initial; ((Function)0x43a5e0)(state.list,point,&state.count); expectedState=state;
    state=initial; RemoveCollisionPoint(state.list,point,&state.count); checks++;
    if(memcmp(&state,&expectedState,sizeof(state))) {printf("Remove mismatch %u\n",checks);fflush(stdout);ExitProcess(3);}
}
int main(int argc,char** argv) {
    if(argc!=2 || !Map(argv[1])) return 4;
    // Real CollisionPoint.cpp CRT initializers supply these shared objects.
    const CollisionVec3* zero=reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero);
    const CollisionVec3* axis=reinterpret_cast<const CollisionVec3*>(&kCollisionPointYAxis);
    if(zero->x!=0 || zero->y!=0 || zero->z!=0 || axis->x!=0 || axis->y!=1 || axis->z!=0) return 78;
    memset(&state,0xa5,sizeof(state)); state.count=0; allocationMode=0;
    CollisionPoint* p=AddCollisionPoint(1,state.list,zero,0,0,&state.count,0.6f,0x12345678);
    productionVtable=*(DWORD*)p;
    if(p!=Allocated() || state.list[0]!=p || state.count!=1 || p->surfaceOwner!=0x12345678 ||
       p->normalForce!=0.6f || p->penetration!=-999 || p->inContact || p->surfaceNormal.y!=1) return 5;
    // First occurrence only; deletion removes an embedded pointer, never frees it.
    state.list[0]=p; state.list[1]=p; state.list[2]=(CollisionPoint*)0x12340000; state.count=3;
    RemoveCollisionPoint(state.list,p,&state.count);
    if(state.count!=2 || state.list[0]!=p || state.list[1]!=(CollisionPoint*)0x12340000 || state.list[2]) return 6;
    const unsigned bits[]={0,0x80000000,0x3f19999a,0xbe99999a,0x3f800000,0xbf800000,
        0x00000001,0x7f7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345};
    for(unsigned seed=0;seed<64;seed++) {
        memset(&initial,0xa5,sizeof(initial));
        for(int i=0;i<10;i++) initial.list[i]=(seed+i)%5 ? (CollisionPoint*)(0x12340000+4*((seed+i)%4)) : 0;
        for(i=0;i<3;i++) initial.position[i]=bits[(seed+i)%12];
        float threshold,force; memcpy(&threshold,&bits[seed%12],4); memcpy(&force,&bits[(seed+4)%12],4);
        for(int count=0;count<6;count++) for(int capacity=-1;capacity<7;capacity++) for(allocationMode=0;allocationMode<3;allocationMode++) {
            initial.count=count; CompareAdd(capacity,threshold,force,seed&1?0x87654321:0,0xfedc0000+seed);
        }
        for(count=0;count<10;count++) for(int target=0;target<6;target++) {
            initial.count=count;
            CompareRemove(target==5?0:(CollisionPoint*)(0x12340000+4*target));
        }
    }
    printf("Bike contacts: %u allocation, capacity, layout, removal and failure cases match\n",checks);
    return 0;
}
