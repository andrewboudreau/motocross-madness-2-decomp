// Complete candidate validation: actual game helpers and finite OS contracts.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include "GameUi.h"
#include "UIDialog.h"
#include "BackgroundImage.h"
extern "C" long __cdecl _ftol();
extern CameraRect* g_ControlDrawClipRect;extern CameraRect g_ControlDrawRect,g_DrawImageClipRect,g_DrawImageSourceRect;
extern int g_ControlDrawResult,g_AlignedTextX,g_AlignedTextY,g_ControlTextX,g_ControlTextY,g_TextRunIndex,g_TextRunX;
extern unsigned int g_TextRunColor;extern SIZE g_AlignedTextSize,g_ControlTextSize;
struct Scratch{CameraRect control,clip,source;SIZE aligned,text;int result,ax,ay,tx,ty,index,x;unsigned int color;int clipPointer;};
static Scratch Snapshot(){Scratch s;s.control=g_ControlDrawRect;s.clip=g_DrawImageClipRect;s.source=g_DrawImageSourceRect;s.aligned=g_AlignedTextSize;s.text=g_ControlTextSize;s.result=g_ControlDrawResult;s.ax=g_AlignedTextX;s.ay=g_AlignedTextY;s.tx=g_ControlTextX;s.ty=g_ControlTextY;s.index=g_TextRunIndex;s.x=g_TextRunX;s.color=g_TextRunColor;s.clipPointer=(int)g_ControlDrawClipRect;return s;}
static BYTE referenceSpace[0x800000]={1};
struct World{BYTE control[0x210],dialog[sizeof(UIDialog)],otherDialog[sizeof(UIDialog)],target[0x80],texture[0x80],background[0x100],surface[3][32],regions[4*64],anim[0x100],frame[0x30],text[32],stateText[32],runs[72];UnknownGameUiState states[2];BYTE guard[0x300];int held[3],gets[3],releases[3];};
struct Event{int kind;DWORD receiver,args[12];RECT rect;World before,after;Scratch first,last;};
struct Result{World world;Scratch scratch;int value,complete,count;DWORD exception,kind,address;unsigned fpStatus,fpControl;Event events[96];};
struct Scenario{int table,texture,source,background,stateText,controlText,stateLength,controlLength,selected,runs,align,shadow,font,left,top,width,height,mx,my,frameWidth,imageWidth,imageHeight,colorKey,redraw,dirty,copyValid,passes,beginFail,dcNull,dcStatus,apiStatus,blitResult,mutateAt,mutation,throwAt,fault,alias;float sx,sy;unsigned control;};
static World w;static Result got,want;static Scenario sc;static BYTE*guard;static BYTE*receiver;static int pass,checks;static DWORD table[70],surfaceTable[27],targetTable[5];
static DWORD& At(void*p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static float& F(void*p,int offset){return *(float*)((BYTE*)p+offset);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)ExitProcess(80);union{T p;DWORD n;}u;u.p=p;return u.n;}
static void Capture(){memcpy(w.guard,guard+4096-sizeof(w.guard),sizeof(w.guard));}
static Event& Log(int k,void*self){if(got.count==96)RaiseException(0xe0490002,0,0,0);Event&e=got.events[got.count++];e.kind=k;e.receiver=(DWORD)self;Capture();e.before=w;e.first=Snapshot();return e;}
static void Finish(){Event&e=got.events[got.count-1];Capture();e.after=w;e.last=Snapshot();}
static void Boundary(){if(sc.mutateAt==got.count){switch(sc.mutation){
case 0:At(w.control,0x1bc)=77;break;
case 1:At(w.control,0x2c)=31;At(w.control,0x34)=81;break;
case 2:At(w.control,0x1f0)=1;break;
case 3:At(w.control,0xa4)=!sc.source;break;
case 4:At(w.control,0xc0)=(DWORD)(w.text+1);At(w.control,0xd0)=2;break;
case 5:At(w.control,0xe8)=-4;At(w.control,0xec)=6;break;
case 6:At(w.dialog,0x110)=0;break;
case 7:At(w.target,0x48)=(DWORD)w.surface[2];break;
case 8:At(w.frame,0xc)=33;break;
case 9:At(w.control,0x1f4)=(DWORD)(w.states+1);At(w.control,0x1f0)=0;break;
case 10:At(w.control,0xc8)=0x123456;At(w.control,0xd4)=1;break;
case 12:At(w.control,0xb8)=(DWORD)w.otherDialog;break;
case 11:At(w.background,0x6c)=(DWORD)w.surface[2];At(w.background,0x70)=0x5557;break;
}}Finish();if(sc.throwAt==got.count)RaiseException(0xe0491234,0,0,0);}
class Probe{public:int GetText(){return ((UIMultiState*)this)->UIMultiState::GetControlText();}int GetLength(){return ((UIMultiState*)this)->UIMultiState::GetTextLength();}int GetLines(){return ((UIMultiState*)this)->UIMultiState::GetTextLines();}void* GetExtent(){return ((UIMultiState*)this)->UIMultiState::GetTextExtent();}int Blit(void*r,void*t,void*s,int f){Event&e=Log(3,this);e.rect=*(RECT*)r;e.args[0]=(DWORD)t;e.args[1]=(DWORD)s;e.args[2]=f;Boundary();return sc.blitResult;}};
int BackgroundImage::GetDc(void**dc,CameraRect*rect,int index,int dirty,int*frames,int*more,CameraRect*clip){Event&e=Log(6,this);e.rect=*(RECT*)rect;e.args[0]=index;e.args[1]=dirty;e.args[2]=*frames;Boundary();*dc=sc.dcNull?0:(void*)0x5555;*more=(++pass<sc.passes);heldDcSurface=(UnknownSurfaceInterface*)w.surface[1];heldDc=*dc;if(*dc)w.held[1]++;Finish();return !sc.beginFail;}
static int SurfaceIndex(void*p){for(int i=0;i<3;i++)if(p==w.surface[i])return i;ExitProcess(84);return 0;}
static long __stdcall GetDc(void*self,void**out){Event&e=Log(16,self);int i=SurfaceIndex(self);Boundary();*out=sc.dcNull?0:(void*)(0x5555+i);w.gets[i]++;if(*out)w.held[i]++;Finish();return sc.dcStatus;}
static long __stdcall ReleaseDc(void*self,void*dc){Event&e=Log(17,self);e.args[0]=(DWORD)dc;int i=SurfaceIndex(self);Boundary();w.releases[i]++;if(dc)w.held[i]--;Finish();return sc.dcStatus;}
static long __stdcall Blt(void*self,void*rect,void*source,void*sourceRect,int flags,void*effects){Event&e=Log(18,self);e.rect=*(RECT*)rect;e.args[0]=(DWORD)source;e.args[1]=flags;e.args[2]=(DWORD)effects;if(sourceRect){RECT&r=*(RECT*)sourceRect;e.args[3]=r.left;e.args[4]=r.top;e.args[5]=r.right;e.args[6]=r.bottom;}Boundary();return sc.dcStatus;}
static HGDIOBJ WINAPI Select(HDC dc,HGDIOBJ object){Event&e=Log(19,dc);e.args[0]=(DWORD)object;Boundary();return(HGDIOBJ)0x9999;}
static HRGN WINAPI Region(int l,int t,int r,int b){Event&e=Log(20,0);e.rect.left=l;e.rect.top=t;e.rect.right=r;e.rect.bottom=b;Boundary();return(HRGN)0x2222;}
static int WINAPI Clip(HDC dc,HRGN region){Event&e=Log(21,dc);e.args[0]=(DWORD)region;Boundary();return sc.apiStatus;}
static BOOL WINAPI Delete(HGDIOBJ object){Log(22,object);Boundary();return sc.apiStatus;}
static int WINAPI BkMode(HDC dc,int mode){Event&e=Log(23,dc);e.args[0]=mode;Boundary();return sc.apiStatus;}
static COLORREF WINAPI Color(HDC dc,COLORREF color){Event&e=Log(24,dc);e.args[0]=color;Boundary();return sc.apiStatus;}
static BOOL WINAPI Text(HDC dc,int x,int y,LPCSTR text,int count){Event&e=Log(25,dc);e.args[0]=x;e.args[1]=y;e.args[2]=count;for(int i=0;i<count&&i<32;i++)((BYTE*)&e.args[3])[i]=text[i];Boundary();return sc.apiStatus;}
static BOOL WINAPI Measure(HDC dc,LPCSTR text,int count,LPSIZE size){Event&e=Log(26,dc);e.args[0]=count;for(int i=0;i<count&&i<32;i++)((BYTE*)&e.args[3])[i]=text[i];Boundary();size->cx=count*7;size->cy=13;Finish();return sc.apiStatus;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int HookSource(){BYTE* base=(BYTE*)GetModuleHandleA(0);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)base;if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE)return 0;DWORD length=pe->OptionalHeader.SizeOfImage,rva=pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;if(!rva||rva>length-sizeof(IMAGE_IMPORT_DESCRIPTOR))return 0;int found=0;
 for(IMAGE_IMPORT_DESCRIPTOR* desc=(IMAGE_IMPORT_DESCRIPTOR*)(base+rva);; ++desc){if((BYTE*)(desc+1)>base+length)return 0;if(!desc->Name)break;if(desc->Name>=length||!desc->OriginalFirstThunk||desc->OriginalFirstThunk>=length||desc->FirstThunk>=length)return 0;DWORD* lookup=(DWORD*)(base+desc->OriginalFirstThunk);DWORD* iat=(DWORD*)(base+desc->FirstThunk);
 for(int n=0;;++n){if((BYTE*)(lookup+n+1)>base+length||(BYTE*)(iat+n+1)>base+length)return 0;if(!lookup[n])break;if(!(lookup[n]&IMAGE_ORDINAL_FLAG)&&lookup[n]<length-3){const char* name=(const char*)(base+lookup[n]+2);DWORD target=0;



 if(!strcmp(name,"SelectObject"))target=(DWORD)Select;if(!strcmp(name,"CreateRectRgn"))target=(DWORD)Region;
 if(!strcmp(name,"SelectClipRgn"))target=(DWORD)Clip;if(!strcmp(name,"DeleteObject"))target=(DWORD)Delete;
 if(!strcmp(name,"SetBkMode"))target=(DWORD)BkMode;if(!strcmp(name,"SetTextColor"))target=(DWORD)Color;
 if(!strcmp(name,"TextOutA"))target=(DWORD)Text;if(!strcmp(name,"GetTextExtentPoint32A"))target=(DWORD)Measure;
 if(target){DWORD old;if(!VirtualProtect(&iat[n],4,PAGE_READWRITE,&old))return 0;iat[n]=target;DWORD ignored;VirtualProtect(&iat[n],4,old,&ignored);++found;}}}}printf("hooks %d\n",found);return found==8;}

