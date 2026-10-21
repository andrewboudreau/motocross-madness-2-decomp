// Validation only: complete standings method, actual helpers, finite scripted GDI/COM.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <locale.h>
#include <stddef.h>
#include "TrackOverlay.h"
#include "TrackGame.h"
#include "BikeRaceCore.h"
#include "PCTextureMap.h"
#include "Display.h"
TrackGame* g_TrackGame;
extern "C" int __cdecl _output();
extern "C" long __cdecl _ftol();
static BYTE referenceSpace[0x800000]={1};
struct Surface{DWORD table;int locks,unlocks,held,dcHeld,blits;};
struct World{
 BYTE overlay[sizeof(StatsOverlay)],game[2][sizeof(TrackGame)],race[sizeof(BikeRace)],owner[sizeof(TrackGameViewOwner)];
 BYTE racers[12][0x1700],texture[3][sizeof(PCTextureMap)],display[sizeof(UnknownDisplay)];
 BYTE pixels[6][4096],guards[2][4096];DWORD slots[12];Surface surfaces[6];
 DWORD font,gamePointer;int locks,unlocks,draws,blits;
};
struct Event{int kind,who,status,n;DWORD a,b,c;char text[128];RECT rect;SIZE size;World before,after;};
struct Result{World world;Event events[40];int count,complete,result;DWORD exception,access,address;unsigned fpStatus,fpControl;};
struct Scenario{int count,own,reverse,disabled,mode,cached,tie,scoreVariant,timeCase,width,format,pitch,rows,whole,resource,status,dcStatus,releaseStatus,lockFail,unlockFail,bltFail,keyFail,levels,key,partial,alias,mutation,mutateAt,throwAt,fault,available;unsigned control;};
static World w,initial;static Result got,want;static Scenario sc;static int checks,avs,throws;static DWORD textureTable[15],surfaceTable[33];static BYTE*guard[2];static int vb[2]={-4,0x1608};static StatsOverlay*receiver;
static DWORD& At(void*p,int n){return *(DWORD*)((BYTE*)p+n);}
static StatsOverlay* OverlayObject(){return (StatsOverlay*)w.overlay;}
static PCTextureMap* Texture(int i){return (PCTextureMap*)w.texture[i];}
template<class T>static DWORD Address(T p){if(sizeof(p)!=4)ExitProcess(80);union{T p;DWORD n;}u;u.p=p;return u.n;}
class TextureCalls{public:int Upload(UnknownRect*r,int n){return ((PCTextureMap*)this)->PCTextureMap::Upload(r,n);}void*Lock(void*r,long*p,int f){return ((PCTextureMap*)this)->PCTextureMap::LockSurface(r,p,f);}int Unlock(void*r){return ((PCTextureMap*)this)->PCTextureMap::UnlockSurface(r);}};
static void Number(char*&p,int v){if(v<0){*p++='-';v=-v;}char b[16];int n=0;do{b[n++]=(char)('0'+v%10);v/=10;}while(v);while(n)*p++=b[--n];}
static int Score(int id){return sc.tie?17:(sc.reverse?id*13-50:50-id*13);}
static void Line(char*text,int rank,int id){char*p=text;Number(p,rank);*p++=')';*p++=' ';*p++='R';Number(p,id);*p++=' ';*p++=':';*p++=' ';Number(p,Score(id));*p=0;}
static void Capture(){for(int i=0;i<2;i++)memcpy(w.guards[i],guard[i],4096);w.gamePointer=(DWORD)g_TrackGame;}
static void Sync(){*(DWORD*)0x56e26c=(DWORD)g_TrackGame;}
static Event& Begin(int kind,int who){if(got.count>=40)ExitProcess(81);Capture();Event&e=got.events[got.count++];e.kind=kind;e.who=who;e.before=w;return e;}
static void Finish(Event&e){Capture();e.after=w;}
static void Boundary(RECT*rect){
 if(sc.mutateAt==got.count){switch(sc.mutation){
 case 0:g_TrackGame=(TrackGame*)w.game[1];break;
 case 1:OverlayObject()->largeFont=(void*)0x7777;break;
 case 2:OverlayObject()->sharedTexture=Texture(0);break;
 case 3:OverlayObject()->overlayTexture=Texture(0);break;
 case 4:Texture(1)->systemSurface=(UnknownSurfaceInterface*)&w.surfaces[0];break;
 case 5:Texture(2)->systemSurface=(UnknownSurfaceInterface*)&w.surfaces[1];break;
 case 6:OverlayObject()->rowRects[0].left=-77;OverlayObject()->rowRects[0].right=111;break;
 case 7:memcpy(OverlayObject()->rowText[0],"changed row",12);break;
 case 8:OverlayObject()->sourceRect.top=1;break;
 case 9:Texture(1)->textureWidth=2;break;
 case 10:Texture(1)->usesColorKey=1;break;
 case 11:w.font=0x9999;break;
 case 12:if(rect){rect->left=-31;rect->right=90;}break;
 case 13:OverlayObject()->sharedTexture=(TextureMap*)1;break;
 case 14:OverlayObject()->overlayTexture=(TextureMap*)1;break;
 }}Sync();
 if(sc.throwAt==got.count){Finish(got.events[got.count-1]);RaiseException(0xe051abcd,0,0,0);}
}
static int Index(void*p){for(int i=0;i<6;i++)if(p==&w.surfaces[i])return i;ExitProcess(82);return 0;}
static void Text(Event&e,const char*p,int n){if(n<0||n>=128)ExitProcess(83);e.n=n;memcpy(e.text,p,n);e.text[n]=0;}
static BYTE* Pixels(int i){if(sc.alias&&i==1)return w.pixels[0]+1024;if(sc.alias==2&&i==2)return w.pixels[1]+1024;return w.pixels[i]+1024;}
static long __stdcall Lock(void*self,void*rect,UnknownSurfaceDesc*desc,int flags,void*event){
 int i=Index(self);BYTE expected[124]={0};*(DWORD*)expected=124;if(rect||event||memcmp(expected,desc,124))ExitProcess(84);
 if(flags!=(w.locks?0x801:0x811))ExitProcess(113);Event&e=Begin(2,i);e.a=flags;w.surfaces[i].locks++;int fail=(sc.lockFail&(1<<w.locks++))!=0;e.status=fail?(int)0x80004005:0;
 desc->pitch=sc.pitch;desc->surface=Pixels(i);if(sc.fault==7&&i==1)desc->surface=guard[0]+4096-sc.available;if(sc.fault==8&&i==0)desc->surface=guard[0]+4096-sc.available;
 if(!fail)w.surfaces[i].held++;Boundary(0);Finish(e);return e.status;
}
static long __stdcall Unlock(void*self,void*rect){int i=Index(self);if(rect)ExitProcess(85);Event&e=Begin(3,i);w.surfaces[i].unlocks++;int fail=(sc.unlockFail&(1<<w.unlocks++))!=0;e.status=fail?1:0;if(!fail)w.surfaces[i].held--;Boundary(0);Finish(e);return e.status;}
static long __stdcall GetDc(void*self,void**out){int i=Index(self);Event&e=Begin(4,i);e.status=sc.dcStatus;*out=(void*)(0x12340000+i);if(!e.status)w.surfaces[i].dcHeld++;Boundary(0);Finish(e);return e.status;}
static long __stdcall ReleaseDc(void*self,void*dc){int i=Index(self);Event&e=Begin(11,i);e.a=(DWORD)dc;e.status=sc.releaseStatus;if(!e.status)w.surfaces[i].dcHeld--;Boundary(0);Finish(e);return e.status;}
static long __stdcall Blt(void*self,long x,long y,void*source,void*rect,int flags){int i=Index(self),j=Index(source);Event&e=Begin(12,i);e.a=j;e.b=flags;e.c=(x&0xffff)|(y<<16);e.rect=*(RECT*)rect;e.status=sc.bltFail;w.surfaces[i].blits++;w.blits++;
 RECT*r=(RECT*)rect;if(x<0||y<0||r->left<0||r->top<0||r->right>8||r->bottom>128)ExitProcess(86);
 if(!e.status)for(int row=0;row<r->bottom-r->top;row++)for(int col=0;col<(r->right-r->left)*2;col++)Pixels(i)[(y+row)*sc.pitch+x*2+col]=Pixels(j)[(r->top+row)*sc.pitch+r->left*2+col];
 Boundary(r);Finish(e);return e.status;
}
static long __stdcall Attached(void*self,UnknownSurfaceCaps*c,void**out){int i=Index(self);if(c->caps!=0x401000||c->caps2||c->caps3||c->caps4)ExitProcess(87);Event&e=Begin(13,i);e.a=c->caps;int next=i==1?4:i==3?5:-1;e.status=(!sc.levels||next<0)?1:0;*out=e.status?0:&w.surfaces[next];Boundary(0);Finish(e);return e.status;}
static long __stdcall Key(void*self,int flags,void*key){int i=Index(self);Event&e=Begin(14,i);e.a=flags;e.b=*(DWORD*)key;e.status=sc.keyFail;Boundary(0);Finish(e);return e.status;}
static COLORREF WINAPI Bk(HDC dc,COLORREF color){if(color!=1)ExitProcess(114);Event&e=Begin(5,-1);e.a=(DWORD)dc;e.b=color;e.status=sc.status;Boundary(0);Finish(e);return e.status;}
static int WINAPI Mode(HDC dc,int mode){if(mode!=1)ExitProcess(115);Event&e=Begin(6,-1);e.a=(DWORD)dc;e.b=mode;e.status=sc.status;Boundary(0);Finish(e);return e.status;}
static COLORREF WINAPI Color(HDC dc,COLORREF color){if(color!=0xffffff)ExitProcess(116);Event&e=Begin(7,-1);e.a=(DWORD)dc;e.b=color;e.status=sc.status;Boundary(0);Finish(e);return e.status;}
static HGDIOBJ WINAPI Select(HDC dc,HGDIOBJ object){Event&e=Begin(8,-1);e.a=(DWORD)dc;e.b=(DWORD)object;DWORD old=w.font;e.c=old;w.font=(DWORD)object;Boundary(0);Finish(e);return (HGDIOBJ)old;}
static BOOL WINAPI Measure(HDC dc,LPCSTR text,int n,LPSIZE size){Event&e=Begin(9,-1);e.a=(DWORD)dc;Text(e,text,n);size->cx=n*7;size->cy=13;e.size=*size;e.status=sc.status;Boundary(0);Finish(e);return e.status;}
static int WINAPI Draw(HDC dc,LPCSTR text,int n,LPRECT rect,UINT flags){if(flags!=0x120)ExitProcess(117);Event&e=Begin(10,-1);e.a=(DWORD)dc;e.b=flags;Text(e,text,n);e.rect=*rect;int i=(DWORD)dc-0x12340000;if(i<0||i>=6)ExitProcess(88);int row=w.draws++;if(row>=8)ExitProcess(89);*(WORD*)(Pixels(i)+row*sc.pitch)=0x7fff;e.status=sc.status;Boundary(rect);Finish(e);return e.status;}
static int WINAPI Load(HINSTANCE h,UINT id,LPSTR out,int size){Event&e=Begin(1,-1);e.a=(DWORD)h;e.b=id;e.c=size;strcpy(out,"Time left");e.status=sc.status?9:0;Boundary(0);Finish(e);return e.status;}
static void Defaults(){memset(&sc,0,sizeof(sc));sc.count=5;sc.own=4;sc.disabled=-1;sc.width=4;sc.format=4444;sc.pitch=16;sc.rows=4;sc.whole=1;sc.resource=1;sc.status=1;sc.partial=1;sc.control=_MCW_EM|_PC_64|_RC_NEAR;}
static void Setup(){
 memset(&w,0,sizeof(w));memset(&got,0,sizeof(got));receiver=OverlayObject();g_TrackGame=(TrackGame*)w.game[0];g_TrackGame->quarryEvent=(TrackGameViewOwner*)w.owner;g_TrackGame->quarryEvent->raceView=(UnknownKrustyBikeView*)w.race;
 g_TrackGame->playerCount=sc.count;g_TrackGame->mode.raceSettings.raceMode=sc.mode;g_TrackGame->mode.raceSettings.aiRacerCount=sc.count-1;g_TrackGame->mode.raceSettings.multiplayerAiRacerCount=0;g_TrackGame->mode.raceSettings.timeLimitMinutes=3.5f;g_TrackGame->quarryEvent->elapsedMinutes=1;g_TrackGame->quarryEvent->elapsedSeconds=15;
 if(sc.timeCase){float times[]={-75.0f,-0.5f,0.0f,59.75f,60.0f,60.25f,119.75f,120.0f,3599.0f};g_TrackGame->mode.raceSettings.timeLimitMinutes=0;g_TrackGame->quarryEvent->elapsedMinutes=0;g_TrackGame->quarryEvent->elapsedSeconds=-times[sc.timeCase-1];}
 g_TrackGame->resourceInstance=sc.resource?(void*)0x1111:0;g_TrackGame->display=(UnknownDisplay*)w.display;((UnknownDisplay*)w.display)->partialTextureUploadResult=sc.partial;
 BikeRace*r=(BikeRace*)w.race;r->localRacer=(UnknownBikeRaceRacer*)w.racers[sc.own];r->racerSlots=(UnknownBikeRaceRacer**)w.slots;
 int order[12],used=0;for(int i=0;i<sc.count;i++){w.slots[i]=(DWORD)w.racers[i];At(w.racers[i],4)=(DWORD)vb;w.racers[i][0x1631]=(BYTE)(i!=sc.disabled);char*p=(char*)w.racers[i]+0x5e0;*p++='R';Number(p,i);*p=0;float value=(float)Score(i);if(sc.scoreVariant)value+=(sc.scoreVariant-3)*0.25f;*(float*)(w.racers[i]+0x768)=value;if(i!=sc.disabled)order[used++]=i;}
 if(sc.reverse&&!sc.tie)for(int j=0;j<used/2;j++){int t=order[j];order[j]=order[used-1-j];order[used-1-j]=t;}
 int ownRank=0;while(order[ownRank]!=sc.own)ownRank++;
 StatsOverlay*o=OverlayObject();o->largeFont=(void*)0x2222;o->backingTexture=Texture(0);o->overlayTexture=Texture(1);o->sharedTexture=Texture(2);o->sourceRect.left=0;o->sourceRect.top=0;o->sourceRect.right=sc.whole?sc.width:sc.width-1;o->sourceRect.bottom=sc.rows;
 int first=sc.mode!=0&&sc.mode!=4,rows=used+first;if(rows>6)rows=6;
 if(first)strcpy(o->drawnRowText[0],sc.resource&&sc.status?"Time left":"Resource String Unavailable");
 for(int row=first;row<rows;row++){int rank=row-first;if(row==rows-1&&ownRank>=rank)rank=ownRank;Line(o->drawnRowText[row],rank+1,order[rank]);}
 if(!sc.cached)memset(o->drawnRowText,0,sizeof(o->drawnRowText));
 for(int rr=0;rr<8;rr++){o->rowRects[rr].left=rr*3-17;o->rowRects[rr].right=180-rr*2;o->rowRects[rr].top=rr*13-21;o->rowRects[rr].bottom=rr*13-8;}
 for(int t=0;t<3;t++){PCTextureMap*map=Texture(t);At(map,0)=(DWORD)textureTable;map->textureWidth=sc.width;map->textureHeight=128;map->mapFormat=sc.format;map->systemSurface=(UnknownSurfaceInterface*)&w.surfaces[t];map->textureSurface=(UnknownSurfaceInterface*)&w.surfaces[3];map->usesColorKey=sc.key;map->colorKeyValue=0x1234;}
 for(int n=0;n<6;n++){w.surfaces[n].table=(DWORD)surfaceTable;for(int k=0;k<4096;k++)w.pixels[n][k]=(BYTE)(17*k+29*n);}
 for(int g=0;g<2;g++)for(int k=0;k<4096;k++)guard[g][k]=(BYTE)(k*11+g*17);w.font=0x4444;memcpy(w.game[1],w.game[0],sizeof(TrackGame));((TrackGame*)w.game[1])->mode.raceSettings.raceMode=0;((TrackGame*)w.game[1])->mode.raceSettings.timeLimitMinutes=4.5f;
 switch(sc.fault){case 1:receiver=(StatsOverlay*)1;break;case 2:g_TrackGame=(TrackGame*)1;break;case 3:g_TrackGame->quarryEvent=(TrackGameViewOwner*)1;break;case 4:At(w.racers[0],4)=1;break;case 5:o->sharedTexture=(TextureMap*)1;break;case 6:o->overlayTexture=(TextureMap*)1;break;case 9:receiver=(StatsOverlay*)(guard[1]+4096-(0x5b0+sc.available));break;}
 Sync();Capture();initial=w;_clearfp();_controlfp(sc.control,_MCW_EM|_MCW_PC|_MCW_RC);
}
static int Filter(EXCEPTION_POINTERS*p){got.exception=p->ExceptionRecord->ExceptionCode;if(got.exception==EXCEPTION_ACCESS_VIOLATION){got.access=p->ExceptionRecord->ExceptionInformation[0];got.address=p->ExceptionRecord->ExceptionInformation[1];}return EXCEPTION_EXECUTE_HANDLER;}
static void Run(int retail){Setup();__try{if(retail){typedef int(StatsOverlay::*Fn)();union{Fn p;DWORD n;}u;u.n=0x519a20;if(sizeof(Fn)!=4)ExitProcess(90);got.result=(receiver->*u.p)();}else got.result=receiver->DrawStandings();got.complete=1;}__except(Filter(GetExceptionInformation())){}got.fpStatus=_statusfp();got.fpControl=_controlfp(0,0);Capture();got.world=w;}
static void Check(){Run(1);want=got;Run(0);checks++;if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])at++;printf("Mismatch%d at%d count%d own%d mode%d cached%d failure%x/%x mutation%d@%d fault%d complete%d/%d exception%x/%x\n",checks,at,sc.count,sc.own,sc.mode,sc.cached,sc.lockFail,sc.unlockFail,sc.mutation,sc.mutateAt,sc.fault,want.complete,got.complete,want.exception,got.exception);ExitProcess(2);}if(got.exception==EXCEPTION_ACCESS_VIOLATION)avs++;if(got.exception==0xe051abcd)throws++;}
static void Success(){if(!got.complete||got.exception)ExitProcess(91);}
static void RowOracle(){if(sc.tie||sc.scoreVariant||sc.mutateAt||sc.fault||sc.throwAt)return;int order[12],used=0;for(int i=0;i<sc.count;i++)if(i!=sc.disabled)order[used++]=i;if(sc.reverse)for(int j=0;j<used/2;j++){int t=order[j];order[j]=order[used-1-j];order[used-1-j]=t;}int own=0;while(order[own]!=sc.own)own++;int first=sc.mode!=0&&sc.mode!=4,rows=used+first;if(rows>6)rows=6;StatsOverlay*o=OverlayObject();char expected[128];for(int row=0;row<rows;row++){if(first&&row==0)strcpy(expected,sc.resource&&sc.status?"Time left":"Resource String Unavailable");else{int rank=row-first;if(row==rows-1&&own>=rank)rank=own;Line(expected,rank+1,order[rank]);}if(strcmp(expected,o->rowText[row]))ExitProcess(92);if(!sc.dcStatus&&strcmp(expected,o->drawnRowText[row]))ExitProcess(93);}if(sc.cached){if(got.count!=(first&&sc.resource?1:0)||got.result!=1)ExitProcess(94);}else{if(got.result!=!(sc.dcStatus||sc.releaseStatus))ExitProcess(95);if(!sc.dcStatus&&w.font!=0x4444)ExitProcess(96);if(!sc.dcStatus&&w.draws!=rows)ExitProcess(97);if(!sc.dcStatus){int draw=0,selects=0;for(int e=0;e<got.count;e++){Event&v=got.events[e];if(v.kind==10){if(v.a!=0x12340002||v.b!=0x120||memcmp(&v.rect,&((StatsOverlay*)initial.overlay)->rowRects[draw],16)||strcmp(v.text,o->rowText[draw]))ExitProcess(118);draw++;}if(v.kind==8){if(v.a!=0x12340002||v.b!=(selects?0x4444:0x2222)||v.c!=(selects?0x2222:0x4444))ExitProcess(119);selects++;}if(v.kind==11&&v.a!=0x12340002)ExitProcess(120);}if(draw!=rows||selects!=2||o->rowExtent.cx!=(int)strlen(o->rowText[0])*7||o->rowExtent.cy!=13)ExitProcess(121);}}}
static void PixelOracle(){
 if(sc.cached||sc.mutateAt||sc.throwAt||sc.fault||sc.alias||sc.lockFail||sc.unlockFail)return;
 BYTE expected[6][4096];memcpy(expected,initial.pixels,sizeof(expected));int bytes=2*(sc.whole?sc.width*sc.rows:sc.width-1);
 if(sc.whole){for(int x=0;x<bytes;x++)expected[1][1024+x]=expected[0][1024+x];}else for(int y=0;y<sc.rows;y++)for(int x=0;x<bytes;x++)expected[1][1024+y*sc.pitch+x]=expected[0][1024+y*sc.pitch+x];
 if(!sc.dcStatus){for(int row=0;row<w.draws;row++){expected[2][1024+row*sc.pitch]=0xff;expected[2][1024+row*sc.pitch+1]=0x7f;}}
 if(!sc.dcStatus&&!sc.releaseStatus){if(sc.format==4444||sc.format==1555){for(int y=0;y<128;y++)for(int x=0;x<sc.width;x++){int at=1024+y*sc.pitch+x*2;unsigned v=expected[2][at]|expected[2][at+1]<<8;if((v&0x7fff)==0x7fff){unsigned color=sc.format==4444?0xffff:0xfbde;expected[1][at]=(BYTE)color;expected[1][at+1]=(BYTE)(color>>8);expected[2][at]=expected[2][at+1]=0;}}}
 if(!sc.bltFail){for(int y=0;y<128;y++)for(int x=0;x<sc.width*2;x++)expected[3][1024+y*sc.pitch+x]=expected[1][1024+y*sc.pitch+x];if(sc.levels)for(int y=0;y<64;y++)for(int x=0;x<(sc.width/2)*2;x++)expected[5][1024+y*sc.pitch+x]=expected[4][1024+y*sc.pitch+x];}}
 if(memcmp(expected,w.pixels,sizeof(expected)))ExitProcess(98);
 if(!sc.mutateAt){for(int i=0;i<6;i++)if(w.surfaces[i].held)ExitProcess(99);if(w.surfaces[2].dcHeld!=(!sc.dcStatus&&!!sc.releaseStatus))ExitProcess(100);}
}

