// Complete blend comparison on prepared storage with actual math and a bounded primary callback.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include "motion/Motnctrl.h"
#include "math/FastMath.h"
void InitFastInvSqrtTable();
extern unsigned char g_FastInvSqrtTable[128];
// Validation-only zero-adjustment thiscall entry. The genuine virtual-base
// Character member-pointer representation is 12 bytes, so no address is copied into it.
struct Entry { void Blend(int,int,float,float); };
typedef char ActualLayout[(sizeof(Character)==0x1cc && sizeof(Motion)==0xcc &&
 sizeof(MotionPoseList)==12 && sizeof(CharacterPose)==44 &&
 offsetof(Character,currentMotion)==0x18 && offsetof(Character,currentFrame)==0x1c &&
 offsetof(Motion,frames)==0xa4 && offsetof(Motion,frameCount)==0xa8 &&
 offsetof(MotionPoseList,poses)==4 && offsetof(MotionPoseList,count)==8 &&
 offsetof(CharacterPose,nodeIndex)==0 && offsetof(CharacterPose,hasPose)==4 &&
 offsetof(CharacterPose,axisZ)==8 && offsetof(CharacterPose,axisY)==20 &&
 offsetof(CharacterPose,position)==32 && sizeof(void(Character::*)(int,int,float,float))==12)?1:-1];
static BYTE referenceSpace[0x800000]={1};
struct World { BYTE actor[sizeof(Character)]; Motion motion[2]; MotionPoseList frames[2][8]; CharacterPose poses[2][8][4]; };
static World world, initial;
static DWORD table[11];
static BYTE* framePage[2];
static BYTE* posePage;
static BYTE* actorPage;
static Character* actorOverride;
static int useActorOverride;
static BYTE actorInitial[4096];
static BYTE frameInitial[2][4096];
static int faults;
static DWORD mode;
static unsigned exceptionMask=_MCW_EM;
static int mutation, aliasMode;
static DWORD rng;
static float Coordinate(){rng=rng*1664525u+1013904223u;return ((int)(rng%20001)-10000)/997.0f;}
static float Phase(int i){if(i<9)return i/8.0f;DWORD b=0x3f000000u|((i*1457341u)&0x7fffff);float f;memcpy(&f,&b,4);return f;}

