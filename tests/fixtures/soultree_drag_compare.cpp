// Validation only: complete retail slot 6 versus separately compiled C++.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "soultree/SoultreePhysicsBaseObjectCore.h"
#include "math/FastMath.h"
void InitFastSqrtTable();
static BYTE referenceSpace[0x800000]={1};
struct World { BYTE guard[16],object[0x250],tail[16];float output[4],budget; };
struct Observation { World state;DWORD exception,kind,address,fp,control,faultFp;BYTE page[0x300];DWORD readOnlyBudget; };
struct Scenario { float previous,speed,drag,mass,time,budget,v[3];int alias,invalid,round,precision,seed; };
static BYTE* faultPage;static float* readOnlyBudget;
static World world;static Observation observed,expected;static Scenario scenario;static int checks;
static DWORD& Word(int offset){return *(DWORD*)(world.object+offset);}
static float& Field(int offset){return *(float*)(world.object+offset);}
static float Bits(DWORD x){float f;memcpy(&f,&x,4);return f;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 BYTE* call=(BYTE*)0x40b4b3;if(*call!=0xe8||(DWORD)(call+5)+*(DWORD*)(call+1)!=0x460b50)return 0;*(DWORD*)(call+1)=(DWORD)FastSqrt-(DWORD)(call+5);
 return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS* e){DWORD c=e->ExceptionRecord->ExceptionCode;if(c!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=c;observed.kind=e->ExceptionRecord->ExceptionInformation[0];observed.address=e->ExceptionRecord->ExceptionInformation[1];observed.faultFp=e->ContextRecord->FloatSave.StatusWord&0x3f;return EXCEPTION_EXECUTE_HANDLER;}
class RetailCall {public:void Call(Vec3*,float*);};
static void Run(int retail){memset(&observed,0,sizeof(observed));for(int i=0;i<sizeof(world);++i)((BYTE*)&world)[i]=(BYTE)(i*37+scenario.seed*13);
 Field(0xb8)=scenario.previous;Field(0xbc)=scenario.speed;Field(0x148)=scenario.drag;Field(0x158)=scenario.mass;Field(0x13c)=scenario.time;
 for(i=0;i<3;++i){Field(0x64+i*4)=scenario.v[i];world.output[i]=(float)(i+1);}world.output[3]=19;world.budget=scenario.budget;
 for(i=0;i<0x1000;++i)faultPage[i]=(BYTE)(i*19+scenario.seed);
 for(i=0;i<4;++i)((float*)(faultPage+0xff0))[i]=(float)(i+1);
 DWORD protection;VirtualProtect(readOnlyBudget,4096,PAGE_READWRITE,&protection);*readOnlyBudget=scenario.budget;
 Vec3* out=(Vec3*)world.output;float* budget=&world.budget;SoultreePhysicsBaseObject* object=(SoultreePhysicsBaseObject*)world.object;
 static const int offsets[]={0x64,0xb8,0xbc,0x13c,0x148,0x158,0x1ac,0x1b0,0x1b4,0x1b8,0x1bc,0x1c0};
 if(scenario.alias>=1&&scenario.alias<=12)budget=(float*)(world.object+offsets[scenario.alias-1]);
 if(scenario.alias>=13&&scenario.alias<=24)out=(Vec3*)(world.object+offsets[scenario.alias-13]);
 if(scenario.alias>=25&&scenario.alias<=27)budget=world.output+scenario.alias-25;
 if(scenario.alias>=28&&scenario.alias<=30){out=(Vec3*)(world.object+0x1ac+4*(scenario.alias-28));budget=(float*)(world.object+0x1ac+4*(scenario.alias-28));}
 if(scenario.invalid==1)out=(Vec3*)1;if(scenario.invalid==2)budget=(float*)1;if(scenario.invalid==3){out=(Vec3*)1;budget=(float*)1;}if(scenario.invalid==4)object=0;
 if(scenario.invalid>=5&&scenario.invalid<=7)out=(Vec3*)(faultPage+4096-(scenario.invalid-4)*4);
 if(scenario.invalid==8){budget=readOnlyBudget;VirtualProtect(readOnlyBudget,4096,PAGE_READONLY,&protection);}
 if(scenario.invalid>=9){static const int lengths[]={0x64,0xb8,0xbc,0x1ac,0x1b0,0x1b4,0x1b8,0x1bc,0x1c0};int n=lengths[scenario.invalid-9];object=(SoultreePhysicsBaseObject*)(faultPage+4096-n);memcpy(object,world.object,n);}
 _fpreset();_controlfp(scenario.round|scenario.precision,_MCW_RC|_MCW_PC);
 __try{if(retail){union{void(RetailCall::*member)(Vec3*,float*);DWORD address;}call;call.address=0x40b410;(((RetailCall*)object)->*call.member)(out,budget);}else object->SoultreePhysicsBaseObject::UnknownVirtualSlot6(out,budget);}__except(Filter(GetExceptionInformation())){}
 observed.fp=_statusfp();observed.control=_controlfp(0,0);observed.state=world;memcpy(observed.page,faultPage+0xd00,0x300);memcpy(&observed.readOnlyBudget,readOnlyBudget,4);_fpreset();}
static void Fail(int code){printf("FAIL %d case%d alias%d invalid%d round%x prec%x seed%d inputs %.9g %.9g %.9g %.9g %.9g %.9g vec%.9g %.9g %.9g\n",code,checks,scenario.alias,scenario.invalid,scenario.round,scenario.precision,scenario.seed,scenario.previous,scenario.speed,scenario.drag,scenario.mass,scenario.time,scenario.budget,scenario.v[0],scenario.v[1],scenario.v[2]);exit(1);}
static void Compare(){Run(1);expected=observed;Run(0);if(memcmp(&observed,&expected,sizeof(observed))){BYTE* a=(BYTE*)&observed;BYTE* b=(BYTE*)&expected;for(int i=0;i<sizeof(observed);++i)if(a[i]!=b[i])printf("offset%x actual%02x retail%02x\n",i,a[i],b[i]);Fail(1);}++checks;}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.previous=1;scenario.speed=1;scenario.drag=1;scenario.mass=1;scenario.time=1;scenario.budget=1;scenario.v[0]=1;scenario.precision=_PC_64;scenario.seed=11;}
static DWORD rng=0x705123ab;static DWORD Random(){rng=rng*1664525+1013904223;return rng;}
int main(int argc,char** argv){if(argc!=2||!MapReference(argv[1]))return 3;InitFastSqrtTable();faultPage=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);readOnlyBudget=(float*)VirtualAlloc(0,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;if(!faultPage||!readOnlyBudget||!VirtualProtect(faultPage+4096,4096,PAGE_NOACCESS,&old))return 4;
 Base();Compare();if(world.output[0]!=0||world.output[1]!=2||world.output[2]!=3||world.budget!=0)Fail(90);
 Base();scenario.v[0]=3;scenario.mass=8;scenario.budget=8;Compare();if(world.output[0]!=-2||world.budget!=7.625f)Fail(94);
 Base();scenario.budget=0.5f;Compare();if(world.output[0]!=0.5f||world.budget!=0)Fail(91);
 Base();scenario.budget=-1;Compare();if(world.output[0]!=1||world.budget!=-1)Fail(92);
 Base();scenario.previous=0.001f;scenario.invalid=3;Compare();if(observed.exception||world.output[0]!=1)Fail(93);
 // Independent progressive-fault checks: the first output write remains visible.
 Base();scenario.invalid=5;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=0||observed.address!=(DWORD)(faultPage+4096)||*(float*)(faultPage+4092)!=3)Fail(95);
 Base();scenario.invalid=8;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)readOnlyBudget||world.output[0]!=1||*readOnlyBudget!=1)Fail(96);
 const float values[]={0,-0.0f,-1,0.0001f,0.001f,0.125f,0.5f,1,2,3,17,128};
 for(int a=0;a<12;++a)for(int b=0;b<12;++b)for(int c=0;c<12;++c)for(int branch=0;branch<4;++branch){Base();scenario.speed=values[a];scenario.budget=values[b];scenario.v[0]=values[c];scenario.v[1]=branch==1?0.75f:0;scenario.v[2]=branch==2?-0.25f:0;if(branch==3){scenario.mass=3;scenario.time=7;}Compare();}
 for(int alias=0;alias<=30;++alias)for(int path=0;path<7;++path)for(int invalid=0;invalid<=17;++invalid){Base();scenario.alias=alias;scenario.invalid=invalid;if(path==1)scenario.budget=0.25f;if(path==2)scenario.previous=0;if(path==3)scenario.v[0]=0;if(path==4)scenario.speed=-1;if(path==5){scenario.v[0]=2;scenario.mass=0;}if(path==6)scenario.budget=8;Compare();}
 const DWORD special[]={0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x3a83126e,0x3a83126f,0x3a831270,0x3f7fffff,0x3f800000,0x3f800001,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc12345,0x7f812345};
 const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},precision[]={_PC_64,_PC_53,_PC_24};
 for(int r=0;r<4;++r)for(int p=0;p<3;++p)for(int s=0;s<18;++s)for(int field=0;field<9;++field){Base();scenario.round=rounds[r];scenario.precision=precision[p];((float*)&scenario)[field]=Bits(special[s]);Compare();}
 for(int n=0;n<32768;++n){Base();scenario.seed=n;scenario.round=rounds[n%4];scenario.precision=precision[(n/4)%3];for(int f=0;f<9;++f)((float*)&scenario)[f]=(float)((int)(Random()%8193)-4096)/((float)(1+(Random()%128)));scenario.previous=1;if(n%31==0)scenario.alias=(n/31)%31;Compare();}
 for(int raw=0;raw<32768;++raw){Base();scenario.seed=raw;scenario.round=rounds[raw%4];scenario.precision=precision[(raw/4)%3];for(int f=0;f<9;++f){DWORD bits=Random();if((bits&0x7f800000)==0x7f800000)bits^=0x00800000;((float*)&scenario)[f]=Bits(bits);}if(raw%2==0)scenario.previous=1;Compare();}
 printf("SoultreeDrag: %d real-math, alias, floating-point and fault scenarios match\n",checks);return 0;}
