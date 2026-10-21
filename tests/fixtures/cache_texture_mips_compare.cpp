// Validation-only ABI views and bounded COM mip chains; no production fallback.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
extern "C" long __cdecl _ftol();
class TrackGame;struct UnknownTextureRegion;
class CacheTexture {public:int CopyMipLevels(UnknownTextureRegion*,int);};
class ManagedTexture {public:void UpdatePageCoords(int);};
class ActualManagedTexture {public:void UpdatePageCoords(int);};
static BYTE referenceSpace[0x800000]={1};
struct Surface {DWORD table;int refs,attached,blits;int lastRect[4];BYTE bytes[16];};
struct World {BYTE cache[0x1a0],textures[2][0xc0],regions[2][0x30],groups[2][0x258],games[2][0x80],displays[2][0x5d0];Surface surfaces[2][10];};
static World world;TrackGame* g_TrackGame;
struct Scenario {int seed,width,height,sourceSide,level,left,top,right,bottom,sourceDepth,destinationDepth,partial,initialTexels,chainMode,failAt,status,outputMode,writeOnFailure,blitResult,uploadResult,throwAt,mutateAt,mutation,invalid,helperPage,helperRegion,round;};
static Scenario scenario;
struct State {World world;int game,attachments,fpStatus;};
struct Event {int kind,who,args[6],rect[4];BYTE caps[16];State state;};
struct Observation {int result,completed,exception,faultKind,faultAddress,fpStatus,count;Event events[100];State state;};
static Observation observed,expected;static int checks,retailActive,attachments;
static DWORD surfaceTable[33],cacheTables[2][10];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d check%d ref%d last%d\n",code,checks,retailActive,observed.count?observed.events[observed.count-1].kind:0);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.cache)return 1;for(int i=0;i<2;++i){if(p==world.textures[i])return 10+i;if(p==world.regions[i])return 20+i;if(p==world.groups[i])return 30+i;if(p==world.games[i])return 40+i;if(p==world.displays[i])return 50+i;for(int j=0;j<10;++j)if(p==&world.surfaces[i][j])return 100+i*10+j;}return(int)p;}
static void FromReference(){if(retailActive)g_TrackGame=*(TrackGame**)0x56e26c;}
static void ToReference(){if(retailActive)*(TrackGame**)0x56e26c=g_TrackGame;}
static void Snapshot(State& s){FromReference();s.world=world;s.game=Identity(g_TrackGame);s.attachments=attachments;s.fpStatus=_statusfp();}
static Event& Begin(int kind,const void* who=0,int a=0,int b=0,int c=0,int d=0){FromReference();if(observed.count>=100)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(who);e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;return e;}
static void Finish(){Snapshot(observed.events[observed.count-1].state);
 if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:g_TrackGame=(TrackGame*)world.games[1];At(world.cache,0x80)=(DWORD)world.groups[1];break;
 case 1:At(world.cache,0x70)=(DWORD)&world.surfaces[1][2];At(world.textures[0],0x70)=(DWORD)&world.surfaces[0][2];break;
 case 2:At(world.cache,0x14)=32;At(world.cache,0x18)=16;At(world.textures[0],0x14)=8;break;
 case 3:At(world.regions[0],0x14)=(DWORD)world.textures[1];At(world.regions[0],0x1c)=0x3e800000;At(world.regions[0],0x24)=0x3f400000;break;
 case 4:At(world.cache,0)=(DWORD)cacheTables[1];At(world.displays[0],0x5bc)=1;break;
 case 5:At(world.cache,0x80)=0;break;
 case 6:At(world.games[0],0xc)=0;break;
 }}ToReference();if(scenario.throwAt==observed.count)throw 0x7a1;
}
void ManagedTexture::UpdatePageCoords(int level){Begin(1,this,level);Finish();((ActualManagedTexture*)this)->UpdatePageCoords(level);}
struct Hooks {
 int Upload(void* rect,int level){Event& e=Begin(5,this,level);memcpy(e.rect,rect,16);Finish();At(this,0x18c)++;return scenario.uploadResult;}
 int AlternateUpload(void* rect,int level){int result=Upload(rect,level);observed.events[observed.count-1].args[1]=1;return result;}
};
static long __stdcall Attached(void* self,void* caps,void** out){int id=Identity(self);if(id<100||id>=120)Fail(82);int chain=(id-100)/10,index=(id-100)%10;Event& e=Begin(2+chain,self,Identity(*out));memcpy(e.caps,caps,16);BYTE expectedCaps[16];memset(expectedCaps,0,16);*(DWORD*)expectedCaps=0x401000;if(memcmp(caps,expectedCaps,16))Fail(83);Finish();++attachments;++((Surface*)self)->attached;
 // An external scripted HRESULT, not an execution cutoff: at most twelve
 // successful attachments, including deliberately cyclic/aliased chains.
 int result=(index>=(chain?scenario.destinationDepth:scenario.sourceDepth)||attachments>12)?-1:0;
 if(scenario.failAt==attachments)result=scenario.status;
 if(attachments>12)result=-1;
 if(scenario.outputMode!=2&&(!result||scenario.writeOnFailure)){
  Surface* next=0;if(scenario.outputMode!=1){int nextIndex=index+1;if(nextIndex>9)nextIndex=9;if(scenario.chainMode==1)nextIndex=index;if(scenario.chainMode==2){chain=1-chain;nextIndex=0;}next=&world.surfaces[chain][nextIndex];++next->refs;}*out=next;
 }return result;
}
static long __stdcall Blt(void* self,void* rect,void* source,void* sourceRect,unsigned flags,void* effects){Event& e=Begin(4,self,Identity(source),Identity(sourceRect),flags,Identity(effects));memcpy(e.rect,rect,16);Finish();Surface* s=(Surface*)self;++s->blits;memcpy(s->lastRect,rect,16);for(int i=0;i<16;++i)s->bytes[i]^=(BYTE)(Identity(source)+i);return scenario.blitResult;}
static unsigned long __stdcall Release(void* self){Begin(6,self);Finish();return --((Surface*)self)->refs;}
static DWORD Replacement(DWORD va){if(va==0x510610)return Address(&ManagedTexture::UpdatePageCoords);if(va==0x534a88)return(DWORD)_ftol;Fail(84);return 0;}
static int MapReference(const char* path,const char* patches){
 BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);
 f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int count=0;while(fread(entry,sizeof(entry),1,f)==1){BYTE* site=(BYTE*)entry[0];if(entry[0]<0x5102d0||entry[0]+5>0x5104fe||*site!=0xe8||entry[2]!=0xe8||(DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1])return 0;*(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);++count;}fclose(f);return count==5&&FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=code;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{int(CacheTexture::*method)(UnknownTextureRegion*,int);DWORD address;}call;call.method=&CacheTexture::CopyMipLevels;if(retailActive)call.address=0x5102d0;
 __try{observed.result=(((CacheTexture*)world.cache)->*call.method)((UnknownTextureRegion*)world.regions[0],scenario.level);observed.completed=1;}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;attachments=0;memset(&world,0x65+scenario.seed%15,sizeof(world));
 for(int n=0;n<2;++n){At(world.games[n],0xc)=(DWORD)world.displays[n];At(world.displays[n],0x5bc)=n?1:scenario.partial;At(world.groups[n],0x1d0)=scenario.initialTexels+n*7;At(world.textures[n],0x14)=scenario.sourceSide;At(world.textures[n],0x70)=(DWORD)&world.surfaces[0][n];At(world.textures[n],0x80)=scenario.helperPage?(DWORD)world.cache:0;At(world.textures[n],0x94)=scenario.helperRegion?(DWORD)world.regions[n]:0;At(world.regions[n],0x14)=(DWORD)world.textures[n];At(world.regions[n],0x18)=6+n;At(world.regions[n],0x1c)=scenario.left;At(world.regions[n],0x20)=scenario.top;At(world.regions[n],0x24)=scenario.right;At(world.regions[n],0x28)=scenario.bottom;
  for(int j=0;j<10;++j){Surface& s=world.surfaces[n][j];s.table=(DWORD)surfaceTable;s.refs=10+n*10+j;s.attached=s.blits=0;memset(s.lastRect,0,sizeof(s.lastRect));for(int k=0;k<16;++k)s.bytes[k]=(BYTE)(j*37+k*17+n+scenario.seed);}}
 At(world.cache,0)=(DWORD)cacheTables[0];At(world.cache,0x14)=scenario.width;At(world.cache,0x18)=scenario.height;At(world.cache,0x70)=(DWORD)&world.surfaces[1][0];At(world.cache,0x80)=(DWORD)world.groups[0];At(world.cache,0x188)=0;At(world.cache,0x18c)=0;
 g_TrackGame=(TrackGame*)world.games[0];if(scenario.invalid==1)g_TrackGame=0;if(scenario.invalid==2)At(world.games[0],0xc)=0;if(scenario.invalid==3)At(world.cache,0x80)=0;if(scenario.invalid==4)At(world.regions[0],0x14)=0;if(scenario.invalid==5)At(world.textures[0],0x70)=0;if(scenario.invalid==6)At(world.cache,0x70)=0;ToReference();_fpreset();_controlfp(scenario.round,_MCW_RC);
 try{Invoke();}catch(int code){observed.exception=code;}observed.fpStatus=_statusfp();Snapshot(observed.state);_fpreset();
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int i=0;while(a[i]==b[i])++i;printf("Mismatch%d offset%d %02x/%02x dims%d,%d source%d level%d coords%x,%x,%x,%x depths%d/%d partial%d fail%d/%d mode%d/%d invalid%d throw%d mutate%d/%d events%d/%d fp%x/%x\n",checks,i,a[i],b[i],scenario.width,scenario.height,scenario.sourceSide,scenario.level,scenario.left,scenario.top,scenario.right,scenario.bottom,scenario.sourceDepth,scenario.destinationDepth,scenario.partial,scenario.failAt,scenario.status,scenario.chainMode,scenario.outputMode,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count,expected.fpStatus,observed.fpStatus);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.width=64;scenario.height=32;scenario.sourceSide=16;scenario.level=0;scenario.left=0x3e800000;scenario.top=0x3e800000;scenario.right=0x3f000000;scenario.bottom=0x3f000000;scenario.sourceDepth=scenario.destinationDepth=5;scenario.helperPage=scenario.helperRegion=1;scenario.partial=1;scenario.initialTexels=37;scenario.uploadResult=-7;}
static int Count(int kind){int count=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++count;return count;}
static Event& Find(int kind){for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)return observed.events[i];Fail(85);return observed.events[0];}
static void BasicOracle(){if(!observed.completed||observed.exception||observed.result!=1||Count(4)!=5||Count(5)!=1||Count(6)||attachments!=10||At(world.cache,0x188)||At(world.cache,0x18c)!=1)Fail(90);int rect[4]={16,8,32,16},whole[4]={16,8,32,16};unsigned area=37;int level=0;for(int i=0;i<observed.count;++i){Event& e=observed.events[i];if(e.kind==4){if(e.who!=110+level||e.args[0]!=100+level||e.args[1]||e.args[2]!=0x1000000||e.args[3]||memcmp(e.rect,rect,16))Fail(91);area+=(rect[3]-rect[1])*(rect[2]-rect[0]);for(int k=0;k<4;++k)rect[k]/=2;++level;}if(e.kind==5&&(e.args[0]!=-1||memcmp(e.rect,whole,16)))Fail(92);}if(At(world.groups[0],0x1d0)!=area||At(world.textures[0],0x84)!=0x3e800000||At(world.textures[0],0x88)!=0x3e800000||At(world.textures[0],0x8c)!=0x3e800000||At(world.textures[0],0xa8)!=6)Fail(93);for(int j=0;j<10;++j)for(int c=0;c<2;++c)if(world.surfaces[c][j].refs!=10+c*10+j+(j>=1&&j<=5?1:0))Fail(94);}
int main(int argc,char** argv){if(argc!=3||!MapReference(argv[1],argv[2]))return 3;surfaceTable[2]=(DWORD)Release;surfaceTable[5]=(DWORD)Blt;surfaceTable[12]=(DWORD)Attached;cacheTables[0][9]=Address(&Hooks::Upload);cacheTables[1][9]=Address(&Hooks::AlternateUpload);
 Base();Compare();BasicOracle();Base();scenario.partial=0;Compare();if(observed.result!=1||Count(5)||At(world.cache,0x188)!=1)return 4;
 Base();scenario.sourceSide=64;scenario.failAt=1;scenario.status=1;Compare();if(observed.result||Count(4)||Count(5)||At(world.cache,0x188))return 5;
 Base();scenario.blitResult=1;Compare();if(observed.result||attachments||At(world.groups[0],0x1d0)!=37||Count(5))return 6;
 Base();scenario.failAt=1;scenario.status=-7;Compare();if(observed.result!=1||Count(4)!=1||attachments!=2||Count(5)!=1||At(world.groups[0],0x1d0)!=165)return 7;
 Base();scenario.sourceSide=64;scenario.sourceDepth=7;scenario.destinationDepth=7;Compare();if(observed.result!=1||Count(4)!=5||attachments!=12||Find(4).args[0]!=102||Find(4).who!=110||Count(5)!=1)return 12;
 Base();scenario.throwAt=2;Compare();if(observed.exception!=0x7a1||Count(5)||At(world.groups[0],0x1d0)!=37)return 8;
 const int sides[]={1,2,4,8,16,32,64,128,256};for(int w=0;w<9;++w)for(int s=0;s<9;++s)for(int level=0;level<5;++level)for(int mode=0;mode<8;++mode){Base();scenario.width=sides[w];scenario.height=sides[(w+3)%9];scenario.sourceSide=sides[s];scenario.level=level;scenario.partial=mode&1;scenario.helperPage=mode&2;scenario.helperRegion=mode&4;scenario.seed=checks;Compare();if(Count(6))return 9;}
 const int coords[]={0xbf800000,0xbdcccccd,0,0x3dcccccd,0x3eaaaaab,0x3f000000,0x3f800000,0x3fc00000,0x4effffff,0x7f800000,0xff800000,0x7fc01234};const int rounds[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
 for(int a=0;a<12;++a)for(int b=0;b<12;++b)for(int round=0;round<4;++round){Base();scenario.left=coords[a];scenario.top=coords[b];scenario.right=coords[(a+3)%12];scenario.bottom=coords[(b+5)%12];scenario.round=rounds[round];scenario.sourceSide=sides[(a+b)%9];scenario.level=(a+b)%9;Compare();}
 const int statuses[]={0,1,-1,(-2147483647-1),2147483647};for(int fail=1;fail<=14;++fail)for(int status=0;status<5;++status)for(int mode2=0;mode2<8;++mode2){Base();scenario.sourceSide=64;scenario.failAt=fail;scenario.status=statuses[status];scenario.outputMode=mode2%4;scenario.writeOnFailure=mode2&4;scenario.partial=mode2&1;Compare();}
 for(int chain=0;chain<3;++chain)for(int depthS=0;depthS<10;++depthS)for(int depthD=0;depthD<10;++depthD){Base();scenario.chainMode=chain;scenario.sourceDepth=depthS;scenario.destinationDepth=depthD;scenario.sourceSide=128;Compare();}
 for(int r=0;r<5;++r)for(int b2=0;b2<5;++b2)for(int u=0;u<5;++u){Base();scenario.partial=statuses[r];scenario.blitResult=statuses[b2];scenario.uploadResult=statuses[u];Compare();}
 for(int path=0;path<4;++path){Base();scenario.sourceSide=path&1?64:16;scenario.partial=path&2?0:1;Compare();int count=observed.count;for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<7;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=6;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 10;}
 const int dims[]={-2147483647-1,-2,-1,0,1,3,2147483647};for(int dw=0;dw<7;++dw)for(int dh=0;dh<7;++dh)for(int ds=0;ds<7;++ds){Base();scenario.width=dims[dw];scenario.height=dims[dh];scenario.sourceSide=dims[ds];scenario.initialTexels=dims[(dw+dh)%7];Compare();}
 // The negative-side first loop would not terminate on an endlessly
 // successful cyclic chain. Our dependency deliberately fails after 12.
 Base();scenario.left=0x3f800000;scenario.right=0;scenario.sourceSide=16;scenario.chainMode=1;Compare();if(observed.result||attachments!=13||Count(4)||Count(5))return 11;
 printf("CacheTextureMips: %d complete mip-chain, rectangle, ownership-boundary, FP and failure scenarios match\n",checks);return 0;
}
