// Complete UIEditBox handler, actual UI/sound helpers; explicit platform boundaries.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class UIEditBox {public:virtual int OnChar(int);};
class UIControl {public:virtual void RequestRedraw();};
class TrackGame;TrackGame* g_TrackGame;
class Probe {public:void Report(int*);int Child(int);void Redraw();};
static BYTE referenceSpace[0x800000]={1};
struct World {BYTE control[0x250],dialog[2][0x580],gui[0x380],user[2][0x1e0],target[2][0x60],surface[2][8],children[2][0x30],text[2][128],sound[2][0x200],group[0x50],audio[0x480],game[0x10],buffers[3][8];DWORD duplicates[2];};
struct Scenario {int value,length,cursor,capacity,focus,lead,drop,accepted,dbcs,previousNull,handled,baseGate,child,childResult,dcFail,measureFail,sounds,audio,groupMode,status,volume,volumeFail,playFail,mutateAt,mutation,throwAt,nullKind,guardKind,guardCut,alias,seed;};
struct Event {int kind,who,args[12],result;BYTE payload[128];World before,after;};
struct Observation {int result,complete,exception,faultKind,faultAddress,count;Event events[40];World state;BYTE page[4096];};
static World world;static Scenario sc;static Observation got,want;static int checks;static BYTE* guard;static BYTE* activeControl;static DWORD controls[70],dialogs[32],surfaces[32],children[27],soundBuffers[24];static char acceptedText[]="aBC09 ";
static DWORD& At(void* p,int o){return *(DWORD*)((BYTE*)p+o);}
static void Fail(int code){printf("Invariant%d check%d\n",code,checks);fflush(stdout);ExitProcess(code);}
extern "C" void Unexpected(){Fail(88);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Id(void* p){return (int)p;}
static Event& Begin(int kind,void* receiver){if(got.count>=40)Fail(81);Event& e=got.events[got.count++];e.kind=kind;e.who=Id(receiver);e.before=world;return e;}
static void Boundary(){if(sc.mutateAt==got.count){switch(sc.mutation){
case 0:At(activeControl,0x1ec)=0;break;
case 1:At(activeControl,0xc0)=(DWORD)world.text[1];break;
case 2:At(activeControl,0xd0)=2;break;
case 3:At(activeControl,0x1f0)=1;break;
case 4:At(activeControl,0xb8)=(DWORD)world.dialog[1];break;
case 5:At(world.user[0],0x1d4)=0;break;
case 6:At(activeControl,0x228)=1;At(activeControl,0x22c)=1;break;
case 7:At(activeControl,0x204)=(DWORD)world.sound[1];At(activeControl,0x208)=0;break;
case 8:At(activeControl,0x128)=0;At(world.dialog[0],0xd8)=0x3344;break;
case 9:At(activeControl,0x210)=(DWORD)acceptedText;break;
case 10:At(activeControl,0xc0)=0;break;
case 11:At(activeControl,0xb8)=0;break;
case 12:At(world.target[0],0x48)=(DWORD)world.surface[1];break;
case 13:At(activeControl,0x74)=0x1357;world.text[0][0]='Z';break;
}}if(sc.throwAt==got.count)RaiseException(0xe0451234,0,0,0);}
void Probe::Report(int* p){Event& e=Begin(1,this);int kind=p[2];memcpy(e.args,p,6*4);e.args[8]=p[8];if(kind==11){e.args[6]=p[6];e.args[7]=p[7];}Boundary();if(kind==11){p[8]=(sc.handled&1)?1:0;p[6]=sc.baseGate;}else if(kind==10)p[8]=(sc.handled&2)?1:0;else if(kind==19)p[8]=(sc.handled&4)?1:0;e.result=p[8];e.after=world;}
int Probe::Child(int value){Event& e=Begin(2,this);e.args[0]=value;Boundary();e.result=sc.childResult;e.after=world;return sc.childResult;}
void Probe::Redraw(){Event& e=Begin(3,this);Boundary();((UIControl*)this)->UIControl::RequestRedraw();e.after=world;}
static int IsLead(BYTE c){return sc.dbcs&&((c>=0x81&&c<=0x9f)||(c>=0xe0&&c<=0xfc));}
static BOOL WINAPI Lead(BYTE c){Event& e=Begin(4,0);e.args[0]=c;Boundary();e.result=IsLead(c);e.after=world;return e.result;}
static LPSTR WINAPI Previous(LPCSTR start,LPCSTR current){Event& e=Begin(5,0);e.args[0]=Id((void*)start);e.args[1]=Id((void*)current);const char* p=start;const char* prev=start;while(p<current){prev=p;BYTE c=*p++;if(IsLead(c)&&p<current)++p;}Boundary();e.result=sc.previousNull?0:Id((void*)prev);e.after=world;return (LPSTR)e.result;}
static HGDIOBJ WINAPI Select(HDC dc,HGDIOBJ object){Event& e=Begin(6,dc);e.args[0]=(int)object;Boundary();e.result=0x4321;e.after=world;return(HGDIOBJ)e.result;}
static BOOL WINAPI Measure(HDC dc,LPCSTR text,int length,LPSIZE size){Event& e=Begin(7,dc);e.args[0]=Id((void*)text);e.args[1]=length;if(length>128)Fail(82);if(length>0)memcpy(e.payload,text,length);int width=0;for(int i=0;i<length;++i)width+=((BYTE)text[i]%7)+1;Boundary();size->cx=width;size->cy=17;e.result=(sc.measureFail||length<0)?0:1;e.after=world;return e.result;}
static long __stdcall GetDc(void* self,void** dc){Event& e=Begin(8,self);Boundary();if(!sc.dcFail)*dc=(void*)(self==world.surface[0]?0x1234:0x2345);e.result=sc.dcFail?0x80004005:0;e.after=world;return e.result;}
static long __stdcall ReleaseDc(void* self,void* dc){Event& e=Begin(9,self);e.args[0]=Id(dc);Boundary();e.result=-37;e.after=world;return e.result;}
static long __stdcall Status(void* self,DWORD* status){Event& e=Begin(10,self);Boundary();*status=self==world.buffers[2]?0:sc.status;e.args[0]=*status;e.result=0;e.after=world;return 0;}
static long __stdcall Volume(void* self,long volume){Event& e=Begin(11,self);e.args[0]=volume;Boundary();e.result=sc.volumeFail?-1:0;e.after=world;return e.result;}
static long __stdcall Play(void* self,DWORD a,DWORD b,DWORD c){Event& e=Begin(12,self);e.args[0]=a;e.args[1]=b;e.args[2]=c;Boundary();e.result=sc.playFail?-1:0;e.after=world;return e.result;}
static void WINAPI Lock(LPCRITICAL_SECTION){Unexpected();}
static int HookSource(){BYTE* base=(BYTE*)GetModuleHandleA(0);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)base;if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(base+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE)return 0;DWORD length=pe->OptionalHeader.SizeOfImage,rva=pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;if(!rva||rva>length-sizeof(IMAGE_IMPORT_DESCRIPTOR))return 0;int found=0;
 for(IMAGE_IMPORT_DESCRIPTOR* desc=(IMAGE_IMPORT_DESCRIPTOR*)(base+rva);; ++desc){if((BYTE*)(desc+1)>base+length)return 0;if(!desc->Name)break;if(desc->Name>=length||!desc->OriginalFirstThunk||desc->OriginalFirstThunk>=length||desc->FirstThunk>=length)return 0;DWORD* lookup=(DWORD*)(base+desc->OriginalFirstThunk);DWORD* iat=(DWORD*)(base+desc->FirstThunk);
 for(int n=0;;++n){if((BYTE*)(lookup+n+1)>base+length||(BYTE*)(iat+n+1)>base+length)return 0;if(!lookup[n])break;if(!(lookup[n]&IMAGE_ORDINAL_FLAG)&&lookup[n]<length-3){const char* name=(const char*)(base+lookup[n]+2);DWORD target=0;if(!strcmp(name,"CharPrevA"))target=(DWORD)Previous;if(!strcmp(name,"IsDBCSLeadByte"))target=(DWORD)Lead;if(!strcmp(name,"SelectObject"))target=(DWORD)Select;if(!strcmp(name,"GetTextExtentPoint32A"))target=(DWORD)Measure;if(target){DWORD old;if(!VirtualProtect(&iat[n],4,PAGE_READWRITE,&old))return 0;iat[n]=target;DWORD ignored;VirtualProtect(&iat[n],4,old,&ignored);++found;}}}}return found==4;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int PatchCall(DWORD site,DWORD target,DWORD replacement){BYTE* p=(BYTE*)site;if(*p!=0xe8||(DWORD)(p+5)+*(DWORD*)(p+1)!=target)return 0;*(DWORD*)(p+1)=replacement-(DWORD)(p+5);return 1;}
static int PatchReference(){const DWORD sites[]={0x47422d,0x474269,0x474507};for(int i=0;i<3;++i)if(!PatchCall(sites[i],0x536d00,(DWORD)memmove))return 0;
 // Only the six verified direct CRT calls are redirected; game helper calls stay intact.
 const DWORD strchrSites[]={0x47400e,0x47402f,0x47404c};for(i=0;i<3;++i)if(!PatchCall(strchrSites[i],0x535f10,(DWORD)strchr))return 0;
 *(DWORD*)0x5502f0=(DWORD)Previous;*(DWORD*)0x5501bc=(DWORD)Lead;*(DWORD*)0x550078=(DWORD)Select;*(DWORD*)0x550084=(DWORD)Measure;
 *(DWORD*)0x55025c=(DWORD)Lock;*(DWORD*)0x550284=(DWORD)Lock;
 return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=0xe0451234)return EXCEPTION_CONTINUE_SEARCH;got.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){got.faultKind=e->ExceptionRecord->ExceptionInformation[0];got.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(int retail){__try{union{int(UIEditBox::*p)(int);DWORD a;}call;call.a=0x474150;got.result=retail?(((UIEditBox*)activeControl)->*call.p)(sc.value):((UIEditBox*)activeControl)->UIEditBox::OnChar(sc.value);got.complete=1;}__except(Filter(GetExceptionInformation())){}}
static void Setup(){memset(&world,0,sizeof(world));memset(guard,0x59,4096);activeControl=world.control;
 for(int i=0;i<2;++i){At(world.dialog[i],0)=(DWORD)dialogs;At(world.dialog[i],0x34)=(DWORD)world.user[i];At(world.dialog[i],0x18)=(DWORD)world.target[i];At(world.dialog[i],0xd8)=0x5678+i;At(world.target[i],0x48)=(DWORD)world.surface[i];At(world.surface[i],0)=(DWORD)surfaces;At(world.children[i],0)=(DWORD)children;world.children[i][0x25]=1;At(world.sound[i],8)=(DWORD)world.group;At(world.sound[i],12)=(DWORD)world.buffers[i];At(world.sound[i],0x20)=(DWORD)world.duplicates;At(world.sound[i],0x24)=1;At(world.sound[i],0x1a0)=-22;At(world.sound[i],0x1e8)=0x20;world.sound[i][0x1f5]=1;for(int j=0;j<128;++j)world.text[i][j]=(BYTE)('a'+(j+i)%25);world.text[i][sc.length]=0;}
 for(i=0;i<3;++i)At(world.buffers[i],0)=(DWORD)soundBuffers;
 world.duplicates[0]=(DWORD)world.buffers[2];world.duplicates[1]=0;world.group[0x2c]=sc.groupMode==1?0:1;world.group[0x25]=sc.groupMode==2?4:0;
 At(world.game,4)=(DWORD)world.audio;world.audio[0x2c]=(BYTE)sc.audio;g_TrackGame=(TrackGame*)world.game;*(DWORD*)0x56e26c=(DWORD)world.game;
 At(world.control,0)=(DWORD)controls;At(world.control,0xb8)=(DWORD)world.dialog[0];At(world.control,0xbc)=(DWORD)world.gui;At(world.control,0xc0)=(DWORD)world.text[0];At(world.control,0xd0)=sc.length;At(world.control,0x1ec)=sc.cursor;At(world.control,0x1f0)=sc.capacity;At(world.control,0x228)=sc.lead;At(world.control,0x22c)=sc.drop;At(world.control,0x74)=0x314;At(world.control,0x128)=0x8765;At(world.control,0x1b8)=7;At(world.control,0x1bc)=5;strcpy((char*)world.control+0xf4,"EditPlayerName");At(world.gui,0x34c)=sc.volume;
 if(sc.sounds&1)At(world.control,0x204)=(DWORD)world.sound[0];if(sc.sounds&2)At(world.control,0x208)=(DWORD)world.sound[1];if(sc.accepted)At(world.control,0x210)=(DWORD)acceptedText;
 if(sc.dbcs&&sc.length>=2){world.text[0][0]=0x81;world.text[0][1]=0x40;}
 if(sc.child){At(world.control,0x10)=(DWORD)world.children[0];At(world.children[0],0xc)=(DWORD)world.children[1];world.children[0][0x25]=(BYTE)(sc.child==2?9:1);}
 if(sc.alias==1){memcpy(world.control+0xf4,world.text[0],sc.length+1);At(world.control,0xc0)=(DWORD)(world.control+0xf4);}
 if(sc.alias==2){memcpy(world.control+0x230,world.text[0],sc.length+1);At(world.control,0xc0)=(DWORD)(world.control+0x230);}
 if(sc.alias==3)At(world.control,0x210)=(DWORD)world.text[0];
 if(sc.guardKind==1){BYTE* dst=guard+4096-sc.guardCut;memcpy(dst,world.text[0],sc.guardCut);At(world.control,0xc0)=(DWORD)dst;}
 if(sc.guardKind==2){BYTE* dst=guard+4096-sc.guardCut;memcpy(dst,world.control,sc.guardCut);activeControl=dst;}
 if(sc.guardKind==3){BYTE* dst=guard+4096-sc.guardCut;memcpy(dst,world.dialog[0],sc.guardCut);At(activeControl,0xb8)=(DWORD)dst;}
 if(sc.nullKind==1)At(activeControl,0xb8)=0;if(sc.nullKind==2)At(world.dialog[0],0x34)=0;if(sc.nullKind==3)At(activeControl,0xc0)=0;if(sc.nullKind==4)At(world.target[0],0x48)=0;if(sc.nullKind==5)At(activeControl,0xbc)=0;if(sc.nullKind==6)At(activeControl,0x210)=1;
 At(world.user[0],0x1d4)=sc.focus?(DWORD)activeControl:0;At(world.user[1],0x1d4)=(DWORD)activeControl;
}
static void Run(int retail){memset(&got,0,sizeof(got));got.result=-999;Setup();Invoke(retail);got.state=world;memcpy(got.page,guard,4096);}
static void Compare(){Run(1);want=got;Run(0);++checks;if(memcmp(&want,&got,sizeof(got))){BYTE*a=(BYTE*)&want,*b=(BYTE*)&got;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch%d off%d %02x/%02x value%x length%d cursor%d capacity%d focus%d lead%d drop%d accepted%d dbcs%d sound%d status%d mutation%d/%d throw%d null%d guard%d/%d events%d/%d exceptions%x/%x fault%x/%x\n",checks,offset,a[offset],b[offset],sc.value,sc.length,sc.cursor,sc.capacity,sc.focus,sc.lead,sc.drop,sc.accepted,sc.dbcs,sc.sounds,sc.status,sc.mutateAt,sc.mutation,sc.throwAt,sc.nullKind,sc.guardKind,sc.guardCut,want.count,got.count,want.exception,got.exception,want.faultAddress,got.faultAddress);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&sc,0,sizeof(sc));sc.value='q';sc.length=3;sc.cursor=1;sc.capacity=16;sc.focus=1;sc.baseGate=1;sc.audio=1;sc.volume=-600;sc.seed=7;}
static int Count(int kind){int n=0;for(int i=0;i<got.count;++i)if(got.events[i].kind==kind)++n;return n;}
static void Success(int result){if(!got.complete||got.exception||got.result!=result)Fail(90);}
static void TextOracle(){if(sc.dbcs||sc.lead||sc.drop||sc.alias)return;char expected[128];for(int i=0;i<sc.length;++i)expected[i]='a'+i%25;int length=sc.length,cursor=sc.cursor;expected[length]=0;char c=(char)sc.value;
 if(c==8){if(length&&cursor>0){for(i=cursor-1;i<length;++i)expected[i]=expected[i+1];--length;--cursor;}}
 else if(c!=9&&c!=27&&c!=13){if(sc.accepted&&!strchr(acceptedText,c)){char alt=c;if(c>='A'&&c<='Z')alt=c+32;else if(c>='a'&&c<='z')alt=c-32;c=strchr(acceptedText,alt)?alt:0;}if(c&&length<sc.capacity-1){for(i=length;i>=cursor;--i)expected[i+1]=expected[i];expected[cursor++]=c;++length;}}
 Success(1);if(memcmp((void*)At(world.control,0xc0),expected,length+1)||At(world.control,0xd0)!=(DWORD)length||At(world.control,0x1ec)!=(DWORD)cursor)Fail(96);}
int main(int argc,char**argv){if(sizeof(int(UIEditBox::*)(int))!=4)Fail(105);if(argc!=2||!HookSource()||!MapReference(argv[1]))return 3;guard=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&old))return 4;controls[50]=Address(&Probe::Redraw);dialogs[29]=Address(&Probe::Report);children[20]=Address(&Probe::Child);surfaces[17]=(DWORD)GetDc;surfaces[26]=(DWORD)ReleaseDc;soundBuffers[9]=(DWORD)Status;soundBuffers[15]=(DWORD)Volume;soundBuffers[12]=(DWORD)Play;if(!PatchReference())return 5;
 Base();Compare();Success(1);if(strcmp((char*)world.text[0],"aqbc")||At(world.control,0x1ec)!=2||At(world.control,0xd0)!=4||Count(7)!=2||Count(1)!=2||At(world.control,0x1bc)!=4)Fail(91);
 Base();sc.value=8;Compare();Success(1);if(strcmp((char*)world.text[0],"bc")||At(world.control,0x1ec)!=0||At(world.control,0xd0)!=2)Fail(92);
 Base();sc.value=13;sc.handled=2;Compare();Success(1);if(strcmp((char*)world.text[0],"abc")||Count(8)||Count(1)!=2)Fail(93);
 Base();sc.focus=0;Compare();Success(0);if(Count(1)||Count(8))Fail(94);
 for(int value=-128;value<256;++value)for(int pos=0;pos<4;++pos)for(int mode=0;mode<8;++mode){Base();sc.value=value;sc.cursor=pos;sc.dbcs=mode&1;sc.accepted=(mode>>1)&1;sc.capacity=(mode&4)?4:16;Compare();Success(1);TextOracle();}
 for(int length=0;length<=8;++length)for(int cursor=0;cursor<=length;++cursor)for(int capacity=0;capacity<12;++capacity)for(int state=0;state<4;++state){Base();sc.length=length;sc.cursor=cursor;sc.capacity=capacity;sc.lead=state&1;sc.drop=state>>1;Compare();Success(1);TextOracle();}
 const int values[]={8,9,13,27,'a','Q',0x81,0x40,0x100,0x7fffffff,(int)0x80000000};
 for(int v=0;v<11;++v)for(int f=0;f<2;++f)for(int h=0;h<8;++h)for(int g=0;g<2;++g)for(int c=0;c<3;++c){Base();sc.value=values[v];sc.focus=f;sc.handled=h;sc.baseGate=g;sc.child=c;sc.childResult=c==1;Compare();if(got.exception||!got.complete)Fail(104);}
 for(v=0;v<3;++v)for(int s=0;s<4;++s)for(int audio=0;audio<2;++audio)for(int group=0;group<3;++group)for(int status=0;status<2;++status)for(int failure=0;failure<4;++failure){Base();sc.value=v==0?'q':v==1?8:0x81;sc.capacity=3;sc.sounds=s;sc.audio=audio;sc.groupMode=group;sc.status=status;sc.volumeFail=failure&1;sc.playFail=failure>>1;Compare();Success(1);}
 for(int path=0;path<9;++path){Base();if(path==1)sc.value=8;if(path==2)sc.value=13;if(path==3)sc.value=0x81;if(path==4){sc.lead=1;sc.drop=1;}if(path==5){sc.accepted=1;sc.value='A';}if(path==6){sc.sounds=3;sc.status=1;sc.capacity=3;}if(path==7)sc.child=1;if(path==8)sc.dcFail=1;Compare();int n=got.count;for(int at=1;at<=n;++at){sc.throwAt=at;Compare();if(got.exception!=0xe0451234||got.count!=at)Fail(95);sc.throwAt=0;for(int mutation=0;mutation<14;++mutation){sc.mutateAt=at;sc.mutation=mutation;Compare();}sc.mutateAt=0;}}
 for(int kind=1;kind<=6;++kind)for(v=0;v<3;++v){Base();sc.nullKind=kind;sc.value=values[v];sc.sounds=1;Compare();}
 for(kind=1;kind<=3;++kind)for(int v=0;v<4;++v){int cuts[]={1,2,3,4,7,8,0xb8,0xbc,0xc0,0xc4,0xd0,0xd4,0x1ec,0x1f0,0x204,0x208,0x210,0x214,0x228,0x22c,0x230};for(int k=0;k<21;++k){if(kind==1&&cuts[k]>128)continue;Base();sc.guardKind=kind;sc.guardCut=cuts[k];sc.value=values[v];Compare();}}
 for(int alias=1;alias<=3;++alias)for(int value=0;value<256;++value)for(int cursor=0;cursor<4;++cursor){Base();sc.alias=alias;sc.value=value;sc.cursor=cursor;Compare();Success(1);}
 for(length=0;length<4;++length)for(int capacity=0;capacity<6;++capacity)for(int lead=0;lead<2;++lead)for(int drop=0;drop<2;++drop){Base();sc.length=length;sc.cursor=length;sc.capacity=capacity;sc.dbcs=1;sc.value=0x81;sc.lead=lead;sc.drop=drop;Compare();Success(1);if(!lead){int room=length<capacity-2;if(At(world.control,0x228)!=1||At(world.control,0x22c)!=(DWORD)!room||At(world.control,0xd0)!=(DWORD)(length+room))Fail(97);}}
 Base();sc.sounds=3;sc.status=1;sc.capacity=3;sc.volume=-12000;Compare();Success(1);if(At(world.sound[0],0x1a0)!=(DWORD)-10000||At(world.sound[1],0x1a0)!=(DWORD)-10000||Count(12)!=2||Count(11)!=4)Fail(98);
 sc.volume=100;Compare();Success(1);if(At(world.sound[0],0x1a0)||At(world.sound[1],0x1a0))Fail(99);sc.volumeFail=1;Compare();Success(1);if(At(world.sound[0],0x1a0)!=(DWORD)-22||Count(12)!=2)Fail(100);
 Base();sc.dcFail=1;Compare();Success(1);if(Count(7)||Count(9)||Count(3)!=1)Fail(101);
 Base();sc.guardKind=1;sc.guardCut=3;Compare();if(got.exception!=EXCEPTION_ACCESS_VIOLATION||got.faultAddress!=(int)(guard+4096)||got.complete)Fail(103);
 Base();sc.measureFail=1;Compare();Success(1);if(Count(7)!=2||At(world.control,0xe0)!=12||At(world.control,0x1fc)!=9)Fail(102);
 printf("EditBoxOnChar: %d actual-helper, text, callback and fault scenarios match\n",checks);return 0;}
