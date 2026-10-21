// Private ABI boundary views only. Production compiles actual shared headers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
extern "C" void __cdecl __CxxFrameHandler();
class TextureMapManager;class TrackGame;class PCTextureMap;
struct CameraRect {int left,top,right,bottom;};
class GUIManager {public:PCTextureMap* CopyScreenToTexture(int,CameraRect*);};
class GameObject {protected:void DisableByClassName(const char*);void EnableByClassName(const char*);public:static DWORD DisableAddress();static DWORD EnableAddress();};
class BackgroundImage {public:int ReleaseDc();};
class RenderTarget {public:void AdvanceFrame();};
class PCTextureMap {public:PCTextureMap(TextureMapManager*,int);};
static BYTE referenceSpace[0x800000]={1};
struct World {BYTE games[2][0x80],targets[2][0x80],displays[2][0x200],children[2][0x40],gui[0x3e0],background[0x40],texture[0x80],managers[2][0x40];BYTE source[2][32768],capture[8192];CameraRect rect;};
static World world;TrackGame* g_TrackGame;
struct Scenario {int seed,bpp,format,width,height,left,top,pad,negativePitch,dim,rect,background,primary,lockResult,callbackReturn,failMalloc,failNew,throwAt,mutateAt,mutation,invalid;};
static Scenario scenario;
struct State {World world;int game,rawLive,textureLive;};
struct Event {int kind,who,args[20];char text[32];State state;};
struct Observation {int result,completed,exception,faultKind,faultAddress,count,allocSize,deletes;Event events[24];State state;};
static Observation observed,expected;static int checks,retailActive,rawLive,textureLive;
static DWORD gameTable[10],surfaceTable[33],textureTable[5];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d check%d ref%d last%d\n",code,checks,retailActive,observed.count?observed.events[observed.count-1].kind:0);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.gui)return 1;if(p==world.background)return 2;if(p==world.texture)return 3;if(p==world.capture)return 4;for(int i=0;i<2;++i){if(p==world.games[i])return 10+i;if(p==world.targets[i])return 20+i;if(p==world.children[i])return 30+i;if(p==world.displays[i])return 40+i;if(p==world.managers[i])return 50+i;if(p==world.source[i])return 60+i;}return (int)p;}
static void Snapshot(State& s){s.world=world;s.game=Identity(g_TrackGame);s.rawLive=rawLive;s.textureLive=textureLive;}
static Event& Begin(int kind,const void* p=0,int a=0,int b=0,int c=0,int d=0){if(observed.count>=24)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(p);e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;return e;}
static void Finish(){Event& e=observed.events[observed.count-1];Snapshot(e.state);
 if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:g_TrackGame=(TrackGame*)world.games[1];*(TrackGame**)0x56e26c=g_TrackGame;break;
 case 1:At(world.games[0],0x10)=(DWORD)world.targets[1];At(world.games[0],0xc)=(DWORD)world.displays[1];At(world.games[0],0x34)=(DWORD)world.children[1];break;
 case 2:At(world.targets[0],0x28)=scenario.format^0x55;At(world.gui,0xd8)=(DWORD)world.managers[1];world.rect.left=-1;world.rect.top=2;world.rect.right=5;world.rect.bottom=5;break;
 case 3:At(world.displays[0],0x1a8)=(DWORD)world.source[1];At(world.displays[0],0x19c)=(DWORD)world.source[1];for(int j=0;j<8192;++j)world.capture[j]^=0x95;for(j=0;j<32768;++j)world.source[0][j]^=0x63;At(world.source[0],0)=(DWORD)surfaceTable;break;
 }}if(scenario.throwAt==observed.count)throw 0x771;
}
static int SourceFile(const char* p){const char* a=strrchr(p,'\\'),*b=strrchr(p,'/');if(!a||(b&&b>a))a=b;a=a?a+1:p;if(strcmp(a,"GUIManager.cpp")&&strcmp(a,"GuiScreenTexture.cpp"))Fail(82);return 1;}
int PixelFormatBytesPerPixel(int format){Begin(1,0,format);Finish();return scenario.bpp;}
int BackgroundImage::ReleaseDc(){Begin(2,this);Finish();return scenario.callbackReturn;}
void GameObject::DisableByClassName(const char* s){Event& e=Begin(3,this);if(strlen(s)>31)Fail(83);strcpy(e.text,s);Finish();}
void GameObject::EnableByClassName(const char* s){Event& e=Begin(7,this);if(strlen(s)>31)Fail(83);strcpy(e.text,s);Finish();}
DWORD GameObject::DisableAddress(){return Address(&GameObject::DisableByClassName);}DWORD GameObject::EnableAddress(){return Address(&GameObject::EnableByClassName);}
void RenderTarget::AdvanceFrame(){Begin(6,this);Finish();}
void* DebugMalloc(unsigned size,const char* file,int line){Begin(9,0,size,SourceFile(file),line);Finish();observed.allocSize=size;if(size>8192)throw 0x772;if(scenario.failMalloc)return 0;rawLive=1;return world.capture;}
void DebugFree(void* p,const char* file,int line){Begin(14,p,SourceFile(file),line);Finish();if(p && p!=world.capture)Fail(84);rawLive=0;}
void* operator new(unsigned size,const char* file,int line){Begin(11,0,size,SourceFile(file),line);Finish();if(size!=0x80)Fail(85);if(scenario.failNew)return 0;textureLive=1;return world.texture;}
void operator delete(void* p,const char* file,int line){Begin(15,p,SourceFile(file),line);Finish();if(p && p!=world.texture)Fail(86);if(p)textureLive=0;++observed.deletes;}
PCTextureMap::PCTextureMap(TextureMapManager* manager,int flags){Begin(12,this,Identity(manager),flags);Finish();At(this,0)=(DWORD)textureTable;At(this,4)=0x73461299;}
struct Hooks {
 PCTextureMap* Construct(TextureMapManager* manager,int flags){return new(this) PCTextureMap(manager,flags);}
 int Render(){Begin(4,this);Finish();return scenario.callbackReturn;}
 int Present(){Begin(5,this);Finish();return scenario.callbackReturn;}
 int Create(void* bits,int width,int height,int pitch,int width2,int format,int format2,void* palette,int flags,void* other,int a,int b,int c,int d,void* choice,int e,unsigned color){
  Event& event=Begin(13,this,Identity(bits),width,height,pitch);int args[]={width2,format,format2,Identity(palette),flags,Identity(other),a,b,c,d,Identity(choice),e,(int)color};memcpy(event.args+4,args,sizeof(args));Finish();return scenario.callbackReturn;
 }
};
static long __stdcall Lock(void* self,void* rect,void* desc,int flags,void* event){
 BYTE want[124];memset(want,0,124);*(DWORD*)want=124;if(memcmp(want,desc,124))Fail(87);Begin(8,self,Identity(rect),flags,Identity(event));Finish();
 int n=self==world.source[1];if(self!=world.source[0] && !n)Fail(88);memset(desc,0x64,124);At(desc,0)=124;At(desc,8)=scenario.height;At(desc,12)=scenario.width;int pitch=(scenario.width+scenario.pad)*scenario.bpp;if(scenario.negativePitch)pitch=-pitch;At(desc,16)=pitch;At(desc,36)=scenario.invalid==4?0:(DWORD)(world.source[n]+16384);return scenario.lockResult;
}
static long __stdcall Unlock(void* self,void* rect){Begin(10,self,Identity(rect));Finish();return scenario.callbackReturn;}
static DWORD Replacement(DWORD va){switch(va){case 0x511970:return(DWORD)PixelFormatBytesPerPixel;case 0x404c80:return Address(&BackgroundImage::ReleaseDc);case 0x468dd0:return GameObject::DisableAddress();case 0x468f10:return GameObject::EnableAddress();case 0x4e8cc0:return Address(&RenderTarget::AdvanceFrame);case 0x4a2e20:return(DWORD)DebugMalloc;case 0x4a2e60:return(DWORD)DebugFree;case 0x4a3010:return(DWORD)(void*(__cdecl*)(unsigned,const char*,int))operator new;case 0x4a3060:return(DWORD)(void(__cdecl*)(void*,const char*,int))operator delete;case 0x53471a:return(DWORD)__CxxFrameHandler;case 0x4c5f00:return Address(&Hooks::Construct);}Fail(89);return 0;}
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)){printf("map coverage %p end%p\n",referenceSpace,referenceSpace+sizeof(referenceSpace));return 0;}
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 || dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE || pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
       pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 ||
       pe->OptionalHeader.SizeOfHeaders>(DWORD)size || pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD sectionsOffset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if(sectionsOffset>(DWORD)size || pe->FileHeader.NumberOfSections>((DWORD)size-sectionsOffset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(section[i].PointerToRawData>(DWORD)size || section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData ||
           section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage || section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){++records;BYTE* site=(BYTE*)entry[0];
        if(!((entry[0]>=0x486170 && entry[0]+5<=0x4864f0)||(entry[0]>=0x54b870 && entry[0]+5<=0x54b894)) || *site!=entry[2] || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1]){printf("patch failure %x %x %x\n",entry[0],entry[1],entry[2]);return 0;}
        *(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);
    }fclose(f);return records==13 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{PCTextureMap*(GUIManager::*method)(int,CameraRect*);DWORD address;}call;call.method=&GUIManager::CopyScreenToTexture;if(retailActive)call.address=0x486170;
 __try{observed.result=Identity((((GUIManager*)world.gui)->*call.method)(scenario.dim,scenario.rect?&world.rect:0));observed.completed=1;}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;rawLive=textureLive=0;memset(&world,0x65+scenario.seed%15,sizeof(world));
 for(int n=0;n<2;++n){At(world.games[n],0)=(DWORD)gameTable;At(world.games[n],0xc)=(DWORD)world.displays[n];At(world.games[n],0x10)=(DWORD)world.targets[n];At(world.games[n],0x34)=(DWORD)world.children[n];At(world.targets[n],0x28)=scenario.format+n;At(world.displays[n],0x1a8)=scenario.primary?0:(DWORD)world.source[n];At(world.displays[n],0x19c)=(DWORD)world.source[n];for(int j=0;j<32768;++j)world.source[n][j]=(BYTE)(j*37+(j>>8)*17+n*23+scenario.seed);At(world.source[n],0)=(DWORD)surfaceTable;}
 At(world.gui,0x3c)=scenario.background?(DWORD)world.background:0;At(world.gui,0xd8)=(DWORD)world.managers[0];world.rect.left=scenario.left;world.rect.top=scenario.top;world.rect.right=scenario.left+scenario.width;world.rect.bottom=scenario.top+scenario.height;
 g_TrackGame=(TrackGame*)world.games[0];if(scenario.invalid==1)g_TrackGame=0;if(scenario.invalid==2)At(world.games[0],0x10)=0;if(scenario.invalid==3){At(world.displays[0],0x1a8)=0;At(world.displays[0],0x19c)=0;}*(TrackGame**)0x56e26c=g_TrackGame;
 try{Invoke();}catch(int code){observed.exception=code;}Snapshot(observed.state);
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int i=0;while(a[i]==b[i])++i;printf("Mismatch%d offset%d %02x/%02x bpp%d dims%d,%d dim%d rect%d fail%d/%d invalid%d throw%d mutate%d/%d events%d/%d\n",checks,i,a[i],b[i],scenario.bpp,scenario.width,scenario.height,scenario.dim,scenario.rect,scenario.failMalloc,scenario.failNew,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.bpp=4;scenario.format=0x222;scenario.width=7;scenario.height=5;scenario.left=2;scenario.top=1;scenario.pad=3;scenario.background=1;scenario.callbackReturn=-7;}
static int Count(int kind){int n=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++n;return n;}
static void CheckPixels(){
 BYTE wanted[8192];memset(wanted,0x65+scenario.seed%15,sizeof(wanted));int width=scenario.width,height=scenario.height,bpp=scenario.bpp;int pitch=(width+scenario.pad)*bpp;if(scenario.negativePitch)pitch=-pitch;int left=scenario.rect?scenario.left:0,top=scenario.rect?scenario.top:0;
 for(int y=0;y<height;++y)for(int x=0;x<width;++x){BYTE* pixel=wanted+(y*width+x)*bpp;const BYTE* input=world.source[0]+16384+(y+top)*pitch+left*bpp+x*bpp;
  for(int k=0;k<bpp;++k)pixel[k]=input[k];if(scenario.dim){if(bpp==1){if((x&1)==(y&1))pixel[0]=0;}else if(bpp==2){unsigned short v=(unsigned short)(input[0]|(input[1]<<8));unsigned short mask=scenario.format==0x22b?0x7bde:0xf7de;v=(v>>1)&(mask>>1);pixel[0]=(BYTE)v;pixel[1]=(BYTE)(v>>8);}else if(bpp==3 || bpp==4){pixel[0]=input[0]/2;pixel[1]=input[1]/2;pixel[2]=input[2]/2;}}}
 if(memcmp(wanted,world.capture,sizeof(wanted)) || rawLive || !textureLive || observed.result!=3 || Count(14)!=1)Fail(90);
}
int main(int argc,char** argv){if(argc!=3 || !MapReference(argv[1],argv[2]))return 3;gameTable[8]=Address(&Hooks::Render);gameTable[9]=Address(&Hooks::Present);surfaceTable[25]=(DWORD)Lock;surfaceTable[32]=(DWORD)Unlock;textureTable[4]=Address(&Hooks::Create);
 Base();Compare();if(!observed.completed || observed.result!=3 || rawLive || !textureLive || observed.allocSize!=140 || Count(3)!=2 || Count(7)!=2 || Count(10)!=1 || Count(13)!=1 || Count(14)!=1 || observed.deletes)return 4;
 int createAt=0,ctorAt=0;for(int i=0;i<observed.count;++i){if(observed.events[i].kind==13)createAt=i+1;if(observed.events[i].kind==12)ctorAt=i+1;}
 Event& e=observed.events[createAt-1];if(e.args[0]!=4 || e.args[1]!=7 || e.args[2]!=5 || e.args[3]!=7 || e.args[4]!=7 || e.args[8]!=4 || e.args[12]!=2 || e.args[13]!=1 || e.args[15]!=0x80 || e.args[16]!=0xff00ff)return 5;
 for(i=0;i<5;++i)if(memcmp(world.capture+i*28,world.source[0]+16384+i*40,28))return 6;
 Base();scenario.lockResult=1;Compare();if(!observed.completed || observed.result || rawLive || textureLive || Count(9) || Count(10))return 7;
 Base();scenario.throwAt=ctorAt;Compare();if(observed.exception!=0x771 || !rawLive || textureLive || observed.deletes!=1 || Count(15)!=1)return 8;
 Base();scenario.throwAt=createAt;Compare();if(observed.exception!=0x771 || !rawLive || !textureLive || observed.deletes || Count(14))return 9;
 Base();scenario.failNew=1;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.faultAddress || observed.faultKind || !rawLive || textureLive || observed.deletes!=1 || Count(15)!=1)return 10;
 const int widths[]={0,1,2,3,4,7,8,15};const int heights[]={0,1,2,3,6};const int bpps[]={0,1,2,3,4,5};
 for(int bp=0;bp<6;++bp)for(int w=0;w<8;++w)for(int h=0;h<5;++h)for(int mode=0;mode<32;++mode){Base();scenario.seed=checks+1;scenario.bpp=bpps[bp];scenario.width=widths[w];scenario.height=heights[h];scenario.dim=mode&1;scenario.rect=(mode>>1)&1;scenario.negativePitch=(mode>>2)&1;scenario.primary=(mode>>3)&1;scenario.format=mode&16?0x22b:0x222;scenario.background=mode&4;scenario.left=(w%3)-1;scenario.top=(h%3)-1;Compare();CheckPixels();}
 const int results[]={0,1,-1,(-2147483647-1),2147483647};for(int r=0;r<5;++r)for(int bp2=1;bp2<=4;++bp2)for(int mode2=0;mode2<8;++mode2){Base();scenario.lockResult=results[r];scenario.callbackReturn=results[(r+2)%5];scenario.bpp=bp2;scenario.dim=mode2&1;scenario.rect=mode2&2;scenario.failMalloc=mode2&4;Compare();}
 for(int path=0;path<4;++path){Base();scenario.bpp=path+1;scenario.dim=1;scenario.rect=1;Compare();int count=observed.count;for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<4;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=4;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 11;}
 const int dims[]={-3,-1,0,1,2147483647,(-2147483647-1)};for(int dw=0;dw<6;++dw)for(int dh=0;dh<6;++dh){Base();scenario.width=dims[dw];scenario.height=dims[dh];scenario.throwAt=11;Compare();}
 for(int nw=-3;nw<=3;++nw)for(int nh=-3;nh<=0;++nh){Base();scenario.width=nw;scenario.height=nh;scenario.dim=0;Compare();}
 printf("GuiScreenTexture: %d complete capture, pixels, state, callback, allocation and unwind scenarios match\n",checks);return 0;
}
