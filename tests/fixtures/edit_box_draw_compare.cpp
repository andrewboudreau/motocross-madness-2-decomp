// Complete Draw with actual strict helper bodies and finite platform contracts.
// Reference calls share the independently dual-strict helpers; no whole-menu claim.
#include <windows.h>
#include <imm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "GameUi.h"
#include "UIDialog.h"
#include "BackgroundImage.h"
#include "TrackGame.h"
#include "Display.h"
TrackGame* g_TrackGame;
MemTagStack* g_MemTagStack;int g_UnclaimedBytes;
extern CameraRect* g_ControlDrawClipRect;extern CameraRect g_ControlDrawRect;
extern int g_ControlDrawResult,g_AlignedTextX,g_AlignedTextY;
extern SIZE g_AlignedTextSize;extern CameraRect g_DrawImageClipRect,g_DrawImageSourceRect;
struct Scratch {CameraRect control,clip,source;SIZE size;int result,x,y,clipPointer,bytes;};
static Scratch ScratchState(){Scratch r;r.control=g_ControlDrawRect;r.clip=g_DrawImageClipRect;r.source=g_DrawImageSourceRect;r.size=g_AlignedTextSize;r.result=g_ControlDrawResult;r.x=g_AlignedTextX;r.y=g_AlignedTextY;r.clipPointer=g_ControlDrawClipRect?((g_ControlDrawClipRect==&g_ControlDrawRect)?2:1):0;r.bytes=g_UnclaimedBytes;return r;}

