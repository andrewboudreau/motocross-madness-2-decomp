// Validation only: complete standalone source and private pinned retail shifts.
// Prepared raw receivers avoid unrelated constructors; no production retail mapping.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/GR_BitString.h"
struct View {DWORD vptr;int bits;BYTE* data;DWORD cursor;DWORD mask;int used;int fill;int borrowed;};
typedef char Layout[sizeof(View)==32&&sizeof(GR_BitString)==32&&offsetof(View,fill)==24?1:-1];
static BYTE referenceSpace[0x800000]={1};
static BYTE initial[1024],storage[1024];static BYTE* page;
struct Result {BYTE storage[1024],page[4096];DWORD exception,kind,address;};
static Result observed,expected;static int checks;static BYTE oracle[256];
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

static int Filter(EXCEPTION_POINTERS* p){if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;observed.exception=p->ExceptionRecord->ExceptionCode;observed.kind=p->ExceptionRecord->ExceptionInformation[0];observed.address=p->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;}
typedef void(__fastcall* Shift)(View*,void*,int);
static void Run(int retail,int direction,int count,int bits,int fill,int offset,int mode,int wrapper){
 memset(&observed,0,sizeof(observed));memcpy(storage,initial,sizeof(storage));for(int i=0;i<4096;i++)page[i]=(BYTE)(i*19+31);
 View* v=(View*)(storage+64);v->vptr=0x550f50;v->bits=bits;v->data=storage+256+offset;v->fill=fill;v->borrowed=1;
 if(mode==1)v->data=0;
 if(mode>=2&&mode<=6)v->data=page+4096-(mode-2)*4;
 if(mode==7)v->data=(BYTE*)v;
 if(mode==8)v->data=(BYTE*)&v->fill;
 if(mode==9)v=0;
 if(mode==10)v=(View*)(page+4096-8);
 if(mode==11)v->data=page-4;
 __try {
  if(retail){DWORD target=wrapper==2?0x423e90:wrapper?(direction?0x423940:0x4238c0):(direction?0x423b70:0x4239c0);((Shift)target)(v,0,wrapper==2&&!direction?-count:count);}
  else {GR_BitString* p=(GR_BitString*)v;if(wrapper==2)p->GR_BitString::UnknownVirtualSlot2(direction?count:-count);else if(wrapper){if(direction)p->ShiftTowardBack(count);else p->ShiftTowardFront(count);}else{if(direction)p->ShiftBitsTowardBack(count);else p->ShiftBitsTowardFront(count);}}
 }__except(Filter(GetExceptionInformation())){}
 memcpy(observed.storage,storage,sizeof(storage));memcpy(observed.page,page,4096);
}
static void Check(int dir,int count,int bits,int fill,int offset,int mode,int wrapper){Run(1,dir,count,bits,fill,offset,mode,wrapper);expected=observed;Run(0,dir,count,bits,fill,offset,mode,wrapper);if(memcmp(&observed,&expected,sizeof(observed))){int p=0;while(p<sizeof(observed)&&((BYTE*)&observed)[p]==((BYTE*)&expected)[p])p++;printf("FAIL case%d dir%d count%d bits%d fill%x offset%d mode%d wrapper%d byte%d exception%x/%x address%x/%x\n",checks,dir,count,bits,fill,offset,mode,wrapper,p,observed.exception,expected.exception,observed.address,expected.address);exit(1);}checks++;}
static void Seed(unsigned seed){DWORD random=seed+1;for(int i=0;i<sizeof(initial);i++){random=random*1664525+1013904223;initial[i]=seed==0?0:seed==1?255:seed==2?(i&1?255:0):(BYTE)(random>>24);}}
static void Oracle(int dir,int count,int words,int fill,int offset){
 const BYTE* input=initial+256+offset;memset(oracle,0,sizeof(oracle));int n=words*32;
 for(int bit=0;bit<n;bit++){int from=dir?bit-count:bit+count;int one=from<0||from>=n?fill!=0:(input[from/8]>>(7-from%8))&1;if(one)oracle[bit/8]|=128>>(bit%8);}
 if(observed.exception||memcmp(observed.storage+256+offset,oracle,words*4)){printf("ORACLE FAIL dir%d count%d words%d fill%x\n",dir,count,words,fill);exit(2);}
}
int main(int argc,char**argv){if(argc!=2||!MapReference(argv[1]))return 3;page=(BYTE*)VirtualAlloc(0,12288,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;if(!page||!VirtualProtect(page,4096,PAGE_NOACCESS,&old))return 4;page+=4096;if(!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 4;
 const int fills[]={0,-1,1,0x12345678};const int bitCounts[]={1,7,31,32,33,63,64,65,95,127,128,255,256};
 for(int dir=0;dir<2;dir++)for(int seed=0;seed<32;seed++){Seed(seed);for(int b=0;b<13;b++)for(int f=0;f<4;f++)for(int offset=0;offset<4;offset++)for(int count=1;count<32;count++){Check(dir,count,bitCounts[b],fills[f],offset,0,0);Oracle(dir,count,(bitCounts[b]+31)/32,fills[f],offset);}}
 const int counts[]={0,1,7,31,32,33,63,64,65,95,128,257};
 for(int wrapper=1;wrapper<=2;wrapper++)for(dir=0;dir<2;dir++)for(int seed=0;seed<8;seed++){Seed(seed);for(int b=0;b<13;b++)for(int f=0;f<2;f++)for(int c=0;c<12;c++){Check(dir,counts[c],bitCounts[b],fills[f],0,0,wrapper);Oracle(dir,counts[c],(bitCounts[b]+31)/32,fills[f],0);}}
 for(dir=0;dir<2;dir++)for(int mode=1;mode<=11;mode++)for(int count=1;count<=31;count+=15)for(int f=0;f<2;f++){Seed(7);Check(dir,count,128,fills[f],0,mode,0);}
 // Empty and null data virtual dispatch are early-outs, not direct-helper claims.
 for(dir=0;dir<2;dir++)for(int c=0;c<12;c++){Seed(11);Check(dir,counts[c],0,0,0,0,2);if(observed.exception)return 5;Check(dir,counts[c],128,0,0,1,2);if(observed.exception)return 6;}
 // Independent partial-write oracle: front processes word0 before faulting on word1.
 Seed(3);Check(0,1,64,0,0,3,0);BYTE first[4];for(int q=0;q<4;q++)first[q]=(BYTE)((4092+q)*19+31);DWORD value=((DWORD)first[0]<<24)|((DWORD)first[1]<<16)|((DWORD)first[2]<<8)|first[3];value<<=1;
 if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address!=(DWORD)(page+4096)||observed.page[4092]!=(BYTE)(value>>24)||observed.page[4093]!=(BYTE)(value>>16)||observed.page[4094]!=(BYTE)(value>>8)||observed.page[4095]!=(BYTE)value)return 7;
 // The back shift writes the last readable word before reaching the lower guard.
 Seed(3);Check(1,1,128,0,0,11,0);if(observed.exception!=EXCEPTION_ACCESS_VIOLATION||observed.kind||observed.address!=(DWORD)(page-4))return 8;
 for(q=0;q<4;q++)first[q]=(BYTE)((8+q)*19+31);value=((DWORD)first[0]<<24)|((DWORD)first[1]<<16)|((DWORD)first[2]<<8)|first[3];value>>=1;
 // Its carry from the prior word is already ORed into the high bit.
 if(((BYTE)(7*19+31))&1)value|=0x80000000;
 if(observed.page[8]!=(BYTE)(value>>24)||observed.page[9]!=(BYTE)(value>>16)||observed.page[10]!=(BYTE)(value>>8)||observed.page[11]!=(BYTE)value)return 9;
 printf("BitStringShifts: %d complete-body, independent bit-oracle, wrapper, alias and fault cases match\n",checks);return 0;
}