static int MapReference(const char*path){
    BYTE*mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000||(DWORD)referenceSpace+sizeof(referenceSpace)<0x690000||!VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old))return 0;
    FILE*f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<(long)sizeof(IMAGE_DOS_HEADER))return 0;BYTE*raw=(BYTE*)malloc(size);if(!raw||fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER*dos=(IMAGE_DOS_HEADER*)raw;if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>size-(long)sizeof(IMAGE_NT_HEADERS))return 0;
    IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||pe->OptionalHeader.ImageBase!=0x400000||pe->OptionalHeader.SizeOfImage>0x290000||pe->OptionalHeader.SizeOfHeaders>(DWORD)size||pe->OptionalHeader.SizeOfHeaders>pe->OptionalHeader.SizeOfImage)return 0;
    DWORD offset=(BYTE*)IMAGE_FIRST_SECTION(pe)-raw;if(offset>(DWORD)size||pe->FileHeader.NumberOfSections>((DWORD)size-offset)/sizeof(IMAGE_SECTION_HEADER))return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER*section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;i++){if(section[i].PointerToRawData>(DWORD)size||section[i].SizeOfRawData>(DWORD)size-section[i].PointerToRawData||section[i].VirtualAddress>pe->OptionalHeader.SizeOfImage||section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage-section[i].VirtualAddress)return 0;memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);}
    free(raw);return 1;
}
static void PatchCall(DWORD site,DWORD expected,DWORD target){if(*(BYTE*)site!=0xe8||site+5+*(DWORD*)(site+1)!=expected)ExitProcess(85);*(DWORD*)(site+1)=target-site-5;}