static BYTE referenceSpace[0x800000]={1};
struct World { BYTE edit[0x250],dialog[0x580],user[0x200],game[0x340],display[0x100],target[0x80],texture[0x100],background[0x200],text[64],surface[3][32],regions[8*64];BYTE guard[0x250];int held[3],gets[3],releases[3]; };
struct Event {int kind;DWORD receiver,args[12];RECT rect;World before,after;Scratch scratchBefore,scratchAfter;};
struct Result {World state;Scratch scratch;BYTE allocated[64];int value,complete,count;DWORD exception,kind,address;Event events[64];};
struct Scenario {int align,focus,font,textWidth,texture,freeze,brush,passes,beginFail,mutateAt,mutation,throwAt,nullText,dcStatus,dcNull,imeNull,imeClosed,apiStatus,copyValid,dirty,left,top,width,height,marginX,marginY,shadow,fault,alias;};
static BYTE*guard;static BYTE*editReceiver;static World w;static Result got,want;static Scenario sc;static int checks,pass;static DWORD table[70],targetTable[10],surfaceTable[27];
static DWORD& At(void*p,int offset){return *(DWORD*)((BYTE*)p+offset);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)ExitProcess(80);union{T p;DWORD n;}u;u.p=p;return u.n;}
static void CaptureGuard(){memcpy(w.guard,guard+4096-0x250,0x250);}
static Event& Log(int k,void*self){if(got.count==64)ExitProcess(81);Event&e=got.events[got.count++];e.kind=k;e.receiver=(DWORD)self;CaptureGuard();e.before=w;e.scratchBefore=ScratchState();return e;}
static void Finish(){Event&e=got.events[got.count-1];CaptureGuard();e.after=w;e.scratchAfter=ScratchState();}
static void Boundary(){if(sc.mutateAt==got.count){switch(sc.mutation){case 0:At(w.user,0x1d4)=0;break;case 1:At(w.edit,0x1fc)=100;break;case 2:At(w.edit,0x2c)=30;break;case 3:At(w.edit,0x160)=0;break;case 4:At(w.edit,0x1bc)=77;break;case 5:At(w.edit,0x20c)=0;break;case 6:At(w.display,0x6c)=1;break;
case 7:At(w.dialog,0x110)=0;break;
case 8:At(w.target,0x48)=(DWORD)w.surface[2];break;
case 9:At(w.edit,0xc0)=(DWORD)(w.text+1);At(w.edit,0xd0)=3;break;
case 10:At(w.edit,0xe8)=-4;At(w.edit,0xec)=6;break;
case 11:At(w.background,0x6c)=(DWORD)w.surface[2];At(w.background,0x70)=0x5557;break;
case 12:At(w.edit,0xd4)=1;At(w.edit,0xcc)=0x123456;break;}}Finish();if(sc.throwAt==got.count)RaiseException(0xe0471234,0,0,0);}
class Probe{public:int Width();void Redraw();int Blit(void*,void*,void*,int);};
int Probe::Width(){Event&e=Log(1,this);Boundary();return ((UIControl*)this)->UIControl::GetWidth();}
void Probe::Redraw(){Log(2,this);Boundary();((UIControl*)this)->UIControl::RequestRedraw();Finish();}
int Probe::Blit(void*rect,void*texture,void*source,int flags){Event&e=Log(3,this);e.rect=*(RECT*)rect;e.args[0]=(DWORD)texture;e.args[1]=(DWORD)source;e.args[2]=flags;Boundary();return 1;}
// GetDc is the sole scripted lower-level game operation. It always writes its
// documented outputs; the actual Begin/End/Release ownership logic runs above it.
int BackgroundImage::GetDc(void**dc,CameraRect*rect,int index,int dirty,int*frames,int*more,CameraRect*clip){Event&e=Log(6,this);e.rect=*(RECT*)rect;e.args[0]=index;e.args[1]=dirty;e.args[2]=*frames;Boundary();*dc=sc.dcNull?0:(void*)0x5555;*more=(++pass<sc.passes);heldDcSurface=(UnknownSurfaceInterface*)w.surface[1];heldDc=*dc;if(*dc)w.held[1]++;Finish();return !sc.beginFail;}
void UIDialog::DrawTextRuns(void*,int,int,const char*,int,UnknownGameUiTextRun*,unsigned int,int,unsigned int){ExitProcess(83);}
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
static BOOL WINAPI Measure(HDC dc,LPCSTR text,int count,LPSIZE size){Event&e=Log(26,dc);e.args[0]=count;Boundary();size->cx=count*7;size->cy=13;return sc.apiStatus;}
static HGDIOBJ WINAPI Stock(int n){Event&e=Log(10,0);e.args[0]=n;Boundary();return(HGDIOBJ)0x7777;}
static int WINAPI Frame(HDC dc,const RECT*r,HBRUSH b){Event&e=Log(11,dc);e.rect=*r;e.args[0]=(DWORD)b;Boundary();return sc.apiStatus;}
static HIMC WINAPI Context(HWND wnd){Log(12,wnd);Boundary();return sc.imeNull?0:(HIMC)0x8888;}
static BOOL WINAPI Open(HIMC ctx){Log(13,(void*)ctx);Boundary();return !sc.imeClosed;}
static BOOL WINAPI Composition(HIMC ctx,COMPOSITIONFORM*c){Event&e=Log(14,(void*)ctx);e.args[0]=c->dwStyle;e.args[1]=c->ptCurrentPos.x;e.args[2]=c->ptCurrentPos.y;Boundary();return sc.apiStatus;}
static BOOL WINAPI Release(HWND wnd,HIMC ctx){Event&e=Log(15,wnd);e.args[0]=(DWORD)ctx;Boundary();return sc.apiStatus;}

static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}

