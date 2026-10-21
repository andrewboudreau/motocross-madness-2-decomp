#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <new>
#include "../../src/reconstructed/ResourceManager.h"
#include <stddef.h>
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/KrustyBikeCamera.h"
#include "../../src/reconstructed/FileStream.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/krusty2/math/Vec3.h"
extern "C" void __cdecl __CxxFrameHandler();
Vec3 Vec3Normalize(const Vec3&);
Vec3 CrossProductCall(const Vec3&,const Vec3&);
void InitFastInvSqrtTable();
static BYTE referenceSpace[0x800000];
UnknownResourceManager* g_ResourceManager=0;
struct CameraAccess:FollowCamera{using FollowCamera::frames;using FollowCamera::field_0x33c;using FollowCamera::frameCount;};
typedef char CameraLayout[(sizeof(FollowCamera)==0x384 && sizeof(Matrix4)==64 && sizeof(Vec3)==12 && sizeof(Vector3)==12 && sizeof(void(FollowCamera::*)(const char*))==4 && offsetof(CameraAccess,frameCount)==0x334 && offsetof(CameraAccess,frames)==0x338 && offsetof(CameraAccess,field_0x33c)==0x33c)?1:-1];
typedef char StreamLayout[(sizeof(UnknownTextureStream)==0x134 && offsetof(UnknownTextureStream,fileHandle)==0x14 && offsetof(UnknownTextureStream,sliceOffset)==0x130 && offsetof(UnknownTextureStream,archives)==0x12c)?1:-1];
typedef char VectorLayout[(offsetof(Vec3,x)==0 && offsetof(Vec3,y)==4 && offsetof(Vec3,z)==8 && offsetof(Vector3,x)==0 && offsetof(Vector3,y)==4 && offsetof(Vector3,z)==8)?1:-1];
struct Block{int size,alive;BYTE data[8192];};
struct Event{int kind,size,line,frameCount;void*pointer;void*frames;void*values;};
struct Result{BYTE camera[sizeof(FollowCamera)];Block heap[4];int blocks,cursor,reads,seeks,exception,complete;unsigned access,address,fp,fpControl;BYTE guarded[4096];int diagnosticDeletes;Event events[12];int eventCount;BYTE previousFrames[128],previousValues[8];};
static BYTE previousFrames[128],previousValues[8];
static Event events[12];static int eventCount,allocationMutation=-1,mutationReached,expectedFrameCount=-1,eolMode;static const char*sourcePath;
static BYTE*guardPage;static int guardAllocation=-1,guardBytes=0,diagnosticDeletes;static void*allocated[4];
static int failAllocation=-1,throwAllocation=-1,pathMode=0,preexisting=0;static unsigned control=_MCW_EM|_PC_64|_RC_NEAR;
static BYTE initialCamera[sizeof(FollowCamera)];
static BYTE camera[sizeof(FollowCamera)];static Block heap[4];static int blocks,cursor,reads,seeks,lineCount,seed;static char lines[12][600];static Result got,want;
static void CheckFile(const char*file){
 char normalized[1024],expected[1024];if(!file||strlen(file)>=sizeof(normalized)||strlen(sourcePath)>=sizeof(expected))ExitProcess(95);
 strcpy(normalized,file);strcpy(expected,sourcePath);for(int i=0;normalized[i];i++)if(normalized[i]=='/')normalized[i]='\\';for(i=0;expected[i];i++)if(expected[i]=='/')expected[i]='\\';
 if(strcmp(normalized,"D:\\aardvark\\VC\\krusty2\\FollowCam.cpp")&&strcmp(normalized,expected))ExitProcess(96);
}
static void Record(int kind,int n,int line,void*pointer){if(eventCount>=12)ExitProcess(97);Event&e=events[eventCount++];CameraAccess*c=(CameraAccess*)camera;e.kind=kind;e.size=n;e.line=line;e.pointer=pointer;e.frameCount=c->frameCount;e.frames=c->frames;e.values=c->field_0x33c;}
void* operator new(unsigned int n,const char*file,int line){
 CheckFile(file);if(n>8192||blocks>=4)ExitProcess(90);int index=blocks++;Block&b=heap[index];b.size=n;
 if(line!=(index==0?0x3c7:index==1?0x3e1:0x3e2))ExitProcess(98);
 Record(1,n,line,0);
 CameraAccess*c=(CameraAccess*)camera;
 if(allocationMutation==index){mutationReached++;if(index==0)c->frameCount=123;else if(index==1)c->frameCount=3;else c->frames=(Matrix4*)previousFrames;}
 if(index==throwAllocation)throw 77;if(index==failAllocation)return 0;b.alive=1;memset(b.data,0xa5,sizeof(b.data));allocated[index]=index==guardAllocation?guardPage+4096-guardBytes:b.data;return allocated[index];
}
void operator delete(void*p){if(!p)return;Record(2,0,0,p);for(int i=0;i<blocks;i++)if(p==allocated[i]){heap[i].alive=0;return;}ExitProcess(91);}
void operator delete(void*p,const char*file,int line){CheckFile(file);if(line!=0x3c7)ExitProcess(99);Record(3,0,line,p);diagnosticDeletes++;operator delete(p);}




