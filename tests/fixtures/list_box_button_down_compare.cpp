// Private comparison: real GetRowHeight and PtInRect; scripted selection/events/base.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class UIListBox {public:virtual int ButtonDown(int,int);int GetRowHeight(int);int SelectRow(int);};
class UIControl {public:virtual int ButtonDown(int,int);};
class ActualList {public:int Height(int);};
class BaseProbe {public:int Base(int,int);};
class Probe {public:void Report(int*);};
static BYTE referenceSpace[0x800000]={1};
struct World {BYTE control[0x250],dialog[0x40],users[2][0x1e0],rows[2][8][56];POINT point;};
struct Scenario {int button,pointKind,x,y,hover,selectable,scroll,first,visible,rowCount,height,flags,handled,writeHandled,selectResult,baseResult,throwAt,mutateAt,mutation,nullKind,alias,seed;};
struct Event {int kind,who,args[6],result;World before,after;};
struct Observation {int result,completed,exception,faultKind,faultAddress,count;Event events[32];World state;};
static World world;static Scenario scenario;static Observation observed,expected;static int checks;static DWORD controlTable[67];static BOOL(WINAPI* RealPtInRect)(const RECT*,POINT);static BYTE* guard;
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d case%d\n",code,checks);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(void* p){if(!p)return 0;if(p==world.control)return 1;if(p==world.dialog)return 2;if(p==&world.point)return 3;return(int)p;}
static Event& Begin(int kind,void* who){if(observed.count>=32)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);e.before=world;return e;}
static void Boundary(){if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:world.point.x=2;world.point.y=8;break;
 case 1:At(world.control,0x1f0)=2;At(world.control,0x200)=1;break;
 case 2:At(world.control,0x200)=0;break;
 case 3:At(world.control,0x214)=(DWORD)world.rows[1];break;
 case 4:At(world.rows[0][0],4)^=1;break;
 case 5:At(world.rows[0][0],12)=5;break;
 case 6:At(world.control,0x2c)=6;At(world.control,0x30)=7;break;
 case 7:At(world.dialog,0x34)=(DWORD)world.users[1];break;
 case 8:At(world.control,0x224)=0;At(world.control,0x228)=0;break;
 case 9:At(world.control,0xb8)=0;break;
 case 10:At(world.control,0x214)=0;break;
 case 11:At(world.control,0x1ec)=0;break;
 }}if(scenario.throwAt==observed.count)throw 0x735;}
