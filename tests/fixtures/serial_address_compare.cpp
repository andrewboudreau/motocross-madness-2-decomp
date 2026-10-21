// Validation only: real complete converter and unmodified retail helper closure.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "NetProcs.h"
#include "DlgProcs.h"
static BYTE referenceSpace[0x800000]={1};
struct Result {BYTE world[256],page[4096];DWORD exception,kind,address;};
static BYTE world[256],initial[256],*pages;
static Result observed,expected;
static int sourceOffset=64,destinationOffset=128,role,cut,checks;
static DWORD input[5];
static int Filter(EXCEPTION_POINTERS* p){
 if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 observed.exception=p->ExceptionRecord->ExceptionCode;
 observed.kind=p->ExceptionRecord->ExceptionInformation[0];
 observed.address=p->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Setup(DWORD port,DWORD baud,DWORD stop,DWORD parity,DWORD flow,int delta=64,int fault=0,int boundary=0){
 memset(initial,0x59,sizeof(initial));input[0]=port;input[1]=baud;input[2]=stop;input[3]=parity;input[4]=flow;
 memcpy(initial+64,input,20);destinationOffset=64+delta;role=fault;cut=boundary;
}
static void Run(int retail){
 DWORD old;if(!VirtualProtect(pages,8192,PAGE_READWRITE,&old))ExitProcess(20);
 memset(pages,0x6b,8192);memcpy(world,initial,256);memset(&observed,0,sizeof(observed));
 SerialSettings* source=(SerialSettings*)(world+sourceOffset);
 NetComPortAddress* target=(NetComPortAddress*)(world+destinationOffset);
 if(role==1){source=(SerialSettings*)(pages+4096-cut);memcpy(source,input,20);}
 if(role==2)target=(NetComPortAddress*)(pages+4096-cut);
 if(role==3)source=0;if(role==4)target=0;
 if(!VirtualProtect(pages+4096,4096,PAGE_NOACCESS,&old))ExitProcess(21);
 __try{if(retail)((void(__cdecl*)(SerialSettings*,NetComPortAddress*))0x4ae410)(source,target);
 else BuildComPortAddress(source,target);}__except(Filter(GetExceptionInformation())){}
 memcpy(observed.world,world,256);memcpy(observed.page,pages,4096);
}
static DWORD Convert(int field,DWORD value){
 static const DWORD baud[]={4800,9600,14400,19200,38400,56000,57600,115200,128000,256000};
 static const DWORD parity[]={0,2,1,3};
 if(field==0)return value+1u;
 if(field==1)return value<10?baud[value]:0;
 if(field==2)return value<3?value:0;
 if(field==3)return value<4?parity[value]:0;
 return value<5?value:0;
}
static void Check(){
 Run(1);expected=observed;Run(0);
 if(memcmp(&observed,&expected,sizeof(observed))){printf("FAIL case%d role%d cut%d delta%d\n",checks,role,cut,destinationOffset-64);exit(1);}
 if(!role){
  // Independent bytewise oracle keeps each read immediately before its write,
  // including source/destination overlap and unaligned x86 record addresses.
  BYTE oracle[256];memcpy(oracle,initial,256);
  for(int field=0;field<5;field++){DWORD value;memcpy(&value,oracle+64+field*4,4);value=Convert(field,value);memcpy(oracle+destinationOffset+field*4,&value,4);}
  if(observed.exception||memcmp(observed.world,oracle,256))ExitProcess(22);
 }
 checks++;
}
static int MapReference(const char* path);
int main(int argc,char**argv){
 if(argc!=2||!MapReference(argv[1]))return 2;
 pages=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!pages)return 3;
 if(sizeof(SerialSettings)!=20||sizeof(NetComPortAddress)!=20||offsetof(SerialSettings,flowControl)!=16||offsetof(NetComPortAddress,parity)!=12)return 4;
 for(int port=0;port<8;port++)for(int baud=0;baud<10;baud++)for(int stop=0;stop<3;stop++)for(int parity=0;parity<4;parity++)for(int flow=0;flow<5;flow++){Setup(port,baud,stop,parity,flow);Check();}
 const DWORD edges[]={0,1,2,3,4,5,8,9,10,11,0x7fffffff,0x80000000,0xfffffffe,0xffffffff};
 for(int delta=-24;delta<=24;delta++)for(int field=0;field<5;field++)for(int edge=0;edge<14;edge++){
  DWORD values[]={2,6,1,2,4};values[field]=edges[edge];Setup(values[0],values[1],values[2],values[3],values[4],delta);Check();
 }
 DWORD seed=0x12345678;
 for(int n=0;n<20000;n++){
  DWORD values[5];for(int i=0;i<5;i++){seed=1664525u*seed+1013904223u;values[i]=seed;}
  Setup(values[0],values[1],values[2],values[3],values[4],(n%3)?64:(n%49)-24);Check();
 }
 for(int fault=1;fault<=2;fault++)for(int boundary=0;boundary<=24;boundary++)for(int edge=0;edge<14;edge++){
  Setup(edges[edge],edge%10,edge%3,edge%4,edge%5,64,fault,boundary);Check();
 }
 Setup(2,6,1,2,4,64,3);Check();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address) return 5;
 Setup(2,6,1,2,4,64,4);Check();if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address)return 6;
 // A source fault at the third field leaves the first two output stores visible.
 Setup(2,6,1,2,4,64,1,8);Check();DWORD words[5];memcpy(words,observed.world+128,20);
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address!=(DWORD)(pages+4096)||words[0]!=3||words[1]!=57600||words[2]!=0x59595959)return 7;
 // An aligned third output store faults after two complete stores.
 Setup(2,6,1,2,4,64,2,8);Check();memcpy(words,observed.page+4088,8);
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind!=1||observed.address!=(DWORD)(pages+4096)||words[0]!=3||words[1]!=57600)return 8;
 printf("Serial address: %d complete-state, overlap and fault cases match\n",checks);return 0;
}

static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE* raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;
    if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++) {
        if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);
}