UnknownTextureStream* UnknownResourceManager::AddArchive(const char*,int){ExitProcess(201);return 0;}
int UnknownTextureStream::WriteHeader(const char*){ExitProcess(202);return 0;}
struct ConstructorCall{UnknownTextureStream* Stream(UnknownResourceManager*m){return ::new(this)UnknownTextureStream(m);}};
struct DestructorCall{void Stream(){((UnknownTextureStream*)this)->~UnknownTextureStream();}};
template<class T>static DWORD Address(T m){if(sizeof(m)!=4)ExitProcess(93);DWORD p;memcpy(&p,&m,4);return p;}
static int MapReference(const char*path){
    BYTE*mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE*f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE*raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER*dos=(IMAGE_DOS_HEADER*)raw;if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER*section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}
    free(raw);return 1;
}
static void Patch(DWORD site,DWORD old,DWORD next){if(*(BYTE*)site!=0xe8||site+5+*(DWORD*)(site+1)!=old)ExitProcess(94);*(DWORD*)(site+1)=next-site-5;}
static int Fault(EXCEPTION_POINTERS*p){got.exception=p->ExceptionRecord->ExceptionCode;if(got.exception==EXCEPTION_ACCESS_VIOLATION){got.access=p->ExceptionRecord->ExceptionInformation[0];got.address=p->ExceptionRecord->ExceptionInformation[1];}return got.exception==0xe06d7363?EXCEPTION_CONTINUE_SEARCH:EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(DWORD address,const char*path){typedef void(FollowCamera::*Method)(const char*);Method method;memcpy(&method,&address,4);__try{(((FollowCamera*)camera)->*method)(path);got.complete=1;}__except(Fault(GetExceptionInformation())){}}
static void Run(int reference){
 FILE*input=fopen("camera.vue","wb");if(!input)ExitProcess(203);for(int li=0;li<lineCount;li++){fputs(lines[li],input);if(eolMode!=2||li+1<lineCount)fputs(eolMode==0?"\r\n":"\n",input);}fclose(input);
 memset(&got,0,sizeof(got));memset(allocated,0,sizeof(allocated));memset(guardPage,0xa5,4096);diagnosticDeletes=eventCount=mutationReached=0;memset(events,0,sizeof(events));memset(heap,0,sizeof(heap));memset(camera,0x59,sizeof(camera));blocks=cursor=reads=seeks=0;CameraAccess*c=(CameraAccess*)camera;memset(previousFrames,0x39,sizeof(previousFrames));memset(previousValues,0x63,sizeof(previousValues));c->frames=preexisting?(Matrix4*)previousFrames:0;c->field_0x33c=preexisting?(float*)previousValues:0;c->frameCount=77;
 memcpy(initialCamera,camera,sizeof(camera));
 _clearfp();_controlfp(control,_MCW_EM|_MCW_PC|_MCW_RC);
 const char*path=pathMode==1?"missing-does-not-exist.vue":pathMode==2?"":pathMode==3?0:"camera.vue";
 try{Invoke(reference?0x4650e0:Address(&FollowCamera::LoadCameraFile),path);}catch(int code){got.exception=code;}
 got.blocks=blocks;got.cursor=cursor;got.reads=reads;got.seeks=seeks;got.fp=_statusfp();got.fpControl=_controlfp(0,0);memcpy(got.camera,camera,sizeof(camera));memcpy(got.heap,heap,sizeof(heap));memcpy(got.guarded,guardPage,4096);got.diagnosticDeletes=diagnosticDeletes;memcpy(got.events,events,sizeof(events));got.eventCount=eventCount;memcpy(got.previousFrames,previousFrames,sizeof(previousFrames));memcpy(got.previousValues,previousValues,sizeof(previousValues));
 // Fault/throw paths intentionally leave the opened stream alive in retail.
 // Record that state before closing its real FILE solely for test isolation.
 if(blocks&&heap[0].alive&&(guardAllocation!=0||guardBytes>=0x134)){UnknownTextureStream*stream=(UnknownTextureStream*)allocated[0];if(stream->fileHandle)fclose(stream->fileHandle);}
}
static int checks;
static void Check(){Run(1);want=got;Run(0);checks++;if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])at++;printf("GuardAllocation%d bytes%d\n",guardAllocation,guardBytes);printf("Mismatch seed%d line%d at%d values%02x/%02x fail%d throw%d path%d FP%x fullbits%08x/%08x\n%s\n%s\n",seed,lineCount,at,((BYTE*)&want)[at],((BYTE*)&got)[at],failAllocation,throwAllocation,pathMode,control,*(DWORD*)((BYTE*)&want+(at&~3)),*(DWORD*)((BYTE*)&got+(at&~3)),lines[0],lines[1]);ExitProcess(3);}}
static void Success(){if(!got.complete||got.exception)ExitProcess(4);if(pathMode&&memcmp(camera,initialCamera,sizeof(camera)))ExitProcess(13);}
static void GuardOracle(){
 BYTE expected[4096];memset(expected,0xa5,sizeof(expected));unsigned faultAddress=0;
 if(guardAllocation==0&&guardBytes<0x134){
  int offsets[]={8,16,1,20,3,28,12,304,4,2,0,32,24,300,292,296};
  int widths[]={4,4,1,4,1,4,4,4,4,1,1,1,1,4,4,4};
  unsigned values[]={0,0,0,0,0,0,0,0,0,'F','A','O','E',0,(unsigned)(guardPage+4096-guardBytes+0x21),(unsigned)(guardPage+4096-guardBytes+0x21)};
  for(int i=0;i<16;i++){
   if(offsets[i]+widths[i]>guardBytes){faultAddress=(unsigned)(guardPage+4096-guardBytes+(offsets[i]>guardBytes?offsets[i]:guardBytes));break;}
   memcpy(expected+4096-guardBytes+offsets[i],&values[i],widths[i]);
  }
 }else if(guardAllocation==1){
  int offsets[]={0,4,16,20,8,24,32,12,28,36,40,44,48,52,56,60};
  float values[]={1,0,0,1,0,0,0,0,0,0,1,0,1.25f,2.75f,-3.5f,1};
  for(int i=0;i<16;i++){
   if(offsets[i]+4>guardBytes){faultAddress=(unsigned)(guardPage+4096-guardBytes+(offsets[i]>guardBytes?offsets[i]:guardBytes));break;}
   memcpy(expected+4096-guardBytes+offsets[i],&values[i],4);
  }
 }else if(guardAllocation==2){
  if(guardBytes<4)faultAddress=(unsigned)(guardPage+4096);else{float value=4.5f;memcpy(expected+4096-guardBytes,&value,4);}
 }else return;
 if(faultAddress){if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.access!=1||got.address!=faultAddress){printf("Guard fault oracle allocation%d bytes%d got%x wanted%x exception%x access%u\n",guardAllocation,guardBytes,got.address,faultAddress,got.exception,got.access);ExitProcess(26);}}
 else Success();
 if(memcmp(guardPage,expected,4096)){printf("Guard prefix oracle allocation%d bytes%d\n",guardAllocation,guardBytes);ExitProcess(29);}
}
static void Ownership(){CameraAccess*c=(CameraAccess*)camera;int count=expectedFrameCount<0?lineCount:expectedFrameCount;if(blocks!=3||heap[0].size!=0x134||heap[1].size!=count*64||heap[2].size!=count*4||heap[0].alive||!heap[1].alive||!heap[2].alive||c->frameCount!=count||c->frames!=(Matrix4*)heap[1].data||c->field_0x33c!=(float*)heap[2].data)ExitProcess(10);for(int i=0;i<128;i++)if(previousFrames[i]!=0x39)ExitProcess(11);for(i=0;i<8;i++)if(previousValues[i]!=0x63)ExitProcess(11);}
static unsigned random=0x4597;static float F(){random=random*1664525u+1013904223u;return ((int)(random%200001)-100000)*0.003271f;}
int main(int argc,char**argv){setvbuf(stdout,0,_IONBF,0);if(argc!=3||!MapReference(argv[1]))return 1;sourcePath=argv[2];guardPage=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD guardOld;if(!guardPage||!VirtualProtect(guardPage+4096,4096,PAGE_NOACCESS,&guardOld))return 25;InitFastInvSqrtTable();*(void**)0x572b44=0;
DWORD constructor=Address(&ConstructorCall::Stream),destructor=Address(&DestructorCall::Stream),alloc=(DWORD)(void*(__cdecl*)(unsigned int,const char*,int))operator new,del=(DWORD)(void(__cdecl*)(void*))operator delete;
Patch(0x46510f,0x4a3010,alloc);Patch(0x465131,0x460d10,constructor);Patch(0x46515f,0x460f50,Address(&UnknownTextureStream::Open));Patch(0x46518a,0x461aa0,Address(&UnknownTextureStream::ReadLine));Patch(0x4651a2,0x5476f0,(DWORD)_strnicmp);Patch(0x4651c3,0x461aa0,Address(&UnknownTextureStream::ReadLine));Patch(0x4651e3,0x4a3010,alloc);Patch(0x465212,0x4a3010,alloc);Patch(0x46522c,0x461340,Address(&UnknownTextureStream::Seek));Patch(0x465240,0x461aa0,Address(&UnknownTextureStream::ReadLine));Patch(0x46525e,0x5476f0,(DWORD)_strnicmp);Patch(0x4652b3,0x53530b,(DWORD)sscanf);Patch(0x4653b6,0x5087b0,(DWORD)Vec3Normalize);Patch(0x4653dc,0x5087b0,(DWORD)Vec3Normalize);Patch(0x46540a,0x515600,(DWORD)CrossProductCall);Patch(0x465498,0x461aa0,Address(&UnknownTextureStream::ReadLine));Patch(0x4654ae,0x460d60,destructor);Patch(0x4654b4,0x4a30c0,del);Patch(0x54abb1,0x4a3060,(DWORD)(void(__cdecl*)(void*,const char*,int))operator delete);if(*(BYTE*)0x54abbf!=0xe9||0x54abc4+*(DWORD*)0x54abc0!=0x53471a) return 2;*(DWORD*)0x54abc0=(DWORD)__CxxFrameHandler-0x54abc4;FlushInstructionCache(GetCurrentProcess(),(void*)0x400000,0x290000);if((FollowCamera*)(KrustyBikeCamera*)camera!=(FollowCamera*)camera)return 34;
for(seed=0;seed<1000;seed++){lineCount=seed%5;for(int i=0;i<lineCount;i++)sprintf(lines[i],"CAMERA C 0 0 0 %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g",F(),F(),F(),F(),F(),F(),F(),F(),F(),F());Check();Success();Ownership();}
// Independent exact unit-axis, position and trailing-value oracle across FP modes.
lineCount=1;strcpy(lines[0],"CAMERA C 0 0 0 0 1 0 0 0 1 1.25 -3.5 2.75 4.5");
unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++){control=_MCW_EM|pcs[pc]|rcs[rc];Check();Success();float*matrix=(float*)heap[1].data;float expected[]={1,0,0,0,0,1,0,0,0,0,1,0,1.25f,2.75f,-3.5f,1};for(int k=0;k<16;k++)if(matrix[k]!=expected[k])ExitProcess(5);if(*(float*)heap[2].data!=4.5f||((CameraAccess*)camera)->frameCount!=1||heap[0].alive||!heap[1].alive||!heap[2].alive)ExitProcess(6);}
for(int pc3=0;pc3<3;pc3++)for(int rc3=0;rc3<4;rc3++)for(int varied=0;varied<32;varied++){control=_MCW_EM|pcs[pc3]|rcs[rc3];lineCount=varied%5;for(int j=0;j<lineCount;j++)sprintf(lines[j],"CAMERA C 0 0 0 %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g",F(),F(),F(),F(),F(),F(),F(),F(),F(),F());Check();Success();Ownership();}
control=_MCW_EM|_PC_64|_RC_NEAR;lineCount=1;strcpy(lines[0],"CAMERA C 0 0 0 0 1 0 0 0 1 1.25 -3.5 2.75 4.5");
preexisting=1;Check();Success();Ownership();preexisting=0;
for(pathMode=1;pathMode<=3;pathMode++){Check();Success();if(blocks!=1||heap[0].alive||((CameraAccess*)camera)->frameCount!=77||((CameraAccess*)camera)->frames)ExitProcess(7);}pathMode=0;preexisting=1;pathMode=1;Check();Success();if(((CameraAccess*)camera)->frames!=(Matrix4*)previousFrames||((CameraAccess*)camera)->field_0x33c!=(float*)previousValues||((CameraAccess*)camera)->frameCount!=77||heap[0].alive)ExitProcess(12);preexisting=0;pathMode=0;
for(failAllocation=0;failAllocation<3;failAllocation++){Check();if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||blocks!=(failAllocation?3:1)||heap[0].alive!=(failAllocation!=0)||(failAllocation&&((CameraAccess*)camera)->frameCount!=1))ExitProcess(8);}failAllocation=-1;
for(throwAllocation=0;throwAllocation<3;throwAllocation++){Check();if(got.complete||got.exception!=77||blocks!=throwAllocation+1||heap[0].alive!=(throwAllocation!=0))ExitProcess(9);}throwAllocation=-1;
// Progressive allocation guards: constructor, matrices and parsed scalar.
for(guardAllocation=0;guardAllocation<3;guardAllocation++){
 int maximum=guardAllocation==0?0x134:guardAllocation==1?64:4;
 for(guardBytes=0;guardBytes<=maximum;guardBytes+=4){
  Check();GuardOracle();
  if(guardAllocation==0&&guardBytes<maximum){if(blocks!=1||heap[0].alive||diagnosticDeletes!=1||memcmp(camera,initialCamera,sizeof(camera))){printf("Constructor unwind alloc%d bytes%d alive%d deletes%d\n",guardAllocation,guardBytes,heap[0].alive,diagnosticDeletes);ExitProcess(27);}}
  if(guardAllocation!=0&&guardBytes<maximum&&(!heap[0].alive||diagnosticDeletes)){ExitProcess(28);}
 }
}
guardAllocation=-1;
// All records remain fully initialized. Test plain text line boundaries and
// skipped lines, including a complete record with a long ignored suffix.
lineCount=4;expectedFrameCount=2;
strcpy(lines[0],"# skipped camera comment");strcpy(lines[1],"cAmErA C 0 0 0 0 1 0 0 0 1 1.25 -3.5 2.75 4.5");strcpy(lines[2],"");strcpy(lines[3],"CAMERA C 0 0 0 0 1 0 0 0 1 1.25 -3.5 2.75 4.5");
for(eolMode=0;eolMode<3;eolMode++){Check();Success();Ownership();}
for(int padding=0;padding<=520;padding+=13){int len=strlen(lines[3]);while(len>0&&lines[3][len-1]==' ')lines[3][--len]=0;memset(lines[3]+len,' ',padding);lines[3][len+padding]=0;eolMode=padding%3;Check();Success();Ownership();}
eolMode=0;expectedFrameCount=-1;lineCount=1;strcpy(lines[0],"CAMERA C 0 0 0 0 1 0 0 0 1 1.25 -3.5 2.75 4.5");
for(allocationMutation=0;allocationMutation<3;allocationMutation++){
 Check();Success();if(mutationReached!=1||eventCount!=4||heap[0].alive)ExitProcess(30);
 CameraAccess*c=(CameraAccess*)camera;
 if(allocationMutation==0&&(c->frameCount!=1||heap[1].size!=64||heap[2].size!=4))ExitProcess(31);
 if(allocationMutation==1&&(c->frameCount!=3||heap[1].size!=64||heap[2].size!=12))ExitProcess(32);
 if(allocationMutation==2&&(c->frames!=(Matrix4*)previousFrames||*(float*)previousFrames!=1.0f||*(float*)(previousFrames+60)!=1.0f||heap[1].data[0]!=0xa5))ExitProcess(33);
}allocationMutation=-1;
printf("CameraLoad: %d real-stream/math cases match\n",checks);return 0;}
