// Private whole-operation comparison. COM, OS and game dependencies are
// scripted; this program never creates a real media stream or display surface.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/MediaControl.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/PCRenderTarget.h"
extern "C" const UnknownGuid CLSID_AMMultiMediaStream,IID_IAMMultiMediaStream,MSPID_PrimaryVideo,MSPID_PrimaryAudio,IID_IDirectDrawMediaStream,IID_IDirectDraw,IID_IDirectDrawSurface4;
static const UnknownGuid* guids[]={&CLSID_AMMultiMediaStream,&IID_IAMMultiMediaStream,&MSPID_PrimaryVideo,&MSPID_PrimaryAudio,&IID_IDirectDrawMediaStream,&IID_IDirectDraw,&IID_IDirectDrawSurface4};
static DWORD guidAddresses[]={0x558ec0,0x558ed0,0x558eb0,0x558ea0,0x558e90,0x556080,0x5560f0};
static BYTE referenceSpace[0x800000]={1};
enum{Multi,Video,VideoDraw,DisplayDraw,Draw,Surface,Sample,SampleSurface,RenderSurface,FrameSurface};
struct Object {DWORD table;int refs,calls;BYTE tail[20];};
struct World {BYTE control[0x80],targets[2][sizeof(PCRenderTarget)],displays[2][sizeof(UnknownDisplay)];Object objects[20];char names[2][260];};
static World world;
static DWORD comTable[23],mediaTable[27];
struct Event {int kind,who,args[8],releaseCount;BYTE payload[520];World state;};
struct Observation {int result,exception,faultKind,faultAddress,count,releases,addrefs,outputs;Event events[48];World state;};
static Observation observed,expected;
struct Scenario {unsigned seed;int installed,resultAt,status,alias,width,height,pathLength,wideReturn,outputMode,writeOnFailure,initial,mutateAt,mutation,throwAt,invalid;};
static Scenario scenario;
static int retailActive,checks;
static MediaControl* Media(){return (MediaControl*)world.control;}
// Confirmed BaseObject refcount +4 and GameObject render-context +0x18.
// Raw fixture storage exposes these protected fields without changing headers.
static int& Ref(){return *(int*)(world.control+4);}
static void*& Context(){return *(void**)(world.control+0x18);}
static PCRenderTarget* Target(int n){return (PCRenderTarget*)world.targets[n];}
static UnknownDisplay* Display(int n){return (UnknownDisplay*)world.displays[n];}
static void Fail(int code){printf("Invariant%d check%d ref%d installed%d stage%d status%x alias%d output%d/%d initial%d invalid%d throw%d mutation%d/%d count%d last%d\n",code,checks,retailActive,scenario.installed,scenario.resultAt,scenario.status,scenario.alias,scenario.outputMode,scenario.writeOnFailure,scenario.initial,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,observed.count,observed.count?observed.events[observed.count-1].kind:-1);fflush(stdout);ExitProcess(code);}
static int ObjectIndex(const void* p){if(!p)return -1;for(int n=0;n<20;++n)if(p==&world.objects[n])return n;Fail(81);return -1;}
static int TargetIndex(const void* p){if(p==Target(0))return 0;if(p==Target(1))return 1;if(!p)return -1;Fail(81);return -1;}
static Object* Obj(int role,int bank=0){if(scenario.alias==1)role=0;else if(scenario.alias==2 && role<=VideoDraw)role=0;else if(scenario.alias==3 && (role==DisplayDraw || role==Draw))role=DisplayDraw;else if(scenario.alias==4 && (role==Surface || role==SampleSurface || role==RenderSurface || role==FrameSurface))role=Surface;return &world.objects[bank*10+role];}
static int Guid(const void* p){for(int n=0;n<7;++n)if(!memcmp(p,guids[n],16))return n;Fail(82);return -1;}
static int Status(int kind){return scenario.resultAt==kind?scenario.status:0;}
static Event& Begin(int kind,int who=0,int a=0,int b=0,int c=0,int d=0){if(observed.count>=48)Fail(83);Event& e=observed.events[observed.count++];e.kind=kind;e.who=who;e.args[0]=a;e.args[1]=b;e.args[2]=c;e.args[3]=d;e.releaseCount=observed.releases;e.state=world;if(who>=0 && who<20 && kind>=4 && kind!=8 && kind!=19)++world.objects[who].calls;return e;}
static void Finish(){if(scenario.mutateAt==observed.count){MediaControl* m=Media();switch(scenario.mutation){
case 0:Context()=Target(1);break;
case 1:Target(0)->targetDisplay=Display(1);Target(1)->targetDisplay=Display(0);m->displayDirectDraw=(UnknownDirectDrawInterface*)Obj(DisplayDraw,1);break;
case 2:m->multiMediaStream=(UnknownMultiMediaStreamInterface*)Obj(Multi,1);m->videoStream=(UnknownMediaStreamInterface*)Obj(Video,1);m->videoDrawStream=(UnknownMediaStreamInterface*)Obj(VideoDraw,1);m->directDraw=(UnknownDirectDrawInterface*)Obj(Draw,1);m->frameWidth^=0x55;m->frameHeight^=0xaa;m->doneCallback=(void(__cdecl*)(UIDialog*))0x12776655;break;
case 3:Target(0)->renderSurface=(UnknownSurfaceInterface*)Obj(RenderSurface,1);Target(1)->renderSurface=(UnknownSurfaceInterface*)Obj(RenderSurface,0);m->frameWidth=-7;m->frameHeight=0x7fffffff;break;
}}if(scenario.throwAt==observed.count)throw 0x761;}
static void Out(void** out,int role,int stage){if(scenario.resultAt==stage && scenario.outputMode==2)return;if(Status(stage)<0 && !scenario.writeOnFailure)return;Object* object=(scenario.resultAt==stage && scenario.outputMode==1)?0:Obj(role,scenario.initial&1);*out=object;if(object)++object->refs;++observed.outputs;}
GameObject* GameObject::SetRenderContext(void* target){if(this!=Media())Fail(84);Begin(1,TargetIndex(target));Finish();renderContext=target;return (GameObject*)0x11223344;}
int MediaControl::IsDirectShowInstalled(){Begin(2,0,scenario.installed);Finish();return scenario.installed;}
struct CallViews {
    GameObject* Context(void* target){return ((GameObject*)this)->GameObject::SetRenderContext(target);}
    int Release(){if((void*)this!=Media())Fail(85);Begin(19,0,Ref());Finish();++observed.releases;--Ref();return scenario.status;}
};
static HRESULT __stdcall Create(const UnknownGuid* clsid,void* outer,DWORD flags,const UnknownGuid* iid,void** out){if(Guid(clsid)!=0 || outer || flags!=1 || Guid(iid)!=1 || out!=(void**)&Media()->multiMediaStream)Fail(86);Begin(3,0,Guid(clsid),flags,Guid(iid),ObjectIndex(*out));Finish();Out(out,Multi,3);return Status(3);}
static long __stdcall Initialize(void* self,int type,unsigned long flags,void* graph){if(type || flags || graph)Fail(87);Begin(4,ObjectIndex(self),type,flags,(int)graph);Finish();return Status(4);}
static long __stdcall AddMedia(void* self,void* object,const UnknownGuid* purpose,unsigned long flags,void** out){int id=Guid(purpose),kind=id==2?5:6;if((id!=2 && id!=3) || out || flags!=(id==3?1:0) || (id==3 && object))Fail(88);Begin(kind,ObjectIndex(self),ObjectIndex(object),id,flags);Finish();return Status(kind);}
static long __stdcall AddRef(void* self){int n=ObjectIndex(self);Begin(7,n);Finish();++world.objects[n].refs;++observed.addrefs;return scenario.status;}
static int __stdcall Wide(UINT page,DWORD flags,const char* source,int length,unsigned short* out,int capacity){if(page || flags || length!=-1 || capacity!=260 || !out)Fail(89);int n=source==world.names[0]?0:source==world.names[1]?1:-1;if(n<0)Fail(89);Event& e=Begin(8,n,page,flags,length,capacity);strcpy((char*)e.payload,source);Finish();for(int k=0;k<260;++k)out[k]=k<=scenario.pathLength?(unsigned char)source[k]:0;return scenario.wideReturn;}
static long __stdcall OpenMovieFile(void* self,const unsigned short* path,unsigned long flags){if(flags)Fail(90);Event& e=Begin(9,ObjectIndex(self),flags);memcpy(e.payload,path,520);Finish();return Status(9);}
static long __stdcall GetMedia(void* self,const UnknownGuid* purpose,void** out){if(Guid(purpose)!=2 || out!=(void**)&Media()->videoStream)Fail(91);Begin(10,ObjectIndex(self),Guid(purpose),ObjectIndex(*out));Finish();Out(out,Video,10);return Status(10);}
static long __stdcall Query(void* self,const UnknownGuid* iid,void** out){int id=Guid(iid),kind=id==4?11:id==5?13:id==6?16:0;void** expectedOut=kind==11?(void**)&Media()->videoDrawStream:kind==13?(void**)&Media()->directDraw:(void**)&Media()->sampleSurface;if(!kind || out!=expectedOut)Fail(92);Begin(kind,ObjectIndex(self),id,ObjectIndex(*out));Finish();Out(out,kind==11?VideoDraw:kind==13?Draw:SampleSurface,kind);return Status(kind);}
static long __stdcall GetFormat(void* self,UnknownMediaSurfaceDesc* desc,void* palette,void* desired,void* flags){if(palette || desired || flags || desc->size!=0x6c)Fail(93);Event& e=Begin(12,ObjectIndex(self));memcpy(e.payload,desc,0x6c);for(int k=4;k<0x6c;++k)if(((BYTE*)desc)[k])Fail(94);Finish();for(k=0;k<0x6c;++k)((BYTE*)desc)[k]=(BYTE)(scenario.seed+k*17);desc->size=0x6c;desc->width=scenario.width;desc->height=scenario.height;return Status(12);}
static long __stdcall CreateSurface(void* self,UnknownSurfaceDesc* desc,void** out,void* outer){if(outer || !out)Fail(95);int kind=out==(void**)&Media()->frameSurface?18:14;unsigned size=kind==14?0x6c:0x7c;if(desc->size!=size)Fail(96);Event& e=Begin(kind,ObjectIndex(self),ObjectIndex(*out),size);memcpy(e.payload,desc,size);Finish();Out(out,kind==14?Surface:FrameSurface,kind);return Status(kind);}
static long __stdcall CreateSample(void* self,void* surface,void* rect,unsigned long flags,void** out){if(rect || flags!=1 || out!=(void**)&Media()->videoSample)Fail(97);Begin(15,ObjectIndex(self),ObjectIndex(surface),flags,ObjectIndex(*out));Finish();Out(out,Sample,15);return Status(15);}
static long __stdcall GetDesc(void* self,UnknownSurfaceDesc* desc){if(desc->size!=0x7c)Fail(98);Event& e=Begin(17,ObjectIndex(self));memcpy(e.payload,desc,0x7c);for(int k=4;k<0x7c;++k)if(((BYTE*)desc)[k])Fail(98);Finish();for(k=0;k<0x7c;++k)((BYTE*)desc)[k]=(BYTE)(scenario.seed+k*19);desc->size=0x7c;return Status(17);}
template<class T>static DWORD Address(T method){union{T method;DWORD address;}u;u.method=method;return u.address;}
static DWORD imports[]={(DWORD)Create,(DWORD)Wide};
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
    *(DWORD*)0x5503fc=(DWORD)Create;*(DWORD*)0x5501a0=(DWORD)Wide;
    for(int g=0;g<7;++g)if(memcmp(guids[g],(void*)guidAddresses[g],16))return 0;
    BYTE* candidate=(BYTE*)Address(&MediaControl::Open);if(!VirtualProtect(candidate,656,PAGE_EXECUTE_READWRITE,&old))return 0;
    f=fopen(patches,"rb");if(!f)return 0;DWORD entry[3];int records=0;
    while(fread(entry,sizeof(entry),1,f)==1){++records;
        if(entry[0]==0){BYTE* site=(BYTE*)entry[1];if(site<(BYTE*)0x4a2560 || site+5>(BYTE*)0x4a27e6 || *site!=0xe8 || (DWORD)(site+5)+*(DWORD*)(site+1)!=entry[2])return 0;DWORD hook=entry[2]==0x4692f0?Address(&CallViews::Context):entry[2]==0x4a23d0?(DWORD)MediaControl::IsDirectShowInstalled:0;if(!hook)return 0;*(DWORD*)(site+1)=hook-(DWORD)(site+5);}
        else if(entry[0]==1){if(entry[1]+4>656 || entry[2]>1)return 0;*(DWORD*)(candidate+entry[1])=(DWORD)&imports[entry[2]];}
        else return 0;
    }fclose(f);return records==4 && FlushInstructionCache(GetCurrentProcess(),0,0);
}
static int Filter(EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(){union{MediaControl*(MediaControl::*method)(void*,const char*,void(*)(UIDialog*),UIDialog*);DWORD address;}call;call.method=&MediaControl::Open;if(retailActive)call.address=0x4a2560;
    __try{observed.result=(int)(Media()->*call.method)(scenario.invalid==1?0:Target(0),world.names[0],(void(*)(UIDialog*))0x12345678,(UIDialog*)0x23456789);}__except(Filter(GetExceptionInformation())){}
}
static void Run(int retail){retailActive=retail;memset(&observed,0,sizeof(observed));observed.result=-501;memset(&world,0x53+scenario.seed%19,sizeof(world));MediaControl* m=Media();*(DWORD*)m=(DWORD)mediaTable;Ref()=7;Context()=Target(0);
    for(int n=0;n<20;++n){world.objects[n].table=(DWORD)comTable;world.objects[n].refs=10+n;world.objects[n].calls=0;}
    for(n=0;n<2;++n){Target(n)->targetDisplay=Display(n);Target(n)->renderSurface=(UnknownSurfaceInterface*)Obj(RenderSurface,n);Display(n)->directDraw=(UnknownDirectDrawInterface*)Obj(DisplayDraw,n);for(int k=0;k<scenario.pathLength;++k)world.names[n][k]='A'+(k+n)%26;world.names[n][scenario.pathLength]=0;}
    m->multiMediaStream=scenario.initial?(UnknownMultiMediaStreamInterface*)Obj(Multi,1):0;m->videoStream=scenario.initial?(UnknownMediaStreamInterface*)Obj(Video,1):0;m->videoDrawStream=scenario.initial?(UnknownMediaStreamInterface*)Obj(VideoDraw,1):0;m->directDraw=scenario.initial?(UnknownDirectDrawInterface*)Obj(Draw,1):0;m->sampleSurface=scenario.initial?(UnknownSurfaceInterface*)Obj(SampleSurface,1):0;m->frameSurface=scenario.initial?(UnknownSurfaceInterface*)Obj(FrameSurface,1):0;m->videoSample=scenario.initial?(UnknownStreamSampleInterface*)Obj(Sample,1):0;
    if(scenario.invalid==2)Target(0)->targetDisplay=0;if(scenario.invalid==3)Display(0)->directDraw=0;if(scenario.invalid==4)Target(0)->renderSurface=0;
    try{Invoke();}catch(int code){observed.exception=code;}observed.state=world;
}
static void Compare(){Run(1);expected=observed;Run(0);++checks;if(memcmp(&expected,&observed,sizeof(observed))){BYTE* a=(BYTE*)&expected;BYTE* b=(BYTE*)&observed;int off=0;while(a[off]==b[off])++off;printf("Mismatch%d offset%d %02x/%02x stage%d status%x alias%d out%d write%d initial%d invalid%d throw%d mutation%d/%d events%d/%d\n",checks,off,a[off],b[off],scenario.resultAt,scenario.status,scenario.alias,scenario.outputMode,scenario.writeOnFailure,scenario.initial,scenario.invalid,scenario.throwAt,scenario.mutateAt,scenario.mutation,expected.count,observed.count);fflush(stdout);ExitProcess(2);}}
static void Base(){memset(&scenario,0,sizeof(scenario));scenario.seed=19;scenario.installed=1;scenario.width=640;scenario.height=480;scenario.pathLength=31;scenario.wideReturn=32;}
static int Count(int kind){int n=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==kind)++n;return n;}
int main(int argc,char** argv){if(argc!=3 || !MapReference(argv[1],argv[2]))return 3;if(sizeof(MediaControl)!=0x80 || sizeof(UnknownMediaSurfaceDesc)!=0x6c || sizeof(UnknownSurfaceDesc)!=0x7c)return 4;
    comTable[0]=(DWORD)Query;comTable[1]=(DWORD)AddRef;comTable[4]=(DWORD)GetMedia;comTable[6]=(DWORD)CreateSurface;comTable[9]=(DWORD)GetFormat;comTable[12]=(DWORD)Initialize;comTable[13]=(DWORD)CreateSample;comTable[15]=(DWORD)AddMedia;comTable[16]=(DWORD)OpenMovieFile;comTable[22]=(DWORD)GetDesc;mediaTable[2]=Address(&CallViews::Release);
    Base();Compare();if(observed.result!=(int)Media() || observed.releases || observed.addrefs!=1 || observed.outputs!=8 || Media()->frameRect[0] || Media()->frameRect[1] || Media()->frameRect[2]!=640 || Media()->frameRect[3]!=480 || Media()->doneCallback!=(void(*)(UIDialog*))0x12345678 || Media()->doneOwner!=(UIDialog*)0x23456789)return 5;
    int descriptorAt=0;for(int i=0;i<observed.count;++i)if(observed.events[i].kind==18)descriptorAt=i;
    UnknownSurfaceDesc* sent=(UnknownSurfaceDesc*)observed.events[descriptorAt].payload;
    if(!descriptorAt || sent->size!=0x7c || sent->flags!=0x1007 || sent->width!=640 || sent->height!=480 || sent->caps[0]!=0x2800 || observed.events[descriptorAt].payload[16]!=(BYTE)(19+16*19) || Obj(Multi)->refs!=12)return 12;
    Base();scenario.installed=0;Compare();if(observed.result || observed.releases!=1 || observed.count!=3 || observed.outputs || Ref()!=6)return 6;
    Base();scenario.resultAt=6;scenario.status=0x80040256;Compare();if(observed.result!=(int)Media() || observed.releases || Count(18)!=1)return 7;
    Base();scenario.resultAt=6;scenario.status=0x80040255;Compare();if(observed.result || observed.releases!=1 || observed.addrefs || Count(8))return 8;
    Base();scenario.resultAt=3;scenario.status=1;Compare();if(observed.result || observed.releases!=1 || Count(4))return 9;
    Base();scenario.resultAt=4;scenario.status=1;Compare();if(observed.result!=(int)Media() || observed.releases || Count(18)!=1)return 10;
    const int results[]={0,1,2,-1,(-2147483647-1),2147483647,(int)0x80040256,(int)0x80040255,(int)0x80040257,(int)0x80004005};
    for(int stage=3;stage<=18;++stage)for(int r=0;r<10;++r)for(int alias=0;alias<5;++alias)for(int output=0;output<3;++output)for(int initial=0;initial<2;++initial)for(int writes=0;writes<2;++writes){Base();scenario.seed=checks+1;scenario.resultAt=stage;scenario.status=results[r];scenario.alias=alias;scenario.outputMode=output;scenario.initial=initial;scenario.writeOnFailure=writes;scenario.wideReturn=results[(r+3)%10];Compare();}
    const int sizes[]={0,1,320,640,-1,(-2147483647-1),2147483647};const int lengths[]={0,1,15,127,258,259};
    for(int width=0;width<7;++width)for(int height=0;height<7;++height)for(int length=0;length<6;++length)for(int wide=0;wide<4;++wide){Base();scenario.seed=checks+1;scenario.width=sizes[width];scenario.height=sizes[height];scenario.pathLength=lengths[length];scenario.wideReturn=wide?scenario.pathLength+1:0;scenario.alias=(width+height)%5;Compare();}
    for(int path=0;path<4;++path){Base();scenario.alias=path;if(path==3){scenario.resultAt=12;scenario.status=-1;}Compare();int count=observed.count;
        for(int at=1;at<=count;++at){scenario.throwAt=at;Compare();scenario.throwAt=0;for(int mutation=0;mutation<4;++mutation){scenario.mutateAt=at;scenario.mutation=mutation;Compare();}scenario.mutateAt=0;}}
    for(int invalid=1;invalid<=4;++invalid){Base();scenario.invalid=invalid;Compare();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.releases)return 11;}
    printf("MediaControlOpen: %d complete COM, descriptor, state, alias and failure scenarios match\n",checks);return 0;
}
