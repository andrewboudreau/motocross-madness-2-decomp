// Private scripted ABI views only; the standalone candidate uses canonical headers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
extern "C" void __cdecl __CxxFrameHandler();
class TextureMap;class TextureMapManager;class TrackGame;
class PCTextureMap {public:PCTextureMap(TextureMapManager*,int);virtual TextureMap* Clone();};
class ManagedTexture {public:ManagedTexture(TextureMapManager*);};
class ManagedTextureGroup {public:void AddTexture(ManagedTexture*);};
int ActualPixelFormatBytesPerPixel(int);
static BYTE referenceSpace[0x800000]={1};
struct Surface {DWORD table;int locks,unlocks;BYTE rest[20];};
struct World {BYTE source[0xd0],copy[0xd0],games[2][0x80],managers[2][32],groups[2][32],palettes[2][0x710],surfacePalettes[2][32],pixels[2][64];Surface surfaces[2];};
static World world;TrackGame* g_TrackGame;
struct Scenario {int seed,cached,format,keyEnabled,key,caps,pitch,width,height,minimum,surface,lockResult,unlockResult,created,keyResult,failNew,allocationAlias,throwAt,mutateAt,mutation,invalid;};
static Scenario scenario;
struct State {World world;int game,live,registered,deleted;};
struct Event {int kind,who,args[18];BYTE data[124];State state;};
struct Observation {int result,completed,exception,faultKind,faultAddress,count;Event events[16];State state;};
static Observation observed,expected;static int checks,retailActive,live,registered,deleted;
static DWORD textureTables[2][19],surfaceTable[33];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d check%d ref%d last%d\n",code,checks,retailActive,observed.count?observed.events[observed.count-1].kind:0);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.source)return 1;if(p==world.copy)return 2;for(int i=0;i<2;++i){if(p==world.games[i])return 10+i;if(p==world.managers[i])return 20+i;if(p==world.groups[i])return 30+i;if(p==world.palettes[i])return 40+i;if(p==world.surfacePalettes[i])return 50+i;if(p==world.pixels[i])return 60+i;if(p==&world.surfaces[i])return 70+i;}return(int)p;}
static void* Allocated(){return scenario.allocationAlias?(void*)world.source:(void*)world.copy;}
static void FromReference(){if(retailActive)g_TrackGame=*(TrackGame**)0x56e26c;}
static void ToReference(){if(retailActive)*(TrackGame**)0x56e26c=g_TrackGame;}
static void Snapshot(State& s){FromReference();s.world=world;s.game=Identity(g_TrackGame);s.live=live;s.registered=registered;s.deleted=deleted;}
static Event& Begin(int kind,const void* p=0,int a=0,int b=0,int c=0,int d=0){FromReference();if(observed.count>=16)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(p);e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;return e;}
static void Finish(){Snapshot(observed.events[observed.count-1].state);
 if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:g_TrackGame=(TrackGame*)world.games[1];At(world.source,0x10)=(DWORD)world.managers[1];At(world.source,0x90)=(DWORD)world.groups[1];break;
 case 1:At(world.source,0x70)=(DWORD)&world.surfaces[1];At(world.source,0x78)=(DWORD)world.surfacePalettes[1];break;
 case 2:At(world.source,0x20)=565;At(world.source,0x14)=17;At(world.source,0x18)=19;At(world.source,0x1c)=3;break;
 case 3:At(world.source,0x30)=1;At(world.source,0x34)=123;At(world.source,0x2c)=(DWORD)world.palettes[1];break;
 case 4:At(Allocated(),0)=(DWORD)textureTables[1];break;
 case 5:At(world.source,0x70)=0;break;
 case 6:At(world.source,0x30)=0;At(world.source,0x68)^=1;break;
 }}ToReference();if(scenario.throwAt==observed.count)throw 0x791;
}
static int SourceFile(const char* p){const char* a=strrchr(p,'\\'),*b=strrchr(p,'/');if(!a||(b&&b>a))a=b;a=a?a+1:p;if(strcmp(a,"PCTexMap.cpp")&&strcmp(a,"TextureClone.cpp"))Fail(82);return 1;}
void* operator new(unsigned size,const char* file,int line){Begin(1,0,size,SourceFile(file),line);Finish();if(size!=0x80&&size!=0xbc)Fail(83);if(scenario.failNew)return 0;live=1;return Allocated();}
void operator delete(void* p,const char* file,int line){Begin(10,p,SourceFile(file),line);Finish();if(p&&p!=Allocated())Fail(84);if(p)live=0;++deleted;}
PCTextureMap::PCTextureMap(TextureMapManager* manager,int flags){Begin(2,this,Identity(manager),flags);Finish();At(this,0)=(DWORD)textureTables[0];At(this,0x10)=(DWORD)manager;At(this,0x68)=0x200;}
ManagedTexture::ManagedTexture(TextureMapManager* manager){Begin(3,this,Identity(manager));Finish();At(this,0)=(DWORD)textureTables[0];At(this,0x10)=(DWORD)manager;At(this,0x68)=0x200;}
void ManagedTextureGroup::AddTexture(ManagedTexture* texture){Begin(4,this,Identity(texture));Finish();At(this,0)++;At(texture,0x90)=(DWORD)this;++registered;}
int PixelFormatBytesPerPixel(int format){Begin(6,0,format);Finish();return ActualPixelFormatBytesPerPixel(format);}
struct Hooks {
 TextureMap* Clone(){return((PCTextureMap*)this)->PCTextureMap::Clone();}
 PCTextureMap* Construct(TextureMapManager* manager,int flags){return new(this) PCTextureMap(manager,flags);}
 ManagedTexture* ConstructManaged(TextureMapManager* manager){return new(this) ManagedTexture(manager);}
 void* Delete(unsigned flags){Begin(11,this,flags);Finish();if(flags&1)live=0;if(registered)--registered;++deleted;return this;}
 int Create(void* bits,int width,int height,int stride,int minimum,int sourceFormat,int format,void* palette,int flags,void* surfacePalette,int check,int unused,int u,int v,void* choice,int threshold,unsigned key){Event& e=Begin(7,this,Identity(bits),width,height,stride);int args[]={minimum,sourceFormat,format,Identity(palette),flags,Identity(surfacePalette),check,unused,u,v,Identity(choice),threshold,(int)key};memcpy(e.args+4,args,sizeof(args));if(bits)memcpy(e.data,bits,64);Finish();At(this,0x14)=width;At(this,0x18)=height;At(this,0x20)=format;return scenario.created;}
 int AlternateCreate(void* bits,int width,int height,int stride,int minimum,int sourceFormat,int format,void* palette,int flags,void* surfacePalette,int check,int unused,int u,int v,void* choice,int threshold,unsigned key){int result=Create(bits,width,height,stride,minimum,sourceFormat,format,palette,flags,surfacePalette,check,unused,u,v,choice,threshold,key);observed.events[observed.count-1].args[17]=1;return result;}
 int Key(unsigned color){Begin(9,this,color);Finish();At(this,0x30)=1;At(this,0x34)=color;return scenario.keyResult;}
};
static long __stdcall Lock(void* self,void* rect,void* desc,int flags,void* event){BYTE want[124];memset(want,0,124);*(DWORD*)want=124;if(memcmp(want,desc,124))Fail(85);Event& e=Begin(5,self,Identity(rect),flags,Identity(event));memcpy(e.data,desc,124);Finish();memset(desc,0x64,124);At(desc,0)=124;At(desc,16)=scenario.pitch;At(desc,36)=scenario.invalid==4?0:(DWORD)world.pixels[self==&world.surfaces[1]];At(desc,104)=scenario.caps;if(!scenario.lockResult)++((Surface*)self)->locks;return scenario.lockResult;}
static long __stdcall Unlock(void* self,void* rect){Begin(8,self,Identity(rect));Finish();--((Surface*)self)->locks;++((Surface*)self)->unlocks;return scenario.unlockResult;}
static DWORD Replacement(DWORD va){switch(va){case 0x4a3010:return(DWORD)(void*(__cdecl*)(unsigned,const char*,int))operator new;case 0x4a3060:return(DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;case 0x510500:return Address(&Hooks::ConstructManaged);case 0x4c5f00:return Address(&Hooks::Construct);case 0x50c6c0:return Address(&ManagedTextureGroup::AddTexture);case 0x511970:return(DWORD)PixelFormatBytesPerPixel;case 0x53471a:return(DWORD)__CxxFrameHandler;}Fail(86);return 0;}
static int MapReference(const char* path,const char* patches){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;
 if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
 if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD sectionsOffset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(sectionsOffset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-sectionsOffset)/sizeof(IMAGE_SECTION_HEADER))return 0;
 memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;while(fread(entry,sizeof(entry),1,f)==1){++records;BYTE* site=(BYTE*)entry[0];if(!((entry[0]>=0x4c71c0&&entry[0]+5<=0x4c7418)||(entry[0]>=0x54cd00&&entry[0]+5<=0x54cd3e))||*site!=entry[2]||(DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1])return 0;*(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);}fclose(f);return records==9&&FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=EXCEPTION_INT_DIVIDE_BY_ZERO&&code!=EXCEPTION_INT_OVERFLOW)return EXCEPTION_CONTINUE_SEARCH;observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{TextureMap*(Hooks::*method)();DWORD address;}call;call.method=&Hooks::Clone;if(retailActive)call.address=0x4c71c0;
 __try{observed.result=Identity((((Hooks*)world.source)->*call.method)());observed.completed=1;}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;live=registered=deleted=0;memset(&world,0x65+scenario.seed%15,sizeof(world));
 for(int n=0;n<2;++n){At(world.games[n],0x3c)=(DWORD)world.managers[1-n];At(world.groups[n],0)=10+n;world.surfaces[n].table=(DWORD)surfaceTable;world.surfaces[n].locks=world.surfaces[n].unlocks=0;for(int j=0;j<0x710;++j)world.palettes[n][j]=(BYTE)(j*37+n*31+scenario.seed);for(j=0;j<64;++j)world.pixels[n][j]=(BYTE)(j*17+n*23+scenario.seed);}
 At(world.source,0)=(DWORD)textureTables[0];At(world.source,0x10)=(DWORD)world.managers[0];At(world.source,0x14)=scenario.width;At(world.source,0x18)=scenario.height;At(world.source,0x1c)=scenario.minimum;At(world.source,0x20)=scenario.format;At(world.source,0x2c)=(DWORD)world.palettes[0];At(world.source,0x30)=scenario.keyEnabled;At(world.source,0x34)=scenario.key;At(world.source,0x68)=scenario.cached?0x101:0x100;At(world.source,0x70)=scenario.surface?(DWORD)&world.surfaces[0]:0;At(world.source,0x78)=(DWORD)world.surfacePalettes[0];At(world.source,0x90)=(DWORD)world.groups[0];
 g_TrackGame=(TrackGame*)world.games[0];if(scenario.invalid==1)g_TrackGame=0;if(scenario.invalid==2)At(world.source,0x90)=0;if(scenario.invalid==3)At(world.source,0x2c)=0;ToReference();try{Invoke();}catch(int code){observed.exception=code;}Snapshot(observed.state);
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int i=0;while(a[i]==b[i])++i;printf("Mismatch%d offset%d %02x/%02x cached%d format%d key%d/%x caps%x pitch%d statuses%d,%d,%d fail%d alias%d invalid%d throw%d mutate%d/%d events%d/%d\n",checks,i,a[i],b[i],scenario.cached,scenario.format,scenario.keyEnabled,scenario.key,scenario.caps,scenario.pitch,scenario.lockResult,scenario.unlockResult,scenario.created,scenario.failNew,scenario.allocationAlias,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.format=555;scenario.width=8;scenario.height=4;scenario.minimum=1;scenario.surface=1;scenario.pitch=34;scenario.created=1;scenario.keyResult=-7;scenario.key=0x5321;}
static int Count(int kind){int count=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++count;return count;}
static Event& Find(int kind){for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)return observed.events[i];Fail(87);return observed.events[0];}
static unsigned KeyOracle(){unsigned key=(unsigned)scenario.key;if(scenario.format==8){BYTE* p=world.palettes[0]+16+3*key;return p[0]*65536u+p[1]*256u+p[2];}unsigned r=scenario.format==555?(key/1024)%32:(key/2048)%32;unsigned g=scenario.format==555?(key/32)%32:(key/32)%64;return r*8*65536+g*(scenario.format==555?8:4)*256+(key%32)*8;}
static void CheckSuccess(){if(!observed.completed||observed.result!=2||observed.exception||!live||deleted||Count(5)!=1||Count(7)!=1||Count(8)!=1||world.surfaces[0].locks||world.surfaces[0].unlocks!=1)Fail(90);Event& e=Find(7);int wanted[]={60,scenario.width,scenario.height,scenario.pitch/ActualPixelFormatBytesPerPixel(scenario.format),scenario.minimum,scenario.format,scenario.format,40,(!(scenario.caps&0x30000000)?8:0)|(!(scenario.caps&0x1000)?4:0),50,0,0,2,1,0,0x80,0xff00ff,0};if(memcmp(e.args,wanted,sizeof(wanted))||memcmp(e.data,world.pixels[0],64))Fail(91);Event& alloc=Find(1);if(alloc.args[0]!=(scenario.cached?0xbc:0x80)||alloc.args[1]!=1||alloc.args[2]!=(scenario.cached?976:986))Fail(92);Event& ctor=Find(scenario.cached?3:2);if(ctor.args[0]!=(scenario.cached?20:21)||registered!=scenario.cached||Count(4)!=scenario.cached)Fail(93);if(scenario.keyEnabled){if(Count(9)!=1||Find(9).args[0]!=(int)KeyOracle()||At(world.copy,0x34)!=KeyOracle())Fail(94);}else if(Count(9))Fail(95);}
int main(int argc,char** argv){if(argc!=3||!MapReference(argv[1],argv[2]))return 3;for(int n=0;n<2;++n){textureTables[n][0]=Address(&Hooks::Delete);textureTables[n][4]=n?Address(&Hooks::AlternateCreate):Address(&Hooks::Create);textureTables[n][18]=Address(&Hooks::Key);}surfaceTable[25]=(DWORD)Lock;surfaceTable[32]=(DWORD)Unlock;
 for(int cached=0;cached<2;++cached){Base();scenario.cached=cached;Compare();CheckSuccess();int ctorAt=0,createAt=0;for(int k=0;k<observed.count;++k){if(observed.events[k].kind==2||observed.events[k].kind==3)ctorAt=k+1;if(observed.events[k].kind==7)createAt=k+1;}
  scenario.throwAt=ctorAt;Compare();if(observed.exception!=0x791||live||deleted!=1||Count(10)!=1||Find(10).args[1]!=(cached?976:986)||Count(11))return 4;
  scenario.throwAt=createAt;Compare();if(observed.exception!=0x791||!live||deleted||world.surfaces[0].locks!=1||Count(8))return 5;
  scenario.throwAt=0;scenario.lockResult=1;Compare();if(observed.result||live||deleted!=1||Count(7)||Count(8)||Find(11).args[0]!=1)return 6;
  scenario.lockResult=0;scenario.created=0;Compare();if(observed.result||live||deleted!=1||Count(8)!=1||world.surfaces[0].locks)return 7;
  scenario.created=1;scenario.unlockResult=1;Compare();if(observed.result||live||deleted!=1||Count(9))return 8;
 }
 const int formats[]={8,555,565,1555,4444,888,8888,0,999};const int caps[]={0,0x1000,0x10000000,0x20000000,0x30000000,0x30001000,-1};const int pitches[]={-35,-1,0,1,2,3,35,2147483647,(-2147483647-1)};
 for(int f=0;f<9;++f)for(int c=0;c<7;++c)for(int p=0;p<9;++p)for(int mode=0;mode<4;++mode){Base();scenario.seed=checks;scenario.format=formats[f];scenario.caps=caps[c];scenario.pitch=pitches[p];scenario.cached=mode&1;scenario.keyEnabled=mode&2;scenario.key=123;Compare();if(ActualPixelFormatBytesPerPixel(scenario.format))CheckSuccess();else if(observed.exception!=EXCEPTION_INT_DIVIDE_BY_ZERO||world.surfaces[0].locks!=1||!live||deleted)return 9;}
 for(int fmt=0;fmt<2;++fmt)for(int key=0;key<65536;++key){Base();scenario.format=fmt?565:555;scenario.keyEnabled=1;scenario.key=key;scenario.cached=key&1;Compare();CheckSuccess();}
 for(int key8=0;key8<256;++key8){Base();scenario.format=8;scenario.keyEnabled=1;scenario.key=key8;scenario.cached=key8&1;Compare();CheckSuccess();}
 const int results[]={0,1,-1,(-2147483647-1),2147483647};for(int c2=0;c2<2;++c2)for(int lock=0;lock<5;++lock)for(int unlock=0;unlock<5;++unlock)for(int create=0;create<5;++create)for(int color=0;color<5;++color){Base();scenario.cached=c2;scenario.keyEnabled=1;scenario.lockResult=results[lock];scenario.unlockResult=results[unlock];scenario.created=results[create];scenario.keyResult=results[color];Compare();}
 for(int path=0;path<6;++path){Base();scenario.cached=path&1;scenario.keyEnabled=1;scenario.format=path<2?8:(path<4?555:565);scenario.key=123;Compare();int count=observed.count;for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<7;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int cached2=0;cached2<2;++cached2)for(int surface=0;surface<2;++surface)for(int fail=0;fail<2;++fail){Base();scenario.cached=cached2;scenario.surface=surface;scenario.failNew=fail;Compare();if(fail&&cached2&&(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.faultAddress!=0x68||deleted!=1||Find(10).who))return 10;}
 for(int invalid=1;invalid<=4;++invalid)for(int cached3=0;cached3<2;++cached3){Base();scenario.cached=cached3;scenario.invalid=invalid;scenario.format=8;scenario.key=123;scenario.keyEnabled=1;Compare();}
 const int dims[]={-2147483647-1,-1,0,1,3,2147483647};for(int w=0;w<6;++w)for(int h=0;h<6;++h)for(int min=0;min<6;++min){Base();scenario.width=dims[w];scenario.height=dims[h];scenario.minimum=dims[min];scenario.cached=(w+h+min)&1;Compare();CheckSuccess();}
 for(int alias=0;alias<2;++alias)for(int ca=0;ca<2;++ca)for(int sf=0;sf<2;++sf){Base();scenario.allocationAlias=alias;scenario.cached=ca;scenario.surface=sf;Compare();}
 printf("TextureClone: %d complete allocation, copy, color-key, state, callback and unwind scenarios match\n",checks);return 0;
}
