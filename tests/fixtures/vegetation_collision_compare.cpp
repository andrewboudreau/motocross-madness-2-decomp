#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "../../src/reconstructed/EcoSystem.h"
static BYTE referenceSpace[0x800000]={1};
static int MapReference(const char* path) {
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
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);
}

static void Stop(int x){fflush(stdout);ExitProcess(x);}
// Validation only: complete placement plus actual hull transform and strict math.
extern "C" void RunTransformSource(void*,const void*);
void InitFastSqrtTable();extern unsigned long g_FastSqrtTable[256];
struct World { BYTE eco[0x600],def[0x220],vegetation[0x40],object[0xc0],shape[0x220],tree[0x24]; CollisionObject* objects[2]; };
static World world,initial,oracle;
struct Result { World world;BYTE page[4096];DWORD result,exception,kind,address,status,control; };
static Result actual,expected;
static BYTE* page;static int checks,faults;static DWORD cw;
static DWORD& D(BYTE* p,int offset){return *(DWORD*)(p+offset);}
static float& F(BYTE* p,int offset){return *(float*)(p+offset);}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 actual.exception=e->ExceptionRecord->ExceptionCode;actual.kind=e->ExceptionRecord->ExceptionInformation[0];actual.address=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Setup(int type,int parameter,int coord,int units,int available,int alias){
 memset(&world,0x69,sizeof(world));
 D(world.eco,0x58+7*4)=(DWORD)world.def;F(world.eco,0x5a8)=units*0.25f;
 D(world.def,0x20c)=(DWORD)world.objects;world.objects[0]=(CollisionObject*)world.object;world.objects[1]=(CollisionObject*)world.object;
 F(world.def,0x184)=1;F(world.def,0x180)=2;F(world.def,0x188)=4;
 F(world.def,0x190)=1;F(world.def,0x18c)=2;F(world.def,0x194)=4;
 Vegetation* v=(Vegetation*)world.vegetation;v->definitionIndex=7;v->heightParam=(BYTE)(255-parameter);v->radiusParam=(BYTE)parameter;
 v->quantizedPosition.x=(unsigned short)coord;v->quantizedPosition.y=(unsigned short)(coord+1);v->quantizedPosition.z=(unsigned short)(coord+2);
 D(world.object,0x50)=type;D(world.object,0x54)=(DWORD)world.shape;
 if(available>=0)D(world.object,0x54)=(DWORD)(page+4096-available);
 if(alias)D(world.object,0x54)=(DWORD)(world.object+0x54-(type==4?0x14:0x20));
 initial=world;
}
static void Run(int retail){
 world=initial;for(int i=0;i<4096;i++)page[i]=(BYTE)(i*13+7);memset(&actual,0,sizeof(actual));
 g_EcoSystem=(EcoSystem*)world.eco;*(EcoSystem**)0x59aebc=g_EcoSystem;
 _fpreset();_controlfp(cw,_MCW_PC|_MCW_RC|_MCW_EM);
 typedef CollisionObject* (Vegetation::*Fn)(int);union {Fn fn;DWORD raw;} f;
 if(sizeof(Fn)!=4)Stop(92);f.fn=&Vegetation::GetCollisionObject;if(retail)f.raw=0x4570a0;
 __try{actual.result=(DWORD)(((Vegetation*)world.vegetation)->*f.fn)(1);}__except(Filter(GetExceptionInformation())){}
 actual.status=_statusfp();actual.control=_controlfp(0,0);actual.world=world;memcpy(actual.page,page,4096);_fpreset();_controlfp(cw,_MCW_PC|_MCW_RC|_MCW_EM);
}
static float Parameter(int p){return p<128?2.0f-p/128.0f:2.0f+2.0f*(255-p)/128.0f;}
static void Check(int type,int parameter,int coord,int units,int available,int alias){
 Setup(type,parameter,coord,units,available,alias);Run(1);expected=actual;Run(0);
 if(memcmp(&actual,&expected,sizeof(actual))){
  BYTE*a=(BYTE*)&actual,*b=(BYTE*)&expected;int off=0;while(off<sizeof(actual)&&a[off]==b[off])off++;
  printf("FAIL %d type%d p%d c%d u%d available%d alias%d cw%08x off%d actual%02x expected%02x av%08x/%08x\n",checks,type,parameter,coord,units,available,alias,cw,off,a[off],b[off],actual.address,expected.address);Stop(3);}
 if(available<0&&!alias){
  oracle=initial;
  if(type==3||type==4){int r=type==4?0x14:0x20,t=type==4?0x4c:0x58;
   F(oracle.shape,r)=Parameter(parameter)*0.5f;F(oracle.shape,r+4)=Parameter(255-parameter)*0.5f;
   F(oracle.shape,t)=(unsigned short)coord*(units*0.25f);F(oracle.shape,t+4)=(unsigned short)(coord+1)*(units*0.25f);F(oracle.shape,t+8)=(unsigned short)(coord+2)*(units*0.25f);}
  if(actual.exception||actual.result!=(DWORD)world.object||memcmp(&actual.world,&oracle,sizeof(oracle)))Stop(5);
 } else {
  if(actual.exception!=EXCEPTION_ACCESS_VIOLATION||actual.kind!=1)Stop(6);faults++;
  if(alias){if(D(actual.world.object,0x54)!=0x3f800000||actual.address!=0x3f800000+(type==4?0x18:0x24))Stop(7);}
  else {
   BYTE prefix[4096];for(int i=0;i<4096;i++)prefix[i]=(BYTE)(i*13+7);
   int offs[5]={type==4?0x14:0x20,type==4?0x18:0x24,type==4?0x4c:0x58,type==4?0x50:0x5c,type==4?0x54:0x60};
   float vals[5]={Parameter(parameter)*0.5f,Parameter(255-parameter)*0.5f,(unsigned short)coord*(units*0.25f),(unsigned short)(coord+1)*(units*0.25f),(unsigned short)(coord+2)*(units*0.25f)};
   for(i=0;i<5;i++){if(offs[i]+4>available)break;memcpy(prefix+4096-available+offs[i],&vals[i],4);}
   if(memcmp(prefix,actual.page,4096)){int k=0;while(k<4096&&prefix[k]==actual.page[k])k++;printf("PREFIX t%d a%d k%d exp%02x actual%02x addr%08x page%08x\n",type,available,k,prefix[k],actual.page[k],actual.address,(DWORD)page);Stop(8);}
  }
 }
 checks++;
}

