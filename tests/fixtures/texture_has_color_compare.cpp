// Private complete-function comparison using strict source lock/unlock helpers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class TextureMap;
int TextureHasColor(TextureMap*,int);
class ActualTexture {public:void* Lock(void*,long*,int);int Unlock(void*);};
static BYTE referenceSpace[0x800000]={1};
struct Surface {DWORD table;int locks,unlocks;};
struct World {BYTE texture[0xc0];WORD pixels[256];Surface surfaces[2];};
struct State {World world;BYTE guard[4096];};
struct Scenario {int format,key,width,height,lockStatus,unlockStatus,bits,seed,match,fill,throwAt,mutateAt,mutation,nullTexture,nullSurface;};
struct Event {int kind,who,flags,rect,event,result;BYTE input[124],output[124];State before,after;};
struct Observation {int result,completed,exception,faultKind,faultAddress,count;Event events[2];State state;};
static World world;static Scenario scenario;static Observation observed,expected;static int checks;
static BYTE* guard;static DWORD textureTable[15],surfaceTable[33];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant %d at case %d\n",code,checks);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union {T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(void* p){if(!p)return 0;if(p==&world.surfaces[0])return 1;if(p==&world.surfaces[1])return 2;return(int)p;}
static void Snapshot(State& s){s.world=world;memcpy(s.guard,guard,4096);}
static Event& Begin(int kind,void* who){if(observed.count>=2)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);Snapshot(e.before);return e;}
static void Boundary(void* desc=0){if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:At(world.texture,0x20)=555;break;
 case 1:At(world.texture,0x14)=0;break;
 case 2:At(world.texture,0x18)=0;break;
 case 3:At(world.texture,0x14)=3;At(world.texture,0x18)=2;break;
 case 4:At(world.texture,0x20)=8;break;
 case 5:At(world.texture,0x70)=(DWORD)&world.surfaces[1];break;
 case 6:At(world.texture,0x70)=0;break;
 case 7:{for(int i=0;i<256;++i)world.pixels[i]=0;}break;
 case 8:world.pixels[0]=0xf81f;break;
 case 9:if(desc)At(desc,0x24)=1;break;
 case 10:if(desc)At(desc,0x24)=0;break;
 case 11:if(desc)At(desc,0x10)=-17;break;
 }}if(scenario.throwAt==observed.count)throw 0x734;}
static long __stdcall Lock(void* who,void* rect,void* desc,int flags,void* event){Event& e=Begin(1,who);e.rect=Identity(rect);e.event=Identity(event);e.flags=flags;memcpy(e.input,desc,124);BYTE initialized[124];memset(initialized,0,124);At(initialized,0)=124;if(rect||event||flags!=0x11||memcmp(initialized,desc,124))Fail(82);
 if(!scenario.lockStatus){At(desc,0x10)=777;At(desc,0x24)=(DWORD)world.pixels;if(scenario.bits==1)At(desc,0x24)=0;if(scenario.bits==2)At(desc,0x24)=1;if(scenario.bits==3)At(desc,0x24)=(DWORD)(guard+4094);if(scenario.bits==4)At(desc,0x24)=(DWORD)(world.texture+0x80);++((Surface*)who)->locks;}
 Boundary(!scenario.lockStatus?desc:0);e.result=scenario.lockStatus;memcpy(e.output,desc,124);Snapshot(e.after);return scenario.lockStatus;}
static long __stdcall Unlock(void* who,void* rect){Event& e=Begin(2,who);e.rect=Identity(rect);if(rect)Fail(83);++((Surface*)who)->unlocks;if(!scenario.unlockStatus&&((Surface*)who)->locks>0)--((Surface*)who)->locks;Boundary();e.result=scenario.unlockStatus;Snapshot(e.after);return scenario.unlockStatus;}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(int retail){TextureMap* texture=scenario.nullTexture?0:(TextureMap*)world.texture;__try{observed.result=retail?((int(__cdecl*)(TextureMap*,int))0x47b490)(texture,scenario.key):TextureHasColor(texture,scenario.key);observed.completed=1;}__except(Filter(GetExceptionInformation())){}}
static int Packed(int format,int key){unsigned int rgb=(unsigned int)key;int red=(rgb>>16)&255,green=(rgb>>8)&255,blue=rgb&255;return format==565?((red/8)*2048+(green/4)*32+blue/8):((red/8)*1024+(green/8)*32+blue/8);}
static void Run(int retail){memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,scenario.seed&255,sizeof(world));memset(guard,0x55,4096);At(world.texture,0)=(DWORD)textureTable;At(world.texture,0x14)=scenario.width;At(world.texture,0x18)=scenario.height;At(world.texture,0x20)=scenario.format;At(world.texture,0x70)=scenario.nullSurface?0:(DWORD)&world.surfaces[0];
 for(int i=0;i<2;++i){world.surfaces[i].table=(DWORD)surfaceTable;world.surfaces[i].locks=world.surfaces[i].unlocks=0;}for(i=0;i<256;++i)world.pixels[i]=(WORD)scenario.fill;int packed=Packed(scenario.format,scenario.key);if(scenario.match>=0&&scenario.match<256)world.pixels[scenario.match]=(WORD)packed;*(WORD*)(guard+4094)=(WORD)scenario.fill;if(scenario.match==256)*(WORD*)(guard+4094)=(WORD)packed;
 try{Invoke(retail);}catch(int code){observed.exception=code;}Snapshot(observed.state);}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int offset=0;while(a[offset]==b[offset])++offset;printf("Mismatch %d offset%d %02x/%02x format%d key%x wh%d/%d match%d bits%d status%x/%x mutation%d/%d throw%d\n",checks,offset,a[offset],b[offset],scenario.format,scenario.key,scenario.width,scenario.height,scenario.match,scenario.bits,scenario.lockStatus,scenario.unlockStatus,scenario.mutateAt,scenario.mutation,scenario.throwAt);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.format=565;scenario.key=0xff00ff;scenario.width=4;scenario.height=3;scenario.seed=19;scenario.match=-1;scenario.fill=0x3333;}
