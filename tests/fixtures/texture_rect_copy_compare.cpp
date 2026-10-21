// Private complete-body validation. No production retail mapping or fallback.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct UnknownSurfaceInterface;struct UnknownRect;
class PCTextureMap{public:int CopyRectTo(unsigned long,unsigned long,UnknownSurfaceInterface*,UnknownRect*,int,unsigned char*);};
static BYTE referenceSpace[0x800000]={1};
struct Surface{DWORD table;int locks,unlocks;};
struct World{BYTE texture[0xc0],buffers[2][8192],lookup[65536];Surface surfaces[3];int rect[4];};
static World world;
struct Scenario{int seed,table,x,y,width,height,spitch,dpitch,bits,left,top,right,bottom,alias,failAt,status,throwAt,mutateAt,mutation,invalid,word,flags;};
static Scenario scenario;
struct Event{int kind,who,args[6];BYTE descriptor[124];World world;};
struct Observation{int result,completed,exception,faultKind,faultAddress,count;Event events[8];World world;};
static Observation observed,expected;static int checks,retailActive;static DWORD surfaceTable[33];
static DWORD& At(void* p,int o){return *(DWORD*)((BYTE*)p+o);}
static void Fail(int c){printf("Invariant %d check %d retail %d\n",c,checks,retailActive);fflush(stdout);ExitProcess(c);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;for(int i=0;i<3;++i)if(p==&world.surfaces[i])return 10+i;if(p==world.rect)return 20;if(p==world.lookup)return 30;if(p==world.texture)return 40;return(int)p;}
static Event& Begin(int kind,void* who){if(observed.count>=8)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);e.world=world;return e;}
static void Boundary(void* descriptor=0){if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:At(world.texture,0x70)=(DWORD)&world.surfaces[2];break;
 case 1:world.rect[0]=1;world.rect[1]=1;world.rect[2]=2;world.rect[3]=2;break;
 case 2:At(world.texture,0x70)=0;break;
 case 3:if(descriptor)At(descriptor,0x54)=16;break;
 case 4:if(descriptor){At(descriptor,0xc)=1;At(descriptor,8)=1;}break;
 case 5:if(descriptor)At(descriptor,0x24)=(DWORD)(world.lookup+16384);break;
 case 6:world.lookup[0]=0xff;world.buffers[0][4096]=0;world.buffers[1][4096]=0;break;
 case 7:if(descriptor)At(descriptor,0x10)=-17;break;
 }}if(scenario.throwAt==observed.count)throw 0x729;}
static long __stdcall BltFast(void* who,unsigned long x,unsigned long y,void* source,void* rect,int flags){Event& e=Begin(1,who);e.args[0]=x;e.args[1]=y;e.args[2]=Identity(source);e.args[3]=Identity(rect);e.args[4]=flags;if(rect)memcpy(e.descriptor,rect,16);Boundary();return scenario.failAt==observed.count?scenario.status:0;}
static long __stdcall Lock(void* who,void* rect,void* descriptor,int flags,void* event){Event& e=Begin(2,who);e.args[0]=Identity(rect);e.args[1]=flags;e.args[2]=Identity(event);memcpy(e.descriptor,descriptor,124);BYTE zero[124];memset(zero,0,124);*(DWORD*)zero=124;if(memcmp(zero,descriptor,124)||rect||event||flags!=0x811)Fail(82);
 int source=observed.count==1;int result=scenario.failAt==observed.count?scenario.status:0;
 if(!result){At(descriptor,8)=scenario.height;At(descriptor,0xc)=scenario.width;At(descriptor,0x10)=source?scenario.spitch:scenario.dpitch;At(descriptor,0x54)=scenario.bits;BYTE* pixels=world.buffers[source?0:1]+4096;
  if(scenario.alias==1)pixels=world.buffers[0]+4096;if(scenario.alias==2)pixels=world.buffers[0]+4096+(source?0:1);if(scenario.alias==3&&!source)pixels=world.lookup+16384;
  if((source&&scenario.invalid==1)||(!source&&scenario.invalid==2)||scenario.invalid==3)pixels=(BYTE*)(source?1:2);
  At(descriptor,0x24)=(DWORD)pixels;++((Surface*)who)->locks;
 }Boundary(descriptor);return result;}
