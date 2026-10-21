#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "../../src/reconstructed/GR_BitString.h"
extern unsigned int* g_LowBitMaskTable;extern unsigned int g_LowBitMasks[33];extern unsigned int g_BitReaderWord;extern int g_BitReaderBitsUsed;extern unsigned int* g_BitReaderCursor;
static BYTE referenceSpace[0x800000]={1};
struct World{BYTE bytes[256];BYTE objects[2][32];};
struct State{DWORD word;int used;DWORD cursor,mask;};
struct Result{DWORD value;int complete;DWORD exception,kind,address;State state;World world;BYTE page[4096];};
static World world;static Result got,want;static BYTE* guard;static int checks,seed,used,count,alignment,alias,invalid,guardBytes,thisKind;
static DWORD& Word(int r){return r?*(DWORD*)0x578e90:*(DWORD*)&g_BitReaderWord;}
static int& Used(int r){return r?*(int*)0x578e94:g_BitReaderBitsUsed;}
static DWORD*& Cursor(int r){return r?*(DWORD**)0x578e98:*(DWORD**)&g_BitReaderCursor;}
static DWORD*& Mask(int r){return r?*(DWORD**)0x568224:*(DWORD**)&g_LowBitMaskTable;}
static DWORD* Masks(int r){return r?(DWORD*)0x5681a0:(DWORD*)g_LowBitMasks;}
static DWORD Big(BYTE* p){return ((DWORD)p[0]<<24)|((DWORD)p[1]<<16)|((DWORD)p[2]<<8)|p[3];}
static DWORD Normalize(int r,void* p){DWORD n=(DWORD)p;DWORD locations[3]={r?0x578e90:(DWORD)&g_BitReaderWord,r?0x578e94:(DWORD)&g_BitReaderBitsUsed,(DWORD)Masks(r)};DWORD spans[3]={4,4,132};for(int i=0;i<3;++i)if(n-locations[i]<spans[i])return 0xf0000000+i*0x100+n-locations[i];return n;}
static void Snapshot(int r){got.state.word=Word(r);got.state.used=Used(r);got.state.cursor=(alias>=3&&alias<=5&&used+count<32)?Normalize(r,Cursor(r)+1)-4:Normalize(r,Cursor(r));got.state.mask=(alias==1||alias==2)?Normalize(r,Mask(r)+count)-count*4:Normalize(r,Mask(r));got.world=world;memcpy(got.page,guard,4096);}
static int Filter(EXCEPTION_POINTERS*e){if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;got.exception=e->ExceptionRecord->ExceptionCode;got.kind=e->ExceptionRecord->ExceptionInformation[0];got.address=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
static void Invoke(int r){__try{union{unsigned int(GR_BitString::*p)(int);DWORD a;}u;u.p=&GR_BitString::ReadBits;if(sizeof(u.p)!=4)ExitProcess(82);if(r)u.a=0x423ef0;void* self=thisKind==1?0:thisKind==2?(void*)1:world.objects[0];got.value=(((GR_BitString*)self)->*u.p)(count);got.complete=1;}__except(Filter(GetExceptionInformation())){}}
static DWORD OriginalWord(){return 0x9e3779b9u*(seed+1)^0x12345678u;}
static void Prepare(int r){memset(&got,0,sizeof(got));memset(&world,0x55,sizeof(world));memset(guard,0xa9,4096);for(int i=0;i<256;++i)world.bytes[i]=(BYTE)((seed*19+i*13)^(i>>2));Word(r)=OriginalWord();Used(r)=used;Cursor(r)=(DWORD*)(world.bytes+alignment);Mask(r)=Masks(r);
 if(alias==1)Mask(r)=&Word(r)-count;if(alias==2)Mask(r)=(DWORD*)&Used(r)-count;
 if(alias==3)Cursor(r)=(DWORD*)&Used(r)-1;if(alias==4)Cursor(r)=&Word(r)-1;if(alias==5)Cursor(r)=Masks(r)+seed%32-1;
 if(invalid==1)Mask(r)=(DWORD*)1;if(invalid==2)Cursor(r)=(DWORD*)1;if(invalid==3)Cursor(r)=(DWORD*)(guard+4096-guardBytes-4);if(invalid==4)Mask(r)=(DWORD*)(guard+4096-guardBytes-count*4);
}
static void Compare(){Prepare(1);Invoke(1);Snapshot(1);want=got;Prepare(0);Invoke(0);Snapshot(0);++checks;if(memcmp(&got,&want,sizeof(got))){int at=0;while(((BYTE*)&got)[at]==((BYTE*)&want)[at])++at;printf("Mismatch%d off%d seed%d used%d count%d align%d alias%d invalid%d guard%d value%x/%x complete%d/%d state%x/%x %d/%d fault%x/%x\n",checks,at,seed,used,count,alignment,alias,invalid,guardBytes,want.value,got.value,want.complete,got.complete,want.state.word,got.state.word,want.state.used,got.state.used,want.address,got.address);fflush(stdout);ExitProcess(2);}}
static void Oracle(){DWORD result=0;DWORD old=OriginalWord(),next=Big(world.bytes+alignment+4);for(int i=0;i<count;++i){int bit=used+i;DWORD w=bit<32?old:next;result=(result<<1)|((w>>(31-(bit&31)))&1);}
 if(!got.complete||got.exception||got.value!=result||got.state.used!=(used+count)%32||got.state.word!=((used+count>=32)?next:old)||Cursor(0)!=(DWORD*)(world.bytes+alignment+(used+count>=32?4:0))){printf("Oracle%d\n",checks);ExitProcess(3);}}
static int MapReference(const char* path){BYTE* mapped=(BYTE*)0x400000;DWORD old;if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
 FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
 DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
 for(int i=0;i<pe->FileHeader.NumberOfSections;++i){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);}

static Result sequence[24];static DWORD sourceVptr;
static void Construct(int r,int which,int flag){void* object=world.objects[which];BYTE* input=world.bytes+alignment+which*64;if(r){typedef void*(__fastcall*Constructor)(void*,void*,const void*,int,int);((Constructor)0x423d50)(object,0,input,512,flag);if(*(DWORD*)object!=0x550f50)ExitProcess(87);}else{new(object) GR_BitString(input,512,flag);if(sourceVptr&&*(DWORD*)object!=sourceVptr)ExitProcess(88);sourceVptr=*(DWORD*)object;}*(DWORD*)object=0x550f50;}
static void Chain(int flag){for(int r=1;r>=0;--r){Prepare(r);Construct(r,0,flag);int bit=0;BYTE* input=world.bytes+alignment;for(int step=0;step<24;++step){if(step==12){Construct(r,1,flag);bit=0;input=world.bytes+alignment+64;}count=(step*13+seed)%33;memset(&got,0,sizeof(got));Invoke(r);Snapshot(r);DWORD expected=0;for(int b=0;b<count;++b)expected=(expected<<1)|((input[(bit+b)/8]>>(7-((bit+b)&7)))&1);bit+=count;if(!got.complete||got.exception||got.value!=expected||Used(r)!=(bit&31)||Cursor(r)!=(DWORD*)(input+(bit/32)*4)||Word(r)!=Big(input+(bit/32)*4))ExitProcess(89);if(r)sequence[step]=got;else{++checks;if(memcmp(&got,sequence+step,sizeof(got))){printf("Chain mismatch seed%d align%d flag%d step%d\n",seed,alignment,flag,step);ExitProcess(90);}}}}}
int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 80;guard=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;if(!guard||!VirtualProtect(guard+4096,4096,PAGE_NOACCESS,&old))return 81;
 for(seed=0;seed<32;++seed)for(used=0;used<32;++used)for(count=0;count<=32;++count)for(alignment=0;alignment<4;++alignment){Compare();Oracle();}
 alignment=0;for(alias=1;alias<=5;++alias)for(seed=0;seed<8;++seed)for(used=0;used<32;++used)for(count=0;count<=32;++count){Compare();if(!got.complete||got.exception)return 85;}alias=0;
 for(invalid=1;invalid<=4;++invalid)for(guardBytes=0;guardBytes<4;++guardBytes)for(used=0;used<32;++used)for(count=0;count<=32;++count){Compare();if((invalid==1||invalid==4)&&(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.kind!=0))return 83;if((invalid==2||invalid==3)&&used+count<32&&(!got.complete||got.exception))return 86;if((invalid==2||invalid==3)&&used+count>=32){if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.state.word!=OriginalWord()||got.state.used!=used+count-32)return 84;}}
 invalid=0;guardBytes=0;for(thisKind=1;thisKind<=2;++thisKind)for(used=0;used<32;++used)for(count=0;count<=32;++count){Compare();Oracle();}
 thisKind=0;used=0;for(seed=0;seed<32;++seed)for(alignment=0;alignment<4;++alignment)for(int flag=-1;flag<=1;++flag)Chain(flag);
 printf("BitStringRead: %d scenarios PASS\n",checks);return 0;}