static void Identity(BYTE* p){memset(p,0,64);for(int i=0;i<4;i++)F(p,i*20)=1.0f;}
static void HullSetup(int n){
 Setup(0,n%256,n%32,4,-1,0);
 Identity(world.shape+0x88);Identity(world.shape+0xc8);
 F(world.shape,0xc8+48)=(n%7)*0.5f;F(world.shape,0xc8+52)=(n%5)*0.25f;F(world.shape,0xc8+56)=(n%3)*0.125f;
 D(world.shape,0x188)=(DWORD)world.tree;
 for(int i=0;i<3;i++){F(world.tree,4+i*4)=i*0.5f;F(world.tree,0x10+i*4)=1+i*0.5f;}
 if(n>=256){
  DWORD rng=n;for(int k=0;k<12;k++){rng=rng*1664525+1013904223;int row=k/4,col=k%4;if(col<3){F(world.shape,0x88+(row*4+col)*4)=((int)((rng>>8)%2001)-1000)/733.0f;rng=rng*1664525+1013904223;F(world.shape,0xc8+(row*4+col)*4)=((int)((rng>>8)%2001)-1000)/911.0f;}}
 }
 initial=world;
}
static int hullAvailable=-1;
static void HullRun(int retail,int outer){
 world=initial;memset(&actual,0,sizeof(actual));for(int i=0;i<4096;i++)page[i]=(BYTE)(i*13+7);
 if(hullAvailable>=0)memcpy(page+4096-hullAvailable,initial.shape,hullAvailable);
 g_EcoSystem=(EcoSystem*)world.eco;*(EcoSystem**)0x59aebc=g_EcoSystem;
 _fpreset();_controlfp(cw,_MCW_PC|_MCW_RC|_MCW_EM);
 typedef void(CollisionObject::*Set)(const Matrix4*);union{Set fn;DWORD words[2];} st;if(sizeof(Set)!=8)Stop(96);memset(&st,0,sizeof(st));st.words[0]=0x435830;
 typedef CollisionObject*(Vegetation::*Get)(int);union{Get fn;DWORD raw;} g;g.fn=&Vegetation::GetCollisionObject;if(retail)g.raw=0x4570a0;
 BYTE matrix[64];Identity(matrix);F(matrix,48)=3.0f;F(matrix,52)=4.0f;F(matrix,56)=5.0f;
 if(outer==2){memcpy(matrix,world.shape+0xc8,64);Vegetation*v=(Vegetation*)world.vegetation;F(matrix,52)=v->quantizedPosition.y;F(matrix,48)=v->quantizedPosition.x;F(matrix,56)=v->quantizedPosition.z;F(world.shape,0xfc)=F(matrix,52);F(world.shape,0x100)=F(matrix,56);}
 __try{
  if(outer==1)actual.result=(DWORD)(((Vegetation*)world.vegetation)->*g.fn)(0);
  else {if(retail)(((CollisionObject*)world.object)->*st.fn)((const Matrix4*)matrix);else RunTransformSource(world.object,matrix);}
 }__except(Filter(GetExceptionInformation())){}
 actual.status=_statusfp();actual.control=_controlfp(0,0);actual.world=world;memcpy(actual.page,page,4096);_fpreset();_controlfp(cw,_MCW_PC|_MCW_RC|_MCW_EM);
}
static void HullCheck(int n,int outer){
 HullSetup(n);HullRun(1,outer);expected=actual;HullRun(0,outer);
 if(memcmp(&actual,&expected,sizeof(actual))){BYTE*a=(BYTE*)&actual,*b=(BYTE*)&expected;int off=0;while(off<sizeof(actual)&&a[off]==b[off])off++;printf("HULL FAIL n%d outer%d offset%d actual%02x expected%02x words%08x/%08x cw%08x\n",n,outer,off,a[off],b[off],*(DWORD*)(a+(off&~3)),*(DWORD*)(b+(off&~3)),cw);Stop(9);}
 if(actual.exception||(outer==1&&actual.result!=(DWORD)world.object))Stop(10);
 // Independent identity-rotation translation and previous-body propagation.
 int x=outer?n%32:3,y=outer?(n%32)+1:4,z=outer?(n%32)+2:5;
 if(F(actual.world.shape,0xc8+48)!=x||F(actual.world.shape,0xc8+52)!=y||F(actual.world.shape,0xc8+56)!=z)Stop(11);
 if(memcmp(actual.world.shape+0x108,initial.shape+0xc8,48))Stop(12);
 if(outer){if(F(actual.world.shape,0x108+52)!=y||F(actual.world.shape,0x108+56)!=z)Stop(13);}
 else if(memcmp(actual.world.shape+0x108,initial.shape+0xc8,64))Stop(14);
 checks++;
}