int UIListBox::GetRowHeight(int row){Event& e=Begin(1,this);e.args[0]=row;Boundary();int value=((ActualList*)this)->Height(row);e.result=value;e.after=world;return value;}
int UIListBox::SelectRow(int row){Event& e=Begin(3,this);e.args[0]=row;At(world.control,0x1f4)=row;Boundary();e.result=scenario.selectResult;e.after=world;return scenario.selectResult;}
void Probe::Report(int* handled){Event& e=Begin(4,this);if(!handled)Fail(82);e.args[0]=*handled;Boundary();if(scenario.writeHandled)*handled=scenario.handled;e.result=*handled;e.after=world;}
int BaseProbe::Base(int a,int b){Event& e=Begin(5,this);e.args[0]=a;e.args[1]=Identity((void*)b);Boundary();e.result=scenario.baseResult;e.after=world;return scenario.baseResult;}
int UIControl::ButtonDown(int a,int b){return ((BaseProbe*)this)->Base(a,b);}
static BOOL WINAPI ProbePtInRect(const RECT* rect,POINT point){Event& e=Begin(2,0);memcpy(e.args,rect,16);e.args[4]=point.x;e.args[5]=point.y;BOOL result=RealPtInRect(rect,point);Boundary();e.result=result;e.after=world;return result;}
static int HookSourcePt(){BYTE* base=(BYTE*)GetModuleHandleA(0);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)base;if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE)return 0;DWORD length=pe->OptionalHeader.SizeOfImage,rva=pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;if(!rva||rva>length-sizeof(IMAGE_IMPORT_DESCRIPTOR))return 0;int found=0;
 for(IMAGE_IMPORT_DESCRIPTOR* desc=(IMAGE_IMPORT_DESCRIPTOR*)(base+rva);; ++desc){if((BYTE*)(desc+1)>base+length)return 0;if(!desc->Name)break;if(desc->Name>=length||!desc->OriginalFirstThunk||desc->OriginalFirstThunk>=length||desc->FirstThunk>=length)return 0;DWORD* lookup=(DWORD*)(base+desc->OriginalFirstThunk);DWORD* iat=(DWORD*)(base+desc->FirstThunk);
  for(int n=0;;++n){if((BYTE*)(lookup+n+1)>base+length||(BYTE*)(iat+n+1)>base+length)return 0;if(!lookup[n])break;if(!(lookup[n]&IMAGE_ORDINAL_FLAG)&&lookup[n]<length-3){const char* name=(const char*)(base+lookup[n]+2);if(!strcmp(name,"PtInRect")){if(_stricmp((char*)(base+desc->Name),"USER32.dll"))return 0;RealPtInRect=(BOOL(WINAPI*)(const RECT*,POINT))iat[n];DWORD old;if(!VirtualProtect(&iat[n],4,PAGE_READWRITE,&old))return 0;iat[n]=(DWORD)ProbePtInRect;DWORD ignored;VirtualProtect(&iat[n],4,old,&ignored);++found;}}}}
 return found==1;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int PatchReference(){const DWORD sites[]={0x477f1b,0x477f58,0x477f7c,0x477fc2,0x477fd8};const DWORD targets[]={0x4769a0,0x4769a0,0x476a60,0x4721b0,0x4721b0};DWORD replacements[]={Address(&UIListBox::GetRowHeight),Address(&UIListBox::GetRowHeight),Address(&UIListBox::SelectRow),Address(&BaseProbe::Base),Address(&BaseProbe::Base)};
 for(int i=0;i<5;++i){BYTE* p=(BYTE*)sites[i];if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=targets[i])return 0;*(DWORD*)(p+1)=replacements[i]-(DWORD)(p+5);}*(DWORD*)0x5502d4=(DWORD)ProbePtInRect;return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(int retail){int point=(int)&world.point;if(scenario.pointKind==1)point=0;if(scenario.pointKind==2)point=1;if(scenario.pointKind==3)point=(int)(guard+4092);if(scenario.pointKind==4)point=(int)(world.control+0x2c);__try{union{int(UIListBox::*p)(int,int);DWORD a;}call;call.a=0x477e90;observed.result=retail?(((UIListBox*)world.control)->*call.p)(scenario.button,point):((UIListBox*)world.control)->UIListBox::ButtonDown(scenario.button,point);observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,scenario.seed&255,sizeof(world));world.point.x=scenario.x;world.point.y=scenario.y;At(world.control,0)=(DWORD)controlTable;At(world.control,0xb8)=(DWORD)world.dialog;At(world.dialog,0x34)=(DWORD)world.users[0];At(world.users[0],0x1d8)=scenario.hover?(DWORD)world.control:0;At(world.users[1],0x1d8)=0;At(world.control,0x2c)=1;At(world.control,0x30)=2;At(world.control,0x34)=9;At(world.control,0x38)=20;At(world.control,0x1ec)=scenario.rowCount;At(world.control,0x1f0)=scenario.first;At(world.control,0x1f4)=-1;At(world.control,0x200)=scenario.visible;At(world.control,0x214)=(DWORD)world.rows[0];At(world.control,0x224)=scenario.selectable;At(world.control,0x228)=scenario.scroll;
 for(int j=0;j<2;++j)for(int i=0;i<8;++i){At(world.rows[j][i],4)=(scenario.flags>>i)&1;At(world.rows[j][i],12)=scenario.height+(scenario.alias?(i%3):0);if(j)At(world.rows[j][i],4)^=1;}
 if(scenario.nullKind==1)At(world.control,0xb8)=0;if(scenario.nullKind==2)At(world.dialog,0x34)=0;if(scenario.nullKind==3)At(world.control,0x214)=0;try{Invoke(retail);}catch(int code){observed.exception=code;}observed.state=world;}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d offset%d %02x/%02x pointKind%d point%d/%d button%d rows%d/%d height%d flags%x mutation%d/%d throw%d fault%x/%x events%d/%d\n",checks,offset,a[offset],b[offset],scenario.pointKind,scenario.x,scenario.y,scenario.button,scenario.first,scenario.visible,scenario.height,scenario.flags,scenario.mutateAt,scenario.mutation,scenario.throwAt,expected.faultAddress,observed.faultAddress,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.hover=scenario.selectable=scenario.scroll=1;scenario.first=0;scenario.visible=3;scenario.rowCount=8;scenario.height=3;scenario.x=3;scenario.y=3;scenario.handled=scenario.writeHandled=1;scenario.selectResult=-17;scenario.baseResult=-23;scenario.seed=19;}
