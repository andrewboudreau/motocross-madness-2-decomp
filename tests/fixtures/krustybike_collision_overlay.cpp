// Actual collision and overlay bodies; prepared storage, no object constructors.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "collision/CollisionShapeTests.h"
#include "../../src/reconstructed/DebugOverlay.h"
extern "C" void* QueryFrame();
extern "C" void* QueryAxes();
static BYTE referenceSpace[0x800000];
struct World { BYTE objects[2][0x200]; BYTE payload[2][0x220]; BYTE bounds[2][0x44]; BYTE overlay[0x3700]; };
static World world;
struct Observation { World world; BYTE frame[64], axes[180]; DWORD exception,kind,address,faultFp,faultControl; unsigned status,control; int result; };
static Observation got,want;
static int checks,kindA,kindB,family,seed,alias,fault,enabled,shown,startRow,limit,format,method;
static unsigned rounding,precision;
static CollisionObject* Object(int n) { return (CollisionObject*)(world.objects[n]+16); }
static DebugOverlay* Overlay() { return (DebugOverlay*)(world.overlay+16); }
static float Value(int n) { return ((seed*31+n*17)%49-24)*0.125f; }
static void Matrix(Matrix4* m,int n) {
 memset(m,0,64);m->_11=m->_22=m->_33=m->_44=1;
 if(family==1){m->_11=0;m->_12=1;m->_21=-1;m->_22=0;}
 if(family==2){m->_11=-1;m->_33=-1;}
 if(family==3){m->_12=0.25f;m->_23=-0.125f;m->_31=0.5f;}
 if(family==4){m->_11=m->_22=m->_33=0;}
 m->_41=n?Value(5):0;m->_42=n?Value(6):0;m->_43=n?Value(7):0;
 if(family==5&&n)m->_41=100;
}
static void Prepare() {
 memset(&world,0x95,sizeof(world));memset(QueryFrame(),0x36,64);memset(QueryAxes(),0x69,180);
 memset((void*)0x578ef0,0x36,64);memset((void*)0x578f50,0x69,180);
 for(int n=0;n<2;++n){
  CollisionObject* o=Object(n);o->shapeType=n?kindB:kindA;o->shape=world.payload[n]+16;
  CollisionBoxBounds* b=(CollisionBoxBounds*)(world.bounds[n]+16);memset(b,0,0x24);
  b->center.x=Value(n);b->center.y=Value(n+1);b->center.z=Value(n+2);
  b->halfExtents.x=0.5f+(seed%3);b->halfExtents.y=1;b->halfExtents.z=2;
  if(o->shapeType==0){CollisionHullBody*h=(CollisionHullBody*)o->shape;h->triangleTree=b;Matrix(&h->bodyTransform,n);}
  if(o->shapeType==1){CollisionModelBody*m=(CollisionModelBody*)o->shape;m->center=b->center;m->halfExtents=b->halfExtents;Matrix(&m->field_0x88,n);}
  if(o->shapeType==2){CollisionMeshBody*m=(CollisionMeshBody*)o->shape;m->field_0x04=b;Matrix(&m->field_0x08,n);}
 }
 if(alias==1){Object(1)->shapeType=Object(0)->shapeType;Object(1)->shape=Object(0)->shape;}
 if(fault==3)Object(0)->shape=0;
 if(fault==4)Object(1)->shape=0;
 if(fault==5&&kindA==0)((CollisionHullBody*)Object(0)->shape)->triangleTree=0;
 DebugOverlay*d=Overlay();d->isEnabled=(unsigned char)enabled;d->shownPage=shown;d->pageRowCount=startRow;d->rowLimit=limit;
 memset(d->rowText,0,sizeof(d->rowText));
 for(n=0;n<64;++n){strcpy(d->rows[n].text,"old text");d->rows[n].highlighted=7;d->rows[n].color=0x123456;}
}
static int Filter(EXCEPTION_POINTERS*e){
 if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 got.exception=e->ExceptionRecord->ExceptionCode;
 if(got.exception==EXCEPTION_ACCESS_VIOLATION){got.kind=e->ExceptionRecord->ExceptionInformation[0];got.address=e->ExceptionRecord->ExceptionInformation[1];}
 got.faultFp=e->ContextRecord->FloatSave.StatusWord&63;got.faultControl=e->ContextRecord->FloatSave.ControlWord;return EXCEPTION_EXECUTE_HANDLER;}