static void HullGuard(int n,int available){
 HullSetup(n);hullAvailable=available;D(initial.object,0x54)=(DWORD)(page+4096-available);
 HullRun(1,1);expected=actual;HullRun(0,1);
 if(memcmp(&actual,&expected,sizeof(actual))){printf("HULL GUARD DIFF n%d available%d addr%08x/%08x\n",n,available,actual.address,expected.address);Stop(15);}
 if(actual.exception!=EXCEPTION_ACCESS_VIOLATION||actual.kind!=1||actual.address!=(DWORD)(page+4096))Stop(16);
 BYTE prefix[4096];for(int i=0;i<4096;i++)prefix[i]=(BYTE)(i*13+7);BYTE* shape=prefix+4096-available;memcpy(shape,initial.shape,available);
 F(shape,0xfc)=(n%32)+1;F(shape,0x100)=(n%32)+2;
 for(i=0;i<16;i++){if(0x108+i*4+4>available)break;memcpy(shape+0x108+i*4,shape+0xc8+i*4,4);}
 if(memcmp(prefix,actual.page,4096)||memcmp(&actual.world,&initial,sizeof(initial)))Stop(17);
 hullAvailable=-1;checks++;faults++;
}
int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 90;
 typedef char Layout[sizeof(Vegetation)==0x1c&&offsetof(Vegetation,definitionIndex)==0x12&&offsetof(CollisionObject,shapeData)==0x54?1:-1];
 page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 93;
 InitFastSqrtTable();memcpy((void*)0x5dafe0,g_FastSqrtTable,1024);
 int pcs[3]={_PC_24,_PC_53,_PC_64};int rcs[4]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++){
  cw=pcs[pc]|rcs[rc]|_MCW_EM;
  for(int n=0;n<512;n++){HullCheck(n,0);HullCheck(n,2);HullCheck(n,1);}
  for(int g=0x108;g<0x148;g++)HullGuard(g%32,g);
  for(int t=3;t<=4;t++)for(int p=0;p<256;p++)for(int u=-1;u<=1;u++)Check(t,p,p*257,u,-1,0);
  for(t=1;t<=2;t++)Check(t,128,65535,3,-1,0);
  for(t=3;t<=4;t++){int end=t==4?0x58:0x64;for(int a=0;a<end;a++)Check(t,0,4,4,a,0);Check(t,0,4,4,-1,1);}
 }
 printf("Vegetation with hull paths: %d cases, %d access faults\n",checks,faults);return 0;
}
