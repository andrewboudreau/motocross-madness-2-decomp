// Private whole-body comparison; actual helpers, bounded scripted surfaces.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
class TrackGame;class TextureMap;
class UIButton{public:virtual int ContainsPoint(POINT,int);};
class UIMultiState{public:virtual int ContainsPoint(POINT,int);};
class ActualControl{public:TextureMap* Query(int);};
class ActualMulti{public:TextureMap* Query(int);};
class Probe{public:TextureMap* Texture(int);};
int ActualBytesPerPixel(int);
extern "C" void _ftol();
static BYTE referenceSpace[0x800000]={1};
struct Surface{DWORD table;int locks,unlocks;};
struct World{BYTE control[0x200],dialog[0xa00],games[2][0x3500],target[0x100],textures[2][0xc0],anims[2][0xf8],frames[2][0x24],states[2][32],pixels[8192];Surface surfaces[2];};
static World world;TrackGame* g_TrackGame;
struct Scenario{int multi,seed,x,y,left,top,right,bottom,scaleX,scaleY,format,pitch,key,fill,lockStatus,unlockStatus,invalid,round,state,frameSound,selected,focus,alias,throwAt,mutateAt,mutation;};
static Scenario scenario;
struct State{World world;int game,fp;};
struct Event{int kind,who,args[6],result;BYTE descriptor[124];State before,after;};
struct Observation{int result,completed,exception,faultKind,faultAddress,count,fp;Event events[8];State state;};
static Observation observed,expected;static int checks,retailActive;static DWORD controlTable[64],surfaceTable[33];static BOOL(WINAPI* RealPtInRect)(const RECT*,POINT);
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d case%d retail%d\n",code,checks,retailActive);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.control)return 1;if(p==world.control+0x2c)return 2;for(int i=0;i<2;++i){if(p==world.textures[i])return 10+i;if(p==&world.surfaces[i])return 20+i;if(p==world.games[i])return 30+i;}return(int)p;}
static void Sync(){if(retailActive)*(TrackGame**)0x56e26c=g_TrackGame;}
static void Snapshot(State& s){s.world=world;s.game=Identity(g_TrackGame);s.fp=_statusfp();}
static Event& Begin(int kind,const void* who){if(observed.count>=8)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);Snapshot(e.before);return e;}
static void Boundary(void* desc=0){if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:At(world.control,0x2c)+=2;At(world.control,0x30)-=1;break;
 case 1:At(world.dialog,0x9c4)=0x40000000;At(world.dialog,0x9c8)=0xbf800000;break;
 case 2:At(world.textures[0],0x70)=(DWORD)&world.surfaces[1];break;
 case 3:At(world.target,0x28)=8;break;
 case 4:g_TrackGame=(TrackGame*)world.games[1];break;
 case 5:At(world.control,0x60)=4;At(world.control,0x1f0)=1;break;
 case 6:if(desc)At(desc,0x10)=-17;break;
 case 7:if(desc)At(desc,0x40)^=0xffff;break;
 case 8:if(desc)At(desc,0x24)=(DWORD)(world.pixels+4097);break;
 case 9:memset(world.pixels+4000,0xa7,256);break;
 case 10:At(world.textures[0],0x70)=0;break;
 case 11:At(world.control,0xb8)=0;break;
 }}Sync();if(scenario.throwAt==observed.count)throw 0x733;}
