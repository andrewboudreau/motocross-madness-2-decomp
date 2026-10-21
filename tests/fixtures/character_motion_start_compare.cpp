// Private validation only: actual standalone candidate and actual strict pose-list
// helper execute on prepared storage. Only virtual slot 4 is scripted.
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "motion/Motnctrl.h"
static unsigned char referenceSpace[0x800000];
union CharacterStorage { double align; unsigned char bytes[sizeof(D3DIMSoultreeCharacter)+32]; };
static CharacterStorage receiver;
static Motion motion;
static MotionPoseList* frames;
static BYTE* framePage;
static CharacterPose poses[4];
static DWORD vtable[12];
static int eventCount, callbackMode, faultMode;
struct Event { DWORD pose;int mask;unsigned char character[sizeof(receiver)];MotionPoseList frame; };
static Event events[8];
struct Result { CharacterStorage character;Motion source;MotionPoseList lists[257];BYTE page[4096];CharacterPose poses[4];Event events[8];int count;DWORD exception,access,address,faultControl,faultStatus;unsigned status,control; };
static Result referenceResult,candidateResult;
static int PreparedFrame(MotionPoseList* frame) {
 return (DWORD)frame-(DWORD)framePage<=4096-sizeof(MotionPoseList);
}
static Character* Object(){return (Character*)(receiver.bytes+16);}
static void __fastcall Callback(Character* self,int unused,int pose,int mask) {
 if(eventCount>=8)RaiseException(0xe0005678,0,0,0);
 Event& e=events[eventCount++];
 e.pose=(DWORD)pose;e.mask=mask;memcpy(e.character,&receiver,sizeof(receiver));
 // An arbitrary invalid index can land on a readable system module despite
 // guard pages. Stop at this observable callback before touching foreign data.
 // This is an explicit scripted callback exception, not a production clamp.
 if(!PreparedFrame(self->currentFrame))
  RaiseException(0xe0005679,0,0,0);
 e.frame=*self->currentFrame;
 if(callbackMode==1)self->currentMotion=0;
 if(callbackMode==2)self->currentFrame->count=1;
 if(callbackMode==3 && eventCount==1)RaiseException(0xe0001234,0,0,0);
 if(callbackMode==4)self->currentFrame->poses=poses+1;
 if(callbackMode==5)self->currentFrame->count=0;
 if(callbackMode==6 && eventCount==2)RaiseException(0xe0001234,0,0,0);
 if(callbackMode==7)self->currentFrame=frames+128;
}
template<class T>static DWORD Address(T f){DWORD n;memcpy(&n,&f,4);const unsigned char* b=(const unsigned char*)&f;for(int i=4;i<sizeof(f);++i)if(b[i])ExitProcess(78);return n;}
static int MapReference(const char* path) {
 BYTE* mapped=(BYTE*)0x400000;DWORD old;
 if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||
    !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* file=fopen(path,"rb");if(!file)return 0;
 fseek(file,0,SEEK_END);long size=ftell(file);rewind(file);
 if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;
 BYTE* raw=(BYTE*)malloc(size);
 if(!raw||fread(raw,1,size,file)!=(size_t)size)return 0;fclose(file);
 IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||
    dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
 IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||
    pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||
    pe->OptionalHeader.SizeOfHeaders>(DWORD)size||
    pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
 if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
 memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
 IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int n=0;n<pe->FileHeader.NumberOfSections;++n){
  if(section[n].PointerToRawData>(DWORD)size||section[n].SizeOfRawData>(DWORD)size-section[n].PointerToRawData||
     section[n].VirtualAddress>pe->OptionalHeader.SizeOfImage||
     section[n].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[n].VirtualAddress)return 0;
  memcpy(mapped+section[n].VirtualAddress,raw+section[n].PointerToRawData,section[n].SizeOfRawData);
 }
 free(raw);
 BYTE* call=(BYTE*)0x4a8c36;
 if(*call!=0xe8||(DWORD)(call+5)+*(DWORD*)(call+1)!=0x4a8a80)return 0;
 *(DWORD*)(call+1)=Address(&Character::ApplyPoseListSlot4)-(DWORD)(call+5);
 return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static void Reset(int frameCount,int cb) {
 memset(framePage,0,4096);
 memset(&receiver,0xa5,sizeof(receiver));memset(&motion,0x3c,sizeof(motion));memset(poses,0x69,sizeof(poses));memset(events,0,sizeof(events));
 memset(vtable,0,sizeof(vtable));vtable[4]=(DWORD)&Callback;*(DWORD**)Object()=vtable;
 for(int i=0;i<257;++i){frames[i].frameIndex=i-128;frames[i].poses=poses;frames[i].count=(i%4);}
 motion.frames=frames+128;motion.frameCount=frameCount;eventCount=0;callbackMode=cb;
}
static int Filter(EXCEPTION_POINTERS* e,Result* r) {
 r->exception=e->ExceptionRecord->ExceptionCode;
 if(r->exception>=0xc000008d && r->exception<=0xc0000093){r->faultControl=e->ContextRecord->FloatSave.ControlWord;r->faultStatus=e->ContextRecord->FloatSave.StatusWord;_clearfp();}
 if(r->exception==EXCEPTION_ACCESS_VIOLATION){r->access=e->ExceptionRecord->ExceptionInformation[0];r->address=e->ExceptionRecord->ExceptionInformation[1];}
 return EXCEPTION_EXECUTE_HANDLER;
}
struct Entry { void Invoke(int,float); };
static void Run(DWORD fn,int count,DWORD phaseBits,int cb,unsigned control,Result* result) {
 Reset(count,cb);memset(result,0,sizeof(*result));_fpreset();_control87(control,_MCW_RC|_MCW_PC|_MCW_EM);
 float phase;memcpy(&phase,&phaseBits,4);
 typedef void (Entry::*Method)(int,float); Method method; if(sizeof(method)!=4)ExitProcess(79);memcpy(&method,&fn,4);
 Character* target=Object();Motion* input=&motion;
 if(faultMode==1)input=0;
 if(faultMode==2)motion.frames=0;
 if(faultMode==3)target=0;
 if(faultMode==4)input=(Motion*)1;
 if(faultMode==5)motion.frames=(MotionPoseList*)1;
 __try { (((Entry*)target)->*method)((int)input,phase); }
 __except(Filter(GetExceptionInformation(),result)){}
 result->status=_status87();result->control=_control87(0,0);
 _fpreset();_control87(_CW_DEFAULT,_MCW_RC|_MCW_PC|_MCW_EM);
 result->character=receiver;result->source=motion;memcpy(result->lists,frames,sizeof(result->lists));memcpy(result->page,framePage,4096);memcpy(result->poses,poses,sizeof(poses));memcpy(result->events,events,sizeof(events));result->count=eventCount;
}
static int checks;
static DWORD candidate;
static void Compare(int count,DWORD bits,int callback,unsigned control) {
 Run(0x4a8bf0,count,bits,callback,control,&referenceResult);
 Run(candidate,count,bits,callback,control,&candidateResult);
 // Compiler register-stack allocation is not part of this bounded comparison.
 // Preserve and compare every other fault-status bit, including condition bits.
 DWORD referenceTop=referenceResult.faultStatus&0x3800;
 DWORD candidateTop=candidateResult.faultStatus&0x3800;
 referenceResult.faultStatus&=~0x3800;candidateResult.faultStatus&=~0x3800;
 if(memcmp(&referenceResult,&candidateResult,sizeof(Result))){
  printf("FAIL case%d count%d phase%08x cb%d control%x fault%d exception%x/%x status%x/%x faultSW%x/%x TOP%x/%x\n",
   checks,count,bits,callback,control,faultMode,referenceResult.exception,candidateResult.exception,
   referenceResult.status,candidateResult.status,referenceResult.faultStatus,candidateResult.faultStatus,referenceTop,candidateTop);
  exit(1);
 }
 ++checks;
}
static void Require(int condition,int code){if(!condition){printf("Outcome FAIL %d case%d\n",code,checks);exit(1);}}
int main(int argc,char** argv) {
 if(argc!=3||!MapReference(argv[1]))return 2;
 candidate=strtoul(argv[2],0,16);
 BYTE* frameRegion=(BYTE*)VirtualAlloc((void*)0x30000000,0x4000000,MEM_RESERVE,PAGE_NOACCESS);
 if(!frameRegion)return 3;
 framePage=(BYTE*)VirtualAlloc(frameRegion+0x2000000,4096,MEM_COMMIT,PAGE_READWRITE);
 if(!framePage)return 4;
 frames=(MotionPoseList*)(framePage+512);
 Require(!PreparedFrame((MotionPoseList*)0xfffffff8)&&!PreparedFrame((MotionPoseList*)0)&&
         PreparedFrame((MotionPoseList*)framePage)&&
         PreparedFrame((MotionPoseList*)(framePage+4096-sizeof(MotionPoseList)))&&
         !PreparedFrame((MotionPoseList*)(framePage+4097-sizeof(MotionPoseList))),18);
 const DWORD phases[]={0,0x80000000,0x3f000000,0x3f800000,0x3e800000,0x3fc00000,0xbf000000,0xbf800000,1,0x80000001,0x007fffff,0x00800000,0x3effffff,0x3f000001,0x7f800000,0xff800000,0x7fc00001,0x7f800001,0x4f000000,0xcf000000};
 const unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 const unsigned pc[]={_PC_24,_PC_53,_PC_64};
 const unsigned exceptions[]={_EM_INVALID,_EM_DENORMAL,_EM_ZERODIVIDE,_EM_OVERFLOW,_EM_UNDERFLOW,_EM_INEXACT};
 int r,p,c,b;
 for(r=0;r<4;++r)for(p=0;p<3;++p)for(c=0;c<18;++c)
  for(b=0;b<20;++b)for(int cb=0;cb<8;++cb)
   Compare(c,phases[b],cb,_MCW_EM|rc[r]|pc[p]);
 // Every exception-mask combination, all current rounding/precision modes.
 // Each run resets the x87 stack: an unmasked FISTP does not necessarily pop.
 for(int maskBits=0;maskBits<64;++maskBits)for(r=0;r<4;++r)for(p=0;p<3;++p)
  for(c=0;c<18;++c)for(b=0;b<20;++b){
   unsigned mask=0;
   for(int bit=0;bit<6;++bit)if(maskBits&(1<<bit))mask|=exceptions[bit];
   Compare(c,phases[b],0,mask|rc[r]|pc[p]);
  }
 const int extraCounts[]={-1,-17,(-2147483647-1),2147483647,16777216,16777217};
 for(r=0;r<4;++r)for(p=0;p<3;++p)for(c=0;c<6;++c)for(b=0;b<20;++b)
  Compare(extraCounts[c],phases[b],0,_MCW_EM|rc[r]|pc[p]);
 unsigned random=0x86ae3951;
 for(int seed=0;seed<8192;++seed){
  random=random*1664525+1013904223;DWORD bits=random;
  if((bits&0x7f800000)==0x7f800000)bits^=0x00800000;
  Compare(seed%6?seed%18:extraCounts[(seed/6)%6],bits,seed%8,_MCW_EM|rc[seed%4]|pc[(seed/4)%3]);
 }
 for(faultMode=1;faultMode<=5;++faultMode)for(r=0;r<4;++r)
  for(c=1;c<=4;++c)for(b=0;b<4;++b)Compare(c,phases[b],0,_MCW_EM|rc[r]|_PC_64);
 faultMode=0;
 // Independent expected outcomes prevent a shared dependency adapter from
 // making matching state alone sufficient evidence for frame selection.
 Compare(4,0x3f800000,0,_MCW_EM|_RC_NEAR|_PC_64);
 Require(Object()->currentMotion==&motion&&Object()->currentFrame==frames+130&&eventCount==2,10);
 Require(events[0].pose==(DWORD)poses&&events[1].pose==(DWORD)(poses+1)&&events[0].mask==0,11);
 Compare(4,0x3f800000,0,_MCW_EM|_RC_UP|_PC_64);
 Require(Object()->currentFrame==frames+131&&eventCount==3,12);
 Compare(4,0x3f800000,2,_MCW_EM|_RC_NEAR|_PC_64);
 Require(eventCount==1&&frames[130].count==1,13);
 Compare(4,0x3f800000,3,_MCW_EM|_RC_NEAR|_PC_64);
 Require(candidateResult.exception==0xe0001234&&eventCount==1&&Object()->currentFrame==frames+130,14);
 Compare(0,0x7f800000,0,(_MCW_EM&~_EM_INVALID)|_RC_NEAR|_PC_24);
 Require(candidateResult.exception==EXCEPTION_FLT_INVALID_OPERATION&&Object()->currentFrame==frames+127,15);
 faultMode=1;Compare(1,0,0,_MCW_EM|_RC_NEAR|_PC_64);
 Require(candidateResult.exception==EXCEPTION_ACCESS_VIOLATION&&candidateResult.address==0xa8&&
         (DWORD)Object()->currentMotion==0xa5a5a5a5,16);
 faultMode=2;Compare(1,0,0,_MCW_EM|_RC_NEAR|_PC_64);
 Require(candidateResult.exception==EXCEPTION_ACCESS_VIOLATION&&candidateResult.address==8&&
         Object()->currentMotion==&motion&&Object()->currentFrame==0,17);
 printf("Character motion start: %d state, callback, rounding and exception scenarios match\n",checks);
 return 0;
}
