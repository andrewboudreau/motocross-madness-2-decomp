// Private comparison of the actual standalone primary audio-format candidate.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/SoundInterface.h"
static unsigned char referenceSpace[0x800000];
struct World { unsigned char audio[sizeof(PCSoundInterface)];DWORD buffer[8]; };
static World world;
static DWORD table[21];
static int active,mode,callbackResult,checks;
struct Observation {int result,calls,exception,faultKind,faultAddress;unsigned char format[16];World before,after;};
static Observation observed,expected;
static PCSoundInterface* Object(){return (PCSoundInterface*)world.audio;}
static long __stdcall SetFormat(void* self,const UnknownWaveFormat* format){
    if(self!=world.buffer || observed.calls++)ExitProcess(81);
    memcpy(observed.format,format,16);observed.before=world;
    if(mode==1){world.audio[0x2c]^=1;Object()->primaryBuffer=0;world.buffer[3]^=0x12345678;}
    if(mode==3)throw 71;
    return callbackResult;
}
static int MapReference(const char* path) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) { printf("Bad reference reservation %p\n",referenceSpace);return 0; }
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static int Filter(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    observed.exception=e->ExceptionRecord->ExceptionCode;observed.faultKind=e->ExceptionRecord->ExceptionInformation[0];
    observed.faultAddress=e->ExceptionRecord->ExceptionInformation[1];return EXCEPTION_EXECUTE_HANDLER;
}
static int Invoke(int retail,int rate,int stereo,int bits){
    union { int(PCSoundInterface::*method)(int,int,int);DWORD address; } call;
    call.method=&PCSoundInterface::SetPrimaryFormat;if(retail)call.address=0x4be910;
    __try{return (Object()->*call.method)(rate,stereo,bits);}__except(Filter(GetExceptionInformation())){return -500;}
}
static void Run(int retail,int rate,int stereo,int bits){
    memset(&observed,0,sizeof(observed));memset(&world,0x53,sizeof(world));world.audio[0x2c]=(BYTE)active;
    world.buffer[0]=(DWORD)table;Object()->primaryBuffer=mode==2?0:(UnknownSoundBuffer*)world.buffer;
    observed.result=-501;try{observed.result=Invoke(retail,rate,stereo,bits);}catch(int e){observed.exception=e;}
    observed.after=world;
}
static void Compare(int rate,int stereo,int bits){
    Run(1,rate,stereo,bits);expected=observed;Run(0,rate,stereo,bits);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){
        printf("Mismatch%d rate%d stereo%d bits%d active%d mode%d result%d\n",checks,rate,stereo,bits,active,mode,callbackResult);ExitProcess(2);
    }
    if(!(active&1)){if(observed.result||observed.calls||observed.exception)ExitProcess(82);}
    else if(mode==2){if(observed.exception!=EXCEPTION_ACCESS_VIOLATION || observed.faultKind || observed.faultAddress)ExitProcess(83);}
    else{
        if(observed.calls!=1 || *(unsigned short*)observed.format!=1 ||
           *(unsigned short*)(observed.format+2)!=(stereo?2:1))ExitProcess(84);
        if(mode==3){if(observed.exception!=71)ExitProcess(85);}
        else if(observed.exception || observed.result!=(callbackResult>=0))ExitProcess(86);
    }
}
int main(int argc,char**argv){
    if(argc!=2 || !MapReference(argv[1]))return 3;
    if(sizeof(PCSoundInterface)!=0x478 || sizeof(UnknownWaveFormat)!=20 ||
       offsetof(PCSoundInterface,primaryBuffer)!=0x468 || offsetof(UnknownWaveFormat,extraSize)!=16)return 4;
    for(int i=0;i<21;++i)table[i]=(DWORD)abort;table[14]=(DWORD)SetFormat;
    active=1;for(int b=0;b<65536;++b){Compare(22050,0,b);Compare(22050,1,b);}
    const int flags[]={0,1,2,255},stereos[]={0,1,-1,(int)0x80000000},returns[]={0,1,-1,0x7fffffff,(int)0x80000000};
    unsigned int random=123456789;
    for(int seed=0;seed<128;++seed){
        random=random*1664525u+1013904223u;int rate=(int)random;
        random=random*1664525u+1013904223u;int bits=(int)random;
        for(int a=0;a<4;++a)for(int st=0;st<4;++st)for(int r=0;r<5;++r)for(mode=0;mode<4;++mode){
            active=flags[a];callbackResult=returns[r];Compare(rate,stereos[st],bits);
        }
    }
    printf("PrimarySoundFormat: %d PCM, return, callback and fault scenarios match\n",checks);return 0;
}