static int HookSource(){BYTE* base=(BYTE*)GetModuleHandleA(0);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)base;if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE)return 0;DWORD length=pe->OptionalHeader.SizeOfImage,rva=pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;if(!rva||rva>length-sizeof(IMAGE_IMPORT_DESCRIPTOR))return 0;int found=0;
 for(IMAGE_IMPORT_DESCRIPTOR* desc=(IMAGE_IMPORT_DESCRIPTOR*)(base+rva);; ++desc){if((BYTE*)(desc+1)>base+length)return 0;if(!desc->Name)break;if(desc->Name>=length||!desc->OriginalFirstThunk||desc->OriginalFirstThunk>=length||desc->FirstThunk>=length)return 0;DWORD* lookup=(DWORD*)(base+desc->OriginalFirstThunk);DWORD* iat=(DWORD*)(base+desc->FirstThunk);
 for(int n=0;;++n){if((BYTE*)(lookup+n+1)>base+length||(BYTE*)(iat+n+1)>base+length)return 0;if(!lookup[n])break;if(!(lookup[n]&IMAGE_ORDINAL_FLAG)&&lookup[n]<length-3){const char* name=(const char*)(base+lookup[n]+2);DWORD target=0;
 if(!strcmp(name,"FrameRect"))target=(DWORD)Frame;if(!strcmp(name,"GetStockObject"))target=(DWORD)Stock;
 if(!strcmp(name,"ImmGetContext"))target=(DWORD)Context;if(!strcmp(name,"ImmGetOpenStatus"))target=(DWORD)Open;
 if(!strcmp(name,"ImmSetCompositionWindow"))target=(DWORD)Composition;if(!strcmp(name,"ImmReleaseContext"))target=(DWORD)Release;
 if(!strcmp(name,"SelectObject"))target=(DWORD)Select;if(!strcmp(name,"CreateRectRgn"))target=(DWORD)Region;
 if(!strcmp(name,"SelectClipRgn"))target=(DWORD)Clip;if(!strcmp(name,"DeleteObject"))target=(DWORD)Delete;
 if(!strcmp(name,"SetBkMode"))target=(DWORD)BkMode;if(!strcmp(name,"SetTextColor"))target=(DWORD)Color;
 if(!strcmp(name,"TextOutA"))target=(DWORD)Text;if(!strcmp(name,"GetTextExtentPoint32A"))target=(DWORD)Measure;
 if(target){DWORD old;if(!VirtualProtect(&iat[n],4,PAGE_READWRITE,&old))return 0;iat[n]=target;DWORD ignored;VirtualProtect(&iat[n],4,old,&ignored);++found;}}}}printf("hooks %d\n",found);return found==14;}
