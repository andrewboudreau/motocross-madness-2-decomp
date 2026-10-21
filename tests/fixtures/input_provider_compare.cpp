// Execute the actual input predicate and keyboard modifier provider.
// The reference functions and their direct call are unpatched.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/KeyboardDevice.h"
static BYTE referenceSpace[0x800000]={1};
TrackGame* g_TrackGame;
static DWORD gameWords[(sizeof(TrackGame)+3)/4];
static DWORD controlWords[(sizeof(ControlInterface)+3)/4];
static DWORD keyboardWords[(sizeof(KeyboardDevice)+3)/4];
class KeyboardProbe : public KeyboardDevice {
public:
 static void Seed(KeyboardDevice* p,int state,int key) {
  ((KeyboardProbe*)p)->modifierState=state;
  ((KeyboardProbe*)p)->keyStates[0x29].state=key;
 }
};
typedef char EventLayout[sizeof(UnknownControlEvent)==20 && offsetof(UnknownControlEvent,kind)==4 ? 1:-1];
static int Map(const char* path){
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 || !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    if(size<sizeof(IMAGE_DOS_HEADER) || ((IMAGE_DOS_HEADER*)raw)->e_lfanew<0 || ((IMAGE_DOS_HEADER*)raw)->e_lfanew+sizeof(IMAGE_NT_HEADERS)>(unsigned)size)return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+((IMAGE_DOS_HEADER*)raw)->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000 || pe->OptionalHeader.SizeOfHeaders>(unsigned)size)return 0;
    IMAGE_SECTION_HEADER* sections=IMAGE_FIRST_SECTION(pe);
    if((BYTE*)(sections+pe->FileHeader.NumberOfSections)>raw+size)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(sections[i].PointerToRawData>(unsigned)size || sections[i].SizeOfRawData>(unsigned)size-sections[i].PointerToRawData || sections[i].VirtualAddress>pe->OptionalHeader.SizeOfImage || sections[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-sections[i].VirtualAddress)return 0;
        memcpy(mapped+sections[i].VirtualAddress,raw+sections[i].PointerToRawData,sections[i].SizeOfRawData);
    }
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0);
}

static unsigned checks;
static void Check(int control,int kind,int modifier,int eventControl,int eventKind,int keyboard,int state,int testKey,int key) {
 memset(gameWords,0,sizeof(gameWords));memset(controlWords,0,sizeof(controlWords));memset(keyboardWords,0,sizeof(keyboardWords));
 g_TrackGame=(TrackGame*)gameWords;
 g_TrackGame->controlInterface=(ControlInterface*)controlWords;
 g_TrackGame->testKey=testKey;
 g_TrackGame->controlInterface->keyboard=keyboard?(KeyboardDevice*)keyboardWords:0;
 KeyboardProbe::Seed((KeyboardDevice*)keyboardWords,state,key);
 *(TrackGame**)0x56e26c=g_TrackGame;
 UnknownControlEvent event={eventControl,eventKind,0x12345678,-1,0x87654321};
 DWORD gameBefore[sizeof(gameWords)/4],controlBefore[sizeof(controlWords)/4],keyBefore[sizeof(keyboardWords)/4];
 memcpy(gameBefore,gameWords,sizeof(gameWords));memcpy(controlBefore,controlWords,sizeof(controlWords));memcpy(keyBefore,keyboardWords,sizeof(keyboardWords));
 typedef int (__cdecl* Predicate)(int,int,const UnknownControlEvent*,int);
 int expected=((Predicate)0x43caa0)(control,kind,&event,modifier);
 int actual=TestInputEvent(control,kind,&event,modifier);
 if(actual!=expected || memcmp(gameBefore,gameWords,sizeof(gameWords)) || memcmp(controlBefore,controlWords,sizeof(controlWords)) || memcmp(keyBefore,keyboardWords,sizeof(keyboardWords))) {printf("input mismatch %u\n",checks);exit(4);}
 ++checks;
}
int main(int argc,char** argv) {
 if(argc!=2 || !Map(argv[1]))return 3;
 const int masks[]={0,1,2,3,4,8,0x3f,0x40,0x41,0x7f,0x80,0xc0,0x80000000,-1};
 const int states[]={0,1,2,3,4,8,0x3f,0x40,0x80,-1};
 for(int m=0;m<14;++m)for(int s=0;s<10;++s)for(int keyboard=0;keyboard<2;++keyboard)
 for(int debug=0;debug<2;++debug)for(int key=0;key<3;++key)for(int match=0;match<4;++match)
  Check(0x57,0,masks[m],(match&1)?0x57:0x43,(match&2)?0:1,keyboard,states[s],debug,key);
 // Independent ordinary outcomes, including short circuit before global use.
 UnknownControlEvent event={0x57,0,0,0,0};g_TrackGame=0;
 if(TestInputEvent(0x43,0,&event,0x80) || TestInputEvent(0x57,1,&event,0x80))return 5;
 g_TrackGame=(TrackGame*)gameWords;g_TrackGame->controlInterface=(ControlInterface*)controlWords;
 g_TrackGame->controlInterface->keyboard=0;
 if(TestInputEvent(0x57,0,&event,0x80)!=1)return 6;
 g_TrackGame->controlInterface->keyboard=(KeyboardDevice*)keyboardWords;g_TrackGame->testKey=0;
 if(TestInputEvent(0x57,0,&event,0x80))return 7;
 g_TrackGame->testKey=1;KeyboardProbe::Seed((KeyboardDevice*)keyboardWords,0,1);
 if(TestInputEvent(0x57,0,&event,0x80)!=1 || TestInputEvent(0x57,0,&event,0x80000000)!=1)return 8;
 printf("Input predicate: %u complete event and modifier cases match\n",checks);return 0;
}
