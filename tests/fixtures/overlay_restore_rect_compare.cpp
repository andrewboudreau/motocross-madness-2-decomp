// Finite native validation only: actual source texture helpers, scripted COM surfaces.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Overlay.h"
#include "PCTextureMap.h"
#include "Tgafile.h"
static BYTE referenceSpace[0x800000]={1};
struct Surface{DWORD table;int locks,unlocks,held;};
struct World{BYTE object[0x120],texture[2][0x90],buffers[2][4096],guard[2][4096];Surface surfaces[2];UnknownOverlayRect rect;};
struct Event{int kind,who,flags;BYTE desc[124],out[124];World before,after;};
struct Result{World state;int result,complete,count;DWORD exception,kind,address;Event events[8];};
struct Scenario{int format,width,left,top,rows,whole,pitch[2],failMask,nullMask,alias,invalid,guarded,available,mutateAt,mutation,throwAt,unlockStatus;};
static World w;static Result got,want;static Scenario sc;static int checks;static DWORD textureTable[15],surfaceTable[33];static BYTE*guard[2];
static DWORD& At(void*p,int offset){return *(DWORD*)((BYTE*)p+offset);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)ExitProcess(80);union{T p;DWORD n;}u;u.p=p;return u.n;}
class Probe{public:void* Lock(void*r,long*p,int f){return ((PCTextureMap*)this)->PCTextureMap::LockSurface(r,p,f);}int Unlock(void*r){return ((PCTextureMap*)this)->PCTextureMap::UnlockSurface(r);}};
static int Index(void*p){for(int i=0;i<2;i++)if(p==&w.surfaces[i])return i;ExitProcess(81);return -1;}
static void Capture(){for(int i=0;i<2;i++)memcpy(w.guard[i],guard[i],4096);}
static Event& Begin(int kind,int who){if(got.count>=8)ExitProcess(82);Capture();Event&e=got.events[got.count++];e.kind=kind;e.who=who;e.before=w;return e;}
static void Finish(Event&e){Capture();e.after=w;}
static void Boundary(UnknownSurfaceDesc*desc){
 if(sc.mutateAt==got.count){switch(sc.mutation){
 case 0:w.rect.top=1;break;case 1:w.rect.left=2;break;
 case 2:w.rect.right+=3;w.rect.bottom+=2;break;
 case 3:At(w.object,0x2c)=(DWORD)w.texture[0];break;
 case 4:At(w.object,0x30)=(DWORD)w.texture[1];break;
 case 5:At(w.texture[0],0x70)=(DWORD)&w.surfaces[1];break;
 case 6:At(w.texture[1],0x70)=(DWORD)&w.surfaces[0];break;
 case 7:At(w.texture[1],0x20)=8;break;
 case 8:if(desc)desc->pitch=-17;break;
 case 9:if(desc)desc->surface=w.buffers[1]+2048;break;
 case 10:if(desc)desc->surface=0;break;
 }}
 if(sc.throwAt==got.count){Finish(got.events[got.count-1]);RaiseException(0xe048ab12,0,0,0);}
}
static long __stdcall Lock(void*self,void*rect,UnknownSurfaceDesc*desc,int flags,void*event){int i=Index(self);if(rect||event)ExitProcess(82);BYTE expected[124]={0};*(DWORD*)expected=124;if(memcmp(desc,expected,124))ExitProcess(93);Event&e=Begin(1,i);e.flags=flags;memcpy(e.desc,desc,124);w.surfaces[i].locks++;
 desc->pitch=sc.pitch[i];
 if(sc.nullMask&(1<<i))desc->surface=0;
 else if(sc.guarded==1)desc->surface=guard[i]+4096-(i?128:sc.available);
 else if(sc.guarded==2)desc->surface=guard[i]+4096-(i?sc.available:128);
 else desc->surface=w.buffers[sc.alias?0:i]+2048;
 if(!(sc.failMask&(1<<i)))w.surfaces[i].held++;Boundary(desc);memcpy(e.out,desc,124);Finish(e);return(sc.failMask&(1<<i))?0x80004005:0;}
