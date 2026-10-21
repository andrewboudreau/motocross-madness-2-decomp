// Complete standalone candidate; validation-only mapped retail. The scripted
// build observes file API calls; UIINFO_REAL_FILES links actual VC6 CRT I/O.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/TrackGame.h"
static unsigned char referenceSpace[0x800000];
static unsigned char gameStorage[2][sizeof(TrackGame)],modeStorage[sizeof(TrackGameMode)];
TrackGame* g_TrackGame;
static TrackGame* Game(int i){return (TrackGame*)gameStorage[i];}
static void SetGlobal(int i){g_TrackGame=Game(i);*(TrackGame**)0x56e26c=g_TrackGame;}
static TrackGameMode* mode;
static char nameStorage[520],pathStorage[520],install[260],cd[260],newCd[260],input[520];
static char* name;static char* path;
static int seed,owner,switchGlobal,mutation,foundCd,waitCd,openMask,closeResult,outputAlias;
static int defaultRun,opens,closed;
struct Event {
    int op,receiver,offset,value,global;
    char text[520],output[520];
};
struct Observation {
    int result,count,opens,closed,global;
    Event events[16];
    unsigned char games[sizeof(gameStorage)],mode[sizeof(modeStorage)];
    char name[sizeof(nameStorage)],path[sizeof(pathStorage)];
};
static Observation observed,expected;
static void Fail(int code){printf("Fixture invariant %d\n",code);ExitProcess(code);}
static int Global(){return g_TrackGame==Game(0)?0:g_TrackGame==Game(1)?1:-1;}
static int Receiver(TrackGameMode* p){
    if(p==&Game(0)->mode)return 0;if(p==&Game(1)->mode)return 1;
    if(p==(TrackGameMode*)modeStorage)return 2;Fail(81);return -1;
}
static Event& Record(int op,TrackGameMode* self,int offset=0,int value=0){
    if(observed.count>=16)Fail(82);Event& e=observed.events[observed.count++];
    e.op=op;e.receiver=Receiver(self);e.offset=offset;e.value=value;e.global=Global();
    if(!defaultRun)strcpy(e.output,path);return e;
}
static void Fill(TrackGameMode* self,void* pointer,int size,int op){
    int offset=(char*)pointer-(char*)self;Record(op,self,offset,size);
    if(offset<0||offset+size>sizeof(TrackGameMode))Fail(83);
    for(int i=0;i<size;++i)((unsigned char*)pointer)[i]=(unsigned char)(seed*11+op*7+i);
}
void TrackGameMode::ResetInterfaceOptions(UnknownTrackGameModeOptions6ac* p){
    Fill(this,p,sizeof(*p),1);if(switchGlobal==1)SetGlobal(1);
}
void TrackGameMode::ResetSoundOptions(UnknownTrackGameModeOptionsA20* p){Fill(this,p,sizeof(*p),2);}
void TrackGameMode::ResetGraphicsOptions(UnknownTrackGameModeOptionsA4c* p){
    Fill(this,p,sizeof(*p),3);detailLevel=seed-31;if(switchGlobal==2)SetGlobal(1);
}
void TrackGameMode::ChooseDisplayMode(const void* guid,int* out){
    Record(4,this,(char*)out-(char*)mode,(int)guid);if(guid)Fail(84);
    *out=100+seed;field_0x6a4^=0xabcdef;
}
void TrackGameMode::ResetControlOptions(UnknownTrackGameModeOptionsA68* p){Fill(this,p,sizeof(*p),5);}
void TrackGameMode::ReloadResourceStrings(char(*p)[0x80]){Fill(this,p,sizeof(resourceStrings),6);}
void TrackGameMode::ResetGarageOptions(UnknownTrackGameModeOptionsFd8* p){Fill(this,p,sizeof(*p),7);}
void TrackGameMode::ResetSettingsEntries(){Fill(this,&raceSettings,sizeof(raceSettings),8);}
int TrackGameMode::FindCdDirectory(){
    Record(9,this);if(mutation&1)strcpy(cdDataDirectory,newCd);
    if(mutation&4)strcpy(installDirectory,"changed-install");return foundCd;
}
int TrackGameMode::WaitForCd(){
    Record(10,this);if(mutation&2)strcpy(cdDataDirectory,newCd);
    if(mutation&8)strcpy(installDirectory,"changed-install");return waitCd;
}
#ifndef UIINFO_REAL_FILES
extern "C" FILE* __cdecl fopen(const char* file,const char* flags){
    Event& e=Record(11,mode,++opens);strcpy(e.text,file);
    if(strcmp(flags,"r"))Fail(85);
    return (openMask&(1<<(opens-1)))?(FILE*)(0x12340000+opens*16):0;
}
extern "C" int __cdecl fclose(FILE* file){
    Record(12,mode,0,(int)file);if(file!=(FILE*)(0x12340000+opens*16)||closed++)Fail(86);return closeResult;
}
#endif
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)Fail(87);DWORD a;memcpy(&a,&p,4);return a;}
static DWORD Replacement(DWORD p){
    switch(p){
    case 0x522720:return Address(&TrackGameMode::ResetInterfaceOptions);
    case 0x522780:return Address(&TrackGameMode::ResetSoundOptions);
    case 0x5227d0:return Address(&TrackGameMode::ResetGraphicsOptions);
    case 0x522e20:return Address(&TrackGameMode::ChooseDisplayMode);
    case 0x522800:return Address(&TrackGameMode::ResetControlOptions);
    case 0x522840:return Address(&TrackGameMode::ReloadResourceStrings);
    case 0x522cb0:return Address(&TrackGameMode::ResetGarageOptions);
    case 0x522cd0:return Address(&TrackGameMode::ResetSettingsEntries);
    case 0x5476f0:return (DWORD)_strnicmp;
    case 0x534aaf:return (DWORD)sprintf;
    case 0x534ceb:return (DWORD)fopen;
    case 0x523bf0:return Address(&TrackGameMode::FindCdDirectory);
    case 0x523800:return Address(&TrackGameMode::WaitForCd);
    case 0x534c3d:return (DWORD)fclose;
    }return 0;
}
static BYTE* ReadNativeFile(const char* path,DWORD* size){
    HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
    if(f==INVALID_HANDLE_VALUE)return 0;*size=GetFileSize(f,0);BYTE* bytes=(BYTE*)malloc(*size);DWORD read;
    int ok=bytes&&ReadFile(f,bytes,*size,&read,0)&&read==*size;CloseHandle(f);return ok?bytes:0;
}
static int MapReference(const char* path,const char* patches){
    BYTE* mapped=(BYTE*)0x400000;DWORD old,size;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    BYTE* raw=ReadNativeFile(path,&size);if(!raw)return 0;
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    if(size<sizeof(*dos)||dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* s=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i){
        if(s[i].PointerToRawData+s[i].SizeOfRawData>size||s[i].VirtualAddress+s[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+s[i].VirtualAddress,raw+s[i].PointerToRawData,s[i].SizeOfRawData);
    }
    free(raw);raw=ReadNativeFile(patches,&size);if(!raw||size%8)return 0;
    for(DWORD n=0;n<size;n+=8){
        DWORD at=*(DWORD*)(raw+n),target=*(DWORD*)(raw+n+4),replace=Replacement(target);
        int inside=(at>=0x522440&&at+5<=0x5225eb)||(at>=0x5238f0&&at+5<=0x523a53);
        if(!inside||!replace||*(BYTE*)at!=0xe8||at+5+*(DWORD*)(at+1)!=target)return 0;
        *(DWORD*)(at+1)=replace-at-5;
    }
    free(raw);return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static int installType;
static void Reset(){
    memset(&observed,0,sizeof(observed));memset(nameStorage,0,sizeof(nameStorage));memset(pathStorage,0x67,sizeof(pathStorage));
    for(int i=0;i<sizeof(gameStorage);++i)((BYTE*)gameStorage)[i]=(BYTE)(seed*13+i*7);
    for(i=0;i<sizeof(modeStorage);++i)modeStorage[i]=(BYTE)(seed*17+i*11);
    SetGlobal(0);
    mode=owner==0?&Game(0)->mode:owner==1?&Game(1)->mode:(TrackGameMode*)modeStorage;
    opens=closed=0;
    if(defaultRun)return;
    memset(mode->installDirectory,0,sizeof(mode->installDirectory));memset(mode->cdDataDirectory,0,sizeof(mode->cdDataDirectory));
    strcpy(mode->installDirectory,install);strcpy(mode->cdDataDirectory,cd);mode->installType=installType;
    strcpy(nameStorage,input);name=nameStorage;path=pathStorage;
    if(outputAlias==1)path=nameStorage;
    if(outputAlias==2)path=mode->installDirectory;
    if(outputAlias==3)path=mode->cdDataDirectory;
    if(outputAlias==4)name=mode->installDirectory;
    if(outputAlias==5)name=mode->cdDataDirectory;
    if(outputAlias==0||outputAlias>=4){path[0]='!';path[1]=0;}
}
static void Run(int retail){
    Reset();
    if(defaultRun){
        union{void(TrackGameMode::*method)();DWORD a;}call;call.method=&TrackGameMode::ResetDefaults;
        if(retail)call.a=0x522440;(mode->*call.method)();
    }else{
        union{int(TrackGameMode::*method)(const char*,char*);DWORD a;}call;call.method=&TrackGameMode::FindDataFile;
        if(retail)call.a=0x5238f0;observed.result=(mode->*call.method)(name,path);
    }
    observed.opens=opens;observed.closed=closed;observed.global=Global();
    memcpy(observed.games,gameStorage,sizeof(gameStorage));memcpy(observed.mode,modeStorage,sizeof(modeStorage));
    memcpy(observed.name,nameStorage,sizeof(nameStorage));memcpy(observed.path,pathStorage,sizeof(pathStorage));
}
static int checks,defaultChecks;
static void Compare(){
    Run(1);expected=observed;Run(0);++checks;if(defaultRun)++defaultChecks;
    if(memcmp(&observed,&expected,sizeof(observed))){
        BYTE* a=(BYTE*)&expected,*b=(BYTE*)&observed;int at=0;while(at<sizeof(observed)&&a[at]==b[at])++at;
        printf("Mismatch%d byte%d %02x/%02x defaults%d seed%d owner%d switch%d install'%s' cd'%s' input'%s' type%d find%d wait%d opens%d mutate%d alias%d\n",
            checks,at,a[at],b[at],defaultRun,seed,owner,switchGlobal,install,cd,input,installType,foundCd,waitCd,openMask,mutation,outputAlias);ExitProcess(2);
    }
    if(defaultRun){
        if(observed.count!=10||mode->difficulty!=2||mode->savedDifficulty!=2||mode->plateNumber!=101)Fail(88);
        if(memcmp(&mode->raceSettings,&mode->savedSettings,sizeof(mode->raceSettings))||
           memcmp(&mode->raceSettings,&mode->multiplayerSettings,sizeof(mode->raceSettings)))Fail(89);
        for(int i=0;i<6;++i)if(mode->bonusTrackUnlocked[i])Fail(90);
    }
#ifndef UIINFO_REAL_FILES
    else if(observed.closed!=observed.result||observed.opens<1||observed.opens>2)Fail(91);
#endif
}
#ifdef UIINFO_REAL_FILES
static void MakeFile(const char* directory,const char* name){
    char path[520];sprintf(path,"%s\\%s",directory,name);
    HANDLE f=CreateFileA(path,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
    if(f==INVALID_HANDLE_VALUE)Fail(92);const char data[]="text\r\n\x1a";DWORD wrote;
    if(!WriteFile(f,data,sizeof(data),&wrote,0)||wrote!=sizeof(data))Fail(93);CloseHandle(f);
}
#endif
int main(int argc,char**argv){
    if(argc!=4||!MapReference(argv[1],argv[2]))return 3;
    if(sizeof(TrackGameMode)!=0x2dc0||(char*)&Game(0)->mode-(char*)Game(0)!=0x578)return 4;
#ifndef UIINFO_REAL_FILES
    defaultRun=1;
    for(seed=0;seed<128;++seed)for(owner=0;owner<3;++owner)for(switchGlobal=0;switchGlobal<3;++switchGlobal)Compare();
    defaultRun=0;owner=2;switchGlobal=0;seed=71;strcpy(newCd,"N:\\replacement");
    const char* roots[]={"C:\\data","c:\\DATA","","Data","C:\\data\\"};
    const char* cds[]={"D:\\data","","C:\\data"};
    const char* names[]={"track.env","C:\\data\\track.env","c:\\DATA\\Track.env","D:\\data\\track.env","C:\\dataXtrack.env","","Data\\track.env"};
    const int types[]={-1,0,1,2,7},returns[]={0,1,-7};
    for(int r=0;r<5;++r)for(int c=0;c<3;++c)for(int n=0;n<7;++n)for(int t=0;t<5;++t)
    for(openMask=0;openMask<4;++openMask)for(int find=0;find<3;++find)for(int wait=0;wait<3;++wait){
        strcpy(install,roots[r]);strcpy(cd,cds[c]);strcpy(input,names[n]);installType=types[t];
        foundCd=returns[find];waitCd=returns[wait];mutation=(r+c+n+find+wait)&15;closeResult=t&1?-1:0;outputAlias=0;Compare();
    }
    for(outputAlias=1;outputAlias<=5;++outputAlias)for(openMask=0;openMask<4;++openMask)
    for(mutation=0;mutation<16;++mutation)for(int find=0;find<2;++find)for(int wait=0;wait<2;++wait){
        strcpy(install,"C:\\data");strcpy(cd,"D:\\data");strcpy(input,"C:\\data\\track.env");
        installType=1;foundCd=find;waitCd=wait;Compare();
    }
    printf("UiInfo: %d scripted comparisons (%d defaults) match complete state and ordered calls\n",checks,defaultChecks);
#else
    sprintf(install,"%s\\installed",argv[3]);sprintf(cd,"%s\\disc",argv[3]);sprintf(newCd,"%s\\other",argv[3]);
    if(strlen(install)>200||!CreateDirectoryA(install,0)||!CreateDirectoryA(cd,0)||!CreateDirectoryA(newCd,0))return 5;
    MakeFile(install,"local.dat");MakeFile(cd,"disc.dat");MakeFile(newCd,"moved.dat");
    const char* files[]={"local.dat","disc.dat","moved.dat","missing.dat"};
    const int types[]={0,1,2};defaultRun=0;owner=2;seed=17;outputAlias=0;
    for(int f=0;f<4;++f)for(int t=0;t<3;++t)for(foundCd=0;foundCd<2;++foundCd)
    for(waitCd=0;waitCd<2;++waitCd)for(mutation=0;mutation<4;++mutation){
        strcpy(input,files[f]);installType=types[t];Compare();
        int moved=(mutation&1)||(!foundCd&&(mutation&2));
        int expected=f==0||(installType!=2&&(foundCd||waitCd)&&((f==1&&!moved)||(f==2&&moved)));
        if(observed.result!=expected)Fail(94);
        if(expected){
            char expectedPath[520];sprintf(expectedPath,"%s\\%s",f==0?install:moved?newCd:cd,files[f]);
            if(strcmp(path,expectedPath))Fail(95);
            // Exclusive access after the lookup also checks that its CRT FILE
            // has been closed. No fixture handle survives this check.
            HANDLE check=CreateFileA(path,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
            if(check==INVALID_HANDLE_VALUE)Fail(96);CloseHandle(check);
        }else if(path[0])Fail(97);
    }
    printf("UiInfo: %d native CRT file-search comparisons match\n",checks);
#endif
    return 0;
}
