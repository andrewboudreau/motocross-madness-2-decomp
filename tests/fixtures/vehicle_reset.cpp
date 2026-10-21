// Execute the actual reset object through prepared storage and observed calls.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicle/Vehicle.h"

union VehicleStorage { double align; unsigned char bytes[sizeof(Vehicle)+32]; };
union NodeStorage { double align; unsigned char bytes[sizeof(SoultreeObject)+32]; };
union SteeringStorage { double align; unsigned char bytes[sizeof(SteeringControl)+32]; };
struct World { VehicleStorage vehicle; NodeStorage node[2]; SteeringStorage steering[2]; } actual, wanted;
static unsigned long table[37];
static int seed, variant, throwAt, calls, cases;
static Vehicle* VehicleIn(World& w) { return (Vehicle*)(w.vehicle.bytes+16); }
static SoultreeObject* NodeIn(World& w,int i) { return (SoultreeObject*)(w.node[i].bytes+16); }
static SteeringControl* SteeringIn(World& w,int i) { return (SteeringControl*)(w.steering[i].bytes+16); }
static unsigned long Bits(float f) { unsigned long b; memcpy(&b,&f,4); return b; }
static float Float(unsigned long b) { float f; memcpy(&f,&b,4); return f; }
static void Check(int ok,int line) { if(!ok) { printf("vehicle reset failure %d seed%d variant%d throw%d calls%d\n",line,seed,variant,throwAt,calls); exit(1); } }
#define CHECK(x) Check((x),__LINE__)

static void Same() { CHECK(!memcmp(&actual,&wanted,sizeof(actual))); }
static void Mutate(World& w,int phase) {
    Vehicle* v=VehicleIn(w);
    if((variant&1) && phase<=2) {
        v->bodyForward=Vec3((float)(seed+phase), (float)-phase, (float)(seed-phase));
        v->bodyUp=Vec3((float)phase, (float)(seed+1), (float)-seed);
        v->bodyYaw=(float)(seed*3+phase);
    }
    if((variant&2) && phase==2) {
        v->steerState=SteeringIn(actual,1);
        v->poseNode=NodeIn(actual,1);
    }
    if((variant&4) && phase==3) {
        // The next call must reload the current steering object and its node.
        v->steerState=SteeringIn(actual,0);
        SteeringIn(w,0)->node=NodeIn(actual,1);
    }
    if((variant&8) && phase>=3) {
        v->bodyForward.x=(float)(seed+100+phase);
        v->savedForward.z=(float)(-100-phase);
        v->savedYaw=(float)(phase*17);
        v->poseNode=NodeIn(actual,phase&1);
    }
}
static void ExpectedCopies() {
    Vehicle* v=VehicleIn(wanted);
    v->bodyCosRoll=v->bodyCosPitch=1.0f;
    v->bodyRoll=v->bodyPitch=v->bodySinRoll=v->bodySinPitch=0.0f;
    v->savedForward=v->bodyForward; v->savedUp=v->bodyUp;
    v->savedYaw=v->bodyYaw; v->savedPitch=0; v->savedRoll=0;
    v->savedSinRoll=0; v->savedCosRoll=1; v->savedCosPitch=1; v->savedSinPitch=0;
}
static void Begin(int phase) { CHECK(++calls==phase); Same(); }
static void End(int phase) {
    Mutate(actual,phase); Mutate(wanted,phase);
    if(throwAt==phase) throw 1234;
    if(phase==2) ExpectedCopies();
}
class Slots { public: void WriteAxes(int,int); void ReadAxes(); };
void Slots::WriteAxes(int a,int b) { CHECK((void*)this==VehicleIn(actual)); CHECK(a==1 && b==0); Begin(1); End(1); }
void Slots::ReadAxes() { CHECK((void*)this==VehicleIn(actual)); Begin(2); End(2); }
void SoultreeObject::SetPosition(const Vec3& p) {
    Begin(3); CHECK(this==VehicleIn(actual)->steerState->node);
    CHECK(Bits(p.x)==0 && Bits(p.y)==0 && Bits(p.z)==0);
    End(3);
}
void SoultreeObject::SetAxesPtr(const Vec3* z,const Vec3* y,int orthogonalize,int keepZ) {
    Begin(4); Vehicle* v=VehicleIn(actual);
    CHECK(this==v->steerState->node && z==&v->bodyForward && y==&v->bodyUp);
    CHECK(orthogonalize==0 && keepZ==1); End(4);
}
void SteeringControl::SetAngle(float a,SoultreeObject* frame) {
    Begin(5); CHECK(this==VehicleIn(actual)->steerState);
    CHECK(Bits(a)==0 && frame==VehicleIn(actual)->poseNode); End(5);
}
template<class T> static unsigned long Address(T p) { CHECK(sizeof(p)==4); union { T p; unsigned long address; } u; u.p=p; return u.address; }
static void Prepare() {
    memset(&actual,(seed*37+19)&255,sizeof(actual));
    Vehicle* v=VehicleIn(actual); *(unsigned long**)v=table;
    const unsigned long vertical[]={0,0x80000000UL,0x3f800000UL,0xbf800000UL,0x7fc12345UL,0x7f800000UL,0xff800000UL,1};
    v->bodyForward=Vec3((seed&8)?0.0f:2.0f,Float(vertical[seed%8]),(seed&16)?Float(0x80000000UL):4.0f);
    v->bodyUp=Vec3((float)(seed-31),(float)(seed+1),(float)(17-seed));
    v->bodyYaw=(float)(seed*7);
    for(int i=0;i<2;++i) SteeringIn(actual,i)->node=NodeIn(actual,i);
    v->steerState=SteeringIn(actual,0); v->poseNode=NodeIn(actual,0);
    memcpy(&wanted,&actual,sizeof(actual));
    Vehicle* e=VehicleIn(wanted);
    if(v->bodyForward.x==0.0f && v->bodyForward.z==0.0f) {
        unsigned long sign=v->bodyForward.y>=0.0f?0x80000000UL:0;
        e->bodyForward.x=Float(Bits(v->bodyUp.x)^sign);
        e->bodyForward.z=Float(Bits(v->bodyUp.z)^sign);
    }
    e->bodyForward.y=0.0f; e->bodyUp=Vec3(0,1,0);
    calls=0;
}
int main() {
    table[35]=Address(&Slots::WriteAxes); table[34]=Address(&Slots::ReadAxes);
    for(seed=0;seed<64;++seed) for(variant=0;variant<16;++variant) for(throwAt=0;throwAt<=5;++throwAt) {
        Prepare(); int caught=0;
        try { VehicleIn(actual)->Vehicle::UnknownVirtualSlot36(); }
        catch(int code) { CHECK(code==1234); caught=1; }
        CHECK(caught==(throwAt!=0)); CHECK(calls==(throwAt?throwAt:5)); Same(); ++cases;
    }
    printf("Vehicle reset: %d complete-storage, callback and exception cases passed\n",cases);
    return 0;
}
