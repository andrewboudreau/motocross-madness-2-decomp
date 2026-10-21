// Validation only: complete name-tag method, actual native CRT, scripted GDI.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <locale.h>
#include <stddef.h>
#include "TrackOverlay.h"
#include "TrackGame.h"
TrackGame* g_TrackGame;
extern int s_UnknownGlobal68a444;
extern "C" int __cdecl _output();
static BYTE referenceSpace[0x800000]={1};
struct World {
    BYTE object[sizeof(ChatOverlay)],tags[3][sizeof(NameOverlay)];
    BYTE racers[2][sizeof(UnknownChatRacer)],game[2][sizeof(TrackGame)];
    BYTE guards[2][4096];
    DWORD selectedFont,gamePointer;int debugFlag;
};
struct Event {
    int kind,length,rectKind,status;DWORD dc,object,flags;
    char text[128];RECT rect;SIZE size;World before,after;
};
struct Result {World world;Event events[8];int count,complete;DWORD exception,kind,address;unsigned fpStatus,fpControl;};
struct Scenario {
    int cue,count,index,names,positions,position,debug,quarter,nameLength,cueLength;
    int sourceWidth,drawWidth,measure,left,top,right,bottom,selectStatus,apiStatus,drawMutate;
    int mutation,mutateAt,throwAt,fault,available,alias;unsigned control,rawFloat;
};
static World w;static Scenario sc;static Result got,want;
static BYTE*guard[2];static int checks,accessFaults,throws;static BYTE*receiver;
static DWORD& At(void*p,int offset){return *(DWORD*)((BYTE*)p+offset);}
static ChatOverlay* Chat(){return (ChatOverlay*)receiver;}
static NameOverlay* Tag(int n){return (NameOverlay*)w.tags[n];}
static void Capture(){
    for(int i=0;i<2;i++)memcpy(w.guards[i],guard[i],4096);
    w.gamePointer=(DWORD)g_TrackGame;w.debugFlag=s_UnknownGlobal68a444;
}
static void Sync(){*(DWORD*)0x56e26c=(DWORD)g_TrackGame;*(int*)0x68a444=s_UnknownGlobal68a444;}
static Event& Begin(int kind){if(got.count==8)ExitProcess(80);Capture();Event&e=got.events[got.count++];e.kind=kind;e.before=w;return e;}
static void Finish(Event&e){Capture();e.after=w;}
static void Boundary(RECT*rect){
    if(sc.mutateAt==got.count){
        ChatOverlay*c=(ChatOverlay*)w.object;
        switch(sc.mutation){
        case 0:c->racerCount=sc.cue?sc.count+1:sc.index;break;
        case 1:c->nameTags[sc.index]=Tag(1);break;
        case 2:c->racers[sc.index]=(UnknownChatRacer*)w.racers[1];break;
        case 3:c->nameTagRects[sc.index].left=-17;c->nameTagRects[sc.index].right=101;break;
        case 4:Tag(0)->drawWidth=23;Tag(0)->sourceWidth=27;break;
        case 5:c->nameTagFont=(void*)0x8888;break;
        case 6:g_TrackGame=(TrackGame*)w.game[1];break;
        case 7:g_TrackGame->mode.showRiderNames^=1;g_TrackGame->mode.showRiderPositions^=1;break;
        case 8:s_UnknownGlobal68a444^=1;break;
        case 9:memcpy(c->cueRacerName,"mutated cue",12);break;
        case 10:if(rect){rect->left=-31;rect->right=119;rect->top=13;}break;
        case 11:c->nameTags[sc.index]=(NameOverlay*)1;break;
        case 12:c->racers[sc.index]=(UnknownChatRacer*)1;break;
        case 13:Tag(0)->sourceRect.left=57;Tag(0)->sourceRect.right=-9;break;
        case 14:w.selectedFont=0x7777;break;
        }
        Sync();
    }
    if(sc.fault==8&&got.count==3)((ChatOverlay*)w.object)->nameTags[sc.index]=(NameOverlay*)(guard[0]+4096-(0xf8+sc.available));
    if(sc.throwAt==got.count)RaiseException(0xe050cafe,0,0,0);
}
static void CopyText(Event&e,const char*text,int n){if(n<0||n>=128)ExitProcess(81);e.length=n;memcpy(e.text,text,n);e.text[n]=0;}
static HGDIOBJ WINAPI Select(HDC dc,HGDIOBJ object){
    Event&e=Begin(1);e.dc=(DWORD)dc;e.object=(DWORD)object;DWORD previous=w.selectedFont;
    if(!sc.selectStatus)w.selectedFont=(DWORD)object;
    e.status=sc.selectStatus;Boundary(0);Finish(e);
    return sc.selectStatus?(HGDIOBJ)(sc.selectStatus==1?0:0xffffffff):(HGDIOBJ)previous;
}
static int WINAPI Draw(HDC dc,LPCSTR text,int n,LPRECT rect,UINT flags){
    Event&e=Begin(2);e.dc=(DWORD)dc;e.flags=flags;CopyText(e,text,n);e.rect=*rect;
    e.rectKind=((BYTE*)rect>=w.object&&(BYTE*)rect+16<=w.object+sizeof(w.object))?2:1;
    if(sc.drawMutate){rect->left+=7;rect->right-=3;rect->bottom+=2;}
    e.status=sc.apiStatus;Boundary(rect);Finish(e);return sc.apiStatus;
}
static BOOL WINAPI Measure(HDC dc,LPCSTR text,int n,LPSIZE size){
    Event&e=Begin(3);e.dc=(DWORD)dc;CopyText(e,text,n);size->cx=sc.measure;size->cy=19;
    e.size=*size;e.status=sc.apiStatus;Boundary(0);Finish(e);return sc.apiStatus;
}
static int Filter(EXCEPTION_POINTERS*e){
    DWORD code=e->ExceptionRecord->ExceptionCode;
    if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=0xe050cafe)return EXCEPTION_CONTINUE_SEARCH;
    got.exception=code;if(code==EXCEPTION_ACCESS_VIOLATION){got.kind=e->ExceptionRecord->ExceptionInformation[0];got.address=e->ExceptionRecord->ExceptionInformation[1];}
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Defaults(){
    memset(&sc,0,sizeof(sc));sc.count=3;sc.index=1;sc.names=sc.positions=1;sc.position=4;
    sc.nameLength=7;sc.cueLength=9;sc.sourceWidth=80;sc.drawWidth=11;sc.measure=24;
    sc.left=17;sc.top=-3;sc.right=95;sc.bottom=8;sc.apiStatus=1;sc.quarter=-7;
    sc.control=_MCW_EM|_PC_64|_RC_NEAR;
}
static void Setup(){
    memset(&w,0,sizeof(w));memset(&got,0,sizeof(got));receiver=w.object;
    for(int i=0;i<2;i++){memset(guard[i],0xa5,4096);}
    ChatOverlay*c=(ChatOverlay*)w.object;c->racerCount=sc.count;c->nameTagFont=(void*)0x4567;
    for(i=0;i<13;i++){
        c->nameTags[i]=Tag(0);c->nameTagRects[i].left=sc.left;c->nameTagRects[i].top=sc.top;c->nameTagRects[i].right=sc.right;c->nameTagRects[i].bottom=sc.bottom;
        if(i<11)c->racers[i]=(UnknownChatRacer*)w.racers[0];
    }
    for(i=0;i<3;i++){Tag(i)->sourceWidth=sc.sourceWidth+i*7;Tag(i)->drawWidth=sc.drawWidth+i*3;Tag(i)->sourceRect.left=101+i;Tag(i)->sourceRect.top=102+i;Tag(i)->sourceRect.right=103+i;Tag(i)->sourceRect.bottom=104+i;}
    for(i=0;i<2;i++){
        UnknownChatRacer*r=(UnknownChatRacer*)w.racers[i];r->position=sc.position+i;r->driveForceScale=(float)sc.quarter*0.25f;
        if(sc.rawFloat)At(r,0x4a4)=sc.rawFloat;
        for(int j=0;j<sc.nameLength;j++)r->name[j]=(char)('A'+(j+i)%26);r->name[sc.nameLength]=0;
        TrackGame*g=(TrackGame*)w.game[i];g->mode.showRiderNames=sc.names^i;g->mode.showRiderPositions=sc.positions^i;
    }
    for(i=0;i<sc.cueLength;i++)c->cueRacerName[i]=(char)('a'+i%26);c->cueRacerName[sc.cueLength]=0;
    w.selectedFont=0x1234;g_TrackGame=(TrackGame*)w.game[0];s_UnknownGlobal68a444=sc.debug;
    if(sc.alias==1)c->nameTags[sc.index]=(NameOverlay*)(w.object+0x200);
    if(sc.alias==2)c->nameTags[sc.index]=(NameOverlay*)(w.object+0x214);
    if(sc.alias==3)c->nameTags[sc.index]=(NameOverlay*)(w.object+0x208);
    if(sc.fault==1)c->nameTags[sc.index]=(NameOverlay*)1;
    if(sc.fault==2)c->racers[sc.index]=(UnknownChatRacer*)1;
    if(sc.fault==3)g_TrackGame=(TrackGame*)1;
    if(sc.fault==4)receiver=(BYTE*)1;
    if(sc.fault==5){
        BYTE*p=guard[0]+4096-(0x124+sc.available);memcpy(p,w.tags[0],0x124+sc.available);c->nameTags[sc.index]=(NameOverlay*)p;
    }
    if(sc.fault==6){
        BYTE*p=guard[0]+4096-(0x5e0+sc.available);memcpy(p,w.racers[0],0x5e0+sc.available);for(i=0;i<sc.available;i++)p[0x5e0+i]='R';c->racers[sc.index]=(UnknownChatRacer*)p;
    }
    if(sc.fault==7){
        BYTE*p=guard[0]+4096-(0x198+sc.available);memcpy(p,w.object,0x198+sc.available);receiver=p;
    }
    if(sc.fault==8){BYTE*p=guard[0]+4096-(0xf8+sc.available);memcpy(p,w.tags[0],0xf8+sc.available);}
    Sync();_clearfp();_controlfp(sc.control,_MCW_EM|_MCW_PC|_MCW_RC);
}
static void Run(int retail){
    Setup();__try{union{void(ChatOverlay::*p)(void*,int);DWORD address;}call;
        if(sizeof(call.p)!=4)ExitProcess(82);call.address=0x51e3f0;
        if(retail)(Chat()->*call.p)((void*)0x7654,sc.index);else Chat()->DrawNameTag((void*)0x7654,sc.index);
        got.complete=1;
    }__except(Filter(GetExceptionInformation())){}
    got.fpStatus=_statusfp();got.fpControl=_controlfp(0,0);_clearfp();_controlfp(_MCW_EM|_PC_64|_RC_NEAR,_MCW_EM|_MCW_PC|_MCW_RC);
    Capture();got.world=w;
}
static void Check(){
    Run(1);want=got;Run(0);checks++;
    if(got.exception==EXCEPTION_ACCESS_VIOLATION)accessFaults++;if(got.exception==0xe050cafe)throws++;
    if(memcmp(&want,&got,sizeof(got))){int at=0;while(((BYTE*)&want)[at]==((BYTE*)&got)[at])at++;printf("Mismatch %d at%d cue%d index%d count%d names%d positions%d dbg%d mutation%d/%d fault%d avail%d exception%x:%x/%x:%x\n",checks,at,sc.cue,sc.index,sc.count,sc.names,sc.positions,sc.debug,sc.mutation,sc.mutateAt,sc.fault,sc.available,want.kind,want.address,got.kind,got.address);ExitProcess(2);}
}
static void Success(){if(!got.complete||got.exception||got.count!=4)ExitProcess(3);}
static char* Decimal(char*out,unsigned n){char t[16];int count=0;do{t[count++]=(char)('0'+n%10);n/=10;}while(n);while(count)*out++=t[--count];*out=0;return out;}
static void TextOracle(char*out){
    char*p=out;*p=0;
    if(sc.cue){for(int i=0;i<sc.cueLength;i++)*p++=(char)('a'+i%26);*p=0;return;}
    if(sc.debug){int q=sc.quarter;if(q<0){*p++='-';q=-q;}p=Decimal(p,q/4);*p++='.';int fraction=(q%4)*250;*p++=(char)('0'+fraction/100);*p++=(char)('0'+fraction/10%10);*p++=(char)('0'+fraction%10);*p=0;return;}
    if(sc.positions){if(sc.position>0)p=Decimal(p,(unsigned)sc.position);else{*p++='-';*p++='-';}}
    if(sc.positions&&sc.names){*p++=' ';*p++=':';*p++=' ';}
    if(sc.names)for(int j=0;j<sc.nameLength;j++)*p++=(char)('A'+j%26);*p=0;
}
static int HalfFloor(int x){return x>=0?x/2:-1-(int)((~(unsigned)x)/2);}
static void Oracle(){
    Success();char text[128];TextOracle(text);int draw=sc.cue?1:2,measure=sc.cue?2:1;
    Event&d=got.events[draw];Event&m=got.events[measure];
    RECT rect={sc.left,sc.top,sc.right,sc.bottom};if(sc.cue)rect.left=(int)((unsigned)rect.left+sc.drawWidth);
    if(strcmp(d.text,text)||strcmp(m.text,text)||d.length!=(int)strlen(text)||m.length!=(int)strlen(text)||memcmp(&d.rect,&rect,16)||d.flags!=(sc.cue?0x8124:0x8125)||d.rectKind!=(sc.cue?1:2))ExitProcess(4);
    int width=sc.cue?(int)((unsigned)sc.drawWidth+sc.measure):sc.measure;
    RECT source={sc.left,sc.top,sc.right,sc.bottom};
    if(sc.drawMutate&&!sc.cue){source.left+=7;source.right-=3;source.bottom+=2;}
    if((int)((unsigned)width+4)<sc.sourceWidth){width=(int)((unsigned)width+4);if(sc.cue)source.right=(int)((unsigned)source.left+width-1);else{int margin=HalfFloor((int)((unsigned)sc.sourceWidth-width));source.left=(int)((unsigned)source.left+margin);source.right=(int)((unsigned)source.right+1-margin);}}
    else width=sc.sourceWidth;
    if(Tag(0)->drawWidth!=width||memcmp(&Tag(0)->sourceRect,&source,16))ExitProcess(5);
    DWORD saved=sc.selectStatus?(sc.selectStatus==1?0:0xffffffff):0x1234;
    if(got.events[0].object!=0x4567||got.events[3].object!=saved||w.selectedFont!=(sc.selectStatus?0x1234:saved))ExitProcess(6);
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
static void RedirectImport(DWORD slot,DWORD target){
    BYTE*base=(BYTE*)GetModuleHandle(0);IMAGE_NT_HEADERS*pe=(IMAGE_NT_HEADERS*)(base+((IMAGE_DOS_HEADER*)base)->e_lfanew);IMAGE_IMPORT_DESCRIPTOR*imports=(IMAGE_IMPORT_DESCRIPTOR*)(base+pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);const char*name=slot==0x550078?"SelectObject":slot==0x5502e8?"DrawTextA":"GetTextExtentPoint32A";int found=0;DWORD old;
    for(;imports->Name;imports++){DWORD*names=(DWORD*)(base+imports->OriginalFirstThunk),*iat=(DWORD*)(base+imports->FirstThunk);for(int i=0;names[i];i++)if(!(names[i]&0x80000000)&&!strcmp((char*)base+names[i]+2,name)){if(!VirtualProtect(iat+i,4,PAGE_READWRITE,&old))ExitProcess(86);iat[i]=target;found++;}}
    if(found!=1)ExitProcess(87);*(DWORD*)slot=target;
}
static void CrtProof(){
    BYTE*actual=(BYTE*)&sprintf;BYTE*retail=(BYTE*)0x534aaf;
    for(int i=0;i<82;i++)if(!(i>=42&&i<46)&&!(i>=71&&i<75)&&actual[i]!=retail[i])ExitProcess(88);
    if(retail[41]!=0xe8||retail[70]!=0xe8||(DWORD)(retail+46)+*(DWORD*)(retail+42)!=0x539359||(DWORD)(retail+75)+*(DWORD*)(retail+71)!=0x539241)ExitProcess(90);
    if(actual[41]!=0xe8||actual[70]!=0xe8||(DWORD)(actual+46)+*(DWORD*)(actual+42)!=(DWORD)&_output||(DWORD)(actual+75)+*(DWORD*)(actual+71)!=(DWORD)&_flsbuf)ExitProcess(89);
    PatchCall(0x51e439,0x534aaf,(DWORD)&sprintf);PatchCall(0x51e568,0x534aaf,(DWORD)&sprintf);PatchCall(0x51e5a1,0x534aaf,(DWORD)&sprintf);
}
int main(int argc,char**argv){
    if(s_UnknownGlobal68a444!=0)return 1;
    if(argc!=2||!MapReference(argv[1])||!setlocale(LC_ALL,"C"))return 1;CrtProof();
    RedirectImport(0x550078,(DWORD)&Select);RedirectImport(0x5502e8,(DWORD)&Draw);RedirectImport(0x550084,(DWORD)&Measure);
    for(int i=0;i<2;i++){guard[i]=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD old;if(!guard[i]||!VirtualProtect(guard[i]+4096,4096,PAGE_NOACCESS,&old))return 1;}
    if(!FlushInstructionCache(GetCurrentProcess(),0,0))return 1;
    const int positions[]={-2147483647-1,-1,0,1,2,10,2147483647};const int counts[]={0,1,3,10,11};
    for(int cue=0;cue<2;cue++)for(int c=0;c<5;c++)for(int names=0;names<2;names++)for(int pos=0;pos<2;pos++)for(int p=0;p<7;p++)for(int n=0;n<4;n++){
        if(!cue&&!counts[c])continue;Defaults();sc.cue=cue;sc.count=counts[c];sc.index=cue?sc.count:sc.count-1;sc.names=names;sc.positions=pos;sc.position=positions[p];sc.nameLength=n*5;sc.cueLength=n*29;Check();Oracle();
    }
    const int widths[]={-101,-5,-1,0,1,3,4,5,17,31,32,80,101,2147483647,-2147483647-1};
    for(cue=0;cue<2;cue++)for(int sw=0;sw<15;sw++)for(int width=0;width<15;width++)for(int prior=0;prior<5;prior++){
        Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.sourceWidth=widths[sw];sc.measure=widths[width];sc.drawWidth=prior*7-9;sc.left=prior*13-33;sc.right=widths[sw];Check();Oracle();
    }
    for(cue=0;cue<2;cue++)for(int select=0;select<3;select++)for(int api=0;api<3;api++){
        Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.selectStatus=select;sc.apiStatus=api==2?-1:api;Check();Oracle();
    }
    for(int q=-100;q<=100;q++){Defaults();sc.debug=1;sc.quarter=q;Check();Oracle();}
    for(cue=0;cue<2;cue++){Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.drawMutate=1;Check();Oracle();
        UnknownOverlayRect&rect=((ChatOverlay*)w.object)->nameTagRects[sc.index];
        if(rect.left!=sc.left+(cue?0:7)||rect.right!=sc.right-(cue?0:3)||rect.bottom!=sc.bottom+(cue?0:2))ExitProcess(15);
    }
    const unsigned pcs[]={_PC_24,_PC_53,_PC_64};const unsigned rcs[]={_RC_NEAR,_RC_DOWN,_RC_UP,_RC_CHOP};
    const unsigned floats[]={1,0x80000001,0x007fffff,0x00800000,0x3eaaaaab,0xbeaaaaab,0x3f800001,0xc0200001,0x4b800001,0xcb800001,0x7f7fffff,0xff7fffff};
    for(int pc=0;pc<3;pc++)for(int rc=0;rc<4;rc++)for(int f=0;f<12;f++){Defaults();sc.debug=1;sc.rawFloat=floats[f];sc.control=_MCW_EM|pcs[pc]|rcs[rc];Check();Success();}
    for(cue=0;cue<2;cue++)for(int alias=1;alias<=3;alias++){Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.alias=alias;Check();Success();}
    for(cue=0;cue<2;cue++)for(int mutate=0;mutate<15;mutate++)for(int at=1;at<=4;at++){
        Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.mutation=mutate;sc.mutateAt=at;Check();if(got.count<at)ExitProcess(7);
        if(mutate==11||mutate==12){
            int bad=mutate==11?at<4:(!cue&&at==1);
            if(bad){int expectedCount=mutate==12?1:(cue?(at==1?1:3):(at<=2?2:3));DWORD address=mutate==12?0x785:(!cue&&at==3?0xf9:0x125);DWORD access=mutate==12||cue?0:1;
                if(got.complete||got.exception!=EXCEPTION_ACCESS_VIOLATION||got.count!=expectedCount||got.address!=address||got.kind!=access||w.selectedFont!=0x4567)ExitProcess(13);
            }else Success();
        }else Success();
        if((mutate==5||mutate==14)&&at==2&&(got.events[3].object!=0x1234||w.selectedFont!=0x1234))ExitProcess(14);
    }
    for(cue=0;cue<2;cue++)for(int at=1;at<=4;at++){
        Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.throwAt=at;Check();if(got.exception!=0xe050cafe||got.count!=at||got.complete)ExitProcess(8);
        if(at<4&&w.selectedFont!=0x4567)ExitProcess(9);
    }
    for(cue=0;cue<2;cue++)for(int fault=1;fault<=7;fault++)for(int available=0;available<=4;available++){
        if(cue&&fault==2||cue&&fault==3||cue&&fault==6)continue;
        Defaults();sc.cue=cue;sc.index=cue?sc.count:1;sc.fault=fault;sc.available=available;if(fault==6)sc.positions=0;Check();
        if(fault==5&&available==4){Success();}else if(got.exception!=EXCEPTION_ACCESS_VIOLATION||got.complete){printf("Bad fault oracle%d cue%d available%d exception%x complete%d\n",fault,cue,available,got.exception,got.complete);ExitProcess(10);}
        if(fault<=4){DWORD address=fault==1?0x125:fault==2?0x785:fault==3?0xc39:0x125;DWORD access=fault==1&&!cue?1:0;if(got.address!=address||got.kind!=access||w.selectedFont!=(fault==4?0x1234:0x4567))ExitProcess(16);}
    }
    for(int available=0;available<=12;available+=4){
        Defaults();sc.fault=8;sc.available=available;Check();
        if(got.exception!=EXCEPTION_ACCESS_VIOLATION||got.kind!=1||got.address!=(DWORD)(guard[0]+4096)||got.count!=3||got.complete||w.selectedFont!=0x4567)ExitProcess(11);
        RECT expected={sc.left,sc.top,sc.right,sc.bottom};BYTE all[4096];memset(all,0xa5,sizeof(all));memset(all+4096-available-0xf8,0,0xf8);memcpy(all+4096-available,&expected,available);if(memcmp(guard[0],all,4096))ExitProcess(12);
    }
    unsigned random=0x5031cafe;
    for(i=0;i<2048;i++){
        Defaults();random=random*1664525+1013904223;sc.cue=(random>>31)&1;sc.index=sc.cue?sc.count:1;sc.names=(random>>5)&1;sc.positions=(random>>8)&1;sc.position=(int)(random%50)-20;
        random=random*1664525+1013904223;sc.measure=(int)(random%201)-100;sc.sourceWidth=(int)((random>>8)%201)-100;sc.drawWidth=(int)((random>>16)%71)-35;sc.nameLength=(random>>28)&15;sc.cueLength=(random>>19)&63;Check();Oracle();
    }
    if(accessFaults!=64||throws!=8)ExitProcess(17);
    printf("NameTagDraw: %d scenarios match, actual sprintf and scripted GDI; %d AV, %d throws\n",checks,accessFaults,throws);return 0;
}