static int Count(int kind){int n=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++n;return n;}
static void Oracle(int result,int selected,int base){if(observed.exception||!observed.completed||observed.result!=result||At(world.control,0x1f4)!=(DWORD)selected||Count(5)!=base)Fail(90);}
int main(int argc,char** argv){if(argc<2||!HookSourcePt()||!MapReference(argv[1]))return 3;guard=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&old))return 4;At(guard,4092)=3;controlTable[66]=Address(&Probe::Report);if(!PatchReference())return 5;
 Base();scenario.pointKind=2;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultKind||observed.faultAddress!=5||observed.count!=1)Fail(91);
 Base();Compare();Oracle(1,0,0);if(Count(1)!=1||Count(2)!=1||Count(3)!=1||Count(4)!=1)Fail(97);scenario.handled=0;Compare();Oracle(-23,0,1);scenario.handled=-1;Compare();Oracle(1,0,0);scenario.writeHandled=0;Compare();Oracle(-23,0,1);
 Base();scenario.x=10;Compare();Oracle(-23,-1,1);if(Count(1)!=6||Count(2)!=3||Count(3)||Count(4))Fail(98);
 const int outcomes[]={0,1,-77};for(int select=0;select<3;++select)for(int base=0;base<3;++base)for(int handle=0;handle<3;++handle){Base();scenario.selectResult=outcomes[select];scenario.baseResult=outcomes[base];scenario.handled=outcomes[handle];Compare();Oracle(handle?1:outcomes[base],0,handle?0:1);}
 for(int x=-1;x<=10;++x)for(int y=-1;y<=13;++y)for(int flags=0;flags<8;++flags){Base();scenario.x=x;scenario.y=y;scenario.flags=flags;Compare();int row=(y-2)/3;int hit=x>=1&&x<9&&y>=2&&y<11&&!(flags&(1<<row));Oracle(hit?1:-23,hit?row:-1,hit?0:1);}
 const int buttons[]={0,1,2,-1,0x7fffffff};for(int button=0;button<5;++button)for(int hover=0;hover<2;++hover)for(int selectable=0;selectable<2;++selectable)for(int scroll=0;scroll<2;++scroll)for(int point=0;point<2;++point){Base();scenario.button=buttons[button];scenario.hover=hover;scenario.selectable=selectable;scenario.scroll=scroll;scenario.pointKind=point;Compare();int hit=hover&&selectable&&!point&&!button;Oracle(hit?1:-23,hit?0:-1,hit?0:1);if(button==1&&hover&&scroll&&!point&&At(world.control,0x22c)!=3)Fail(92);}
 for(int first=-1;first<=3;++first)for(int visible=-1;visible<=4;++visible)for(int count=0;count<=8;++count)for(int height=-1;height<=4;++height){Base();scenario.first=first;scenario.visible=visible;scenario.rowCount=count;scenario.height=height;Compare();}
 for(int path=0;path<6;++path){Base();if(path==1)scenario.x=10;if(path==2)scenario.flags=7;if(path==3)scenario.handled=0;if(path==4)scenario.button=1;if(path==5)scenario.pointKind=1;Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();if(observed.exception!=0x735||observed.count!=at)Fail(93);scenario.throwAt=0;for(int mutation=0;mutation<12;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int kind=1;kind<=3;++kind){Base();scenario.nullKind=kind;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)Fail(94);}
 Base();scenario.pointKind=3;Compare();if(observed.faultAddress!=(int)(guard+4096)||observed.count!=1)Fail(95);Base();scenario.pointKind=4;Compare();Oracle(1,0,0);
 for(int pattern=0;pattern<256;++pattern)for(int position=0;position<20;++position){Base();scenario.alias=1;scenario.flags=pattern;scenario.y=position;Compare();}
 printf("ListBoxButtonDown: %d row, real-helper, API, callback and fault scenarios match\n",checks);VirtualFree(guard,0,MEM_RELEASE);return 0;}