static long __stdcall Unlock(void* who,void* rect){Event& e=Begin(3,who);e.args[0]=Identity(rect);++((Surface*)who)->unlocks;int result=scenario.failAt==observed.count?scenario.status:0;if(!result)--((Surface*)who)->locks;Boundary();return result;}
static int MapReference(const char* path){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 return FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{int(PCTextureMap::*method)(unsigned long,unsigned long,UnknownSurfaceInterface*,UnknownRect*,int,unsigned char*);DWORD address;}call;call.method=&PCTextureMap::CopyRectTo;if(retailActive)call.address=0x4c7b40;
 __try{observed.result=(((PCTextureMap*)world.texture)->*call.method)(scenario.x,scenario.y,(UnknownSurfaceInterface*)&world.surfaces[scenario.alias==4?0:1],scenario.invalid==4?0:(UnknownRect*)world.rect,scenario.flags,scenario.table?world.lookup:0);observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,0,sizeof(world));for(int i=0;i<3;++i)world.surfaces[i].table=(DWORD)surfaceTable;At(world.texture,0x70)=(DWORD)&world.surfaces[0];
 for(int k=0;k<8192;++k){world.buffers[0][k]=(BYTE)(k*19+scenario.seed);world.buffers[1][k]=(BYTE)(k*7+scenario.seed*3);}for(int n=0;n<65536;++n)world.lookup[n]=(BYTE)((n>>8)*13+(n&255)*7+scenario.seed);
 if(scenario.bits!=8)*(WORD*)(world.buffers[1]+4096)=(WORD)scenario.word;
 world.rect[0]=scenario.left;world.rect[1]=scenario.top;world.rect[2]=scenario.right;world.rect[3]=scenario.bottom;
 try{Invoke();}catch(int code){observed.exception=code;}observed.world=world;}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int o=0;while(a[o]==b[o])++o;printf("Mismatch %d offset%d %02x/%02x bits%d dims%d/%d xy%d/%d rect%d,%d,%d,%d pitch%d/%d alias%d invalid%d fail%d/%d mutation%d/%d throw%d events%d/%d fault%x/%x\n",checks,o,a[o],b[o],scenario.bits,scenario.width,scenario.height,scenario.x,scenario.y,scenario.left,scenario.top,scenario.right,scenario.bottom,scenario.spitch,scenario.dpitch,scenario.alias,scenario.invalid,scenario.failAt,scenario.status,scenario.mutateAt,scenario.mutation,scenario.throwAt,expected.count,observed.count,expected.faultAddress,observed.faultAddress);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.table=1;scenario.bits=8;scenario.width=8;scenario.height=8;scenario.spitch=scenario.dpitch=16;scenario.right=3;scenario.bottom=2;scenario.flags=0x12345678;scenario.seed=19;scenario.word=0x1234;}