static int PatchCall(DWORD site,DWORD target,DWORD replacement){BYTE*p=(BYTE*)site;if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=target)return 0;*(DWORD*)(p+1)=replacement-(DWORD)(p+5);return 1;}
static int PatchReference(){if(memcmp((void*)_ftol,(void*)0x534a88,39))return 0;return PatchCall(0x4785e9,0x534a88,(DWORD)_ftol)&&PatchCall(0x4785fd,0x534a88,(DWORD)_ftol)&&PatchCall(0x47864f,0x404480,Address(&BackgroundImage::DrawImage))&&PatchCall(0x4786c2,0x46ed70,Address(&UIDialog::BeginControlDraw))&&PatchCall(0x47870c,0x472f80,Address(&UIAnim::GetCurrentFrame))&&PatchCall(0x478791,0x46f550,Address(&UIDialog::DrawAlignedText))&&PatchCall(0x4787d8,0x46f6c0,Address(&UIDialog::DrawControlText))&&PatchCall(0x4787e8,0x46eeb0,Address(&UIDialog::EndControlDraw))&&FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS*e){DWORD c=e->ExceptionRecord->ExceptionCode;if(c!=EXCEPTION_ACCESS_VIOLATION&&c!=0xe0491234)return EXCEPTION_CONTINUE_SEARCH;got.exception=c;if(c==EXCEPTION_ACCESS_VIOLATION){got.kind=e->ExceptionRecord->ExceptionInformation[0];got.address=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Defaults(){memset(&sc,0,sizeof(sc));sc.table=sc.stateText=sc.controlText=1;sc.stateLength=sc.controlLength=4;sc.apiStatus=sc.blitResult=1;sc.left=10;sc.top=20;sc.width=70;sc.height=40;sc.mx=2;sc.my=3;sc.font=13;sc.frameWidth=11;sc.imageWidth=31;sc.imageHeight=17;sc.sx=1.5f;sc.sy=0.5f;sc.redraw=6;sc.passes=1;sc.control=_MCW_EM|_PC_64|_RC_NEAR;}
static void Setup(){memset(&w,0,sizeof(w));memset(&got,0,sizeof(got));memset(guard,0xa7,4096);pass=0;
 g_ControlDrawClipRect=0;memset(&g_ControlDrawRect,0,16);memset(&g_DrawImageClipRect,0,16);memset(&g_DrawImageSourceRect,0,16);memset(&g_AlignedTextSize,0,8);memset(&g_ControlTextSize,0,8);g_ControlDrawResult=g_AlignedTextX=g_AlignedTextY=g_ControlTextX=g_ControlTextY=g_TextRunIndex=g_TextRunX=g_TextRunColor=0;
 At(w.control,0)=(DWORD)table;At(w.control,0x18)=(DWORD)w.target;At(w.control,0x2c)=sc.left;At(w.control,0x30)=sc.top;At(w.control,0x34)=sc.left+sc.width;At(w.control,0x38)=sc.top+sc.height;At(w.control,0xb8)=(DWORD)w.dialog;At(w.control,0xa4)=sc.source;At(w.control,0xc0)=sc.controlText?(DWORD)w.text:0;At(w.control,0xd0)=sc.controlText?sc.controlLength:0;At(w.control,0xd8)=sc.align;At(w.control,0xe8)=sc.mx;At(w.control,0xec)=sc.my;At(w.control,0xd4)=sc.shadow;At(w.control,0xc4)=sc.shadow?0xaabbcc:0;At(w.control,0xc8)=0x112233;At(w.control,0x160)=sc.font;At(w.control,0x1bc)=sc.redraw;At(w.control,0x1b8)=1;At(w.control,0x1c0)=sc.dirty;At(w.control,0x1f4)=sc.table?(DWORD)w.states:0;At(w.control,0x1f0)=sc.selected;
 At(w.control,0xdc)=4;At(w.control,0xe0)=28;At(w.control,0x1cc)=sc.texture?(DWORD)w.texture:0;strcpy((char*)w.text,"abcd");strcpy((char*)w.stateText,"WXYZ");
 At(w.anim,0x28)=(DWORD)w.frame;At(w.frame,0xc)=sc.frameWidth;
 for(int i=0;i<2;i++){w.states[i].image=(UIAnim*)w.anim;w.states[i].text=sc.stateText?(char*)w.stateText+i:0;w.states[i].textLength=sc.stateLength;w.states[i].textExtent[0]=4;w.states[i].textExtent[1]=28+i;w.states[i].textLines=sc.runs?(char*)w.runs:0;}
 At(w.runs,0)=(DWORD)w.stateText;At(w.runs,4)=1;At(w.runs,8)=2;At(w.runs,12)=0x123456;At(w.runs,16)=14;At(w.runs,24)=(DWORD)(w.stateText+2);At(w.runs,28)=2;At(w.runs,32)=2;At(w.runs,40)=14;At(w.runs,44)=19;At(w.control,0xe4)=sc.runs?(DWORD)w.runs:0;
 At(w.dialog,0x18)=(DWORD)w.target;At(w.dialog,0xdc)=15;At(w.dialog,0x110)=sc.background?(DWORD)w.background:0;F(w.dialog,0x9c4)=sc.sx;F(w.dialog,0x9c8)=sc.sy;
 At(w.target,0)=(DWORD)targetTable;At(w.target,0x48)=(DWORD)w.surface[0];At(w.target,0x14)=3;At(w.target,0x18)=0;At(w.target,0x1c)=42;At(w.target,0xc)=640;At(w.target,0x10)=480;
 for(int j=0;j<3;j++)At(w.surface[j],0)=(DWORD)surfaceTable;
 At(w.texture,0x14)=sc.imageWidth;At(w.texture,0x18)=sc.imageHeight;At(w.texture,0x30)=sc.colorKey;At(w.texture,0x70)=(DWORD)w.surface[2];At(w.background,0x18)=(DWORD)w.target;At(w.background,0x3c)=(DWORD)w.surface[1];At(w.background,0x40)=sc.copyValid;At(w.background,0x48)=4;At(w.background,0x50)=(DWORD)w.regions;
 for(int k=0;k<4;k++){At(w.regions,k*64)=3;At(w.regions,k*64+0x38)=42;At(w.regions,k*64+12)=640;At(w.regions,k*64+16)=480;}
 memcpy(w.otherDialog,w.dialog,sizeof(w.dialog));At(w.otherDialog,0xdc)=24;At(w.otherDialog,0x110)=0;receiver=w.control;
 if(sc.alias==1){At(w.control,0xc0)=(DWORD)(w.control+0xc8);At(w.control,0xd0)=4;}
 if(sc.alias==2)w.states[0].text=(char*)&w.states[0].textExtent;
 if(sc.fault==1)At(w.control,0x1f4)=1;
 if(sc.fault==2)At(w.control,0xb8)=1;
 if(sc.fault==3){At(w.control,0xc0)=(DWORD)(guard+4094);guard[4094]='x';guard[4095]='y';}
 if(sc.fault==4)At(w.anim,0x28)=1;
 if(sc.fault==5)At(w.target,0x48)=1;
 if(sc.fault==10){At(w.control,0x1f4)=0;At(w.control,0xa4)=1;}
 if(sc.fault==11){At(w.control,0xc0)=0;At(w.control,0xd0)=4;At(w.control,0xa4)=1;}
 if(sc.fault>=6&&sc.fault<=9){int n=(sc.fault-5)*4;At(w.control,0x1f4)=(DWORD)(guard+4096-n);memcpy(guard+4096-n,w.states,n);}
 Capture();_clearfp();_controlfp(sc.control,_MCW_EM|_MCW_PC|_MCW_RC);
}
static void Run(int retail){Setup();__try{union{int(UIMultiState::*p)();DWORD a;}u;if(sizeof(u.p)!=4)ExitProcess(82);u.a=0x478570;got.value=retail?(((UIMultiState*)receiver)->*u.p)():((UIMultiState*)receiver)->UIMultiState::Draw();got.complete=1;}__except(Filter(GetExceptionInformation())){}got.fpStatus=_statusfp();got.fpControl=_controlfp(0,0);_clearfp();_controlfp(_MCW_EM|_PC_64|_RC_NEAR,_MCW_EM|_MCW_PC|_MCW_RC);Capture();got.world=w;got.scratch=Snapshot();}
static void Check(){Run(1);want=got;Run(0);++checks;if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])at++;printf("Mismatch%d at%d source%d table%d texture%d bg%d state%d ctl%d runs%d pass%d mut%d/%d throw%d fault%d fp%x/%x events%d/%d\n",checks,at,sc.source,sc.table,sc.texture,sc.background,sc.stateText,sc.controlText,sc.runs,sc.passes,sc.mutateAt,sc.mutation,sc.throwAt,sc.fault,want.fpStatus,got.fpStatus,want.count,got.count);ExitProcess(2);}if(!sc.throwAt&&!sc.fault&&(!got.complete||got.exception)){printf("Unexpected fault case%d source%d texture%d bg%d runs%d selected%d addr%x events%d\n",checks,sc.source,sc.texture,sc.background,sc.runs,sc.selected,got.address,got.count);ExitProcess(3);}}
int main(int argc,char**argv){setvbuf(stdout,0,_IONBF,0);if(argc!=2)return 4;guard=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&old)||!MapReference(argv[1])||!HookSource()||!PatchReference())return 4;
 surfaceTable[17]=(DWORD)GetDc;surfaceTable[26]=(DWORD)ReleaseDc;surfaceTable[5]=(DWORD)Blt;targetTable[3]=Address(&Probe::Blit);
 table[34]=Address(&Probe::GetText);table[35]=Address(&Probe::GetLength);table[36]=Address(&Probe::GetLines);table[37]=Address(&Probe::GetExtent);
 Defaults();for(sc.source=0;sc.source<2;sc.source++)for(sc.texture=0;sc.texture<2;sc.texture++)for(sc.background=0;sc.background<2;sc.background++)for(sc.stateText=0;sc.stateText<2;sc.stateText++)for(sc.controlText=0;sc.controlText<2;sc.controlText++)for(sc.runs=0;sc.runs<2;sc.runs++)for(sc.selected=0;sc.selected<2;sc.selected++)for(sc.passes=1;sc.passes<3;sc.passes++)Check();

 // Independent text positions for signed dimensions, margins and alignment.
 int widths[]={-71,-1,0,1,69,70,71},fonts[]={-13,-1,0,1,13};
 Defaults();sc.texture=0;sc.background=0;
 for(sc.source=0;sc.source<2;sc.source++)for(sc.align=0;sc.align<64;sc.align++)for(int wi=0;wi<7;wi++)for(int fi=0;fi<5;fi++)for(sc.shadow=0;sc.shadow<2;sc.shadow++){
  sc.width=widths[wi];sc.height=fi*7-13;sc.left=wi-3;sc.top=fi-2;sc.mx=fi-2;sc.my=wi-3;sc.font=fonts[fi];Check();
  int l=sc.left+sc.mx+(sc.source?sc.frameWidth:0),t=sc.top+sc.my,r=sc.left+sc.width-sc.mx,b=sc.top+sc.height-sc.my;
  int flags=sc.source?0x11:sc.align,tw=sc.source?sc.controlLength*7:(flags==9?0:28),fh=sc.source?(sc.font?sc.font:15):15;
  int x=l;if(flags&2)x=(r-l)/2-tw/2+l;else if(flags&4)x=r-tw;
  int y=t;if(flags&16)y=(b-t)/2-fh/2+t;else if(flags&32)y=b-fh;
  int seen=0;for(int e=0;e<got.count;e++)if(got.events[e].kind==25){Event&v=got.events[e];int sh=sc.shadow&&seen==0;if((int)v.args[0]!=x-sh||(int)v.args[1]!=y-sh||v.args[2]!=4)return 10;seen++;}
  if(got.value!=1||seen!=1+sc.shadow||w.held[0]||w.gets[0]!=1||w.releases[0]!=1||(int)At(w.control,0x1bc)!=sc.redraw)return 11;
  int selects=0,deletes=0;for(int e2=0;e2<got.count;e2++){Event&v=got.events[e2];if(v.kind==19){if(selects&&v.args[0]!=0x9999)return 32;selects++;}if(v.kind==22){if(v.receiver!=0x2222)return 33;deletes++;}}if(selects!=2||deletes!=1||At(w.dialog,0x7f04)!=0x9999||At(w.dialog,0x7f00)!=0x2222)return 34;
 }
 // Exact binary fractions make this image rectangle oracle independent of x87.
 Defaults();sc.texture=1;sc.source=1;sc.stateText=sc.controlText=0;
 for(int n=-7;n<=7;n++)for(int m=-7;m<=7;m++)for(int dim=-33;dim<=33;dim+=11){sc.colorKey=n&1;sc.sx=n*0.25f;sc.sy=m*0.25f;sc.imageWidth=dim;sc.imageHeight=dim+3;Check();if(got.value!=1||got.count!=1||got.events[0].kind!=3||got.events[0].args[2]!=(sc.colorKey?0x1008000:0x1000000))return 12;RECT&r=got.events[0].rect;if(r.left!=sc.left||r.top!=sc.top||r.right!=sc.left+dim*n/4||r.bottom!=sc.top+(dim+3)*m/4)return 13;}
 // Real source DrawImage mutation of frames, including finite nonpositive values.
 Defaults();sc.texture=1;sc.background=1;
 for(sc.copyValid=0;sc.copyValid<2;sc.copyValid++)for(sc.dirty=0;sc.dirty<2;sc.dirty++)for(sc.redraw=-2;sc.redraw<=2;sc.redraw++)for(sc.source=0;sc.source<2;sc.source++)for(sc.colorKey=0;sc.colorKey<2;sc.colorKey++){
  Check();int frames=sc.copyValid&&sc.dirty?(sc.redraw?sc.redraw-1:0):3;int expected=frames>=0?frames:sc.redraw;if((int)At(w.control,0x1bc)!=expected||got.value!=1)return 14;for(int e=0;e<got.count;e++)if(got.events[e].kind==18&&got.events[e].args[1]!=(sc.colorKey?0x1008000:0x1000000))return 22;
 }
 // Always-written DC/SIZE output contract, including failing HRESULT and GDI FALSE.
 Defaults();for(sc.source=0;sc.source<2;sc.source++)for(sc.texture=0;sc.texture<2;sc.texture++)for(sc.background=0;sc.background<2;sc.background++)for(sc.beginFail=0;sc.beginFail<2;sc.beginFail++)for(sc.dcNull=0;sc.dcNull<2;sc.dcNull++)for(sc.apiStatus=0;sc.apiStatus<2;sc.apiStatus++)for(sc.blitResult=0;sc.blitResult<2;sc.blitResult++)for(int status=0;status<3;status++){
  sc.dcStatus=status==2?0x80004005:status;Check();int early=sc.texture&&!sc.background&&!sc.blitResult;if(got.value!=!early)return 15;if(early&&(got.count!=1||got.events[0].kind!=3))return 16;if(!early&&!sc.background&&(w.gets[0]!=1||w.releases[0]!=!sc.dcNull||w.held[0]))return 23;if(!early&&sc.background&&w.held[1]!=!sc.dcNull)return 24;
 }
 Defaults();sc.background=1;sc.texture=1;sc.runs=1;sc.shadow=1;sc.passes=2;Check();int reached=got.count;
 for(sc.mutateAt=1;sc.mutateAt<=reached;sc.mutateAt++)for(sc.mutation=0;sc.mutation<13;sc.mutation++){Check();if(got.count<sc.mutateAt||got.value!=1)return 17;}sc.mutateAt=0;
 for(sc.throwAt=1;sc.throwAt<=reached;sc.throwAt++){Check();if(got.exception!=0xe0491234||got.complete||got.count!=sc.throwAt)return 18;}sc.throwAt=0;
 // No arbitrary source-address equivalence: explicit valid field/string overlap.
 Defaults();for(sc.alias=1;sc.alias<=2;sc.alias++)for(sc.source=0;sc.source<2;sc.source++)for(sc.background=0;sc.background<2;sc.background++)Check();

 // Absent table can return normally, but never invent an image for own-text draw.
 Defaults();sc.table=0;for(sc.source=0;sc.source<2;sc.source++)for(sc.texture=0;sc.texture<2;sc.texture++)for(sc.background=0;sc.background<2;sc.background++){sc.controlText=sc.source?0:1;Check();if(got.value!=1||got.count)return 25;}
 Defaults();for(sc.source=0;sc.source<2;sc.source++)for(sc.stateLength=-1;sc.stateLength<=1;sc.stateLength++)for(sc.controlLength=-1;sc.controlLength<=1;sc.controlLength++){Check();if(got.value!=1)return 26;}
 // Actual text-run helper: independent tab, color, shadow and byte transcript.
 Defaults();sc.runs=1;for(sc.source=0;sc.source<2;sc.source++)for(sc.shadow=0;sc.shadow<2;sc.shadow++){
  Check();int l=sc.left+sc.mx+(sc.source?sc.frameWidth:0),y=sc.top+sc.my;if(sc.source)y+=(sc.height-2*sc.my)/2-sc.font/2;int n=0;
  for(int e=0;e<got.count;e++)if(got.events[e].kind==25){Event&v=got.events[e];int run=n/(1+sc.shadow),shadow=sc.shadow&&(n%(1+sc.shadow)==0);int x=l+(run?19:0);if((int)v.args[0]!=x+shadow||(int)v.args[1]!=y+shadow||v.args[2]!=2||memcmp(&v.args[3],run?"YZ":"WX",2))return 27;n++;}
  if(n!=2*(1+sc.shadow)||g_TextRunIndex!=2||g_TextRunColor!=0x123456||g_TextRunX!=l+33)return 28;
 }
 // Masked finite/raw FP inputs, all precision and rounding controls.
 unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 unsigned bits[]={0,0x80000000,1,0x80000001,0x3f000001,0xbf000001,0x3f7fffff,0x3f800001,0x4effffff,0xceffffff,0x00800000,0x7f7fffff};
 Defaults();sc.texture=1;sc.source=1;sc.stateText=sc.controlText=0;
 for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++)for(int bi=0;bi<12;bi++)for(int sign=0;sign<2;sign++){sc.control=_MCW_EM|pcs[pc]|rcs[rc];*(unsigned*)&sc.sx=bits[bi];*(unsigned*)&sc.sy=bits[11-bi];sc.imageWidth=sign?-16777219:16777219;sc.imageHeight=sign?33:-33;Check();}
 unsigned rng=0x49f00d;for(int sample=0;sample<4096;sample++){rng=rng*1664525+1013904223;unsigned x=rng;if((x&0x7f800000)==0x7f800000)x^=0x00800000;*(unsigned*)&sc.sx=x;rng=rng*1664525+1013904223;x=rng;if((x&0x7f800000)==0x7f800000)x^=0x00800000;*(unsigned*)&sc.sy=x;sc.control=_MCW_EM|pcs[sample%3]|rcs[(sample/3)%4];sc.imageWidth=(int)(rng&0xffff)-32768;sc.imageHeight=(int)((rng>>16)&0xffff)-32768;Check();}
 Defaults();sc.source=1;for(sc.fault=1;sc.fault<=11;sc.fault++){if(sc.fault>=6&&sc.fault<=9)sc.source=0;else sc.source=1;Check();if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION)return 19;if(sc.fault==3&&(got.kind!=0||got.address!=(DWORD)(guard+4096)||w.held[0]!=1))return 20;if(sc.fault==11&&(got.address||w.held[0]!=1))return 21;DWORD expected[]={0,9,0x111,(DWORD)(guard+4096),13,1,(DWORD)(guard+4100),(DWORD)(guard+4096),(DWORD)(guard+4096),(DWORD)(guard+4104),0,0};if(got.address!=expected[sc.fault]||got.kind!=0)return 29;if((sc.fault==4||sc.fault==9||sc.fault==10)&&w.held[0]!=1)return 30;}
 Defaults();for(sc.throwAt=1;sc.throwAt<=2;sc.throwAt++){Check();if(got.exception!=0xe0491234||got.complete||w.releases[0]||w.held[0]!=sc.throwAt-1||At(w.dialog,0x7f04))return 31;}
 printf("MultiStateDraw: %d actual-helper scenarios match; reached %d callback sites\n",checks,reached);return 0;}