static void Oracle(int result,int calls,int locks,int unlocks){if(observed.exception||!observed.completed||observed.result!=result||observed.count!=calls||world.surfaces[0].locks!=locks||world.surfaces[0].unlocks!=unlocks)Fail(90);}
int main(int argc,char** argv){if(argc!=2||!MapReference(argv[1]))return 3;guard=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&old))return 4;textureTable[13]=Address(&ActualTexture::Lock);textureTable[14]=Address(&ActualTexture::Unlock);surfaceTable[25]=(DWORD)Lock;surfaceTable[32]=(DWORD)Unlock;
 Base();Compare();Oracle(0,2,0,1);scenario.match=0;Compare();Oracle(1,2,0,1);scenario.unlockStatus=1;Compare();Oracle(0,2,1,1);Base();scenario.lockStatus=1;Compare();Oracle(0,1,0,0);Base();scenario.bits=1;Compare();Oracle(0,1,1,0);
 // Every possible 16-bit word tested against independently expanded RGB.
 for(int format=0;format<2;++format)for(int word=0;word<65536;++word){Base();scenario.format=format?565:555;scenario.width=scenario.height=1;int red=format?((word>>11)&31):((word>>10)&31),green=format?((word>>5)&63):((word>>5)&31),blue=word&31;scenario.key=(red*8<<16)|(green*(format?4:8)<<8)|(blue*8);scenario.fill=word;Compare();int expectedWord=format?word:(word&0x7fff);Oracle(expectedWord&&word==expectedWord,2,0,1);}
 const int keys[]={0,1,7,8,0xffffff,0xff00ff,0x123456,0x80000000,0xffffffff,0x80010203,0x7fffffff};for(format=0;format<2;++format)for(int k=0;k<11;++k)for(int fill=0;fill<2;++fill){Base();scenario.format=format?565:555;scenario.key=keys[k];scenario.width=scenario.height=1;scenario.fill=Packed(scenario.format,scenario.key)^(fill?1:0);Compare();Oracle(Packed(scenario.format,scenario.key)!=0&&!fill,2,0,1);}
 for(int width=-2;width<=8;++width)for(int height=-2;height<=8;++height)for(int place=0;place<5;++place){Base();scenario.width=width;scenario.height=height;scenario.match=place==0?-1:(place==1?0:(place==2?width-1:(place==3?width*height/2:width*height-1)));Compare();int hit=width>0&&height>0&&scenario.match>=0&&scenario.match<width*height;Oracle(hit,2,0,1);}
 const int statuses[]={0,1,-1,0x80000000,0x7fffffff};for(int l=0;l<5;++l)for(int u=0;u<5;++u){Base();scenario.match=0;scenario.lockStatus=statuses[l];scenario.unlockStatus=statuses[u];Compare();Oracle(!l&&!u,l?1:2,(!l&&u)?1:0,l?0:1);}
 for(int path=0;path<6;++path){Base();if(path==1)scenario.match=0;if(path==2)scenario.lockStatus=-1;if(path==3)scenario.key=0;if(path==4)scenario.format=8;if(path==5)scenario.bits=1;Compare();int events=observed.count;for(int at=1;at<=events;++at){scenario.throwAt=at;Compare();if(observed.exception!=0x734||observed.count!=at||world.surfaces[0].locks!=((at==1&&!scenario.lockStatus)?1:0)||world.surfaces[0].unlocks!=(at==2))Fail(91);scenario.throwAt=0;for(int mutation=0;mutation<12;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int bypass=0;bypass<5;++bypass){Base();scenario.bits=2;if(bypass==1)scenario.format=8;if(bypass==2)scenario.key=0;if(bypass==3)scenario.height=0;if(bypass==4)scenario.width=0;Compare();if(!bypass){if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.count!=1||world.surfaces[0].locks!=1)Fail(92);}else Oracle(0,2,0,1);}
 Base();scenario.bits=3;scenario.width=2;scenario.height=1;scenario.match=256;Compare();Oracle(1,2,0,1);scenario.height=2;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.count!=1||world.surfaces[0].locks!=1||observed.faultAddress!=(int)(guard+4098))Fail(93);
 Base();scenario.bits=3;scenario.width=1;scenario.height=2;scenario.match=256;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.count!=1||observed.faultAddress!=(int)(guard+4096))Fail(94);
 for(format=0;format<2;++format){Base();scenario.format=format?565:555;scenario.bits=4;Compare();Oracle(0,2,0,1);}
 Base();scenario.nullTexture=1;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.count)Fail(95);Base();scenario.nullSurface=1;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.count)Fail(96);
 printf("TextureHasColor: %d color, real-helper, COM, callback and guarded-fault scenarios match\n",checks);VirtualFree(guard,0,MEM_RELEASE);return 0;}
