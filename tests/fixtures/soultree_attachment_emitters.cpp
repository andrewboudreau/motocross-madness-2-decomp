// Actual seven store-only methods; complete-page and progressive-fault oracles.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "ParticleEmitters.h"
typedef char Layout[(sizeof(ParticleVec3)==12 && sizeof(DustParticleEmitter)==0x80 && sizeof(SteamParticleEmitter)==0x47c &&
 offsetof(DustParticleEmitter,position)==0x44 && offsetof(DustParticleEmitter,previousPosition)==0x50 &&
 offsetof(DustParticleEmitter,intensity)==0x78 && offsetof(DustParticleEmitter,color)==0x7c &&
 offsetof(SteamParticleEmitter,position)==0x48 && offsetof(SteamParticleEmitter,previousPosition)==0x54 &&
 offsetof(SteamParticleEmitter,field_0x40)==0x40 && offsetof(SteamParticleEmitter,field_0x44)==0x44 &&
 offsetof(SteamParticleEmitter,emitting)==0x60 && offsetof(SteamParticleEmitter,field_0x6c)==0x6c &&
 offsetof(SteamParticleEmitter,field_0x70)==0x70 && offsetof(SteamParticleEmitter,color)==0x478 &&
 sizeof(void(DustParticleEmitter::*)(ParticleVec3,float))==4 &&
 sizeof(void(DustParticleEmitter::*)(unsigned char,unsigned char,int))==4 &&
 sizeof(void(SteamParticleEmitter::*)(ParticleVec3))==4 && sizeof(void(SteamParticleEmitter::*)())==4 &&
 sizeof(void(SteamParticleEmitter::*)(unsigned char,unsigned char,int))==4)?1:-1];