static void RedirectImport(DWORD slot,const char*name,DWORD target){BYTE*base=(BYTE*)GetModuleHandle(0);IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(base+((IMAGE_DOS_HEADER*)base)->e_lfanew);IMAGE_IMPORT_DESCRIPTOR*imports=(IMAGE_IMPORT_DESCRIPTOR*)(base+pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);int found=0;DWORD old;for(;imports->Name;imports++){DWORD*names=(DWORD*)(base+imports->OriginalFirstThunk),*iat=(DWORD*)(base+imports->FirstThunk);for(int i=0;names[i];i++)if(!(names[i]&0x80000000)&&!strcmp((char*)base+names[i]+2,name)){if(!VirtualProtect(iat+i,4,PAGE_READWRITE,&old))ExitProcess(101);iat[i]=target;found++;}}if(found!=1)ExitProcess(102);if(slot)*(DWORD*)slot=target;}

static void CrtProof(){
 BYTE*a=(BYTE*)&qsort;DWORD sh=(DWORD)(a+91)+*(DWORD*)(a+87),sw=(DWORD)(a+142)+*(DWORD*)(a+138);
 for(int i=0;i<340;i++)if(!((i>=87&&i<91)||(i>=138&&i<142)||(i>=205&&i<209)||(i>=216&&i<220))&&a[i]!=((BYTE*)0x534426)[i])ExitProcess(124);
 if(a[86]!=0xe8||a[137]!=0xe8||a[204]!=0xe8||a[215]!=0xe8||(DWORD)(a+209)+*(DWORD*)(a+205)!=sw||(DWORD)(a+220)+*(DWORD*)(a+216)!=sw)ExitProcess(125);
 BYTE*b=(BYTE*)sh;for(i=0;i<78;i++)if(!(i>=58&&i<62)&&b[i]!=((BYTE*)0x53457a)[i])ExitProcess(126);
 if(b[57]!=0xe8||(DWORD)(b+62)+*(DWORD*)(b+58)!=sw||memcmp((void*)sw,(void*)0x5345c8,44)||memcmp((void*)&_ftol,(void*)0x534a88,39))ExitProcess(127);
 a=(BYTE*)&sprintf;BYTE*r=(BYTE*)0x534aaf;for(i=0;i<82;i++)if(!(i>=42&&i<46)&&!(i>=71&&i<75)&&a[i]!=r[i])ExitProcess(128);
 if(r[41]!=0xe8||r[70]!=0xe8||(DWORD)(r+46)+*(DWORD*)(r+42)!=0x539359||(DWORD)(r+75)+*(DWORD*)(r+71)!=0x539241)ExitProcess(129);
 if(a[41]!=0xe8||a[70]!=0xe8||(DWORD)(a+46)+*(DWORD*)(a+42)!=(DWORD)&_output||(DWORD)(a+75)+*(DWORD*)(a+71)!=(DWORD)&_flsbuf)ExitProcess(130);
}