static void Run(int retail,int collision){
 memset(&got,0,sizeof(got));Prepare();_fpreset();_control87(rounding|precision|_MCW_EM,_MCW_RC|_MCW_PC|_MCW_EM);
 __try{
  if(collision){union{int(CollisionObject::*member)(CollisionObject*);DWORD words[2];}u;
   u.member=&CollisionObject::TestMeshBounds;
   if(sizeof(u.member)!=8||u.words[1]!=0)ExitProcess(84);
   if(retail)u.words[0]=0x4392c0;
   got.result=((fault==1?0:Object(0))->*u.member)(fault==2?0:(alias==2?Object(0):Object(1)));
  }else{
   union{void(DebugOverlay::*member)(int,const char*,...);DWORD address;}u;u.member=method?&DebugOverlay::Title:&DebugOverlay::Print;
   if(sizeof(u.member)!=4)ExitProcess(85);
   if(retail)u.address=method?0x447fa0:0x447f40;
   DebugOverlay*d=Overlay();
   switch(format){
   case 0:(d->*u.member)(0,"KrustyBike");break;
   case 1:(d->*u.member)(0,"Position/Heading=");break;
   case 2:(d->*u.member)(0,"%.2f, %.2f, %.2f / (%.0f)",Value(1),Value(2),Value(3),Value(4));break;
   case 3:(d->*u.member)(0,"Last TimeTo60 = %.2f  seconds",Value(5));break;
   case 4:(d->*u.member)(0,"TimeTo60 = %.2f  seconds",Value(6));break;
   case 5:(d->*u.member)(0,"old text");break;
   }
  }
 }__except(Filter(GetExceptionInformation())){}
 got.status=_status87();got.control=_control87(0,0);got.world=world;
 memcpy(got.frame,retail?(void*)0x578ef0:QueryFrame(),64);memcpy(got.axes,retail?(void*)0x578f50:QueryAxes(),180);_fpreset();
}
static void Compare(int collision){Run(1,collision);want=got;Run(0,collision);++checks;
 if(!fault||(fault==5&&kindA!=0)){
  if(want.exception||got.exception)ExitProcess(86);
 }else if(want.exception!=EXCEPTION_ACCESS_VIOLATION||got.exception!=EXCEPTION_ACCESS_VIOLATION)ExitProcess(87);
 if(memcmp(&got,&want,sizeof(got))){unsigned at=0;while(((BYTE*)&got)[at]==((BYTE*)&want)[at])++at;
 printf("Mismatch %d collision%d offset%u kinds%d/%d family%d seed%d alias%d fault%d overlay%d/%d/%d/%d/%d/%d fp%x/%x\n",checks,collision,at,kindA,kindB,family,seed,alias,fault,enabled,shown,startRow,limit,format,method,want.status,got.status);ExitProcess(2);}}
static void PatchCall(DWORD at,DWORD target,void* actual){if(*(BYTE*)at!=0xe8||at+5+*(DWORD*)(at+1)!=target)ExitProcess(4);*(DWORD*)(at+1)=(DWORD)actual-at-5;}
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
 PatchCall(0x447f0e,0x534b10,(void*)strncpy);PatchCall(0x447f79,0x535e71,(void*)vsprintf);PatchCall(0x447fd9,0x535e71,(void*)vsprintf);
 FlushInstructionCache(GetCurrentProcess(),0,0);
 if(offsetof(CollisionObject,shapeType)!=0x50||offsetof(CollisionObject,shape)!=0x54||sizeof(CollisionBoxBounds)!=0x24||offsetof(CollisionHullBody,triangleTree)!=0x188||offsetof(CollisionHullBody,bodyTransform)!=0xc8||offsetof(CollisionModelBody,center)!=0x18||offsetof(CollisionModelBody,field_0x88)!=0x88||offsetof(CollisionMeshBody,field_0x04)!=4||offsetof(CollisionMeshBody,field_0x08)!=8||sizeof(DebugOverlay)!=0x36e0||offsetof(DebugOverlay,rows)!=0xb4||offsetof(DebugOverlay,shownPage)!=0x26c4)return 5;
 unsigned rc[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP},pc[]={_PC_64,_PC_53,_PC_24};
 for(kindA=0;kindA<3;++kindA)for(kindB=0;kindB<3;++kindB)for(seed=0;seed<16;++seed)for(family=0;family<6;++family)for(int r=0;r<4;++r)for(int p=0;p<3;++p){rounding=rc[r];precision=pc[p];Compare(1);}
 for(alias=1;alias<=2;++alias)for(kindA=0;kindA<3;++kindA)for(kindB=0;kindB<3;++kindB)for(seed=0;seed<16;++seed){family=3;Compare(1);}alias=0;
 for(fault=1;fault<=5;++fault)for(kindA=0;kindA<3;++kindA)for(kindB=0;kindB<3;++kindB){family=0;Compare(1);}fault=0;
 kindA=kindB=0;family=5;seed=0;Compare(1);if(got.exception||got.result!=0)return 80;
 family=0;alias=2;Compare(1);if(got.exception||got.result!=1)return 81;alias=0;
 const int starts[]={-1,0,1,62,63},limits[]={0,1,3,64};
 for(seed=0;seed<8;++seed)for(enabled=0;enabled<2;++enabled)for(shown=0;shown<3;++shown)for(int s=0;s<5;++s)for(int l=0;l<4;++l)for(format=0;format<6;++format)for(method=0;method<2;++method){startRow=starts[s];limit=limits[l];Compare(0);}
 enabled=1;shown=0;startRow=-1;limit=64;format=0;method=1;Compare(0);if(got.exception||Overlay()->pageRowCount!=0||strcmp(Overlay()->rows[0].text,"KrustyBike")||Overlay()->rows[0].highlighted!=1||Overlay()->rows[0].color!=0xffffff)return 82;
 format=5;Compare(0);if(got.exception||Overlay()->rows[0].highlighted!=7||Overlay()->rows[0].color!=0x123456)return 83;
 printf("KrustyBike boundaries: %d collision/overlay scenarios match\n",checks);return 0;}