struct Result { World after; BYTE frames[2][4096]; CharacterPose applied[8]; BYTE state[8][sizeof(Character)]; int calls; BYTE actorPage[4096]; DWORD code,address,access,fp,faultControl,faultStatus; };
static Result result, expected;
static Character* Actor(){return useActorOverride?actorOverride:(Character*)world.actor;}
struct Callback {
 void Apply(int ptr,int mask) {
  if((void*)this!=Actor() || mask || result.calls>=8) ExitProcess(80);
  CharacterPose& out=result.applied[result.calls];out=*(CharacterPose*)ptr;
  memset((BYTE*)&out+1,0,3);
  memcpy(result.state[result.calls],world.actor,sizeof(Character));++result.calls;
  if(result.calls==1){
   switch(mutation){
    case 1:Actor()->currentFrame=world.motion[0].frames+7;Actor()->currentFrame->count=1;break;
    case 2:world.motion[0].frames[1].poses=world.poses[0][7];break;
    case 3:world.motion[0].frames=(MotionPoseList*)framePage[1];break;
    case 4:Actor()->currentMotion=&world.motion[1];break;
    case 5:Actor()->currentFrame=world.motion[1].frames+7;Actor()->currentFrame->count=2;break;
    case 6:world.motion[1].frames[1].poses=world.poses[1][7];break;
    case 7:RaiseException(0xe1234567,0,0,0);break;
    case 9:Actor()->currentFrame=0;break;
    case 10:world.motion[0].frames[1].poses=(CharacterPose*)1;break;
    case 11:world.motion[1].frames[1].poses=(CharacterPose*)1;break;
   }
  }
  if(mutation==8&&result.calls==2)RaiseException(0xe1234567,0,0,0);
 }
};
static int Filter(EXCEPTION_POINTERS* p){
 result.code=p->ExceptionRecord->ExceptionCode;
 if(result.code>=0xc000008d&&result.code<=0xc0000093){result.faultControl=p->ContextRecord->FloatSave.ControlWord;result.faultStatus=p->ContextRecord->FloatSave.StatusWord&~0x3800;_clearfp();}
 if(result.code==EXCEPTION_ACCESS_VIOLATION){result.access=p->ExceptionRecord->ExceptionInformation[0];result.address=p->ExceptionRecord->ExceptionInformation[1];}
 return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail,float phase,float weight) {
 memcpy(&world,&initial,sizeof(world));for(int m=0;m<2;m++)memcpy(framePage[m],frameInitial[m],4096);memcpy(actorPage,actorInitial,4096);memset(&result,0,sizeof(result));_fpreset();_control87(mode|exceptionMask,_MCW_PC|_MCW_RC|_MCW_EM);
 __try {
  if(retail){union{void(Entry::*fn)(int,int,float,float);DWORD address;}u;typedef char Abi[sizeof(u.fn)==4?1:-1];u.address=0x4a8c50;(((Entry*)Actor())->*u.fn)((int)&world.motion[0],(int)&world.motion[aliasMode==1?0:1],phase,weight);}
  else Actor()->Method_0x004a8c50((int)&world.motion[0],(int)&world.motion[aliasMode==1?0:1],phase,weight);
 }__except(Filter(GetExceptionInformation())){}
 memcpy(result.actorPage,actorPage,4096);result.fp=_status87()&~0x3800;memcpy(&result.after,&world,sizeof(world));for(m=0;m<2;m++)memcpy(result.frames[m],framePage[m],4096);_fpreset();
}
static int FrameIndex(int count,float phase) {
 _fpreset();_control87(mode,_MCW_PC|_MCW_RC);
 volatile float position=(count-1)*phase;
 volatile float shifted=position-0.5f;
 double down=floor(shifted), rounded;
 switch(mode&_MCW_RC) {
  case _RC_DOWN:rounded=down;break;
  case _RC_UP:rounded=ceil(shifted);break;
  case _RC_CHOP:rounded=shifted<0?ceil(shifted):down;break;
  default:{double part=shifted-down;rounded=down;
   if(part>0.5||(part==0.5&&((int)down&1)))rounded+=1;}
 }
 int value=(int)rounded;_fpreset();return value;
}
static float Lerp(float a,float b,float t){volatile float d=b-a;volatile float scaled=d*t;volatile float out=a+scaled;return out;}
static float Clip(float t){if(t<0)return 0;if(t>1)return 1;return t;}
static void Oracle(int firstCount,int secondCount,float phase,float weight,int seed) {
 int a=FrameIndex(firstCount,phase),b=FrameIndex(secondCount,phase);
 World want=initial;
 Character* expectedActor=(Character*)want.actor;
 expectedActor->currentMotion=&world.motion[0];
 expectedActor->currentFrame=(MotionPoseList*)framePage[0]+a;
 if(memcmp(&result.after,&want,sizeof(want)))ExitProcess(31);
 for(int m=0;m<2;m++)if(memcmp(result.frames[m],frameInitial[m],4096))ExitProcess(32);
 if(a<0||b<0){
  DWORD address=a<0?(DWORD)(framePage[0]-4):(DWORD)(framePage[1]-8);
  if(result.code!=EXCEPTION_ACCESS_VIOLATION||result.access||result.address!=address||result.calls)ExitProcess(33);
  faults++;
 } else {
  if(result.code||result.calls!=3)ExitProcess(34);
  if(seed==0){
   _control87(mode,_MCW_PC|_MCW_RC);
   volatile float ap=(firstCount-1)*phase,bp=(secondCount-1)*phase;
   volatile float at=ap-a,bt=bp-b,wt=1.0f-weight;
   int an=a+1==firstCount?a:a+1,bn=b+1==secondCount?b:b+1;
   for(int i=0;i<3;i++){
    CharacterPose& pose=result.applied[i];
    if(pose.nodeIndex!=i||pose.hasPose!=i||pose.axisZ.x!=0||pose.axisZ.y!=0||pose.axisZ.z!=1||pose.axisY.x!=0||pose.axisY.y!=1||pose.axisY.z!=0)ExitProcess(35);
    for(int c=0;c<3;c++){
     float av=Lerp(((float*)&initial.poses[0][a][i].position)[c],((float*)&initial.poses[0][an][i].position)[c],Clip(at));
     float bv=Lerp(((float*)&initial.poses[1][b][i].position)[c],((float*)&initial.poses[1][bn][i].position)[c],Clip(bt));
     float want=Lerp(av,bv,Clip(wt));
     if(memcmp(&want,((float*)&pose.position)+c,4)){printf("av%08lx bv%08lx ",*(DWORD*)&av,*(DWORD*)&bv);printf("Oracle mode%lx ca%d cb%d phase%.9g weight%.9g i%d c%d a%d b%d at%.9g bt%.9g wt%.9g expected%08lx actual%08lx\n",mode,firstCount,secondCount,phase,weight,i,c,a,b,(double)at,(double)bt,(double)wt,*(DWORD*)&want,((DWORD*)&pose.position)[c]);fflush(stdout);ExitProcess(36);}
    }
   }
   _fpreset();
  }
 }
}