int main(int argc,char**argv){setvbuf(stdout,0,_IONBF,0);if(argc!=2||!MapReference(argv[1])||!setlocale(LC_ALL,"C"))return 1;CrtProof();
 for(int i=0;i<2;i++){guard[i]=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;if(!guard[i]||!VirtualProtect(guard[i]+4096,4096,PAGE_NOACCESS,&old))return 1;}
 textureTable[9]=Address(&TextureCalls::Upload);textureTable[13]=Address(&TextureCalls::Lock);textureTable[14]=Address(&TextureCalls::Unlock);
 surfaceTable[7]=(DWORD)&Blt;surfaceTable[12]=(DWORD)&Attached;surfaceTable[17]=(DWORD)&GetDc;surfaceTable[25]=(DWORD)&Lock;surfaceTable[26]=(DWORD)&ReleaseDc;surfaceTable[29]=(DWORD)&Key;surfaceTable[32]=(DWORD)&Unlock;
 PatchCall(0x519a68,0x521970,Address(&TrackGame::LoadResourceString));PatchCall(0x519a9f,0x518640,(DWORD)&FormatMinutesSeconds);PatchCall(0x519ab8,0x534aaf,(DWORD)&sprintf);PatchCall(0x519bb2,0x534aaf,(DWORD)&sprintf);PatchCall(0x519c33,0x534aaf,(DWORD)&sprintf);PatchCall(0x519acb,0x4204e0,Address(&BikeRace::NextBike));PatchCall(0x519afb,0x4204e0,Address(&BikeRace::NextBike));PatchCall(0x519b11,0x534426,(DWORD)&qsort);PatchCall(0x519ca7,0x4b6710,Address(&Overlay::RestoreRect));PatchCall(0x519dec,0x4b6880,Address(&Overlay::TintRows));
 RedirectImport(0x550070,"SetBkColor",(DWORD)&Bk);RedirectImport(0x550074,"SetBkMode",(DWORD)&Mode);RedirectImport(0x550080,"SetTextColor",(DWORD)&Color);RedirectImport(0x550078,"SelectObject",(DWORD)&Select);RedirectImport(0x550084,"GetTextExtentPoint32A",(DWORD)&Measure);RedirectImport(0x5502e8,"DrawTextA",(DWORD)&Draw);RedirectImport(0,"LoadStringA",(DWORD)&Load);
 Defaults();for(sc.count=1;sc.count<=11;sc.count++)for(sc.own=0;sc.own<sc.count;sc.own++)for(sc.reverse=0;sc.reverse<2;sc.reverse++)for(sc.disabled=-1;sc.disabled<sc.count;sc.disabled++)if(sc.disabled!=sc.own)for(sc.cached=0;sc.cached<2;sc.cached++){Check();Success();RowOracle();PixelOracle();}
 Defaults();int modes[]={0,1,2,3,4,5,-1};for(int m=0;m<7;m++)for(sc.resource=0;sc.resource<2;sc.resource++)for(sc.status=0;sc.status<2;sc.status++)for(sc.cached=0;sc.cached<2;sc.cached++){sc.mode=modes[m];Check();Success();RowOracle();PixelOracle();}
 Defaults();int formats[]={4444,1555,565},pitches[]={8,16,-8},statuses[]={0,1,(int)0x80004005};for(int f=0;f<3;f++)for(int p=0;p<3;p++)for(sc.whole=0;sc.whole<2;sc.whole++)for(sc.rows=0;sc.rows<=4;sc.rows++){sc.format=formats[f];sc.pitch=pitches[p];Check();Success();RowOracle();PixelOracle();}
 Defaults();for(int d=0;d<3;d++)for(int r=0;r<3;r++)for(sc.bltFail=0;sc.bltFail<2;sc.bltFail++)for(sc.keyFail=0;sc.keyFail<2;sc.keyFail++)for(sc.levels=0;sc.levels<2;sc.levels++){sc.dcStatus=statuses[d];sc.releaseStatus=statuses[r];sc.key=1;Check();Success();RowOracle();PixelOracle();}
 Defaults();for(sc.lockFail=0;sc.lockFail<16;sc.lockFail++)for(sc.unlockFail=0;sc.unlockFail<16;sc.unlockFail++){Check();Success();RowOracle();}
 Defaults();for(sc.alias=1;sc.alias<=2;sc.alias++)for(sc.whole=0;sc.whole<2;sc.whole++){Check();Success();RowOracle();}
 Defaults();for(sc.tie=0;sc.tie<2;sc.tie++)for(sc.scoreVariant=0;sc.scoreVariant<=6;sc.scoreVariant++)for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++){unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};sc.control=_MCW_EM|pcs[pc]|rcs[rc];Check();Success();}
 Defaults();sc.mode=2;Check();int boundaries=got.count;
 for(sc.mutateAt=1;sc.mutateAt<=boundaries;sc.mutateAt++)for(sc.mutation=0;sc.mutation<15;sc.mutation++){Check();if(got.count<sc.mutateAt)ExitProcess(103);if(sc.mutation==0&&sc.mutateAt==1&&(w.draws!=5||strcmp(OverlayObject()->rowText[0],"1) R0 : 50")))ExitProcess(131);if(sc.mutation<13)Success();else{int fails=sc.mutation==13?sc.mutateAt<=22:sc.mutateAt<=23;if(fails){DWORD address=sc.mutation==13?(sc.mutateAt<=5?0x71:1):(sc.mutateAt==1?0x15:sc.mutateAt==21?0x21:1);if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.access||got.address!=address||w.font!=0x4444||w.surfaces[2].dcHeld)ExitProcess(104);}else{Success();if(got.result!=1)ExitProcess(104);}}}
 Defaults();sc.mode=2;for(sc.throwAt=1;sc.throwAt<=boundaries;sc.throwAt++){Check();if(got.complete||got.exception!=0xe051abcd||got.count!=sc.throwAt)ExitProcess(105);if(w.font!=(DWORD)(sc.throwAt>=10&&sc.throwAt<=17?0x2222:0x4444)||w.surfaces[2].dcHeld!=(sc.throwAt>=6&&sc.throwAt<=18))ExitProcess(122);int held0=sc.throwAt>=2&&sc.throwAt<=3,held1=sc.throwAt==3||sc.throwAt==4||sc.throwAt==21,held2=sc.throwAt>=20&&sc.throwAt<=22;if(w.surfaces[0].held!=held0||w.surfaces[1].held!=held1||w.surfaces[2].held!=held2)ExitProcess(123);}
 Defaults();for(sc.fault=1;sc.fault<=6;sc.fault++){Check();DWORD addresses[]={0,0x5b1,0x55d,0x35,5,0x71,0x15};if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.access!=(DWORD)(sc.fault==1)||got.address!=addresses[sc.fault]){printf("Fault oracle %d got%x:%x\n",sc.fault,got.access,got.address);ExitProcess(106);}}
 Defaults();sc.rows=1;sc.width=4;for(sc.fault=7;sc.fault<=8;sc.fault++)for(sc.available=0;sc.available<8;sc.available++){Check();if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.address!=(DWORD)(guard[0]+4096)||got.access!=(DWORD)(sc.fault==7))ExitProcess(107);int copied=sc.available&~3;if(w.surfaces[0].held!=1||w.surfaces[1].held!=1||w.unlocks||w.draws)ExitProcess(108);if(sc.fault==7){for(int k=0;k<sc.available;k++){BYTE expected=k<copied?initial.pixels[0][1024+k]:(BYTE)((4096-sc.available+k)*11);if(guard[0][4096-sc.available+k]!=expected)ExitProcess(109);}}else for(int k=0;k<8;k++){BYTE expected=k<copied?(BYTE)((4096-sc.available+k)*11):initial.pixels[1][1024+k];if(w.pixels[1][1024+k]!=expected)ExitProcess(110);}}
 Defaults();sc.fault=9;for(sc.available=0;sc.available<=10;sc.available++){Check();if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.access!=1||got.address!=(DWORD)(guard[1]+4096)||got.count||w.locks)ExitProcess(111);if(memcmp(guard[1]+4096-sc.available,"1) R0 : 50",sc.available))ExitProcess(112);}
 Defaults();sc.mode=2;for(sc.timeCase=1;sc.timeCase<=9;sc.timeCase++)for(int pc2=0;pc2<3;pc2++)for(int rc2=0;rc2<4;rc2++){unsigned pcs[]={_PC_24,_PC_53,_PC_64},rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};sc.control=_MCW_EM|pcs[pc2]|rcs[rc2];Check();Success();RowOracle();PixelOracle();}
 printf("Stats DrawStandings: %d cases match, %d AV, %d throws\n",checks,avs,throws);return 0;
}
