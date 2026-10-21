// Reuse the actual-source manager/find/set scenarios, then exercise the new
// rest-pose route and the bike's actual zero-vector return branch.
#define main CharacterMotionCases
#include "character_motion_routes.cpp"
#undef main
#include "vehicle/KrustyBike.h"

extern "C" const Vec3* BikeLocalZero();
Vec3 Bike::UnknownVirtualSlot76(const Vec3*,const Vec3*) {
    // The bounded zero-vector test must never leave its selected branch.
    ExitProcess(90);return Vec3(1,2,3);
}
static DWORD poseCalls,expectedPoseCount;
static Character* expectedCharacter;
static CharacterPose* expectedPoses;
struct PoseObserver {
    void Apply(int pose,int mask) {
        if((void*)this!=expectedCharacter || poseCalls>=expectedPoseCount || mask ||
           (void*)pose!=expectedPoses+poseCalls)ExitProcess(91);
        poseCalls++;
    }
};
static int TestRestPose() {
    DWORD table[11],words[(sizeof(D3DIMSoultreeCharacter)+31)/4];
    BYTE before[sizeof(words)];CharacterPose poses[7],beforePoses[7];
    void (PoseObserver::*method)(int,int)=&PoseObserver::Apply;
    typedef char MemberPointerSize[sizeof(method)==4?1:-1];
    DWORD address;memcpy(&address,&method,4);
    for(int i=0;i<11;i++)table[i]=0;table[6]=address;
    unsigned cases=0;
    for(int seed=0;seed<64;seed++)for(int count=-1;count<=7;count++) {
        memset(words,seed,sizeof(words));memset(poses,seed+3,sizeof(poses));
        Character* character=(Character*)(words+4);
        *(DWORD**)character=table;character->poseList.poses=poses;character->poseList.count=count;
        memcpy(before,words,sizeof(words));memcpy(beforePoses,poses,sizeof(poses));
        poseCalls=0;expectedPoseCount=count<0?0:count;expectedCharacter=character;expectedPoses=poses;
        character->ApplyRestPose();
        if(poseCalls!=expectedPoseCount || memcmp(before,words,sizeof(words)) ||
           memcmp(beforePoses,poses,sizeof(poses)))return 1;
        cases++;
    }
    printf("Bike rest pose: %u actual helper and callback cases passed\n",cases);
    return 0;
}
static int TestDirectHandles() {
    DWORD words[(sizeof(D3DIMSoultreeCharacter)+31)/4];BYTE expected[sizeof(words)];
    Motion motion;MotionPoseList frame;
    for(int seed=0;seed<128;seed++)for(int hasFrame=0;hasFrame<2;hasFrame++) {
        memset(words,seed,sizeof(words));memset(&motion,seed+13,sizeof(motion));
        motion.frames=hasFrame?&frame:0;Character* character=(Character*)(words+4);
        memcpy(expected,words,sizeof(words));BYTE* e=expected+16;
        Put(e,0xc,0);Put(e,0x18,(DWORD)&motion);Put(e,0x1c,(DWORD)motion.frames);Put(e,0x34,0);
        int handle=(int)&motion;
        character->SetMotion(reinterpret_cast<Motion*>(handle));
        if(memcmp(expected,words,sizeof(words)))return 2;
    }
    printf("Bike motion handles: 256 direct pointer round trips passed\n");return 0;
}
static int TestZero() {
    DWORD words[(sizeof(KrustyBike)+31)/4];BYTE before[sizeof(words)];
    Vec3 a(1,2,3),b(4,5,6);DWORD beforeA[3],beforeB[3];
    memcpy(beforeA,&a,12);memcpy(beforeB,&b,12);
    const Vec3* zero=BikeLocalZero();DWORD bits[3];memcpy(bits,zero,12);
    if(bits[0] || bits[1] || bits[2])return 3;
    for(int seed=0;seed<256;seed++) {
        memset(words,seed,sizeof(words));KrustyBike* bike=(KrustyBike*)(words+4);
        bike->aiKind=(seed%255)+1;memcpy(before,words,sizeof(words));
        struct Guarded { DWORD first;Vec3 value;DWORD last; } result;
        memset(&result,0xc7,sizeof(result));
        result.value=bike->KrustyBike::UnknownVirtualSlot76(&a,&b);
        if(memcmp(&result.value,bits,12) || result.first!=0xc7c7c7c7 || result.last!=0xc7c7c7c7 ||
           memcmp(before,words,sizeof(words)) || memcmp(zero,bits,12) ||
           memcmp(&a,beforeA,12) || memcmp(&b,beforeB,12))return 4;
    }
    printf("Bike local zero: 256 actual caller storage and return guards passed\n");return 0;
}
int main() {
    int result=CharacterMotionCases();if(result)return result;
    if((result=TestRestPose())!=0)return result;
    if((result=TestDirectHandles())!=0)return result;
    return TestZero();
}