static long __stdcall Unlock(void*self,void*rect){int i=Index(self);if(rect)ExitProcess(83);Event&e=Begin(2,i);w.surfaces[i].unlocks++;if(!sc.unlockStatus)w.surfaces[i].held--;Boundary(0);Finish(e);return sc.unlockStatus;}
static int PatchCall(DWORD at,DWORD target,DWORD replacement){BYTE*p=(BYTE*)at;if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=target)return 0;*(DWORD*)(p+1)=replacement-(DWORD)(p+5);return 1;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS*e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION&&e->ExceptionRecord->ExceptionCode!=0xe048ab12)return EXCEPTION_CONTINUE_SEARCH;got.exception=e->ExceptionRecord->ExceptionCode;if(got.exception==EXCEPTION_ACCESS_VIOLATION){got.kind=e->ExceptionRecord->ExceptionInformation[0];got.address=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Setup(){memset(&w,0,sizeof(w));memset(&got,0,sizeof(got));for(int i=0;i<2;i++){At(w.texture[i],0)=(DWORD)textureTable;At(w.texture[i],0x14)=sc.width;At(w.texture[i],0x20)=sc.format;At(w.texture[i],0x70)=(DWORD)&w.surfaces[i];w.surfaces[i].table=(DWORD)surfaceTable;for(int j=0;j<4096;j++){w.buffers[i][j]=(BYTE)(j*13+i*37);guard[i][j]=(BYTE)(j*17+i*29);}}
 At(w.object,0x2c)=(DWORD)w.texture[1];At(w.object,0x30)=(DWORD)w.texture[0];w.rect.left=sc.left;w.rect.top=sc.top;w.rect.right=sc.left+sc.width-(sc.whole?0:1);w.rect.bottom=sc.top+sc.rows;}
static void Run(int retail){Setup();const UnknownOverlayRect*rect=sc.invalid?(UnknownOverlayRect*)1:&w.rect;if(sc.invalid>=2){rect=(UnknownOverlayRect*)(guard[0]+4096-(sc.invalid-1)*4);memcpy((void*)rect,&w.rect,(sc.invalid-1)*4);}__try{union{int(Overlay::*p)(const UnknownOverlayRect*);DWORD n;}u;if(sizeof(u.p)!=4)ExitProcess(92);u.n=0x4b6710;got.result=retail?(((Overlay*)w.object)->*u.p)(rect):((Overlay*)w.object)->RestoreRect(rect);got.complete=1;}__except(Filter(GetExceptionInformation())){}for(int i=0;i<2;i++)memcpy(w.guard[i],guard[i],4096);got.state=w;}
static void Oracle(){
 int bpp=0;switch(sc.format){case 8:bpp=1;break;case 555:case 565:case 1555:case 4444:bpp=2;break;case 888:bpp=3;break;case 8888:bpp=4;break;}
 BYTE expected[4096];for(int i=0;i<4096;i++)expected[i]=(BYTE)(i*13+37);
 if(!sc.failMask&&!sc.nullMask){int rows=sc.whole?1:sc.rows;int count=(sc.whole?sc.width*sc.rows:sc.width-1)*bpp;int source=2048+sc.top*sc.pitch[0]+(sc.whole?0:sc.left*bpp);int dest=2048+sc.top*(sc.whole?sc.pitch[1]:sc.pitch[0])+(sc.whole?0:sc.left*bpp);
  for(int y=0;y<rows;y++){if(count&&(source<0||dest<0||source+count>4096||dest+count>4096))ExitProcess(89);for(int x=0;x<count;x++)expected[dest+x]=(BYTE)((source+x)*13);source+=sc.pitch[0];dest+=sc.pitch[1];}}
 if(memcmp(expected,w.buffers[1],4096))ExitProcess(90);
 for(int j=0;j<4096;j++)if(w.buffers[0][j]!=(BYTE)(j*13))ExitProcess(91);
}
static void Check(){Run(1);want=got;Run(0);checks++;if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])at++;printf("Mismatch%d offset%d fmt%d width%d whole%d pitches%d/%d rows%d rect%d/%d fault%x:%x/%x:%x\n",checks,at,sc.format,sc.width,sc.whole,sc.pitch[0],sc.pitch[1],sc.rows,sc.left,sc.top,want.kind,want.address,got.kind,got.address);ExitProcess(2);}
 if(!sc.guarded&&!sc.alias&&!sc.invalid&&!sc.mutateAt&&!sc.throwAt)Oracle();
 if(!sc.guarded&&!sc.invalid&&!sc.throwAt&&(!got.complete||got.exception))ExitProcess(84);
 if(sc.failMask||sc.nullMask){if(got.result||w.surfaces[0].locks!=1||w.surfaces[1].locks!=1||w.surfaces[0].unlocks||w.surfaces[1].unlocks||w.surfaces[0].held!=!(sc.failMask&1)||w.surfaces[1].held!=!(sc.failMask&2))ExitProcess(85);}
 else if(!sc.guarded&&!sc.invalid&&!sc.mutateAt&&!sc.throwAt){if(got.result!=1||got.count!=4||got.events[0].flags!=0x811||got.events[1].flags!=0x801||w.surfaces[0].unlocks!=1||w.surfaces[1].unlocks!=1||w.surfaces[0].held!=!!sc.unlockStatus||w.surfaces[1].held!=!!sc.unlockStatus)ExitProcess(86);}
}
int main(int argc,char**argv){setvbuf(stdout,0,_IONBF,0);if(argc<2||!MapReference(argv[1]))return 3;for(int i=0;i<2;i++){guard[i]=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!guard[i]||!VirtualProtect(guard[i]+4096,4096,PAGE_NOACCESS,&old))return 4;}textureTable[13]=Address(&Probe::Lock);textureTable[14]=Address(&Probe::Unlock);surfaceTable[25]=(DWORD)Lock;surfaceTable[32]=(DWORD)Unlock;
 for(int c=0;c<3;c++){DWORD site[]={0x4b6740,0x4b6766,0x4b67fe};if(!PatchCall(site[c],0x511970,(DWORD)PixelFormatBytesPerPixel))return 5;}
 int formats[]={8,555,565,888,1555,4444,8888,0},pitches[]={-33,-17,0,17,33};
 for(int f=0;f<8;f++)for(sc.width=1;sc.width<=8;sc.width++)for(sc.whole=0;sc.whole<2;sc.whole++)for(int p=0;p<5;p++)for(int q=0;q<5;q++)for(sc.rows=0;sc.rows<=3;sc.rows++){sc.format=formats[f];sc.pitch[0]=pitches[p];sc.pitch[1]=pitches[q];sc.left=(f%3)-1;sc.top=(p%3)-1;Check();}
 sc.format=8888;sc.width=8;sc.rows=2;sc.pitch[0]=33;sc.pitch[1]=35;sc.left=1;sc.top=1;
 for(sc.whole=0;sc.whole<2;sc.whole++)for(sc.failMask=0;sc.failMask<4;sc.failMask++)for(sc.nullMask=0;sc.nullMask<4;sc.nullMask++)Check();sc.failMask=sc.nullMask=0;
 sc.alias=1;for(sc.whole=0;sc.whole<2;sc.whole++)Check();sc.alias=0;
 sc.left=sc.top=0;sc.width=8;sc.rows=1;
 for(sc.guarded=1;sc.guarded<=2;sc.guarded++)for(sc.available=0;sc.available<=17;sc.available++)for(sc.whole=0;sc.whole<2;sc.whole++){
  Check();int which=sc.guarded-1; // 1 faults source0,2 faults destination1.
  if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.kind!=(DWORD)which||got.address!=(DWORD)(guard[which]+4096)||w.surfaces[0].unlocks||w.surfaces[1].unlocks||w.surfaces[0].held!=1||w.surfaces[1].held!=1)return 6;
  int copied=sc.available&~3; // REP MOVSD faults before its incomplete DWORD.
  for(int k=0;k<sc.available;k++){int destStart=4096-(which?sc.available:128);int sourceStart=4096-(which?128:sc.available);BYTE expected=k<copied?(BYTE)((sourceStart+k)*17):(BYTE)((destStart+k)*17+29);if(guard[1][destStart+k]!=expected)return 9;}
 }sc.guarded=0;
 sc.width=8;sc.rows=2;sc.pitch[0]=33;sc.pitch[1]=35;sc.whole=0;
 for(sc.mutateAt=1;sc.mutateAt<=4;sc.mutateAt++)for(sc.mutation=0;sc.mutation<11;sc.mutation++){Check();if(got.count<sc.mutateAt)return 11;if(sc.mutation==10&&sc.mutateAt<=2){if(got.result||got.count!=2||w.surfaces[0].unlocks||w.surfaces[1].unlocks)return 12;}else if(got.result!=1)return 13;
 if(sc.mutateAt<=2){
  if(sc.mutation==3||sc.mutation==6){if(w.surfaces[0].unlocks!=2||w.surfaces[1].unlocks)return 17;}
  if(sc.mutation==4||sc.mutation==5){if(w.surfaces[0].unlocks||w.surfaces[1].unlocks!=2)return 18;}
 }
}sc.mutateAt=0;
 for(sc.throwAt=1;sc.throwAt<=4;sc.throwAt++){Check();if(got.exception!=0xe048ab12||got.complete||got.count!=sc.throwAt)return 14;if(sc.throwAt<=2&&(w.surfaces[0].unlocks||w.surfaces[1].unlocks))return 15;}sc.throwAt=0;
 for(int status=0;status<3;status++)for(sc.rows=-3;sc.rows<=0;sc.rows++){sc.unlockStatus=status==2?0x80004005:status;Check();if(got.result!=1)return 16;}sc.unlockStatus=0;sc.rows=1;
 sc.invalid=1;Run(1);want=got;Run(0);printf("Invalid rect=1: retail AV %x:%x, candidate AV %x:%x\n",want.kind,want.address,got.kind,got.address);if(want.exception!=EXCEPTION_ACCESS_VIOLATION||got.exception!=EXCEPTION_ACCESS_VIOLATION||want.address!=1||got.address!=1||want.count||got.count||want.complete||got.complete||want.kind||got.kind)return 7;checks++;

 for(sc.invalid=2;sc.invalid<=4;sc.invalid++){Check();if(got.exception!=EXCEPTION_ACCESS_VIOLATION||got.kind!=0||got.address!=(DWORD)(guard[0]+4096)||got.count||got.complete)return 10;}
 printf("Overlay RestoreRect: %d real-helper finite cases match\n",checks);
 return 0;}