static BOOL WINAPI ProbePtInRect(const RECT* rect,POINT point){Event& e=Begin(1,0);e.args[0]=rect==(RECT*)(world.control+0x2c);memcpy(e.args+1,rect,16);e.args[5]=point.x;memcpy(e.descriptor,&point,8);BOOL result=RealPtInRect(rect,point);e.result=result;Boundary();Snapshot(e.after);return result;}
TextureMap* Probe::Texture(int state){Event& e=Begin(2,this);e.args[0]=state;if(state!=-1)Fail(82);Boundary();TextureMap* result=scenario.multi?((ActualMulti*)this)->Query(state):((ActualControl*)this)->Query(state);if(scenario.invalid==1)result=0;e.result=Identity(result);Snapshot(e.after);return result;}
int PixelFormatBytesPerPixel(int format){Event& e=Begin(3,0);e.args[0]=format;Boundary();int result=ActualBytesPerPixel(format);e.result=result;Snapshot(e.after);return result;}
static long __stdcall Lock(void* who,void* rect,void* descriptor,int flags,void* event){Event& e=Begin(4,who);e.args[0]=Identity(rect);e.args[1]=flags;e.args[2]=Identity(event);e.args[3]=At(descriptor,0);if(rect||event||flags!=1||At(descriptor,0)!=124)Fail(83);
 if(!scenario.lockStatus){memset(descriptor,0,124);At(descriptor,0)=124;At(descriptor,0x10)=scenario.pitch;At(descriptor,0x24)=(DWORD)(scenario.alias?world.control+0x100:world.pixels+4096);At(descriptor,0x40)=scenario.key;if(scenario.invalid==2)At(descriptor,0x24)=1;++((Surface*)who)->locks;}
 Boundary(!scenario.lockStatus?descriptor:0);if(!scenario.lockStatus)memcpy(e.descriptor,descriptor,124);e.result=scenario.lockStatus;Snapshot(e.after);return scenario.lockStatus;}
static long __stdcall Unlock(void* who,void* rect){Event& e=Begin(5,who);e.args[0]=Identity(rect);if(rect)Fail(84);++((Surface*)who)->unlocks;if(!scenario.unlockStatus&&((Surface*)who)->locks>0)--((Surface*)who)->locks;Boundary();e.result=scenario.unlockStatus;Snapshot(e.after);return scenario.unlockStatus;}
static int HookSourcePt(){BYTE* base=(BYTE*)GetModuleHandleA(0);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)base;if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE)return 0;DWORD length=pe->OptionalHeader.SizeOfImage,rva=pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;if(!rva||rva>length-sizeof(IMAGE_IMPORT_DESCRIPTOR))return 0;int found=0;
 for(IMAGE_IMPORT_DESCRIPTOR* desc=(IMAGE_IMPORT_DESCRIPTOR*)(base+rva);desc->Name;++desc){if((BYTE*)(desc+1)>base+length||desc->Name>=length||!desc->OriginalFirstThunk||desc->OriginalFirstThunk>=length||desc->FirstThunk>=length)return 0;DWORD* lookup=(DWORD*)(base+desc->OriginalFirstThunk);DWORD* iat=(DWORD*)(base+desc->FirstThunk);
  for(int n=0;lookup[n];++n){if((BYTE*)(lookup+n+1)>base+length||(BYTE*)(iat+n+1)>base+length)return 0;if(!(lookup[n]&IMAGE_ORDINAL_FLAG)&&lookup[n]<length-3){const char* name=(const char*)(base+lookup[n]+2);if(!strcmp(name,"PtInRect")){if(_stricmp((char*)(base+desc->Name),"USER32.dll"))return 0;RealPtInRect=(BOOL(WINAPI*)(const RECT*,POINT))iat[n];DWORD old;if(!VirtualProtect(&iat[n],4,PAGE_READWRITE,&old))return 0;iat[n]=(DWORD)ProbePtInRect;DWORD ignored;VirtualProtect(&iat[n],4,old,&ignored);++found;}}}}
 return found==1;}
