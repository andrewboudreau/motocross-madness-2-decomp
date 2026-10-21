// Native validation: actual QuadTree/BlockAllocator bodies, finite debug heap.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../../src/krusty2/broadphase/Quadtree.h"
void* operator new(unsigned int,void* p){return p;}
static BYTE referenceSpace[0x800000]={1};
static BYTE* arena;
enum{ARENA=65536};
struct Allocation{DWORD pointer;unsigned size;};
struct Event{int kind,line;DWORD pointer;unsigned size;DWORD root,current;int count,nodes;unsigned short stamp;BYTE pool[40];};
struct World{BYTE tree[0x874],object[32],other[32],pool[40];Allocation allocations[512];int count,used;};
struct Result{World world;BYTE arena[ARENA];Event events[512];int calls,complete;DWORD exception,kind,address,current;unsigned status,control;};
struct Scenario{unsigned code,control;int repeat,move,failAt,throwAt,fault,count;float lo,hi;};
static World w;static Result got,want;static Scenario sc;static unsigned checks;static DWORD vtable[2];
static DWORD& At(void*p,int n){return *(DWORD*)((BYTE*)p+n);}
static QuadTree* Tree(){return(QuadTree*)w.tree;}
static void Require(int yes,int line){if(!yes){printf("oracle line%d case%u code%x rep%d move%d fail%d throw%d fault%d\n",line,checks,sc.code,sc.repeat,sc.move,sc.failAt,sc.throwAt,sc.fault);ExitProcess(3);}}
#define CHECK(x) Require(!!(x),__LINE__)
template<class T>static DWORD Address(T p){CHECK(sizeof(p)==4);union{T p;DWORD n;}u;u.p=p;return u.n;}
class FactoryProbe{public:QuadTreeNode* Factory(){return((QuadTree*)this)->QuadTree::UnknownVirtualSlot1();}};
static Event& Boundary(int kind,void*p,unsigned size,const char*file,int line){
 CHECK(got.calls<512);Event&e=got.events[got.calls++];e.kind=kind;e.pointer=(DWORD)p;e.size=size;e.line=line;e.root=(DWORD)Tree()->rootNode;e.current=(DWORD)g_pQuadTree;e.count=Tree()->objectCount;e.nodes=Tree()->liveNodeCount;e.stamp=*(unsigned short*)(w.object+4);memcpy(e.pool,w.pool,40);
 const char*leaf=strrchr(file,'\\');if(!leaf)leaf=strrchr(file,'/');leaf=leaf?leaf+1:file;
 CHECK(!_stricmp(leaf,kind==1?"BlockAllocator.cpp":"Quadtree.cpp"));CHECK(line==(kind==1?132:kind==2?0x690:0x7a));
 if(sc.throwAt==got.calls)RaiseException(0xe0501234,0,0,0);return e;
}
static void* Allocate(unsigned size){CHECK(w.count<512);unsigned span=(size+127)&~127u;CHECK(w.used+span<=ARENA);void*p=arena+w.used;w.used+=span;w.allocations[w.count].pointer=(DWORD)p;w.allocations[w.count++].size=size;return p;}
void* operator new(unsigned size,const char*file,int line){Boundary(0,0,size,file,line);CHECK(size==20);if(sc.failAt==got.calls)return 0;return Allocate(size);}
void operator delete(void*,const char*,int){ExitProcess(86);}
void* DebugMalloc(unsigned size,const char*file,int line){Boundary(1,0,size,file,line);CHECK(size==104);if(sc.failAt==got.calls)return 0;return Allocate(size);}
void DebugFree(void*,const char*,int){ExitProcess(87);}
void* DebugRealloc(void*p,unsigned size,const char*file,int line){Boundary(2,p,size,file,line);CHECK(size==36);if(sc.failAt==got.calls)return 0;int i;for(i=0;i<w.count;i++)if(w.allocations[i].pointer==(DWORD)p)break;CHECK(i<w.count&&w.allocations[i].size==20);if(!sc.move){w.allocations[i].size=size;return p;}void*q=Allocate(size);memcpy(q,p,20);memset(p,0xd3,20);return q;}
static int MapReference(const char* path){BYTE*mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE*f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE*raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER*dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER*section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;i++){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return 1;}
static int PatchCall(DWORD at,DWORD old,DWORD target){BYTE*p=(BYTE*)at;if(*p!=0xe8||at+5+*(DWORD*)(p+1)!=old)return 0;*(DWORD*)(p+1)=target-at-5;return 1;}
static int Patch(){if(*(WORD*)0x4dcaea!=0x0d89||*(DWORD*)0x4dcaec!=0x689b78)return 0;*(DWORD*)0x4dcaec=(DWORD)&g_pQuadTree;return PatchCall(0x4dccbb,0x4dd130,Address(&QuadTreeNode::AddObject))&&PatchCall(0x4dcd05,0x4ddce0,Address(&QuadTreeNode::Grow))&&PatchCall(0x4dcd11,0x4ddce0,Address(&QuadTreeNode::Grow))&&FlushInstructionCache(GetCurrentProcess(),0,0);}
static void Defaults(){memset(&sc,0,sizeof(sc));sc.code=0x10;sc.repeat=1;sc.lo=-3;sc.hi=7;sc.control=_MCW_EM|_PC_64|_RC_NEAR;}
static void Setup(){memset(&w,0,sizeof(w));memset(&got,0,sizeof(got));memset(arena,0xa7,ARENA);At(w.tree,0)=(DWORD)vtable;Tree()->maxDepth=11;for(int i=0;i<12;i++)Tree()->cellSizes[i]=0x4000>>i;g_pQuadTree=Tree();
 Tree()->itemPool=new((void*)w.pool)BlockAllocator(20,104);void*p=Allocate(20);Tree()->rootNode=new(p)QuadTreeNode;Tree()->objectCount=sc.count;*(unsigned short*)(w.object+4)=0x7183;memset(w.other,0x39,32);got.calls=0;
 if(sc.fault==1)Tree()->rootNode=0;if(sc.fault==2)Tree()->rootNode->children=(QuadTreeNode**)1;if(sc.fault==3)Tree()->itemPool=(BlockAllocator*)1;
 if(sc.fault==7)Tree()->rootNode=(QuadTreeNode*)(arena+ARENA-12);if(sc.fault==8)Tree()->rootNode->children=(QuadTreeNode**)(arena+ARENA);
 g_pQuadTree=(QuadTree*)w.other;
 _clearfp();_controlfp(sc.control,_MCW_EM|_MCW_PC|_MCW_RC);
}
static int Filter(EXCEPTION_POINTERS*p){DWORD c=p->ExceptionRecord->ExceptionCode;if(c!=EXCEPTION_ACCESS_VIOLATION&&c!=0xe0501234)return EXCEPTION_CONTINUE_SEARCH;got.exception=c;if(c==EXCEPTION_ACCESS_VIOLATION){got.kind=p->ExceptionRecord->ExceptionInformation[0];got.address=p->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Run(int retail){Setup();QuadTree*t=sc.fault==5?(QuadTree*)1:Tree();QuadTreeObject*o=sc.fault==4?(QuadTreeObject*)1:sc.fault==6?(QuadTreeObject*)(arena+ARENA-4):(QuadTreeObject*)w.object;
 __try{for(int i=0;i<sc.repeat;i++){if(retail){union{void(QuadTree::*p)(QuadTreeObject*,unsigned,float,float);DWORD n;}u;CHECK(sizeof(u.p)==4);u.n=0x4dcac0;(t->*u.p)(o,sc.code,sc.lo,sc.hi);}else t->Insert(o,sc.code,sc.lo,sc.hi);}got.complete=1;}__except(Filter(GetExceptionInformation())){}
 got.status=_statusfp();got.control=_controlfp(0,0);_clearfp();_controlfp(_MCW_EM|_PC_64|_RC_NEAR,_MCW_EM|_MCW_PC|_MCW_RC);got.current=(DWORD)g_pQuadTree;got.world=w;memcpy(got.arena,arena,ARENA);
}
static int visited;
static int CountObjects(QuadTreeNode*n){CHECK(++visited<512);int count=0;if(n->children)for(int i=0;i<4;i++)if(n->children[i])count+=CountObjects(n->children[i]);for(QuadTreeItemLink*l=n->itemList;l;l=l->next)for(int j=0;j<4;j++)if(l->objects[j]){CHECK(l->objects[j]==(QuadTreeObject*)w.object);count++;}
 if((sc.code&0xf0)&&sc.lo<sc.hi){CHECK(n->yCenter==(sc.hi+sc.lo)*0.5f&&n->yHalfExtent==(sc.hi-sc.lo)*0.5f);}else CHECK(n->yCenter==0&&*(DWORD*)&n->yHalfExtent==0xff7fffff);return count;}
static void Topology(){CHECK(got.complete&&!got.exception);if(sc.code==15){CHECK(Tree()->objectCount==sc.count&&*(unsigned short*)(w.object+4)==0x7183&&!got.calls&&g_pQuadTree==(QuadTree*)w.other);return;}
 CHECK((unsigned)Tree()->objectCount==(unsigned)sc.count+sc.repeat&&*(unsigned short*)(w.object+4)==0&&g_pQuadTree==Tree());int flags=(sc.code>>4)&15,total=0;for(int bit=0;bit<4;bit++)if(flags&(1<<bit))total++;visited=0;CHECK(CountObjects(Tree()->rootNode)==total*sc.repeat);CHECK(visited==Tree()->liveNodeCount);
 for(int cell=0;cell<4;cell++)if(flags&(1<<cell)){unsigned x=(sc.code>>17)&0x7ff8,z=(sc.code>>5)&0x7ff8;int depth=sc.code&15,span=0x4000>>depth;x+=(cell&2)?span:0;z+=(cell&1)?span:0;QuadTreeNode*n=Tree()->rootNode;
  for(int level=0;level<=depth;level++){unsigned mask=0x4000>>level;int index=((x&mask)?2:0)+((z&mask)?1:0);CHECK(n->children&&n->children[index]);n=n->children[index];}
  int count=0;for(QuadTreeItemLink*l=n->itemList;l;l=l->next)for(int j=0;j<4;j++)if(l->objects[j]==(QuadTreeObject*)w.object)count++;CHECK(count>=sc.repeat);
 }
}
static void Check(int oracle){Run(1);want=got;Run(0);checks++;if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])at++;printf("DIFF case%u byte%d code%x rep%d move%d fail%d throw%d fault%d events%d/%d AV%x/%x\n",checks,at,sc.code,sc.repeat,sc.move,sc.failAt,sc.throwAt,sc.fault,want.calls,got.calls,want.address,got.address);ExitProcess(2);}if(oracle)Topology();}
static void FailedAllocation(int call,int throwing){CHECK(!got.complete&&got.calls==call);CHECK(Tree()->objectCount==sc.count+1&&*(unsigned short*)(w.object+4)==0&&g_pQuadTree==Tree());int nodes=1;for(int i=0;i<call-1;i++)if(got.events[i].kind==0)nodes++;CHECK(Tree()->liveNodeCount==nodes);if(throwing)CHECK(got.exception==0xe0501234);else{CHECK(got.exception==EXCEPTION_ACCESS_VIOLATION);int k=got.events[call-1].kind;CHECK(got.address==(DWORD)(k==0?12:k==1?0:4));CHECK(got.kind==(DWORD)(k==0?0:1));}}
int main(int argc,char**argv){setvbuf(stdout,0,_IONBF,0);if(argc!=2)return 4;arena=(BYTE*)VirtualAlloc(0,ARENA+4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!arena||!VirtualProtect(arena+ARENA,4096,PAGE_NOACCESS,&old)||!MapReference(argv[1])||!Patch())return 4;vtable[1]=Address(&FactoryProbe::Factory);
 unsigned coords[]={0,8,0x1ff8,0x3ff8,0x4000,0x5ff8,0x7ff8};int reps[]={1,2,5,9};
 Defaults();for(int depth=0;depth<12;depth++)for(int flags=0;flags<16;flags++)for(int pos=0;pos<7;pos++)for(sc.move=0;sc.move<2;sc.move++)for(int rep=0;rep<4;rep++){sc.code=(coords[pos]<<17)|(coords[6-pos]<<5)|(flags<<4)|depth;sc.repeat=reps[rep];Check(1);}
 Defaults();sc.code=15;for(sc.fault=0;sc.fault<=8;sc.fault++){Check(0);CHECK(got.complete&&!got.exception&&!got.calls&&g_pQuadTree==(QuadTree*)w.other&&Tree()->objectCount==0&&*(unsigned short*)(w.object+4)==0x7183);}
 Defaults();sc.code=(0x1ff8u<<17)|(0x5ff8u<<5)|0xf7;sc.repeat=1;for(sc.move=0;sc.move<2;sc.move++){Check(1);int calls=got.calls;for(sc.failAt=1;sc.failAt<=calls;sc.failAt++){Check(0);FailedAllocation(sc.failAt,0);}sc.failAt=0;for(sc.throwAt=1;sc.throwAt<=calls;sc.throwAt++){Check(0);FailedAllocation(sc.throwAt,1);}sc.throwAt=0;}
 Defaults();for(sc.fault=1;sc.fault<=8;sc.fault++){Check(0);CHECK(!got.complete&&got.exception==EXCEPTION_ACCESS_VIOLATION);DWORD addresses[]={0,12,1,0x11,5,0x85d,(DWORD)(arena+ARENA),(DWORD)(arena+ARENA),(DWORD)(arena+ARENA)};CHECK(got.address==addresses[sc.fault]&&got.kind==(DWORD)(sc.fault==4||sc.fault==6));CHECK(got.calls==(sc.fault==3?2:0));CHECK(Tree()->objectCount==(sc.fault==5?0:1));CHECK(*(unsigned short*)(w.object+4)==(sc.fault>=4&&sc.fault<=6?0x7183:0));CHECK(g_pQuadTree==(sc.fault==5?(QuadTree*)w.other:Tree()));}
 Defaults();sc.code=0x10;int counts[]={0,-1,0x7fffffff,(int)0x80000000};for(int c=0;c<4;c++){sc.count=counts[c];Check(1);}
 float ys[]={-100,-3,0,7,100};unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};Defaults();sc.code=0xf3;for(int a=0;a<5;a++)for(int b=0;b<5;b++)for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++){sc.lo=ys[a];sc.hi=ys[b];sc.control=_MCW_EM|pcs[pc]|rcs[rc];Check(1);}
 unsigned rng=0x50abcd;Defaults();sc.code=0xf2;for(int n=0;n<2048;n++){rng=rng*1664525+1013904223;*(unsigned*)&sc.lo=rng;rng=rng*1664525+1013904223;*(unsigned*)&sc.hi=rng;sc.control=_MCW_EM|pcs[n%3]|rcs[n%4];Check(0);CHECK(got.complete&&!got.exception&&Tree()->objectCount==1&&*(unsigned short*)(w.object+4)==0&&g_pQuadTree==Tree());}
 printf("QuadTree Insert: %u complete-body scenarios match\n",checks);return 0;}