static BYTE referenceSpace[0x800000]={1};
static BYTE* page;
static BYTE initial[4096],expected[4096],observed[4096];
static DWORD wantedAccess,wantedAddress,gotAccess,gotAddress,gotException;
static int wantedFault,complete,checks,faults,readOnly;
static DWORD inputs[4];
static int Access(DWORD address,int write,DWORD* bits) {
 DWORD offset=address-(DWORD)page;
 if(offset>4096-4 || (write&&readOnly)){wantedFault=1;wantedAccess=write;wantedAddress=address;return 0;}
 if(write)memcpy(expected+offset,bits,4);else memcpy(bits,expected+offset,4);return 1;
}
static int Store(DWORD self,int offset,DWORD value){return Access(self+offset,1,&value);}
static int Copy(DWORD self,int from,int to){DWORD bits;return Access(self+from,0,&bits)&&Access(self+to,1,&bits);}
static void Model(int op,DWORD self) {
 wantedFault=0;wantedAccess=wantedAddress=0;memcpy(expected,initial,4096);
 int i;DWORD bits;
 switch(op){
 case 0: case 2:
  for(i=0;i<3;i++)if(!Copy(self,(op==0?0x44:0x48)+i*4,(op==0?0x50:0x54)+i*4))return;
  for(i=0;i<3;i++)if(!Store(self,(op==0?0x44:0x48)+i*4,inputs[i]))return;
  if(op==0)Store(self,0x78,inputs[3]);return;
 case 1: case 6: Store(self,op==1?0x7c:0x478,((inputs[0]&255)<<16)|((inputs[1]&255)<<8)|(inputs[2]&255));return;
 case 3: if(!Access(self+0x40,0,&bits)||!Store(self,0x6c,1)||!Store(self,0x44,bits))return;Store(self,0x60,1);return;
 case 4: if(!Store(self,0x44,0x38d1b717)||!Store(self,0x6c,1))return;Store(self,0x70,1);return;
 case 5: if(!Store(self,0x60,0))return;for(i=0;i<3;i++)if(!Copy(self,0x48+i*4,0x54+i*4))return;return;
 }
}
static int Filter(EXCEPTION_POINTERS* p) {
 if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
 gotException=p->ExceptionRecord->ExceptionCode;gotAccess=p->ExceptionRecord->ExceptionInformation[0];gotAddress=p->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int retail,int op,DWORD self) {
 DustParticleEmitter* d=(DustParticleEmitter*)self;SteamParticleEmitter* s=(SteamParticleEmitter*)self;
 ParticleVec3 p;float intensity;memcpy(&p,inputs,12);memcpy(&intensity,inputs+3,4);
 union{void(DustParticleEmitter::*m)(ParticleVec3,float);DWORD a;}dp;
 union{void(DustParticleEmitter::*m)(unsigned char,unsigned char,int);DWORD a;}dc;
 union{void(SteamParticleEmitter::*m)(ParticleVec3);DWORD a;}sp;
 union{void(SteamParticleEmitter::*m)();DWORD a;}sm;
 union{void(SteamParticleEmitter::*m)(unsigned char,unsigned char,int);DWORD a;}sc;
 gotException=gotAccess=gotAddress=0;complete=0;
 __try{
  switch(op){
  case 0: dp.m=&DustParticleEmitter::SetPosition;if(retail)dp.a=0x4b8d90;(d->*dp.m)(p,intensity);break;
  case 1: dc.m=&DustParticleEmitter::SetColor;if(retail)dc.a=0x4b8dd0;(d->*dc.m)((unsigned char)inputs[0],(unsigned char)inputs[1],inputs[2]);break;
  case 2: sp.m=&SteamParticleEmitter::SetPosition;if(retail)sp.a=0x4ba2c0;(s->*sp.m)(p);break;
  case 3: sm.m=&SteamParticleEmitter::Restart;if(retail)sm.a=0x4ba300;(s->*sm.m)();break;
  case 4: sm.m=&SteamParticleEmitter::Burst;if(retail)sm.a=0x4ba320;(s->*sm.m)();break;
  case 5: sm.m=&SteamParticleEmitter::StopEmitting;if(retail)sm.a=0x4ba340;(s->*sm.m)();break;
  case 6: sc.m=&SteamParticleEmitter::SetColor;if(retail)sc.a=0x4ba360;(s->*sc.m)((unsigned char)inputs[0],(unsigned char)inputs[1],inputs[2]);break;
  }complete=1;
 }__except(Filter(GetExceptionInformation())){}
}
static void Check(int op,DWORD self){
 Model(op,self);DWORD old;
 for(int role=0;role<2;role++){
  if(!VirtualProtect(page,4096,PAGE_READWRITE,&old))ExitProcess(10);memcpy(page,initial,4096);
  if(readOnly&&!VirtualProtect(page,4096,PAGE_READONLY,&old))ExitProcess(11);
  Run(role,op,self);memcpy(observed,page,4096);
  if(complete==wantedFault||!!gotException!=wantedFault||(wantedFault&&(gotAccess!=wantedAccess||gotAddress!=wantedAddress))||memcmp(expected,observed,4096)){
   printf("mismatch case%d op%d role%d fault%d/%d access%lu/%lu address%08lx/%08lx\n",checks,op,role,wantedFault,!!gotException,wantedAccess,gotAccess,wantedAddress,gotAddress);ExitProcess(3);
  }
 }checks++;faults+=wantedFault;
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

int main(int argc,char** argv){
 if(argc!=2||!MapReference(argv[1]))return 2;
 BYTE* reserve=(BYTE*)VirtualAlloc(0,12288,MEM_RESERVE,PAGE_NOACCESS);if(!reserve)return 4;
 page=(BYTE*)VirtualAlloc(reserve+4096,4096,MEM_COMMIT,PAGE_READWRITE);if(!page)return 5;
 DWORD state=0x53134;
 for(int seed=0;seed<64;seed++){
  for(int i=0;i<4096;i++){state=state*1664525+1013904223;initial[i]=(BYTE)(state>>24);}
  for(i=0;i<4;i++){state=state*1664525+1013904223;inputs[i]=state;}
  if(seed<4){inputs[0]=inputs[1]=inputs[2]=seed==0?0:seed==1?0x40:seed==2?0x80:0xff;inputs[3]=0;}
  for(int op=0;op<7;op++){
   readOnly=0;Check(op,(DWORD)page+256);
   readOnly=1;Check(op,(DWORD)page+256);readOnly=0;
   Check(op,0);
   // Every aligned accessible-prefix boundary through the largest touched field.
   for(int prefix=0;prefix<=0x480;prefix+=4)Check(op,(DWORD)page+4096-prefix);
  }
 }
 printf("Attachment emitters: %d cases, %d modeled access faults\n",checks,faults);return 0;
}