static int PatchReference(){if(memcmp((void*)_ftol,(void*)0x534a88,39))return 0;const DWORD sites[]={0x47354d,0x473561,0x473597,0x478e80,0x478e94,0x478ece};for(int i=0;i<6;++i){BYTE* p=(BYTE*)sites[i];DWORD original=i%3==2?0x511970:0x534a88;if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=original)return 0;*(DWORD*)(p+1)=(i%3==2?(DWORD)PixelFormatBytesPerPixel:(DWORD)_ftol)-(DWORD)(p+5);}*(DWORD*)0x5502d4=(DWORD)ProbePtInRect;return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int MapReference(const char* path){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=EXCEPTION_INT_DIVIDE_BY_ZERO&&code!=EXCEPTION_INT_OVERFLOW)return EXCEPTION_CONTINUE_SEARCH;observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){POINT point={scenario.x,scenario.y};__try{if(scenario.multi){union{int(UIMultiState::*p)(POINT,int);DWORD a;}call;call.a=0x478e10;observed.result=retailActive?(((UIMultiState*)world.control)->*call.p)(point,scenario.state):((UIMultiState*)world.control)->UIMultiState::ContainsPoint(point,scenario.state);}else{union{int(UIButton::*p)(POINT,int);DWORD a;}call;call.a=0x4734c0;observed.result=retailActive?(((UIButton*)world.control)->*call.p)(point,scenario.state):((UIButton*)world.control)->UIButton::ContainsPoint(point,scenario.state);}observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,scenario.seed&255,sizeof(world));At(world.control,0)=(DWORD)controlTable;At(world.control,0x2c)=scenario.left;At(world.control,0x30)=scenario.top;At(world.control,0x34)=scenario.right;At(world.control,0x38)=scenario.bottom;At(world.control,0xb8)=(DWORD)world.dialog;At(world.dialog,0x9c4)=scenario.scaleX;At(world.dialog,0x9c8)=scenario.scaleY;At(world.target,0x28)=scenario.format;At(world.control,0x60)=scenario.focus?4:0;At(world.control,0x1f0)=scenario.selected;At(world.control,0x1f4)=(DWORD)world.states;
 for(int i=0;i<2;++i){At(world.games[i],0x10)=(DWORD)world.target;At(world.textures[i],0x70)=(DWORD)&world.surfaces[i];world.surfaces[i].table=(DWORD)surfaceTable;world.surfaces[i].locks=world.surfaces[i].unlocks=0;At(world.anims[i],8)=0;At(world.anims[i],0x28)=(DWORD)world.frames[i];At(world.frames[i],8)=scenario.frameSound;At(world.frames[i],0x18)=(DWORD)world.textures[i];At(world.states[i],0)=(DWORD)world.anims[i];At(world.states[i],4)=(DWORD)world.anims[1-i];}
 for(int st=0;st<5;++st)At(world.control,0x16c+st*4)=(DWORD)world.anims[st==4?1:0];memset(world.pixels,scenario.fill,8192);if(scenario.fill<0)for(int pixel=0;pixel<8192;++pixel)world.pixels[pixel]=(BYTE)(pixel*37+pixel/13);g_TrackGame=(TrackGame*)world.games[0];switch(scenario.invalid){case 3:At(world.textures[0],0x70)=0;break;case 4:g_TrackGame=0;break;case 5:At(world.games[0],0x10)=0;break;case 6:At(world.control,0xb8)=0;break;case 7:At(world.control,0x16c)=0;At(world.states[0],0)=0;break;}
 Sync();_fpreset();_controlfp(scenario.round,_MCW_RC);try{Invoke();}catch(int code){observed.exception=code;}observed.fp=_statusfp();Snapshot(observed.state);_fpreset();}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d offset%d %02x/%02x multi%d xy%d/%d rect%d/%d/%d/%d format%d pitch%d scales%x/%x mutation%d/%d throw%d invalid%d counts%d/%d fp%x/%x\n",checks,offset,a[offset],b[offset],scenario.multi,scenario.x,scenario.y,scenario.left,scenario.top,scenario.right,scenario.bottom,scenario.format,scenario.pitch,scenario.scaleX,scenario.scaleY,scenario.mutateAt,scenario.mutation,scenario.throwAt,scenario.invalid,expected.count,observed.count,expected.fp,observed.fp);fflush(stdout);ExitProcess(2);}}