static int MapReference(const char* path) {
    BYTE* mapped = (BYTE*)0x400000;
    DWORD old;
    if ((DWORD)referenceSpace > 0x400000 ||
        (DWORD)referenceSpace + sizeof(referenceSpace) < 0x690000 ||
        !VirtualProtect(mapped, 0x290000, PAGE_EXECUTE_READWRITE, &old))
        return 0;
    FILE* file = fopen(path, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    if (length < (long)sizeof(IMAGE_DOS_HEADER)) return 0;
    BYTE* raw = (BYTE*)malloc(length);
    if (!raw || fread(raw, 1, length, file) != (size_t)length) return 0;
    fclose(file);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)raw;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 ||
        dos->e_lfanew > length - (long)sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)(raw + dos->e_lfanew);
    if (pe->Signature != IMAGE_NT_SIGNATURE ||
        pe->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
        pe->OptionalHeader.ImageBase != 0x400000 ||
        pe->OptionalHeader.SizeOfImage > 0x290000 ||
        pe->OptionalHeader.SizeOfHeaders > (DWORD)length ||
        pe->OptionalHeader.SizeOfHeaders > pe->OptionalHeader.SizeOfImage) return 0;
    DWORD offset = (BYTE*)IMAGE_FIRST_SECTION(pe) - raw;
    if (offset > (DWORD)length || pe->FileHeader.NumberOfSections >
        ((DWORD)length - offset) / sizeof(IMAGE_SECTION_HEADER)) return 0;
    memcpy(mapped, raw, pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(pe);
    for (int i = 0; i < pe->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* section = sections + i;
        if (section->PointerToRawData > (DWORD)length ||
            section->SizeOfRawData > (DWORD)length - section->PointerToRawData ||
            section->VirtualAddress > pe->OptionalHeader.SizeOfImage ||
            section->SizeOfRawData > pe->OptionalHeader.SizeOfImage - section->VirtualAddress)
            return 0;
        memcpy(mapped + section->VirtualAddress, raw + section->PointerToRawData,
               section->SizeOfRawData);
    }
    free(raw);
    return FlushInstructionCache(GetCurrentProcess(), 0, 0);
}