static void PixelOracle(){if(!observed.completed||observed.result!=1||observed.exception||observed.count!=4||world.surfaces[0].locks||world.surfaces[1].locks)Fail(90);
 for(int k=0;k<8192;++k){BYTE value=(BYTE)(k*7+scenario.seed*3);int offset=k-4096;int row=offset>=0?offset/16:-1,col=offset>=0?offset%16:-1;if(row>=0&&row<=2&&col<=3){BYTE src=(BYTE)(k*19+scenario.seed);value=(BYTE)(src*13+value*7+scenario.seed);}if(world.buffers[1][k]!=value)Fail(91);}
}
static void AliasOracle(){World model=observed.events[0].world;BYTE* source=model.buffers[0]+4096;BYTE* target=model.buffers[1]+4096;
 if(scenario.alias==1)target=source;if(scenario.alias==2)target=source+1;if(scenario.alias==3)target=model.lookup+16384;
 for(int row=0;row<3;++row)for(int col=0;col<4;++col){BYTE a=source[row*16+col],b=target[row*16+col];target[row*16+col]=model.lookup[a*256+b];}
 if(memcmp(model.buffers,world.buffers,sizeof(model.buffers))||memcmp(model.lookup,world.lookup,sizeof(model.lookup)))Fail(92);
}
static void ClippedOracle(int changed){if(observed.result!=1||observed.count!=4||world.surfaces[0].locks||world.surfaces[1].locks)Fail(93);for(int k=0;k<8192;++k){BYTE value=(BYTE)(k*7+scenario.seed*3);if(changed&&k==4096+6*16+6){BYTE source=(BYTE)(4096*19+scenario.seed);value=(BYTE)(source*13+value*7+scenario.seed);}if(world.buffers[1][k]!=value)Fail(94);}}
int main(int argc,char** argv){if(argc!=2||!MapReference(argv[1]))return 3;surfaceTable[7]=(DWORD)BltFast;surfaceTable[25]=(DWORD)Lock;surfaceTable[32]=(DWORD)Unlock;
 SYSTEM_INFO info;GetSystemInfo(&info);if((DWORD)info.lpMinimumApplicationAddress<0x10000)return 4;for(DWORD low=0;low<0x10000;low+=info.dwPageSize){MEMORY_BASIC_INFORMATION memory;if(!VirtualQuery((void*)low,&memory,sizeof(memory))||memory.State==MEM_COMMIT)return 5;}
 Base();Compare();PixelOracle();
 for(int oracleAlias=1;oracleAlias<4;++oracleAlias){Base();scenario.alias=oracleAlias;Compare();AliasOracle();}
 Base();scenario.x=6;scenario.y=6;Compare();ClippedOracle(1);
 Base();scenario.x=7;scenario.y=7;Compare();ClippedOracle(0);
Base();scenario.table=0;Compare();if(observed.result!=1||observed.count!=1||observed.events[0].args[4]!=scenario.flags)return 6;
 Base();scenario.failAt=2;scenario.status=-1;Compare();if(observed.result||world.surfaces[0].locks!=1||observed.count!=2)return 7;
 Base();scenario.failAt=3;scenario.status=1;Compare();if(observed.result||world.surfaces[0].locks!=1||world.surfaces[1].locks!=1||observed.count!=3)return 8;
 Base();scenario.bits=16;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultAddress!=0x1234||observed.faultKind||observed.count!=2||world.surfaces[0].locks!=1||world.surfaces[1].locks!=1)return 9;
 // Both buffers invalid: retail reads the destination WORD first.
 Base();scenario.bits=16;scenario.invalid=3;Compare();if(observed.faultAddress!=2)return 10;
 const int pitches[]={-33,-17,-2,-1,0,1,2,17,33};for(int sp=0;sp<9;++sp)for(int dp=0;dp<9;++dp)for(int alias=0;alias<5;++alias){Base();scenario.spitch=pitches[sp];scenario.dpitch=pitches[dp];scenario.alias=alias;scenario.seed=checks;Compare();}
 for(int w=0;w<=9;++w)for(int h=0;h<=9;++h)for(int x=0;x<4;++x)for(int y=0;y<4;++y){Base();scenario.width=w;scenario.height=h;scenario.x=x;scenario.y=y;Compare();}
 for(int left=-3;left<=3;++left)for(int top=-3;top<=3;++top)for(int right=-2;right<=5;++right)for(int bottom=-2;bottom<=5;++bottom){Base();scenario.left=left;scenario.top=top;scenario.right=right;scenario.bottom=bottom;Compare();}
 const int status[]={0,1,-1,(-2147483647-1),2147483647};for(int table=0;table<2;++table)for(int fail=1;fail<=4;++fail)for(int st=0;st<5;++st){Base();scenario.table=table;scenario.failAt=fail;scenario.status=status[st];Compare();}
 for(int path=0;path<4;++path){Base();if(path==1)scenario.table=0;if(path==2)scenario.x=10;if(path==3)scenario.bits=16;Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<8;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=4;++invalid)for(int bits=0;bits<2;++bits){Base();scenario.invalid=invalid;scenario.bits=bits?16:8;Compare();}
 const int formats[]={0,1,15,16,24,32,0x7fffffff};for(int f=0;f<7;++f)for(int word=0;word<256;++word){Base();scenario.bits=formats[f];scenario.word=word*257;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultAddress!=scenario.word||world.surfaces[0].locks!=1||world.surfaces[1].locks!=1)return 11;}
 for(int signedSp=0;signedSp<9;++signedSp)for(int signedDp=0;signedDp<9;++signedDp)for(int coord=0;coord<3;++coord){Base();scenario.bits=16;scenario.spitch=pitches[signedSp];scenario.dpitch=pitches[signedDp];scenario.x=coord;scenario.y=coord;scenario.left=coord;scenario.top=coord;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.count!=2)return 12;}
 for(int everyWord=0;everyWord<65536;++everyWord){Base();scenario.bits=16;scenario.word=everyWord;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultAddress!=everyWord||observed.count!=2)return 13;}
 const int extremes[]={-2147483647-1,-1,0,1,2147483647};for(int x2=0;x2<5;++x2)for(int y2=0;y2<5;++y2){Base();scenario.x=extremes[x2];scenario.y=extremes[y2];Compare();}
 printf("TextureRectCopy: %d complete pixel, clipping, COM, alias, low-address and fault scenarios match\n",checks);return 0;
}