static void Base(int multi){memset(&scenario,0,sizeof(scenario));scenario.multi=multi;scenario.seed=19;scenario.x=3;scenario.y=4;scenario.left=1;scenario.top=2;scenario.right=9;scenario.bottom=10;scenario.scaleX=scenario.scaleY=0x3f800000;scenario.format=565;scenario.pitch=16;scenario.fill=0x35;scenario.key=0x1234;scenario.state=0x12345678;}
static int Count(int kind){int n=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++n;return n;}
static void Oracle(int result){if(!observed.completed||observed.exception||observed.result!=result||observed.count!=5||Count(1)!=1||Count(2)!=1||Count(3)!=1||Count(4)!=1||Count(5)!=1||world.surfaces[0].locks||world.surfaces[0].unlocks!=1||observed.events[0].args[0]!=scenario.multi)Fail(90);}
int main(int argc,char** argv){if(argc!=2||!HookSourcePt()||!MapReference(argv[1])||!PatchReference())return 3;controlTable[48]=Address(&Probe::Texture);surfaceTable[25]=(DWORD)Lock;surfaceTable[32]=(DWORD)Unlock;
 for(int multi=0;multi<2;++multi){Base(multi);Compare();Oracle(1);scenario.key=0x3535;Compare();Oracle(0);scenario.format=8;scenario.key=0x35;Compare();Oracle(0);scenario.key=0x135;Compare();Oracle(0);scenario.key=0x36;Compare();Oracle(1);
  Base(multi);scenario.x=scenario.right;Compare();if(observed.result||observed.count!=1||observed.exception)Fail(91);
  Base(multi);scenario.lockStatus=1;Compare();if(observed.result||observed.count!=3||world.surfaces[0].locks)Fail(92);
  Base(multi);scenario.unlockStatus=-1;Compare();if(observed.result!=1||world.surfaces[0].locks!=1)Fail(93);
  Base(multi);scenario.invalid=2;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||world.surfaces[0].locks!=1||Count(5))Fail(94);
  for(int x=-2;x<=11;++x)for(int y=-2;y<=11;++y){Base(multi);scenario.x=x;scenario.y=y;Compare();int inside=x>=1&&x<9&&y>=2&&y<10;if(observed.result!=inside||observed.exception||observed.count!=(inside?5:1))Fail(95);}
  const int formats[]={8,555,565,888,1555,4444,8888};const int pitches[]={-33,-17,-2,-1,0,1,2,17,33};for(int format=0;format<7;++format)for(int pitch=0;pitch<9;++pitch)for(int alias=0;alias<2;++alias)for(int fill=0;fill<4;++fill){Base(multi);scenario.format=formats[format];scenario.pitch=pitches[pitch];scenario.alias=alias;scenario.fill=fill*85;scenario.key=fill&1?0xffff:0;Compare();}
  // Nonuniform independent pixel oracle detects coordinate/stride errors.
  for(int pf=0;pf<7;++pf)for(int pi=0;pi<9;++pi)for(int half=0;half<2;++half){Base(multi);scenario.format=formats[pf];scenario.pitch=pitches[pi];scenario.fill=-1;scenario.key=0x1234;if(half)scenario.scaleX=scenario.scaleY=0x40000000;Compare();int bpp=pf==0?1:(pf==3?3:(pf==6?4:2));int index=scenario.pitch*(half?1:2)/bpp+(half?1:2);int value=pf==0?world.pixels[4096+index]:*(WORD*)(world.pixels+4096+index*2);Oracle(value!=(pf==0?0x34:0x1234));}
  const int scales[]={0x3f000000,0x3f800000,0x3fc00000,0x40000000,0xbf800000,0x7f800000,0xff800000,0x7fc01234,0};const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};for(int sx=0;sx<9;++sx)for(int sy=0;sy<9;++sy)for(int rounding=0;rounding<4;++rounding){Base(multi);scenario.scaleX=scales[sx];scenario.scaleY=scales[sy];scenario.round=rounds[rounding];Compare();}
  const int statuses[]={0,1,-1,0x80000000,0x7fffffff};for(int lock=0;lock<5;++lock)for(int unlock=0;unlock<5;++unlock){Base(multi);scenario.lockStatus=statuses[lock];scenario.unlockStatus=statuses[unlock];Compare();}
  for(int path=0;path<4;++path){Base(multi);if(path==1)scenario.format=8;if(path==2)scenario.lockStatus=-1;if(path==3)scenario.x=-1;Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<12;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
  for(int invalid=1;invalid<=7;++invalid){Base(multi);scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)Fail(96);}
  for(int focus=0;focus<2;++focus)for(int selected=0;selected<2;++selected)for(int sound=0;sound<2;++sound){Base(multi);scenario.focus=focus;scenario.selected=selected;scenario.frameSound=sound;Compare();if(sound&&observed.exception!=EXCEPTION_ACCESS_VIOLATION)Fail(97);}
  const int badFormats[]={0,16,24,32,-1};for(int bad=0;bad<5;++bad){Base(multi);scenario.format=badFormats[bad];Compare();if(observed.exception!=EXCEPTION_INT_DIVIDE_BY_ZERO||world.surfaces[0].locks!=1||Count(5))Fail(98);}
 }
 printf("ControlHitTest: %d pixel, real-helper, callback, FP and held-lock scenarios match\n",checks);return 0;}
