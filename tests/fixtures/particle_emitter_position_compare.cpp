// Prepared-storage setter boundary: actual source body and untouched retail.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "ParticleEmitters.h"
typedef char Layout[(sizeof(ParticleVec3)==12 && sizeof(SparkParticleEmitter)==0x60c &&
 offsetof(SparkParticleEmitter,position)==0x44 && offsetof(SparkParticleEmitter,previousPosition)==0x50 &&
 sizeof(void(SparkParticleEmitter::*)(ParticleVec3))==4)?1:-1];
static BYTE referenceSpace[0x800000]={1};
static BYTE* page;
static BYTE initial[4096], expected[4096], observed[4096];
static DWORD wantedAccess,wantedAddress,gotAccess,gotAddress,gotException;
static int wantedFault,complete,checks,faults,readOnly;
static int Access(DWORD address,int write,DWORD* bits) {
 DWORD offset=address-(DWORD)page;
 if(offset>4096-4 || (write&&readOnly)) { wantedFault=1;wantedAccess=write;wantedAddress=address;return 0; }
 if(write)memcpy(expected+offset,bits,4);else memcpy(bits,expected+offset,4);
 return 1;
}
static void Model(SparkParticleEmitter* self,const DWORD* input) {
 DWORD bits;wantedFault=0;wantedAccess=wantedAddress=0;
 memcpy(expected,initial,4096);
 for(int i=0;i<3;i++)if(!Access((DWORD)self+0x44+i*4,0,&bits)||!Access((DWORD)self+0x50+i*4,1,&bits))return;
 for(i=0;i<3;i++){bits=input[i];if(!Access((DWORD)self+0x44+i*4,1,&bits))return;}
}
static int Filter(EXCEPTION_POINTERS* p) {
 if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 gotException=p->ExceptionRecord->ExceptionCode;gotAccess=p->ExceptionRecord->ExceptionInformation[0];gotAddress=p->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail,SparkParticleEmitter* self,ParticleVec3 input) {
 gotAccess=gotAddress=gotException=0;complete=0;
 __try {
  if(retail){union{void(SparkParticleEmitter::*method)(ParticleVec3);DWORD address;}call;call.address=0x4b9dc0;(self->*call.method)(input);}
  else self->SetPosition(input);
  complete=1;
 }__except(Filter(GetExceptionInformation())){}
}
static void Check(SparkParticleEmitter* self,const DWORD* input) {
 ParticleVec3 p;memcpy(&p,input,12);Model(self,input);DWORD old;
 for(int retail=0;retail<2;retail++) {
  if(!VirtualProtect(page,4096,PAGE_READWRITE,&old))ExitProcess(10);memcpy(page,initial,4096);
  if(readOnly&&!VirtualProtect(page,4096,PAGE_READONLY,&old))ExitProcess(11);
  Run(retail,self,p);memcpy(observed,page,4096);
  if(complete==wantedFault || !!gotException!=wantedFault || (wantedFault&&(gotAccess!=wantedAccess||gotAddress!=wantedAddress)) || memcmp(observed,expected,4096)) {
   printf("mismatch %d role%d fault%d/%d access%lu/%lu addr%08lx/%08lx\n",checks,retail,wantedFault,!!gotException,wantedAccess,gotAccess,wantedAddress,gotAddress);ExitProcess(3);
  }
 }
 checks++;faults+=wantedFault;
}
static int MapReference(const char* path) {
    BYTE* mapped = (BYTE*)0x400000;
    DWORD old;
    if ((DWORD)referenceSpace > 0x400000 ||
        (DWORD)referenceSpace + sizeof(referenceSpace) < 0x690000 ||
        !VirtualProtect(mapped, 0x290000, PAGE_EXECUTE_READWRITE, &old))
        return 0;
    FILE* file = fopen(path, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    if (length < (long)sizeof(IMAGE_DOS_HEADER)) return 0;
    BYTE* raw = (BYTE*)malloc(length);
    if (!raw || fread(raw, 1, length, file) != (size_t)length) return 0;
    fclose(file);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)raw;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 ||
        dos->e_lfanew > length - (long)sizeof(IMAGE_NT_HEADERS)) return 0;
    IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)(raw + dos->e_lfanew);
    if (pe->Signature != IMAGE_NT_SIGNATURE ||
        pe->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
        pe->OptionalHeader.ImageBase != 0x400000 ||
        pe->OptionalHeader.SizeOfImage > 0x290000 ||
        pe->OptionalHeader.SizeOfHeaders > (DWORD)length ||
        pe->OptionalHeader.SizeOfHeaders > pe->OptionalHeader.SizeOfImage) return 0;
    DWORD offset = (BYTE*)IMAGE_FIRST_SECTION(pe) - raw;
    if (offset > (DWORD)length || pe->FileHeader.NumberOfSections >
        ((DWORD)length - offset) / sizeof(IMAGE_SECTION_HEADER)) return 0;
    memcpy(mapped, raw, pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(pe);
    for (int i = 0; i < pe->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* section = sections + i;
        if (section->PointerToRawData > (DWORD)length ||
            section->SizeOfRawData > (DWORD)length - section->PointerToRawData ||
            section->VirtualAddress > pe->OptionalHeader.SizeOfImage ||
            section->SizeOfRawData > pe->OptionalHeader.SizeOfImage - section->VirtualAddress)
            return 0;
        memcpy(mapped + section->VirtualAddress, raw + section->PointerToRawData,
               section->SizeOfRawData);
    }
    free(raw);
    return FlushInstructionCache(GetCurrentProcess(), 0, 0);
}

int main(int argc,char**argv) {
 if(argc!=2||!MapReference(argv[1]))return 1;
 page=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);DWORD old;
 if(!page||!VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 2;
 DWORD state=0x1234567;
 const DWORD special[]={0,0x80000000,0x7f800000,0xff800000,0x7fc12345,0x7f812345,1,0x7f7fffff};
 for(int seed=0;seed<128;seed++) {
  for(int i=0;i<4096;i++){state=state*1664525+1013904223;initial[i]=(BYTE)(state>>24);}
  DWORD input[3];for(i=0;i<3;i++){state=state*1664525+1013904223;input[i]=seed<8?special[(seed+i)%8]:state;}
  for(int location=0;location<4;location++) for(int alias=0;alias<4;alias++) {
   int offset=64+location*16;DWORD a[3];memcpy(a,alias?initial+offset+0x40+alias*4:(BYTE*)input,12);
   readOnly=0;Check((SparkParticleEmitter*)(page+offset),a);
  }
  for(int cut=0;cut<7;cut++) {readOnly=0;Check((SparkParticleEmitter*)(page+4096-0x44-cut*4),input);}
  readOnly=1;Check((SparkParticleEmitter*)(page+64),input);
  readOnly=0;Check((SparkParticleEmitter*)0,input);
 }
 printf("Emitter position: %d cases, %d modeled access faults\n",checks,faults);return 0;
}