int main(int argc,char**argv) {
 if(argc!=2||!MapReference(argv[1]))return 1;
 for(int m=0;m<2;m++){DWORD old;BYTE* p=(BYTE*)VirtualAlloc(0,12288,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!p||!VirtualProtect(p,4096,PAGE_NOACCESS,&old)||!VirtualProtect(p+8192,4096,PAGE_NOACCESS,&old))return 2;framePage[m]=p+4096;}
 {DWORD old;BYTE* p=(BYTE*)VirtualAlloc(0,12288,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!p||!VirtualProtect(p,4096,PAGE_NOACCESS,&old)||!VirtualProtect(p+8192,4096,PAGE_NOACCESS,&old))return 2;posePage=p+4096;}
 {DWORD old;BYTE* p=(BYTE*)VirtualAlloc(0,12288,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!p||!VirtualProtect(p,4096,PAGE_NOACCESS,&old)||!VirtualProtect(p+8192,4096,PAGE_NOACCESS,&old))return 2;actorPage=p+4096;memset(actorInitial,0x6b,4096);}
 _fpreset();InitFastInvSqrtTable();_fpreset();((void(__cdecl*)())0x460bb0)();
 if(memcmp((void*)0x5daf5c,g_FastInvSqrtTable,128))return 57;_fpreset();
 union{void(Callback::*fn)(int,int);DWORD address;}u;u.fn=&Callback::Apply;typedef char CallbackAbi[sizeof(u.fn)==4?1:-1];table[4]=u.address;
 int cases=0;
 const DWORD modes[]={_PC_24|_RC_NEAR,_PC_53|_RC_NEAR,_PC_64|_RC_NEAR,_PC_24|_RC_DOWN,_PC_53|_RC_DOWN,_PC_64|_RC_DOWN,_PC_24|_RC_UP,_PC_53|_RC_UP,_PC_64|_RC_UP,_PC_24|_RC_CHOP,_PC_53|_RC_CHOP,_PC_64|_RC_CHOP};
 for(int md=0;md<12;md++){mode=modes[md];
 for(int seed=0;seed<8;seed++)for(int ca=1;ca<=6;ca++)for(int cb=1;cb<=6;cb++)for(int phaseIndex=0;phaseIndex<=32;phaseIndex++)for(int w=0;w<=4;w++) {
  memset(&world,0x5a,sizeof(world));*(DWORD*)world.actor=(DWORD)table;
  for(int m=0;m<2;m++){
   rng=seed*51+phaseIndex*29+m+1;world.motion[m].frameCount=m?cb:ca;world.motion[m].frames=(MotionPoseList*)framePage[m];memset(framePage[m],0x5a,4096);
   for(int f=0;f<8;f++){
    world.motion[m].frames[f].frameIndex=f;world.motion[m].frames[f].poses=world.poses[m][f];world.motion[m].frames[f].count=3;
    for(int k=0;k<4;k++){
     CharacterPose& p=world.poses[m][f][k];p.nodeIndex=k;p.hasPose=seed+k;
     p.axisZ=Vec3(Coordinate(),Coordinate(),Coordinate());
     p.axisY=Vec3(Coordinate(),Coordinate(),Coordinate());
     p.position=Vec3(Coordinate(),Coordinate(),Coordinate());
     if(seed==0){p.axisZ=Vec3(0,0,1);p.axisY=Vec3(0,1,0);p.position=Vec3((k+f)*0.125f,(f+m)*0.25f,m*0.5f);}
    }
   }
  }
  for(m=0;m<2;m++)memcpy(frameInitial[m],framePage[m],4096);initial=world;Run(1,Phase(phaseIndex),w/4.0f);expected=result;Run(0,Phase(phaseIndex),w/4.0f);
  if(memcmp(&expected,&result,sizeof(result))){
   int offset=0;while(((BYTE*)&expected)[offset]==((BYTE*)&result)[offset])offset++;
   printf("Mismatch seed%d ca%d cb%d phase%d w%d offset%d expected%02x actual%02x calls%d/%d fault%08lx/%08lx\n",seed,ca,cb,phaseIndex,w,offset,((BYTE*)&expected)[offset],((BYTE*)&result)[offset],expected.calls,result.calls,expected.code,result.code);return 3;
  }
  Oracle(ca,cb,Phase(phaseIndex),w/4.0f,seed);
  if(result.code && result.code!=EXCEPTION_ACCESS_VIOLATION)return 4;
  if(!result.code&&result.calls!=3)return 5;
  cases++;
 }
 }

 mode=_PC_64|_RC_NEAR;
 // Reuse the last prepared finite world, selecting frame 1 with phase 0.3.
 for(int aliases=0;aliases<4;aliases++)for(int change=0;change<12;change++){
  aliasMode=aliases;mutation=change;
  World saved=initial;BYTE savedFrames[2][4096];memcpy(savedFrames,frameInitial,sizeof(savedFrames));
  initial.motion[0].frameCount=initial.motion[1].frameCount=6;
  for(int m=0;m<2;m++)for(int f=0;f<8;f++)((MotionPoseList*)frameInitial[m])[f].count=3;
  if(aliases==2)initial.motion[1].frames=initial.motion[0].frames;
  if(aliases==3)for(int f=0;f<8;f++)((MotionPoseList*)frameInitial[1])[f].poses=((MotionPoseList*)frameInitial[0])[f].poses;
  Run(1,0.3f,0.25f);expected=result;Run(0,0.3f,0.25f);
  if(memcmp(&expected,&result,sizeof(result))){printf("Mutation mismatch alias%d change%d code%lx/%lx calls%d/%d address%lx/%lx\n",aliases,change,expected.code,result.code,expected.calls,result.calls,expected.address,result.address);return 40;}
  int calls=change==1||change==7||change==9||change==10||change==11?1:change==5||change==8?2:3;
  // Mutating the unused second motion cannot affect same-motion input.
  if(aliases==1&&change==11)calls=3;
  DWORD code=change==7||change==8?0xe1234567:change>=9?EXCEPTION_ACCESS_VIOLATION:0;
  if(aliases==1&&change==11)code=0;
  if(result.calls!=calls||result.code!=code){printf("Mutation oracle alias%d change%d code%lx calls%d\n",aliases,change,result.code,result.calls);return 41;}
  if(code==EXCEPTION_ACCESS_VIOLATION&&(result.access||result.address!=(change==9?8:45))){printf("Mutation fault oracle alias%d change%d address%lx access%lu\n",aliases,change,result.address,result.access);return 42;}
  initial=saved;memcpy(frameInitial,savedFrames,sizeof(frameInitial));cases++;
 }

 mutation=aliasMode=0;
 int poseFaults=0;
 for(int owner=0;owner<2;owner++)for(int next=0;next<2;next++)for(int available=0;available<=44;available++){
  World saved=initial;BYTE savedFrames[2][4096];memcpy(savedFrames,frameInitial,sizeof(savedFrames));
  initial.motion[0].frameCount=initial.motion[1].frameCount=6;
  for(int m=0;m<2;m++)for(int f=0;f<8;f++)((MotionPoseList*)frameInitial[m])[f].count=1;
  memset(posePage,0x5a,4096);CharacterPose* guarded=(CharacterPose*)(posePage+4096-available);
  if(available)memcpy(guarded,&initial.poses[owner][1+next][0],available);
  ((MotionPoseList*)frameInitial[owner])[1+next].poses=guarded;
  Run(1,0.3f,0.25f);expected=result;Run(0,0.3f,0.25f);
  if(memcmp(&expected,&result,sizeof(result))){printf("Guard mismatch owner%d next%d available%d code%lx/%lx address%lx/%lx\n",owner,next,available,expected.code,result.code,expected.address,result.address);return 43;}
  if(available<44){int offsets[11]={0,4,8,12,16,20,24,28,32,36,40};
   if(owner){offsets[2]=16;offsets[3]=12;offsets[4]=8;offsets[5]=28;offsets[6]=24;offsets[7]=20;offsets[8]=40;offsets[9]=36;offsets[10]=32;}
   int first=next?2:0;for(int q=first;q<11;q++)if(offsets[q]+(q==0?1:4)>available)break;
   DWORD wantAddress=(DWORD)guarded+offsets[q];if(offsets[q]<available)wantAddress=(DWORD)(posePage+4096);
   if(result.code!=EXCEPTION_ACCESS_VIOLATION||result.access||result.calls||result.address!=wantAddress){printf("Guard oracle owner%d next%d available%d code%lx address%lx\n",owner,next,available,result.code,result.address);return 44;}poseFaults++;}
  else if(result.code||result.calls!=1)return 45;
  if(available&&memcmp(guarded,&initial.poses[owner][1+next][0],available))return 46;
  initial=saved;memcpy(frameInitial,savedFrames,sizeof(frameInitial));cases++;
 }
 // Loop count <=0 performs setup but does not read any pose record.
 for(int count=-3;count<=0;count++){
  BYTE savedFrames[2][4096];memcpy(savedFrames,frameInitial,sizeof(savedFrames));
  for(int m=0;m<2;m++)for(int f=0;f<8;f++)((MotionPoseList*)frameInitial[m])[f].count=count;
  Run(1,0.3f,0.25f);expected=result;Run(0,0.3f,0.25f);
  if(memcmp(&expected,&result,sizeof(result))||result.code||result.calls)return 47;
  memcpy(frameInitial,savedFrames,sizeof(frameInitial));cases++;
 }
 // Finite weight extrapolations are clamped only in each pose interpolation.
 const float weights[]={-4,-1,-0.0001f,1.0001f,2,4};
 for(md=0;md<12;md++)for(int w=0;w<6;w++){
  mode=modes[md];Run(1,0.3f,weights[w]);expected=result;Run(0,0.3f,weights[w]);
  if(memcmp(&expected,&result,sizeof(result))||result.code||result.calls!=3)return 48;cases++;
 }

 const unsigned masks[]={_EM_INVALID,_EM_DENORMAL,_EM_ZERODIVIDE,_EM_OVERFLOW,_EM_UNDERFLOW,_EM_INEXACT};
 const DWORD phaseBits[]={0,0x3e800000,0x3f000000,0x3f800000,0x7fc00001,0x7f800000,0x4f000000,0x00800000};
 for(int mask=0;mask<64;mask++)for(md=0;md<12;md++)for(int ph=0;ph<8;ph++){
  exceptionMask=0;for(int bit=0;bit<6;bit++)if(mask&(1<<bit))exceptionMask|=masks[bit];
  mode=modes[md];float phase;memcpy(&phase,phaseBits+ph,4);
  Run(1,phase,0.25f);expected=result;Run(0,phase,0.25f);
  if(memcmp(&expected,&result,sizeof(result))){int offset=0;while(((BYTE*)&expected)[offset]==((BYTE*)&result)[offset])offset++;printf("Exception mismatch mask%d mode%lx phase%lx offset%d code%lx/%lx calls%d/%d fp%lx/%lx\n",mask,mode,phaseBits[ph],offset,expected.code,result.code,expected.calls,result.calls,expected.fp,result.fp);return 49;}
  if(ph==0&&!(exceptionMask&_EM_INEXACT)){
   World want=initial;((Character*)want.actor)->currentMotion=&world.motion[0];
   if(result.code!=EXCEPTION_FLT_INEXACT_RESULT||result.calls||memcmp(&result.after,&want,sizeof(want)))return 50;
  }
  if(ph==4&&!(exceptionMask&_EM_INVALID)){
   World want=initial;((Character*)want.actor)->currentMotion=&world.motion[0];
   if(result.code!=EXCEPTION_FLT_INVALID_OPERATION||result.calls||memcmp(&result.after,&want,sizeof(want)))return 51;
  }
  cases++;
 }

 exceptionMask=_MCW_EM;mode=_PC_64|_RC_NEAR;useActorOverride=1;mutation=aliasMode=0;
 for(int cut=-1;cut<=2;cut++){
  BYTE savedFrames[2][4096];memcpy(savedFrames,frameInitial,sizeof(savedFrames));
  for(int m=0;m<2;m++)for(int f=0;f<8;f++)((MotionPoseList*)frameInitial[m])[f].count=0;
  actorOverride=cut<0?0:(Character*)(actorPage+4096-0x18-cut*4);
  Run(1,0.3f,0.25f);expected=result;Run(0,0.3f,0.25f);
  if(memcmp(&expected,&result,sizeof(result)))return 52;
  BYTE want[4096];memcpy(want,actorInitial,4096);
  if(cut>=1){DWORD value=(DWORD)&world.motion[0];memcpy(want+4096-cut*4,&value,4);}
  if(cut==2){DWORD value=(DWORD)framePage[0]+12;memcpy(want+4096-4,&value,4);}
  if(memcmp(want,result.actorPage,4096)||result.calls)return 53;
  if(cut<2){DWORD address=cut<0?0x18:(DWORD)actorPage+4096;if(result.code!=EXCEPTION_ACCESS_VIOLATION||result.access!=1||result.address!=address)return 54;}
  else if(result.code)return 55;
  if(memcmp(&result.after,&initial,sizeof(initial)))return 56;
  memcpy(frameInitial,savedFrames,sizeof(frameInitial));cases++;
 }
 printf("Blend initial: %d cases, %d frame-boundary AV, %d pose-boundary AV\n",cases,faults,poseFaults);return 0;
}
