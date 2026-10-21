// Private native comparison of the complete channel-difference candidate.
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/TextureMap.h"
float MeanTextureChannelDifference(TextureMap*,TextureMap*);
static unsigned char referenceSpace[0x800000];
static unsigned char objects[2][sizeof(TextureMap)];
static DWORD table[21];
static unsigned short* pixels[2];
static unsigned short* initialPixels[2];
static int activeWords;
static int width,height,format,padding,failure,mutation,alias;
struct Event { int op,receiver,rect,flags; unsigned char objects[2][sizeof(TextureMap)]; };
struct Observation { float result; int count;Event events[4];unsigned char objects[2][sizeof(TextureMap)]; };
static Observation observed,expected;
static TextureMap* Texture(int i) { return (TextureMap*)objects[i]; }
static int Index(const void* p) { if(p==Texture(0))return 0;if(p==Texture(1))return 1;ExitProcess(81);return 0; }
static Event& Record(int op,const void* p,void* rect,int flags) {
    if(observed.count>=4)ExitProcess(82);
    Event& e=observed.events[observed.count++];e.op=op;e.receiver=Index(p);e.rect=(int)rect;e.flags=flags;
    memcpy(e.objects,objects,sizeof(objects));return e;
}
struct Hooks {
    void* Lock(void* rect,long* pitch,int flags) {
        int n=Index(this);Record(1,this,rect,flags);
        if(rect || flags!=0x811)ExitProcess(83);
        *pitch=(width+padding+n)*2+(padding==3?1:0);
        if(mutation) { Texture(0)->mapFormat=format;Texture(0)->textureWidth=width;Texture(0)->textureHeight=height; }
        if(failure==observed.count)return 0;
        return pixels[n]+16;
    }
    int Unlock(void* rect) {
        int n=Index(this);Record(2,this,rect,0);if(rect)ExitProcess(84);
        Texture(n)->mapFlags^=0x12345678;Texture(n)->textureWidth=99;
        return mutation?-7:0;
    }
};
template<class T> static DWORD Address(T method) { if(sizeof(method)!=4)ExitProcess(85);DWORD a;memcpy(&a,&method,4);return a; }
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
static unsigned int seed;
static void Reset() {
    memset(&observed,0,sizeof(observed));memset(objects,0x53,sizeof(objects));
    unsigned int random=seed+1;activeWords=(width+padding+1)*(height?height:1)+32;
    if(activeWords>0x110000)ExitProcess(91);
    for(int n=0;n<2;++n) {
        *(DWORD*)objects[n]=(DWORD)table;Texture(n)->textureWidth=mutation?1:width;
        Texture(n)->textureHeight=mutation?1:height;Texture(n)->mapFormat=mutation?0:format;
        for(int i=0;i<activeWords;++i) { random=random*1664525u+1013904223u;pixels[n][i]=(unsigned short)(random>>16); }
    }
    // Enumeration cases cover every16-bit input with a distinct partner.
    if(seed<65536) { pixels[0][16]=(unsigned short)seed;pixels[1][16]=(unsigned short)(seed^0xa55a); }
    for(n=0;n<2;++n)memcpy(initialPixels[n],pixels[n],activeWords*2);
}
static void Run(int retail) {
    typedef float(__cdecl* Fn)(TextureMap*,TextureMap*);
    Fn fn=retail?(Fn)0x4cb330:MeanTextureChannelDifference;
    _clearfp();observed.result=fn(Texture(0),Texture(alias?0:1));
    memcpy(observed.objects,objects,sizeof(objects));for(int n=0;n<2;++n)if(memcmp(initialPixels[n],pixels[n],activeWords*2))ExitProcess(92);
}
static int checks;
static void Compare() {
    Reset();Run(1);expected=observed;Reset();Run(0);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))) {
        printf("Mismatch case%d seed%u w%d h%d fmt%x padding%d fail%d mutation%d alias%d result%.9g/%.9g\n",
          checks,seed,width,height,format,padding,failure,mutation,alias,expected.result,observed.result);ExitProcess(2);
    }
    if(failure) {
        int count=failure==1?1:3;
        if(observed.result!=-1.0f || observed.count!=count)ExitProcess(86);
        // Retail's second-lock failure unlocks the second object, not the first.
        if(failure==2 && (observed.events[2].op!=2 || observed.events[2].receiver!=(alias?0:1)))ExitProcess(87);
    } else {
        if(observed.count!=4 || observed.events[2].op!=2 || observed.events[2].receiver!=0 ||
           observed.events[3].op!=2 || observed.events[3].receiver!=(alias?0:1))ExitProcess(88);
        if(width && height && (observed.result<0 || observed.result>1))ExitProcess(89);
        if(alias && width && height && observed.result!=0.0f)ExitProcess(90);
    }
}
int main(int argc,char** argv) {
    if(argc!=2 || !MapReference(argv[1]))return 3;
    if(sizeof(TextureMap)!=0x70)return 4;
    for(int n=0;n<2;++n) {
        pixels[n]=(unsigned short*)malloc(0x220000);initialPixels[n]=(unsigned short*)malloc(0x220000);
        if(!pixels[n] || !initialPixels[n])return 5;
    }
    for(int i=0;i<21;++i)table[i]=(DWORD)abort;table[13]=Address(&Hooks::Lock);table[14]=Address(&Hooks::Unlock);
    width=height=1;
    for(seed=0;seed<65536;++seed)for(int f=0;f<2;++f) { format=f?0x22b:0x235;Compare(); }
    const int widths[]={0,1,2,7,16,31},heights[]={0,1,3,8},formats[]={0x22b,0x235,-1};
    for(int s=0;s<8;++s)for(int w=0;w<6;++w)for(int h=0;h<4;++h)for(int f=0;f<3;++f)
    for(padding=0;padding<4;++padding)for(failure=0;failure<3;++failure)for(alias=0;alias<2;++alias) {
        seed=65536+s;width=widths[w];height=heights[h];format=formats[f];mutation=s&1;Compare();
    }
    failure=0;mutation=0;padding=3;
    const int sizes[]={256,512,1024};
    for(int big=0;big<3;++big)for(int f=0;f<2;++f)for(int s=0;s<4;++s)for(alias=0;alias<2;++alias) {
        width=height=sizes[big];format=f?0x22b:0x235;seed=999999+s;Compare();
    }
    printf("TextureChannelDifference: %d pixel, pitch, failure and callback scenarios match\n",checks);return 0;
}
