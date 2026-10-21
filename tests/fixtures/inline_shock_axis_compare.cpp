// Complete InlineShock axis setup with actual source scene/cache helpers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "math/Math3D.h"
#include "../../src/reconstructed/Suspension.h"
static BYTE referenceSpace[0x800000];
struct World { BYTE nodes[4][0x1a4+32]; BYTE shock[0xd8+32]; };
static World world,initial;
struct Observation { DWORD exception,kind,address,faultFp,faultControl;unsigned control,status;World state;BYTE guard[4096]; };
static Observation observed,expected;
struct Scenario { unsigned seed;int family,parents,caches,minimum,alias,invalid,guardTarget,validBytes;unsigned rounding,precision; };
static Scenario scenario;
static BYTE* guard;
static int checks;
static SoultreeObject* Node(int n){return (SoultreeObject*)(world.nodes[n]+16);}
static InlineShock* Shock(){
 if(scenario.invalid==3)return 0;
 if(scenario.guardTarget==3)return (InlineShock*)(guard+4096-scenario.validBytes);
 if(scenario.alias==1)return (InlineShock*)((BYTE*)Node(0)+0x28);
 if(scenario.alias==2)return (InlineShock*)((BYTE*)Node(0)+0x68);
 if(scenario.alias==3)return (InlineShock*)((BYTE*)Node(1)+0x28);
 return (InlineShock*)(world.shock+16);
}
static void Identity(Matrix4& m){memset(&m,0,sizeof(m));m._11=m._22=m._33=m._44=1.0f;}
static float Bits(unsigned bits){float f;memcpy(&f,&bits,4);return f;}
static void Prepare(){
 unsigned random=scenario.seed+1;for(unsigned i=0;i<sizeof(world);++i){random=random*1664525u+1013904223u;((BYTE*)&world)[i]=(BYTE)(random>>24);}
 memset(guard,0x6a,4096);
 for(int n=0;n<4;++n){SoultreeObject* node=Node(n);Identity(node->localMatrix);Identity(node->worldMatrix);node->parent=0;node->worldValid=(scenario.caches>>n)&1;
   Matrix4* matrices[]={&node->localMatrix,&node->worldMatrix};
   for(int m=0;m<2;++m){Matrix4& a=*matrices[m];float x,y,z;unsigned seed=scenario.seed+n*73+m*19;
     x=((int)(seed%97)-47)*0.125f;y=((int)(seed%59)-23)*0.0625f;z=((int)(seed%83)-31)*0.25f;
     if(scenario.family==1){x=n==0?3.0f:0;y=z=0;}
     if(scenario.family==2){x=y=z=0;}
     if(scenario.family==8){x=n==0?1.0f:0;y=z=0;}
     if(scenario.family==3){x=n?0:Bits(0x3f800001);y=n?0:Bits(0x33800000);z=n?0:Bits(0x33000000);}
     if(scenario.family==4){x=(n&1?-1:1)*Bits(0x71800000);y=(n&1?1:-1)*Bits(0x6f123456);z=(n&1?-1:1)*Bits(0x70fedcba);}
     if(scenario.family==5){x=Bits(1+(seed&255));y=Bits(0x80000100+(seed&511));z=Bits(0x00001000+(seed&1023));}
     if(scenario.family==6){random=random*1664525u+1013904223u;unsigned bits=(random&0x807fffff)|(((random>>24)%254)<<23);x=Bits(bits);
       random=random*1664525u+1013904223u;bits=(random&0x807fffff)|(((random>>24)%254)<<23);y=Bits(bits);
       random=random*1664525u+1013904223u;bits=(random&0x807fffff)|(((random>>24)%254)<<23);z=Bits(bits);}
     a._41=x;a._42=y;a._43=z;
     if(scenario.family==7){a._11=0.5f;a._12=-0.25f;a._13=0.125f;a._21=0.75f;a._22=1.25f;a._23=-0.5f;a._31=-0.625f;a._32=0.375f;a._33=1.5f;}
   }
 }
 if(scenario.parents==1){Node(0)->parent=Node(3);Node(1)->parent=Node(3);}
 if(scenario.parents==2){Node(0)->parent=Node(2);Node(2)->parent=Node(3);Node(1)->parent=Node(3);}
 if(scenario.parents==3){Node(1)->parent=Node(0);Node(0)->parent=Node(3);}
 const float minima[]={-1.0f,0.0f,0.5f,3.0f,10.0f,3.402823466e+38f};
 if(!scenario.guardTarget&&scenario.invalid!=3)Shock()->minLength=minima[scenario.minimum];
 initial=world;
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;
 if(code!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 observed.exception=code;observed.kind=e->ExceptionRecord->ExceptionInformation[0];observed.address=e->ExceptionRecord->ExceptionInformation[1];
 observed.faultFp=e->ContextRecord->FloatSave.StatusWord&63;observed.faultControl=e->ContextRecord->FloatSave.ControlWord;
 return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail){
 memset(&observed,0,sizeof(observed));Prepare();
 SoultreeObject* from=scenario.invalid==1?0:Node(0);SoultreeObject* to=scenario.invalid==2?0:Node(1);
 if(scenario.invalid==4)to=from;
 if(scenario.guardTarget==1)from=(SoultreeObject*)(guard+4096-scenario.validBytes);
 if(scenario.guardTarget==2)to=(SoultreeObject*)(guard+4096-scenario.validBytes);
 union {void(InlineShock::*method)(ShockNode*,ShockNode*);DWORD address;}u;
 u.method=&InlineShock::UpdateAxis;if(retail)u.address=0x4f9f90;
 _fpreset();_control87(scenario.rounding|scenario.precision|_MCW_EM,_MCW_RC|_MCW_PC|_MCW_EM);
 __try{(Shock()->*u.method)((ShockNode*)from,(ShockNode*)to);}__except(Filter(GetExceptionInformation())){}
 observed.control=_control87(0,0);observed.status=_status87();observed.state=world;memcpy(observed.guard,guard,4096);_fpreset();
}
static void ResetScenario(){memset(&scenario,0,sizeof(scenario));}
static void Compare(){Run(1);expected=observed;Run(0);++checks;
 if(memcmp(&observed,&expected,sizeof(observed))){unsigned off=0;BYTE*a=(BYTE*)&expected,*b=(BYTE*)&observed;while(a[off]==b[off])++off;
   printf("Mismatch %d offset%u %02x/%02x seed%u family%d parents%d caches%x min%d alias%d invalid%d guard%d/%d RC%x PC%x exception%x/%x fp%x/%x\n",checks,off,a[off],b[off],scenario.seed,scenario.family,scenario.parents,scenario.caches,scenario.minimum,scenario.alias,scenario.invalid,scenario.guardTarget,scenario.validBytes,scenario.rounding,scenario.precision,expected.exception,observed.exception,expected.status,observed.status);ExitProcess(2);}
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
 if(sizeof(SoultreeObject)!=0x1a4||sizeof(InlineShock)!=0xd8||sizeof(ShockVec3)!=12||sizeof(Vec3)!=12||
    offsetof(InlineShock,axis)!=0xc0||offsetof(InlineShock,minLength)!=0xcc||offsetof(InlineShock,anchorNode)!=0xd0||offsetof(InlineShock,axisLength)!=0xd4||offsetof(InlineShock,sceneNode)!=0x44)return 5;
 unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},pc[]={_PC_64,_PC_53,_PC_24};int caches[]={0,15,5,10};int family,parents,rounding,precision,minimum;
 for(int seed=0;seed<16;++seed)for(family=0;family<8;++family)for(parents=0;parents<4;++parents)for(int cache=0;cache<4;++cache)for(minimum=0;minimum<6;++minimum)for(rounding=0;rounding<4;++rounding)for(precision=0;precision<3;++precision){
   ResetScenario();scenario.seed=seed;scenario.family=family;scenario.parents=parents;scenario.caches=caches[cache];scenario.minimum=minimum;scenario.rounding=rc[rounding];scenario.precision=pc[precision];Compare();}
 for(int alias=1;alias<=3;++alias)for(parents=0;parents<4;++parents)for(family=0;family<8;++family)for(minimum=0;minimum<6;++minimum)for(precision=0;precision<3;++precision){
   ResetScenario();scenario.seed=family*17+alias;scenario.alias=alias;scenario.parents=parents;scenario.family=family;scenario.minimum=minimum;scenario.precision=pc[precision];Compare();}
 for(int invalid=1;invalid<=4;++invalid)for(parents=0;parents<4;++parents)for(family=0;family<8;++family){
   ResetScenario();scenario.seed=family*17+invalid;scenario.invalid=invalid;scenario.parents=parents;scenario.family=family;scenario.precision=_PC_64;Compare();}
 const int nodeEdges[]={0,1,2,3,4,0xe8,0xec,0xf0,0xf4,0x128,0x12c,0x130,0x134,0x138,0x13c,0x140};
 const int shockEdges[]={0,0xc0,0xc1,0xc3,0xc4,0xc7,0xc8,0xcb,0xcc,0xcf,0xd0,0xd4,0xd5,0xd7,0xd8};
 for(int target=1;target<=3;++target)for(int edge=0;edge<(target==3?15:16);++edge)for(rounding=0;rounding<4;++rounding){
   ResetScenario();scenario.family=1;scenario.guardTarget=target;scenario.validBytes=target==3?shockEdges[edge]:nodeEdges[edge];scenario.precision=_PC_64;scenario.rounding=rc[rounding];Compare();}
 for(minimum=0;minimum<6;++minimum){ResetScenario();scenario.family=1;scenario.minimum=minimum;scenario.precision=_PC_64;Compare();
   const float values[]={-1.0f,0.0f,0.5f,3.0f,3.0f,3.0f};
   if(observed.exception||Shock()->axis.x!=1.0f||Shock()->axis.y!=0||Shock()->axis.z!=0||Shock()->axisLength!=3.0f||Shock()->minLength!=values[minimum]||!Node(0)->worldValid||!Node(1)->worldValid)ExitProcess(90);
   if(memcmp(world.shock,initial.shock,16+0xc0)||memcmp(world.shock+16+0xd8,initial.shock+16+0xd8,16))ExitProcess(91);
 }
 ResetScenario();scenario.family=8;scenario.minimum=4;scenario.precision=_PC_64;Compare();
 if(observed.exception||Shock()->axis.x!=1.0f||Shock()->axis.y!=0||Shock()->axis.z!=0||Shock()->axisLength!=1.0f||Shock()->minLength!=1.0f)ExitProcess(93);
 ResetScenario();scenario.family=1;scenario.guardTarget=3;scenario.validBytes=0xc4;scenario.precision=_PC_64;Compare();
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)(guard+4096)||memcmp(guard+4096-4,"\0\0\x40\x40",4))ExitProcess(92);
 printf("InlineShockAxis: %d complete-state/cache/FP scenarios match\n",checks);return 0;
}
