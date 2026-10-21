// Private ABI views and scripted surface boundaries; production uses canonical headers.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class TrackGame;struct UnknownSurfaceInterface;struct UnknownSurfaceDesc;struct UnknownTexturePalette;struct UnknownTextureFormatChoice;
class TextureMapManager {public:void* GrowSecondScratchBuffer(unsigned);};
class PCTextureMap {public:virtual int CreateSurface(void*,int,int,int,int,int,int,UnknownTexturePalette*,int,void*,int,int,int,int,UnknownTextureFormatChoice*,int,unsigned);int CreateSystemSurface(UnknownSurfaceDesc*,int,int*);int DumpLevel(UnknownSurfaceInterface*,const char*);};
int ActualPixelFormatBytesPerPixel(int);void ActualFillPixelFormat(int,void*);int ActualIsFormat4444Or8888(int);
static BYTE referenceSpace[0x800000]={1};
struct Surface {DWORD table;int refs,locks,palettes;BYTE tail[48];};
struct World {BYTE texture[0x100],games[2][0x300],displays[2][0x1b0],managers[2][64],choice[24],palette[128],surfacePalette[64],source[512],destination[2][512],scratch[512];Surface surfaces[4];};
static World world;
TrackGame* g_TrackGame;UnknownSurfaceInterface* g_SharedMipTextureSurfaces[10];UnknownSurfaceInterface* g_SharedTextureSurfaces[10];int g_DebugDumpTextureLevels;
struct Scenario {int seed,width,height,minimum,stride,sourceFormat,format,flags,choice,opaqueFormat,alphaFormat,bits,palette,surfacePalette,existing,shared,blade,cached,checkMemory,dump,fallback,failAt,status,writeOnFailure,outputMode,nullScratch,pitch,stateCount,throwAt,mutateAt,mutation,invalid;};
static Scenario scenario;
struct State {World world;DWORD shared[20];int game,dump;};
struct Event {int kind,who,args[16],formatCount,formats[5];BYTE payload[128];State state;};
struct Observation {int result,completed,exception,faultKind,faultAddress,count;Event events[40];State state;};
static Observation observed,expected;static int checks,retailActive;
static DWORD textureTable[21],surfaceTable[33],drawTable[7];
static DWORD& At(void* p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static void Fail(int code){printf("Invariant%d check%d ref%d last%d\n",code,checks,retailActive,observed.count?observed.events[observed.count-1].kind:0);fflush(stdout);ExitProcess(code);}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(80);union{T p;DWORD a;}u;u.p=p;return u.a;}
static int Identity(const void* p){if(!p)return 0;if(p==world.texture)return 1;if(p==world.palette)return 2;if(p==world.surfacePalette)return 3;if(p==world.source)return 4;if(p==world.choice)return 5;if(p==world.scratch)return 6;for(int i=0;i<2;++i){if(p==world.games[i])return 10+i;if(p==world.displays[i])return 20+i;if(p==world.managers[i])return 30+i;if(p==world.destination[i])return 40+i;}for(i=0;i<4;++i)if(p==&world.surfaces[i])return 50+i;return(int)p;}
static void FromReference(){if(!retailActive)return;g_TrackGame=*(TrackGame**)0x56e26c;memcpy(g_SharedMipTextureSurfaces,(void*)0x68a36c,40);memcpy(g_SharedTextureSurfaces,(void*)0x68a394,40);g_DebugDumpTextureLevels=*(int*)0x689968;}
static void ToReference(){if(!retailActive)return;*(TrackGame**)0x56e26c=g_TrackGame;memcpy((void*)0x68a36c,g_SharedMipTextureSurfaces,40);memcpy((void*)0x68a394,g_SharedTextureSurfaces,40);*(int*)0x689968=g_DebugDumpTextureLevels;}
static void Snapshot(State& s){FromReference();s.world=world;memcpy(s.shared,g_SharedMipTextureSurfaces,40);memcpy(s.shared+10,g_SharedTextureSurfaces,40);s.game=Identity(g_TrackGame);s.dump=g_DebugDumpTextureLevels;}
static Event& Begin(int kind,const void* p=0,int a=0,int b=0,int c=0,int d=0){FromReference();if(observed.count>=40)Fail(81);Event& e=observed.events[observed.count++];e.kind=kind;e.who=Identity(p);e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;return e;}
static void Finish(){Event& e=observed.events[observed.count-1];Snapshot(e.state);
 if(scenario.mutateAt==observed.count){switch(scenario.mutation){
 case 0:g_TrackGame=(TrackGame*)world.games[1];g_DebugDumpTextureLevels=!g_DebugDumpTextureLevels;break;
 case 1:At(world.texture,0x70)=(DWORD)&world.surfaces[2];At(world.texture,0x10)=(DWORD)world.managers[1];At(world.texture,0x78)=(DWORD)world.surfacePalette;break;
 case 2:At(world.texture,0x20)=565;At(world.texture,0x24)=2;At(world.texture,0x14)=9;At(world.texture,0x18)=7;At(world.choice,0xc)=8;At(world.choice,0x10)=8888;break;
 case 3:for(int j=0;j<10;++j){g_SharedTextureSurfaces[j]=(UnknownSurfaceInterface*)&world.surfaces[3];g_SharedMipTextureSurfaces[j]=(UnknownSurfaceInterface*)&world.surfaces[3];}At(world.games[0],0x2d0)=1;At(world.games[0],0x2d4)^=0x400;break;
 }}ToReference();if(scenario.throwAt==observed.count)throw 0x781;
}
static int Result(int stage,int normal=0){return scenario.failAt==stage?scenario.status:normal;}
int IsFormat4444Or8888(int format){Begin(1,0,format);Finish();return ActualIsFormat4444Or8888(format);}
void FillPixelFormat(int format,void* out){Begin(2,0,format);Finish();ActualFillPixelFormat(format,out);}
int PixelFormatBytesPerPixel(int format){Begin(3,0,format);Finish();return ActualPixelFormatBytesPerPixel(format);}
void* TextureMapManager::GrowSecondScratchBuffer(unsigned size){Begin(4,this,size);Finish();return scenario.nullScratch?0:world.scratch;}
static void WriteSurface(void** out,int status,int boolean){if(scenario.outputMode==2)return;if((boolean?!status:status!=0)&&!scenario.writeOnFailure)return;Surface* surface=scenario.outputMode==1?0:&world.surfaces[scenario.outputMode==3?1:(scenario.outputMode==4?2:0)];*out=surface;if(surface)++surface->refs;}
int PCTextureMap::CreateSystemSurface(UnknownSurfaceDesc* desc,int flags,int* formats){Event& e=Begin(5,this,flags);memcpy(e.payload,desc,124);for(int i=0;i<5;++i){e.formats[i]=formats[i];++e.formatCount;if(!formats[i])break;}if(i==5)Fail(82);Finish();int selected=scenario.fallback%(e.formatCount-1);int result=Result(5,1);WriteSurface((void**)((BYTE*)this+0x70),result,1);if(result)At(this,0x20)=e.formats[selected];return result;}
int PCTextureMap::DumpLevel(UnknownSurfaceInterface* surface,const char* name){Begin(15,this,Identity(surface),Identity(name));Finish();return Result(15,-7);}
int ConvertPixels(void* destination,void* source,int width,int height,int destinationStride,int sourceStride,int destinationFormat,int sourceFormat,int dither,UnknownTexturePalette* palette,int alpha,unsigned key){Event& e=Begin(11,destination,Identity(source),width,height,destinationStride);int args[]={sourceStride,destinationFormat,sourceFormat,dither,Identity(palette),alpha,(int)key};memcpy(e.args+4,args,sizeof(args));memcpy(e.payload,source,64);Finish();for(int i=0;i<64;++i)((BYTE*)destination)[i]=(BYTE)(((BYTE*)source)[i]^destinationFormat^i);return Result(11,-7);}
int AverageColor(void* bits,int width,int height,int stride,int format,UnknownTexturePalette* palette){Event& e=Begin(12,bits,width,height,stride,format);e.args[4]=Identity(palette);memcpy(e.payload,bits,64);Finish();return Result(12,0x12345678);}
struct Hooks {
 int Create(void* bits,int width,int height,int stride,int minimum,int sourceFormat,int format,UnknownTexturePalette* palette,int flags,void* surfacePalette,int check,int unused,int u,int v,UnknownTextureFormatChoice* choice,int threshold,unsigned key){return((PCTextureMap*)this)->PCTextureMap::CreateSurface(bits,width,height,stride,minimum,sourceFormat,format,palette,flags,surfacePalette,check,unused,u,v,choice,threshold,key);}
 int Build(int filter){Begin(14,this,filter);Finish();return Result(14,1);}
 int Key(unsigned color){Begin(16,this,color);Finish();At(this,0x30)=1;At(this,0x34)=color;return Result(16,-7);}
};
static unsigned long __stdcall AddRef(void* self){Begin(7,self);Finish();return ++((Surface*)self)->refs;}
static long __stdcall CreateSurface(void* self,void* desc,void** out,void* outer){Event& e=Begin(6,self,Identity(*out),Identity(outer));memcpy(e.payload,desc,124);Finish();int result=Result(6);WriteSurface(out,result,0);return result;}
static long __stdcall SetPalette(void* self,void* palette){Begin(8,self,Identity(palette));Finish();++((Surface*)self)->palettes;return Result(8);}
static long __stdcall GetCaps(void* self,void* caps){Begin(9,self);Finish();memset(caps,0x73,16);return Result(9);}
static long __stdcall Lock(void* self,void* rect,void* desc,int flags,void* event){BYTE want[124];memset(want,0,124);*(DWORD*)want=124;if(memcmp(want,desc,124))Fail(83);Begin(10,self,Identity(rect),flags,Identity(event));Finish();memset(desc,0x64,124);At(desc,0)=124;At(desc,8)=scenario.height+1;At(desc,12)=scenario.width+2;At(desc,16)=scenario.pitch;At(desc,36)=scenario.invalid==4?0:(DWORD)world.destination[self==&world.surfaces[2]];if(!Result(10))++((Surface*)self)->locks;return Result(10);}
static long __stdcall Unlock(void* self,void* rect){Begin(13,self,Identity(rect));Finish();--((Surface*)self)->locks;return Result(13);}
static DWORD Replacement(DWORD va){switch(va){case 0x511ad0:return(DWORD)IsFormat4444Or8888;case 0x5119c0:return(DWORD)FillPixelFormat;case 0x511970:return(DWORD)PixelFormatBytesPerPixel;case 0x511370:return Address(&TextureMapManager::GrowSecondScratchBuffer);case 0x4c68e0:return Address(&PCTextureMap::CreateSystemSurface);case 0x4d1d20:return(DWORD)ConvertPixels;case 0x4d24d0:return(DWORD)AverageColor;case 0x4c84e0:return Address(&PCTextureMap::DumpLevel);}Fail(84);return 0;}
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
        if(!(entry[0]>=0x4c69c0 && entry[0]+5<=0x4c71b1) || *site!=entry[2] || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[1]){printf("patch failure %x %x %x\n",entry[0],entry[1],entry[2]);return 0;}
        *(DWORD*)(site+1)=Replacement(entry[1])-(DWORD)(site+5);
    }fclose(f);return records==20 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){DWORD code=e->ExceptionRecord->ExceptionCode;if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=EXCEPTION_INT_DIVIDE_BY_ZERO&&code!=EXCEPTION_INT_OVERFLOW)return EXCEPTION_CONTINUE_SEARCH;observed.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{int(Hooks::*method)(void*,int,int,int,int,int,int,UnknownTexturePalette*,int,void*,int,int,int,int,UnknownTextureFormatChoice*,int,unsigned);DWORD address;}call;call.method=&Hooks::Create;if(retailActive)call.address=0x4c69c0;
 __try{observed.result=(((Hooks*)world.texture)->*call.method)(scenario.bits?world.source:0,scenario.width,scenario.height,scenario.stride,scenario.minimum,scenario.sourceFormat,scenario.format,scenario.palette?(UnknownTexturePalette*)world.palette:0,scenario.flags,scenario.surfacePalette?world.surfacePalette:0,scenario.checkMemory,0x7788,0x1234,-7,scenario.choice?(UnknownTextureFormatChoice*)world.choice:0,0x80,0xff00ff);observed.completed=1;}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-999;memset(&world,0x65+scenario.seed%15,sizeof(world));
 for(int n=0;n<2;++n){At(world.games[n],0xc)=(DWORD)world.displays[n];At(world.games[n],0x2d0)=scenario.blade;At(world.games[n],0x2d4)=scenario.cached?0x400:0;At(world.displays[n],0x190)=(DWORD)world.managers[n];At(world.managers[n],0)=(DWORD)drawTable;}
 for(n=0;n<4;++n){world.surfaces[n].table=(DWORD)surfaceTable;world.surfaces[n].refs=10+n;world.surfaces[n].locks=0;world.surfaces[n].palettes=0;}
 for(n=0;n<10;++n){g_SharedMipTextureSurfaces[n]=scenario.shared?(UnknownSurfaceInterface*)&world.surfaces[1]:0;g_SharedTextureSurfaces[n]=scenario.shared?(UnknownSurfaceInterface*)&world.surfaces[1]:0;}
 At(world.texture,0)=(DWORD)textureTable;At(world.texture,0x10)=(DWORD)world.managers[0];At(world.texture,0x70)=scenario.existing?(DWORD)&world.surfaces[2]:0;At(world.texture,0x44)=scenario.stateCount;At(world.choice,0xc)=scenario.opaqueFormat;At(world.choice,0x10)=scenario.alphaFormat;
 for(n=0;n<512;++n){world.source[n]=(BYTE)(n*37+scenario.seed);world.destination[0][n]=(BYTE)(n*17+41);world.destination[1][n]=(BYTE)(n*11+43);}
 g_TrackGame=(TrackGame*)world.games[0];g_DebugDumpTextureLevels=scenario.dump;if(scenario.invalid==1)g_TrackGame=0;if(scenario.invalid==2)At(world.games[0],0xc)=0;if(scenario.invalid==3)At(world.displays[0],0x190)=0;ToReference();
 try{Invoke();}catch(int code){observed.exception=code;}Snapshot(observed.state);
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int i=0;while(a[i]==b[i])++i;printf("Mismatch%d offset%d %02x/%02x dims%d,%d min%d format%d/%d flags%x choice%d existing%d shared%d fail%d/%d out%d invalid%d throw%d mutate%d/%d events%d/%d\n",checks,i,a[i],b[i],scenario.width,scenario.height,scenario.minimum,scenario.sourceFormat,scenario.format,scenario.flags,scenario.choice,scenario.existing,scenario.shared,scenario.failAt,scenario.status,scenario.outputMode,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.width=8;scenario.height=4;scenario.minimum=1;scenario.stride=11;scenario.sourceFormat=555;scenario.format=555;scenario.opaqueFormat=565;scenario.alphaFormat=4444;scenario.bits=1;scenario.palette=1;scenario.surfacePalette=1;scenario.pitch=34;}
static int Count(int kind){int n=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++n;return n;}
static int Mips(){int n=1;if(scenario.flags&2){int width=scenario.width/2,height=scenario.height/2;while(width>=scenario.minimum||height>=scenario.minimum){++n;width/=2;height/=2;}}return n;}
static void CheckDescriptor(const Event& e){BYTE wanted[124];memset(wanted,0,124);At(wanted,0)=124;At(wanted,8)=scenario.height;At(wanted,12)=scenario.width;int format=scenario.choice?(ActualIsFormat4444Or8888(scenario.format)?scenario.alphaFormat:scenario.opaqueFormat):scenario.format;ActualFillPixelFormat(format,wanted+72);
 int shared=!(scenario.flags&12)&&!scenario.blade&&!scenario.cached&&(format==555||format==565);
 if(scenario.flags&2){At(wanted,4)=0x21007;At(wanted,24)=Mips();At(wanted,104)=scenario.flags&4?0x400808:0x401808;}else{At(wanted,4)=scenario.flags&4?7:0x1007;At(wanted,104)=scenario.flags&4?0x800:0x1800;}
 if(memcmp(wanted,e.payload,124))Fail(90);
 if(e.kind==5){int want[5]={0,0,0,0,0},n=0;want[n++]=format;if(format==4444)want[n++]=1555;if(format!=555&&format!=565)want[n++]=555;want[n++]=format==565?555:565;want[n++]=0;if(e.formatCount!=n||memcmp(want,e.formats,n*4))Fail(91);}
}
int main(int argc,char** argv){if(argc!=3||!MapReference(argv[1],argv[2]))return 3;textureTable[15]=Address(&Hooks::Build);textureTable[18]=Address(&Hooks::Key);surfaceTable[1]=(DWORD)AddRef;surfaceTable[14]=(DWORD)GetCaps;surfaceTable[25]=(DWORD)Lock;surfaceTable[31]=(DWORD)SetPalette;surfaceTable[32]=(DWORD)Unlock;drawTable[6]=(DWORD)CreateSurface;
 Base();Compare();if(!observed.completed||observed.result!=1||At(world.texture,0x24)!=1||At(world.texture,0x1c)!=8||world.surfaces[0].refs!=12||world.surfaces[0].locks||g_SharedTextureSurfaces[1]!=(UnknownSurfaceInterface*)&world.surfaces[0]||At(world.texture,0x3c)!=0x12345678||At(world.texture,0x44)!=2)return 4;
 int convertAt=0;for(int i=0;i<observed.count;++i){Event& e=observed.events[i];if(e.kind==5||e.kind==6)CheckDescriptor(e);if(e.kind==11){convertAt=i+1;if(e.args[0]!=4||e.args[1]!=8||e.args[2]!=4||e.args[3]!=17||e.args[4]!=11||e.args[5]!=555||e.args[6]!=555||e.args[7]||e.args[8]!=2||e.args[9]!=0x80||e.args[10]!=0xff00ff)return 5;}}
 for(i=0;i<64;++i)if(world.destination[0][i]!=(BYTE)(world.source[i]^555^i))return 6;
 Base();scenario.shared=1;Compare();if(Count(5)||Count(6)||Count(7)!=2||world.surfaces[1].refs!=13)return 7;
 Base();scenario.flags=2;Compare();if(At(world.texture,0x24)!=4||Count(14)!=1||g_SharedMipTextureSurfaces[4]!=(UnknownSurfaceInterface*)&world.surfaces[0])return 8;
 Base();scenario.failAt=10;scenario.status=1;Compare();if(observed.result||world.surfaces[0].refs!=12||Count(13)||Count(11))return 9;
 Base();scenario.throwAt=convertAt;Compare();if(observed.exception!=0x781||world.surfaces[0].locks!=1||Count(13))return 10;
 const int formats[]={8,555,565,888,1555,4444,8888,0,999};const int sources[]={8,555,8888};
 for(int f=0;f<9;++f)for(int flags=0;flags<16;++flags)for(int mask=0;mask<64;++mask){Base();scenario.seed=checks+1;scenario.format=formats[f];scenario.sourceFormat=sources[(mask>>3)%3];scenario.flags=flags;scenario.bits=mask&1;scenario.existing=mask&2;scenario.shared=mask&4;scenario.blade=mask&8;scenario.cached=mask&16;scenario.choice=mask&32;scenario.checkMemory=mask&4;scenario.palette=mask&8;scenario.surfacePalette=mask&16;scenario.dump=mask&2;scenario.fallback=(mask>>4)%4;Compare();for(int e=0;e<observed.count;++e)if(observed.events[e].kind==5||observed.events[e].kind==6)CheckDescriptor(observed.events[e]);}
 const int dims[]={1,2,3,4,7,8,16,32,64,128};const int minima[]={1,2,4,16};for(int w=0;w<10;++w)for(int h=0;h<10;++h)for(int m=0;m<4;++m)for(int shared=0;shared<2;++shared){Base();scenario.width=dims[w];scenario.height=dims[h];scenario.minimum=minima[m];scenario.flags=2;scenario.shared=shared;Compare();if(At(world.texture,0x24)!=(DWORD)Mips())return 11;}
 const int results[]={0,1,-1,(-2147483647-1),2147483647};for(int stage=5;stage<=16;++stage)for(int r=0;r<5;++r)for(int mode=0;mode<12;++mode){Base();scenario.format=formats[mode%7];scenario.flags=mode&2;scenario.failAt=stage;scenario.status=results[r];scenario.outputMode=mode%3;scenario.writeOnFailure=mode&4;scenario.existing=mode&8;scenario.fallback=1;scenario.dump=1;Compare();}
 for(int path=0;path<5;++path){Base();scenario.format=formats[path];scenario.flags=path&3;scenario.choice=path&1;scenario.dump=1;Compare();int count=observed.count;for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<4;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
 for(int invalid=1;invalid<=4;++invalid){Base();scenario.format=888;scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION)return 12;}
 Base();scenario.flags=2;scenario.sourceFormat=8888;scenario.nullScratch=1;Compare();if(observed.result||Count(9)||Count(5))return 13;
 Base();scenario.flags=2;scenario.sourceFormat=8888;scenario.nullScratch=1;scenario.checkMemory=1;Compare();if(observed.result!=1||Count(4))return 14;
 const int pitches[]={-35,-1,0,1,2,3,4,35,2147483647,(-2147483647-1)};
 for(int pi=0;pi<10;++pi)for(int fmt=0;fmt<9;++fmt)for(int count=-3;count<=8;++count){Base();scenario.pitch=pitches[pi];scenario.format=formats[fmt];scenario.existing=1;scenario.stateCount=count;Compare();}
 for(int alias=3;alias<=4;++alias)for(int mask2=0;mask2<16;++mask2){Base();scenario.outputMode=alias;scenario.flags=mask2&3;scenario.shared=mask2&4;scenario.existing=mask2&8;Compare();}
 Base();scenario.format=4444;scenario.fallback=1;Compare();if(observed.result!=1||Count(16)!=1||At(world.texture,0x30)!=1||At(world.texture,0x34)!=0xff00ff||At(world.texture,0x44))return 15;
 Base();scenario.format=4444;Compare();if(observed.result!=1||At(world.texture,0x44)!=4||At(world.texture,0x48)!=0x29||At(world.texture,0x4c)||At(world.texture,0x50)!=0x1b||At(world.texture,0x54)!=1||At(world.texture,0x58)!=0x13||At(world.texture,0x5c)!=0x1234||At(world.texture,0x60)!=0x14||At(world.texture,0x64)!=(DWORD)-7)return 16;
 printf("TextureSurfaceCreate: %d complete descriptor, mip, reference, conversion-boundary and failure scenarios match\n",checks);return 0;
}
