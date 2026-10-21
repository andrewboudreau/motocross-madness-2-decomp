// Private comparison: complete steering operation, actual node/cache helpers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "math/Math3D.h"
#include "motion/SteeringControl.h"
static BYTE referenceSpace[0x800000];
struct World { BYTE nodes[4][0x1a4+32]; BYTE steering[24+32]; };
static World world,initial;
struct Observation { int result;DWORD exception,kind,address,faultFp,faultControl;unsigned control,status;World state;BYTE guard[4096]; };
static Observation observed,expected;
struct Scenario { unsigned seed;int family,parents,frame,caches,alias,invalid,guardTarget,validBytes;unsigned rounding,precision,mask; };
static Scenario scenario;
static BYTE* guard;
static int checks;
static SoultreeObject* Node(int n){return (SoultreeObject*)(world.nodes[n]+16);}
static SteeringControl* Steering(){
 if(scenario.invalid==3)return 0;
 if(scenario.guardTarget==3)return (SteeringControl*)(guard+4096-12-scenario.validBytes);
 if(scenario.alias==1)return (SteeringControl*)((BYTE*)Node(0)+0xac);
 if(scenario.alias==2)return (SteeringControl*)((BYTE*)Node(1)+0xec);
 if(scenario.alias==3)return (SteeringControl*)((BYTE*)Node(2)+0x12c);
 return (SteeringControl*)(world.steering+16);
}
static void Identity(Matrix4& m){memset(&m,0,sizeof(m));m._11=m._22=m._33=m._44=1.0f;}
static float Bits(unsigned bits){float f;memcpy(&f,&bits,4);return f;}
static void Prepare(){
 unsigned random=scenario.seed+1;for(unsigned i=0;i<sizeof(world);++i){random=random*1664525u+1013904223u;((BYTE*)&world)[i]=(BYTE)(random>>24);}
 memset(guard,0x6a,4096);
 for(int n=0;n<4;++n){SoultreeObject* node=Node(n);Identity(node->localMatrix);Identity(node->worldMatrix);node->parent=0;node->worldValid=(scenario.caches>>n)&1;
   Matrix4* matrices[]={&node->localMatrix,&node->worldMatrix};
   for(int m=0;m<2;++m){Matrix4& a=*matrices[m];float x,y,z;
     unsigned seed=scenario.seed+n*73+m*19;
     x=((int)(seed%97)-47)*0.125f;y=((int)(seed%59)-23)*0.0625f;z=((int)(seed%83)-31)*0.25f;
     if(scenario.family==1){x=n==0?3.0f:0;y=z=0;}
     if(scenario.family==2){x=y=z=0;}
     if(scenario.family==3){x=n?0:Bits(0x3f800001);y=n?0:Bits(0x33800000);z=n?0:Bits(0x33000000);}
     if(scenario.family==4){x=(n&1?-1:1)*Bits(0x71800000);y=(n&1?1:-1)*Bits(0x6f123456);z=(n&1?-1:1)*Bits(0x70fedcba);}
     if(scenario.family==5){x=Bits(1+(seed&255));y=Bits(0x80000100+(seed&511));z=Bits(0x00001000+(seed&1023));}
     if(scenario.family==8){x=0;y=0;z=n==0?-2.0f:0;}
     if(scenario.family==9){x=z=0;y=n==0?5.0f:0;if(n==2){a._11=0;a._12=1;a._21=-1;a._22=0;}}
     if(scenario.family==6){random=random*1664525u+1013904223u;unsigned bits=(random&0x807fffff)|(((random>>24)%254)<<23);x=Bits(bits);
       random=random*1664525u+1013904223u;bits=(random&0x807fffff)|(((random>>24)%254)<<23);y=Bits(bits);
       random=random*1664525u+1013904223u;bits=(random&0x807fffff)|(((random>>24)%254)<<23);z=Bits(bits);}
     a._41=x;a._42=y;a._43=z;
     if(scenario.family==7){a._11=0.5f;a._12=-0.25f;a._13=0.125f;a._21=0.75f;a._22=1.25f;a._23=-0.5f;a._31=-0.625f;a._32=0.375f;a._33=1.5f;}
   }
 }
 if(scenario.parents==1){Node(0)->parent=Node(3);Node(1)->parent=Node(3);Node(2)->parent=Node(3);}
 if(scenario.parents==2){Node(0)->parent=Node(2);Node(2)->parent=Node(3);Node(1)->parent=Node(3);}
 if(scenario.parents==3){Node(1)->parent=Node(0);Node(2)->parent=Node(1);}
 initial=world;
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;
 if(code!=EXCEPTION_ACCESS_VIOLATION && !(code>=EXCEPTION_FLT_DENORMAL_OPERAND&&code<=EXCEPTION_FLT_UNDERFLOW))return EXCEPTION_CONTINUE_SEARCH;
 observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.kind=e->ExceptionRecord->ExceptionInformation[0];observed.address=e->ExceptionRecord->ExceptionInformation[1];}
 observed.faultFp=e->ContextRecord->FloatSave.StatusWord&63;observed.faultControl=e->ContextRecord->FloatSave.ControlWord;
 if(code!=EXCEPTION_ACCESS_VIOLATION)_clearfp();return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail){
 memset(&observed,0,sizeof(observed));Prepare();observed.result=-99;
 SoultreeObject* from=scenario.invalid==1?0:Node(0);SoultreeObject* to=scenario.invalid==2?0:Node(1);
 SoultreeObject* frame=scenario.frame==0?0:scenario.frame==1?Node(2):scenario.frame==2?Node(0):Node(1);
 if(scenario.invalid==4)to=from;
 if(scenario.invalid==5)frame=(SoultreeObject*)1;
 if(scenario.guardTarget==1)from=(SoultreeObject*)(guard+4096-scenario.validBytes);
 if(scenario.guardTarget==2)frame=(SoultreeObject*)(guard+4096-scenario.validBytes);
 union {int(SteeringControl::*method)(SoultreeObject*,SoultreeObject*,SoultreeObject*);DWORD address;}u;
 u.method=&SteeringControl::SetAxisFromPoints;if(retail)u.address=0x504d30;
 _fpreset();_control87(scenario.rounding|scenario.precision|scenario.mask,_MCW_RC|_MCW_PC|_MCW_EM);
 __try{observed.result=(Steering()->*u.method)(from,to,frame);}__except(Filter(GetExceptionInformation())){}
 observed.control=_control87(0,0);observed.status=_status87();observed.state=world;memcpy(observed.guard,guard,4096);_fpreset();
}
static void ResetScenario(){memset(&scenario,0,sizeof(scenario));scenario.mask=_MCW_EM;}
static void Compare(){Run(1);expected=observed;Run(0);++checks;
 if(memcmp(&observed,&expected,sizeof(observed))){unsigned off=0;BYTE*a=(BYTE*)&expected,*b=(BYTE*)&observed;while(a[off]==b[off])++off;
   printf("Mismatch %d offset%u %02x/%02x seed%u family%d parents%d frame%d caches%x alias%d invalid%d guard%d/%d RC%x PC%x result%d/%d exception%x/%x fp%x/%x\n",checks,off,a[off],b[off],scenario.seed,scenario.family,scenario.parents,scenario.frame,scenario.caches,scenario.alias,scenario.invalid,scenario.guardTarget,scenario.validBytes,scenario.rounding,scenario.precision,expected.result,observed.result,expected.exception,observed.exception,expected.status,observed.status);ExitProcess(2);}
}
static int MapReference(const char* path){DWORD old;BYTE*mapped=(BYTE*)0x400000;
 if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE*f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;
 BYTE*raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
 IMAGE_DOS_HEADER*dos=(IMAGE_DOS_HEADER*)raw;if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
 IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 IMAGE_SECTION_HEADER*sections=IMAGE_FIRST_SECTION(pe);DWORD offset=(BYTE*)sections-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(*sections))return 0;
 memset(mapped,0,pe->OptionalHeader.SizeOfImage);memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
 for(int n=0;n<pe->FileHeader.NumberOfSections;++n){IMAGE_SECTION_HEADER&s=sections[n];if(s.PointerToRawData>(DWORD)size||s.SizeOfRawData>(DWORD)size-s.PointerToRawData||s.VirtualAddress>pe->OptionalHeader.SizeOfImage||s.SizeOfRawData>pe->OptionalHeader.SizeOfImage-s.VirtualAddress)return 0;memcpy(mapped+s.VirtualAddress,raw+s.PointerToRawData,s.SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 3;
 guard=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&old))return 4;
 if(sizeof(SoultreeObject)!=0x1a4||sizeof(SteeringControl)!=24||offsetof(SteeringControl,axis)!=12||offsetof(SoultreeObject,worldValid)!=0x138)return 5;
 unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},pc[]={_PC_64,_PC_53,_PC_24};int caches[]={0,15,5,10}; int family, parents, frame, rounding, precision;
 for(int seed=0;seed<16;++seed)for(family=0;family<8;++family)for(parents=0;parents<4;++parents)for(frame=0;frame<4;++frame)for(int cache=0;cache<4;++cache)for(rounding=0;rounding<4;++rounding)for(precision=0;precision<3;++precision){
   ResetScenario();scenario.seed=seed;scenario.family=family;scenario.parents=parents;scenario.frame=frame;scenario.caches=caches[cache];scenario.rounding=rc[rounding];scenario.precision=pc[precision];Compare();}
 for(int alias=1;alias<=3;++alias)for(frame=0;frame<4;++frame)for(parents=0;parents<4;++parents)for(family=0;family<8;++family)for(precision=0;precision<3;++precision){
   ResetScenario();scenario.seed=family*17+alias;scenario.alias=alias;scenario.frame=frame;scenario.parents=parents;scenario.family=family;scenario.precision=pc[precision];Compare();}
 for(int invalid=1;invalid<=5;++invalid)for(frame=0;frame<4;++frame)for(parents=0;parents<4;++parents)for(family=0;family<8;++family){
   ResetScenario();scenario.seed=family*17+invalid;scenario.invalid=invalid;scenario.frame=frame;scenario.parents=parents;scenario.family=family;scenario.precision=_PC_64;Compare();}
 const int edges[]={0,1,2,3,4,7,8,11,12,0xe8,0xec,0x100,0x12c,0x130,0x134,0x138,0x13c};
 for(int target=1;target<=3;++target)for(int edge=0;edge<(target==3?9:17);++edge)for(rounding=0;rounding<4;++rounding){
   ResetScenario();scenario.family=1;scenario.frame=1;scenario.guardTarget=target;scenario.validBytes=edges[edge];scenario.precision=_PC_64;scenario.rounding=rc[rounding];Compare();}
 const unsigned unmask[]={_EM_INVALID,_EM_ZERODIVIDE,_EM_OVERFLOW,_EM_UNDERFLOW,_EM_INEXACT,_EM_DENORMAL};
 for(int mask=0;mask<6;++mask)for(family=0;family<8;++family)for(rounding=0;rounding<4;++rounding)for(precision=0;precision<3;++precision){
   ResetScenario();scenario.seed=7;scenario.family=family;scenario.frame=1;scenario.caches=15;scenario.rounding=rc[rounding];scenario.precision=pc[precision];scenario.mask=_MCW_EM&~unmask[mask];Compare();}
 // Independent simple transforms and partial writes, outside the differential oracle.
 for(family=1;family<=9;++family){if(family!=1&&family!=8&&family!=9)continue;
   ResetScenario();scenario.family=family;scenario.frame=1;scenario.precision=_PC_64;Compare();
   Vec3& axis=Steering()->axis;if(observed.exception||observed.result!=1||axis.x!=(family==8?0.0f:1.0f)||axis.y!=0||axis.z!=(family==8?-1.0f:0.0f)||!Node(0)->worldValid||!Node(1)->worldValid||!Node(2)->worldValid)ExitProcess(90);}
 ResetScenario();scenario.family=1;scenario.frame=0;scenario.precision=_PC_64;Compare();
 if(observed.exception||observed.result||memcmp(world.steering,initial.steering,sizeof(world.steering))||!Node(0)->worldValid||!Node(1)->worldValid||Node(2)->worldValid)ExitProcess(91);
 ResetScenario();scenario.family=1;scenario.frame=0;scenario.invalid=3;scenario.precision=_PC_64;Compare();if(observed.exception||observed.result)ExitProcess(92);
 for(int valid=0;valid<=8;valid+=4){ResetScenario();scenario.family=1;scenario.frame=1;scenario.precision=_PC_64;scenario.guardTarget=3;scenario.validBytes=valid;Compare();
   if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)(guard+4096))ExitProcess(93);
   const DWORD values[]={0x3f800000,0,0};if(valid&&memcmp(guard+4096-valid,values,valid))ExitProcess(94);}
 printf("SteeringAxis: %d full-state, cache and FP scenarios match\n",checks);return 0;
}