static int PatchCall(DWORD site,DWORD target,DWORD replacement){BYTE*p=(BYTE*)site;if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=target)return 0;*(DWORD*)(p+1)=replacement-(DWORD)(p+5);return 1;}
static int PatchReference(){
 if(!PatchCall(0x4738e0,0x473da0,Address(&UIEditBox::SetEditText))||!PatchCall(0x47392d,0x404480,Address(&BackgroundImage::DrawImage))||!PatchCall(0x473a69,0x46ed70,Address(&UIDialog::BeginControlDraw))||!PatchCall(0x473abc,0x46f550,Address(&UIDialog::DrawAlignedText))||!PatchCall(0x473b7d,0x46eeb0,Address(&UIDialog::EndControlDraw))||!PatchCall(0x473bb7,0x404c80,Address(&BackgroundImage::ReleaseDc)))return 0;
 if(!PatchCall(0x473bc9,0x533076,(DWORD)Context)||!PatchCall(0x473bd1,0x533070,(DWORD)Open)||!PatchCall(0x473bfc,0x53306a,(DWORD)Composition)||!PatchCall(0x473c13,0x533064,(DWORD)Release))return 0;
 *(DWORD*)0x55005c=(DWORD)Stock;*(DWORD*)0x5502ec=(DWORD)Frame;return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS*e){DWORD c=e->ExceptionRecord->ExceptionCode;if(c!=EXCEPTION_ACCESS_VIOLATION&&c!=0xe0471234)return EXCEPTION_CONTINUE_SEARCH;got.exception=c;if(c==EXCEPTION_ACCESS_VIOLATION){got.kind=e->ExceptionRecord->ExceptionInformation[0];got.address=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Setup(){memset(guard,0xa7,4096);if(sc.nullText){void*p=malloc(64);if(!p)ExitProcess(86);memset(p,0x5a,64);free(p);}memset(&w,0,sizeof(w));memset(&got,0,sizeof(got));pass=0;g_MemTagStack=0;g_UnclaimedBytes=0;g_ControlDrawClipRect=0;memset(&g_ControlDrawRect,0,16);memset(&g_DrawImageClipRect,0,16);memset(&g_DrawImageSourceRect,0,16);memset(&g_AlignedTextSize,0,8);g_ControlDrawResult=g_AlignedTextX=g_AlignedTextY=0;
 At(w.edit,0)=(DWORD)table;At(w.edit,0x18)=(DWORD)w.target;At(w.edit,0x2c)=sc.left;At(w.edit,0x30)=sc.top;At(w.edit,0x34)=sc.left+sc.width;At(w.edit,0x38)=sc.top+sc.height;At(w.edit,0xb8)=(DWORD)w.dialog;At(w.edit,0xc0)=sc.nullText?0:(DWORD)w.text;At(w.edit,0x1f0)=64;strcpy((char*)w.text,"abcd");At(w.edit,0xd0)=4;At(w.edit,0xd8)=sc.align;At(w.edit,0xe8)=sc.marginX;At(w.edit,0xec)=sc.marginY;At(w.edit,0xd4)=sc.shadow;At(w.edit,0x160)=sc.font;At(w.edit,0x1fc)=sc.textWidth;At(w.edit,0x1bc)=6;At(w.edit,0x1b8)=7;At(w.edit,0x1c0)=sc.dirty;At(w.edit,0x20c)=sc.brush?0x1234:0;
 At(w.dialog,0x18)=(DWORD)w.target;At(w.dialog,0x34)=(DWORD)w.user;At(w.dialog,0xdc)=15;At(w.dialog,0x160)=1;At(w.user,0x1d4)=sc.focus?(DWORD)w.edit:0;At(w.target,0)=(DWORD)targetTable;
 if(sc.texture)At(w.edit,0x1cc)=(DWORD)w.texture;if(sc.texture==2)At(w.dialog,0x110)=(DWORD)w.background;
 for(int i=0;i<3;i++)At(w.surface[i],0)=(DWORD)surfaceTable;
 At(w.target,0x48)=(DWORD)w.surface[0];At(w.target,0x14)=3;At(w.target,0x18)=0;At(w.target,0x1c)=42;At(w.target,0xc)=640;At(w.target,0x10)=480;
 At(w.texture,0x70)=(DWORD)w.surface[2];At(w.background,0x18)=(DWORD)w.target;At(w.background,0x3c)=(DWORD)w.surface[1];At(w.background,0x40)=sc.copyValid;At(w.background,0x48)=8;At(w.background,0x50)=(DWORD)w.regions;
 for(int j=0;j<8;j++){At(w.regions,j*64)=3;At(w.regions,j*64+0x38)=42;At(w.regions,j*64+4)=0;At(w.regions,j*64+8)=0;At(w.regions,j*64+12)=640;At(w.regions,j*64+16)=480;}
 At(w.game,0x10)=(DWORD)w.target;At(w.game,0xc)=(DWORD)w.display;At(w.game,0x31c)=0x3333;At(w.display,0x6c)=sc.freeze;g_TrackGame=(TrackGame*)w.game;*(DWORD*)0x56e26c=(DWORD)w.game;
 editReceiver=w.edit;
 if(sc.alias){BYTE*p=sc.alias==1?w.edit+0x224:w.dialog+0x200;memcpy(p,"alias",6);At(w.edit,0xc0)=(DWORD)p;At(w.edit,0xd0)=5;}
 if(sc.fault>=1&&sc.fault<=4){editReceiver=guard+4096-(0x210+sc.fault*4);memcpy(editReceiver,w.edit,0x210+sc.fault*4);At(w.user,0x1d4)=(DWORD)editReceiver;}
 if(sc.fault==5)At(w.edit,0xb8)=1;
 if(sc.fault==6){At(w.edit,0xc0)=(DWORD)(guard+4094);guard[4094]='x';guard[4095]='y';At(w.edit,0xd0)=4;}
 if(sc.fault==7)At(w.target,0x48)=1;
 if(sc.fault==8)At(w.dialog,0x34)=1;
 CaptureGuard();
}
static void Run(int retail){Setup();__try{union{int(UIEditBox::*p)();DWORD a;}u;if(sizeof(u.p)!=4)ExitProcess(82);u.p=&UIEditBox::Draw;if(retail)u.a=0x4738a0;got.value=retail?(((UIEditBox*)editReceiver)->*u.p)():((UIEditBox*)editReceiver)->UIEditBox::Draw();got.complete=1;}__except(Filter(GetExceptionInformation())){}CaptureGuard();got.state=w;got.scratch=ScratchState();if(sc.nullText&&At(w.edit,0xc0)){memcpy(got.allocated,(void*)At(w.edit,0xc0),64);free((void*)At(w.edit,0xc0));}}
static void Check(){Run(1);want=got;Run(0);++checks;if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])++at;printf("Mismatch%d at%d align%d focus%d font%d width%d tex%d freeze%d brush%d passes%d mut%d/%d throw%d exception%x/%x events%d/%d\n",checks,at,sc.align,sc.focus,sc.font,sc.textWidth,sc.texture,sc.freeze,sc.brush,sc.passes,sc.mutateAt,sc.mutation,sc.throwAt,want.exception,got.exception,want.count,got.count);fflush(stdout);ExitProcess(2);}if(!sc.throwAt&&!sc.fault&&(!got.complete||got.exception||got.value!=1))ExitProcess(3);}
int main(int argc,char**argv){setvbuf(stdout,0,_IONBF,0);if(argc!=2)return 4;guard=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD prior;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&prior))return 4;if(!MapReference(argv[1])){puts("Map fail");return 4;}if(!HookSource()){puts("Hook fail");return 4;}if(!PatchReference()){puts("Patch fail");return 4;}surfaceTable[17]=(DWORD)GetDc;surfaceTable[26]=(DWORD)ReleaseDc;surfaceTable[5]=(DWORD)Blt;table[61]=Address(&Probe::Width);table[50]=Address(&Probe::Redraw);targetTable[3]=Address(&Probe::Blit);
 sc.left=10;sc.top=20;sc.width=70;sc.height=40;sc.marginX=2;sc.marginY=3;sc.apiStatus=1;sc.textWidth=100;sc.font=13;sc.align=0x12;sc.focus=1;sc.texture=2;sc.freeze=1;sc.brush=1;sc.passes=1;Check();if(At(w.edit,0x200)!=(DWORD)-30||At(w.edit,0x214)!=80||At(w.edit,0x218)!=38||At(w.edit,0x21c)!=81||At(w.edit,0x220)!=52||At(w.edit,0x1bc)!=3||At(w.edit,0x1c0)!=0)return 5;for(sc.align=0;sc.align<64;++sc.align)for(sc.focus=0;sc.focus<2;++sc.focus)for(sc.texture=0;sc.texture<3;++sc.texture)for(sc.freeze=0;sc.freeze<2;++sc.freeze)for(sc.brush=0;sc.brush<2;++sc.brush)for(sc.passes=1;sc.passes<3;++sc.passes)Check();
 sc.align=0x12;sc.focus=1;sc.texture=2;sc.freeze=1;sc.brush=1;sc.passes=2;
 for(sc.mutateAt=1;sc.mutateAt<=20;++sc.mutateAt)for(sc.mutation=0;sc.mutation<13;++sc.mutation){Check();if(got.count<sc.mutateAt)return 16;}sc.mutateAt=0;
 for(sc.throwAt=1;sc.throwAt<=20;++sc.throwAt){Check();if(got.exception!=0xe0471234||got.count!=sc.throwAt||got.complete)return 17;}sc.throwAt=0;
 for(sc.nullText=0;sc.nullText<2;sc.nullText++)for(sc.beginFail=0;sc.beginFail<2;sc.beginFail++)for(sc.dcStatus=0;sc.dcStatus<2;sc.dcStatus++)for(sc.dcNull=0;sc.dcNull<2;sc.dcNull++)for(sc.imeNull=0;sc.imeNull<2;sc.imeNull++)for(sc.imeClosed=0;sc.imeClosed<2;sc.imeClosed++)Check();
 sc.imeNull=0;sc.imeClosed=0;
 for(int status=0;status<3;status++)for(sc.texture=0;sc.texture<3;sc.texture++)for(sc.nullText=0;sc.nullText<2;sc.nullText++)for(sc.beginFail=0;sc.beginFail<2;sc.beginFail++)for(sc.dcNull=0;sc.dcNull<2;sc.dcNull++)for(sc.apiStatus=0;sc.apiStatus<2;sc.apiStatus++){sc.dcStatus=status==2?0x80004005:status;Check();}
 // Independent rectangle/text/caret arithmetic across signed finite dimensions.
 memset(&sc,0,sizeof(sc));sc.apiStatus=1;sc.passes=1;sc.brush=1;
 int widths[]={-71,-1,0,1,69,70,71},lengths[]={-33,-1,0,32,72},fonts[]={-13,-1,0,1,13};
 for(sc.align=0;sc.align<64;++sc.align)for(int wi=0;wi<7;wi++)for(int li=0;li<5;li++)for(int fi=0;fi<5;fi++){
  sc.width=widths[wi];sc.textWidth=lengths[li];sc.font=fonts[fi];sc.left=wi-3;sc.top=li-3;sc.height=fi*7-13;sc.marginX=li-2;sc.marginY=fi-2;sc.focus=1;sc.shadow=(wi&1);Check();
  int scroll;if(sc.align&1)scroll=sc.width>sc.textWidth?0:sc.width-sc.textWidth;else if(sc.align&2)scroll=sc.width>sc.textWidth?sc.width/2-sc.textWidth/2:sc.width-sc.textWidth;else if(sc.align&4)scroll=sc.width-sc.textWidth;else scroll=sc.width>sc.textWidth?0:sc.width-sc.textWidth;
  int y=sc.top;if(!(sc.align&8)){if(sc.align&16)y+=sc.height/2-sc.font/2;else if(sc.align&32)y+=sc.height-sc.font;}
  int caret=sc.left+scroll+sc.marginX+sc.textWidth,limit=1+sc.left+sc.width-1;if(caret>limit)caret=limit;
  if((int)At(w.edit,0x200)!=scroll||(int)At(w.edit,0x214)!=caret||(int)At(w.edit,0x218)!=y+sc.marginY+1||(int)At(w.edit,0x21c)!=caret+1||(int)At(w.edit,0x220)!=y+sc.marginY+2+(sc.font?sc.font:15)||w.held[0])return 10;
  int texts=0,frames=0;for(int e=0;e<got.count;e++){Event&v=got.events[e];if(v.kind==25){int shadow=sc.shadow&&texts==0;if((int)v.args[0]!=sc.left+scroll+sc.marginX-shadow||(int)v.args[1]!=y+sc.marginY-shadow||v.args[2]!=4)return 11;texts++;}if(v.kind==11)frames++;}
  if(texts!=1+!!sc.shadow||frames!=1||w.gets[0]!=1||w.releases[0]!=1)return 12;
 }
 memset(&sc,0,sizeof(sc));sc.apiStatus=1;sc.left=10;sc.top=20;sc.width=70;sc.height=40;sc.marginX=2;sc.marginY=3;sc.font=13;sc.textWidth=100;sc.align=0x12;sc.focus=1;sc.brush=1;sc.passes=1;
 for(sc.alias=1;sc.alias<=2;sc.alias++)for(sc.texture=0;sc.texture<3;sc.texture++)for(sc.copyValid=0;sc.copyValid<2;sc.copyValid++)for(sc.dirty=0;sc.dirty<2;sc.dirty++)Check();sc.alias=0;sc.texture=0;sc.copyValid=0;sc.dirty=0;
 for(sc.fault=1;sc.fault<=8;sc.fault++){Check();if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION)return 13;if(sc.fault<=4&&(got.kind!=1||got.address!=(DWORD)(guard+4096)))return 14;if(sc.fault==6&&w.held[0]!=1)return 15;if(sc.fault<=4){int expected[]={80,38,81};for(int i=0;i<sc.fault-1;i++)if((int)At(editReceiver,0x214+i*4)!=expected[i])return 18;}}sc.fault=0;
 printf("EditBoxDraw: %d actual-helper and finite platform scenarios match\n",checks);return 0;}
