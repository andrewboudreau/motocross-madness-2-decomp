// Validation only: actual consumer/provider objects, prepared raw storage.
// No constructors/lifetime or complete Tire identity are claimed by this test.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "soultree/SoultreePhysicsBaseObjectCore.h"
#include "soultree/SoultreePhysicsContact.h"
#include "collision/CollisionPoint.h"
#ifdef CONTACT_LAYOUT
#include "../../samples/physics/tire/Tire.h"
extern "C" int TireContactOffset() {
    Tire* tire=(Tire*)0x10000;
    return (char*)static_cast<CollisionPoint*>(tire)-(char*)tire;
}
#else
#include "core/SoultreeObject.h"
void InitFastSqrtTable();
void InitFastInvSqrtTable();
extern "C" int TireContactOffset();
void UpdateCollisionPointWorldPositions(int,CollisionPoint**);
#define SAME_FIELD(a,b) typedef char Layout_##a[offsetof(SoultreeContact,a)==offsetof(CollisionPoint,b)?1:-1]
SAME_FIELD(ownerNode,ownerNode); SAME_FIELD(worldPosition,worldPosition);
SAME_FIELD(contactNormal,surfaceNormal); SAME_FIELD(leverArm,relativePosition);
SAME_FIELD(frictionForce,frictionForce); SAME_FIELD(field_0x98,penetration);
SAME_FIELD(touching,inContact); SAME_FIELD(contactState,field_0xb4); SAME_FIELD(materialId,surfaceType);
typedef char VectorLayout[sizeof(Vec3)==12&&sizeof(CollisionVec3)==12&&offsetof(Vec3,x)==0&&offsetof(Vec3,y)==4&&offsetof(Vec3,z)==8&&offsetof(CollisionVec3,x)==0&&offsetof(CollisionVec3,y)==4&&offsetof(CollisionVec3,z)==8?1:-1];
typedef char BodyLayout[offsetof(SoultreePhysicsBaseObject,collisionPoints)==0x12c&&offsetof(SoultreePhysicsBaseObject,collisionPointCount)==0x130?1:-1];
static BYTE referenceSpace[0x800000]={1};
struct PointStorage { DWORD before[4]; BYTE object[0x2c0]; DWORD after[4]; };
struct NodeStorage { DWORD before[4]; BYTE object[0x1a4]; DWORD after[4]; };
struct World { DWORD before[4]; BYTE body[0x250]; DWORD after[4]; PointStorage points[6]; NodeStorage nodes[3]; CollisionPoint* list[8]; CollisionVec3 args[4]; };
struct Observation { World world; BYTE page[4096]; DWORD exception,kind,address,fp,control,faultFp; };
struct Scenario { int task,seed,count,mode,duplicate,alias,fault,round,precision; };
static World world;
static Observation observed,expected;
static Scenario scenario;
static BYTE* guardPage;
static int checks;
static CollisionPoint* Point(int n) { return (CollisionPoint*)(world.points[n].object+((n&1)?0xb8:0)); }
static SoultreeObject* Node(int n) { return (SoultreeObject*)world.nodes[n].object; }
static SoultreePhysicsBaseObject* Body() { return (SoultreePhysicsBaseObject*)world.body; }
static float Bits(DWORD bits) { float value;memcpy(&value,&bits,4);return value; }
static void Vector(CollisionVec3& v,float a,float b,float c) { v.x=a;v.y=b;v.z=c; }
static void Matrix(Matrix4& m,int seed) {
    memset(&m,0,sizeof(m));
    for(int row=0;row<3;row++)m.m[row][(row+seed%3)%3]=(seed&(1<<row))?-1.0f:1.0f;
    m._41=(float)(seed%7-3);m._42=(float)(seed%5-2);m._43=(float)(seed%3-1);m._44=1;
}
// Unreachable allocation is a fail-fast boundary, not a simulated math provider.
void* operator new(unsigned int,const char*,int) { ExitProcess(90);return 0; }
void operator delete(void*,const char*,int) { ExitProcess(91); }
static int MapReference(const char* path,const char* patches) {
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
    free(raw);f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];
    while(fread(entry,sizeof(entry),1,f)==1) {
        BYTE* call=(BYTE*)entry[0];
        if(*call!=0xe8||(DWORD)(call+5)+*(DWORD*)(call+1)!=entry[1])return 0;
        *(DWORD*)(call+1)=entry[2]-(DWORD)(call+5);
    }
    fclose(f);return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=e->ExceptionRecord->ExceptionCode;
    observed.kind=e->ExceptionRecord->ExceptionInformation[0];observed.address=e->ExceptionRecord->ExceptionInformation[1];
    observed.faultFp=e->ContextRecord->FloatSave.StatusWord&0x3f;return EXCEPTION_EXECUTE_HANDLER;
}
class RetailCall { public:void Slot31();void Relative(const CollisionVec3*,const CollisionVec3*,const CollisionVec3*,const CollisionVec3*,float); };
static void Run(int retail) {
    memset(&observed,0,sizeof(observed));
    for(int i=0;i<sizeof(world);i++)((BYTE*)&world)[i]=(BYTE)(i*13+scenario.seed*17);
    for(i=0;i<4096;i++)guardPage[i]=(BYTE)(i*7+scenario.seed);
    for(i=0;i<3;i++) {
        SoultreeObject* n=Node(i);Matrix(n->localMatrix,scenario.seed+i);Matrix(n->worldMatrix,scenario.seed+i+5);
        n->worldValid=(scenario.mode>>i)&1;n->parent=i&&scenario.mode&8?Node(i-1):0;
    }
    static const float values[]={0,1,-1,.001f,.001001f,-.25f,3,17};
    SoultreePhysicsBaseObject* body=Body();
    body->collisionPoints=(SoultreeContact**)world.list;body->collisionPointCount=scenario.count;
    body->centerOfMass.x=values[scenario.seed%8];body->centerOfMass.y=2;body->centerOfMass.z=-1;
    body->velocity.x=values[(scenario.seed+1)%8];body->velocity.y=values[(scenario.seed+2)%8];body->velocity.z=values[(scenario.seed+3)%8];
    body->worldAngularVelocity.x=values[(scenario.seed+4)%8];body->worldAngularVelocity.y=1;body->worldAngularVelocity.z=-.5f;
    body->linearSpeed=values[(scenario.seed+5)%8];
    for(i=0;i<6;i++) {
        CollisionPoint* p=Point(i);p->ownerNode=((scenario.mode+i)%4==3)?0:(CollisionContactOwner*)Node(i%3);
        Vector(p->localPosition,(float)(i-2),(float)(scenario.seed%7-3),1);
        Vector(p->worldPosition,(float)(i+1),(float)(i-3),(float)(scenario.seed%5));
        Vector(p->surfaceNormal,values[(scenario.seed+i)%8],1,values[(scenario.seed+i+2)%8]);
        Vector(p->relativePosition,3,4,5);
        p->penetration=((scenario.mode+i)%3==0)?-1:0;
        if(scenario.mode==15)p->penetration=Bits(0x7fc12345);
        p->inContact=(scenario.mode+i)%3;*(int*)&p->field_0xb4=(scenario.mode+i)%4;
        world.list[i]=p;
    }
    world.list[6]=world.list[7]=0;
    if(scenario.duplicate){world.list[2]=world.list[0];world.list[5]=world.list[1];}
    for(i=0;i<4;i++)Vector(world.args[i],values[(scenario.seed+i)%8],values[(scenario.seed+i+1)%8],values[(scenario.seed+i+2)%8]);
    const CollisionVec3* args[4]={world.args,world.args+1,world.args+2,world.args+3};
    CollisionPoint* point=Point(scenario.seed%6);
    if(scenario.alias) {
        static const int offsets[]={0x14,0x2c,0x38,0x50,0x78,0x90};
        for(i=0;i<4;i++)args[i]=(CollisionVec3*)((BYTE*)point+offsets[(scenario.alias+i)%6]);
        if(scenario.alias==7)args[0]=args[1]=args[2]=args[3]=&point->relativePosition;
    }
    CollisionPoint** list=world.list;
    if(scenario.fault==1)body=0;
    if(scenario.fault==2){body->collisionPoints=0;list=0;}
    if(scenario.fault==3){world.list[0]=0;point=0;}
    if(scenario.fault>=4&&scenario.fault<=8) {
        static const int prefixes[]={4,0x98,0xb4,0x3c,0x40};int length=prefixes[scenario.fault-4];
        point=(CollisionPoint*)(guardPage+4096-length);memcpy(point,Point(0),length);world.list[0]=point;
    }
    if(scenario.fault==9){Point(0)->ownerNode=(CollisionContactOwner*)1;Point(0)->inContact=1;}
    if(scenario.fault>=10&&scenario.fault<=13)args[scenario.fault-10]=(CollisionVec3*)1;
    if(scenario.fault==14){args[0]=(CollisionVec3*)1;args[3]=(CollisionVec3*)1;}
    _fpreset();_controlfp(scenario.round|scenario.precision,_MCW_RC|_MCW_PC);
    __try {
        if(scenario.task==0) {
            if(retail){union{void(RetailCall::*method)();DWORD address;}call;call.address=0x501e10;(((RetailCall*)body)->*call.method)();}
            else body->SoultreePhysicsBaseObject::UnknownVirtualSlot31();
        } else if(scenario.task==1) {
            if(retail)((void(__cdecl*)(int,CollisionPoint**))0x43aff0)(scenario.count,list);
            else UpdateCollisionPointWorldPositions(scenario.count,list);
        } else {
            if(retail){union{void(RetailCall::*method)(const CollisionVec3*,const CollisionVec3*,const CollisionVec3*,const CollisionVec3*,float);DWORD address;}call;call.address=0x43a640;(((RetailCall*)point)->*call.method)(args[0],args[1],args[2],args[3],Body()->linearSpeed);}
            else point->UpdateRelativeMotion(args[0],args[1],args[2],args[3],Body()->linearSpeed);
        }
    } __except(Filter(GetExceptionInformation())) {}
    observed.fp=_statusfp();observed.control=_controlfp(0,0);observed.world=world;memcpy(observed.page,guardPage,4096);_fpreset();
}
static void Fail(int code) { printf("FAIL %d case%d task%d seed%d count%d mode%d duplicate%d alias%d fault%d RC%x PC%x\n",code,checks,scenario.task,scenario.seed,scenario.count,scenario.mode,scenario.duplicate,scenario.alias,scenario.fault,scenario.round,scenario.precision);exit(1); }
static void Compare() {
    Run(1);expected=observed;Run(0);
    if(memcmp(&observed,&expected,sizeof(observed))) {
        for(int i=0;i<sizeof(observed);i++)if(((BYTE*)&observed)[i]!=((BYTE*)&expected)[i]){printf("offset%x source%02x retail%02x\n",i,((BYTE*)&observed)[i],((BYTE*)&expected)[i]);break;}
        Fail(1);
    }
    checks++;
}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.count=6;scenario.precision=_PC_64;}
int main(int argc,char** argv) {
    if(argc!=3||!MapReference(argv[1],argv[2]))return 3;
    if(TireContactOffset()!=0xb8)return 4;
    InitFastSqrtTable();InitFastInvSqrtTable();
    guardPage=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;
    if(!guardPage||!VirtualProtect(guardPage+4096,4096,PAGE_NOACCESS,&old))return 5;
    // Independent consumer outcomes: inactive contacts reset; active state 2 survives;
    // lever arm is recomputed before the provider consumes its aliased fourth input.
    Base();scenario.seed=1;scenario.mode=1;Compare();
    if(observed.exception||*(int*)&Point(0)->field_0xb4!=1||*(int*)&Point(1)->field_0xb4!=2||*(int*)&Point(2)->field_0xb4!=0||Point(0)->relativePosition.x!=0||Point(0)->relativePosition.y!=-5||Point(0)->relativePosition.z!=2)Fail(90);
    Base();scenario.count=0;scenario.fault=2;Compare();if(observed.exception)Fail(91);
    Base();scenario.count=1;scenario.fault=3;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.address!=4||observed.kind!=0)Fail(92);
    Base();scenario.task=1;scenario.count=1;scenario.seed=0;scenario.mode=0;Compare();
    // Mode 0 point0 is inactive: its position is untouched, including the last component.
    if(observed.exception||Point(0)->worldPosition.x!=1||Point(0)->worldPosition.y!=-3||Point(0)->worldPosition.z!=0)Fail(93);
    Base();scenario.task=1;scenario.count=1;scenario.seed=0;scenario.mode=8;Compare();
    if(observed.exception||Point(0)->worldPosition.x!=-5||Point(0)->worldPosition.y!=-5||Point(0)->worldPosition.z!=0||Node(0)->worldValid!=1)Fail(94);
    const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},precisions[]={_PC_64,_PC_53,_PC_24};
    for(int task=0;task<2;task++)for(int seed=0;seed<64;seed++)for(int count=-1;count<=6;count++)for(int mode=0;mode<16;mode++)for(int duplicate=0;duplicate<2;duplicate++) {
        Base();scenario.task=task;scenario.seed=seed;scenario.count=count;scenario.mode=mode;scenario.duplicate=duplicate;Compare();
    }
    for(int r=0;r<4;r++)for(int p=0;p<3;p++)for(int seed=0;seed<64;seed++)for(int alias=0;alias<8;alias++) {
        Base();scenario.task=2;scenario.seed=seed;scenario.alias=alias;scenario.round=rounds[r];scenario.precision=precisions[p];Compare();
    }
    for(task=0;task<3;task++)for(int fault=1;fault<=14;fault++)for(int seed=0;seed<16;seed++) {
        // A null body is meaningful only for the actual slot31 call; other paths
        // have their independent receiver/list/argument faults below.
        if(fault==1&&task!=0)continue;
        Base();scenario.task=task;scenario.fault=fault;scenario.seed=seed;scenario.count=1;scenario.mode=1;Compare();
    }
    printf("Collision point consumers: %d actual-source, mixed-array, alias, FP and fault scenarios match\n",checks);
    return 0;
}
#endif
