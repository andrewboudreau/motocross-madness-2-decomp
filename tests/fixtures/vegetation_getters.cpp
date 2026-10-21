// Private validation: unchanged strict getters on prepared raw storage.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include "EcoSystem.h"

typedef char Layout[(sizeof(UnknownEcoQuadTreeObject)==0xc &&
    sizeof(UnknownEcoCoordinates)==6 && sizeof(Vegetation)==0x1c &&
    offsetof(Vegetation,quantizedPosition)==0xc &&
    offsetof(Vegetation,definitionIndex)==0x12 && offsetof(Vegetation,radiusParam)==0x15 &&
    offsetof(EcoSystem,definitionTable)==0x58 && offsetof(EcoSystem,vegetationTypeId)==0x599 &&
    offsetof(EcoSystem,unitsPerCoordinate)==0x5a8 && sizeof(EcoSystem)==0x5c4 &&
    sizeof(UnknownEcoDefinition)==0x210 && offsetof(UnknownEcoDefinition,collisionCount)==0x204 &&
    offsetof(UnknownEcoDefinition,meanRadius)==0x18c &&
    offsetof(UnknownEcoDefinition,minRadius)==0x190 &&
    offsetof(UnknownEcoDefinition,maxRadius)==0x194) ? 1 : -1];
static BYTE *ecoPage,*vegetationPage,*guardDefinitionPage;
static BYTE definitions[256][sizeof(UnknownEcoDefinition)];
static BYTE initialDefinitions[sizeof(definitions)],initialPages[12288];
static EcoSystem* eco;
static Vegetation* vegetation;
static DWORD checks,faults,expectedGlobal;
struct Result { DWORD complete,bits,exception,address,access; };
static Result result;
static void Fail(int code) { printf("failure %d at %lu\n",code,checks); ExitProcess(code); }
static void Snapshot() {
    memcpy(initialPages,ecoPage,4096);memcpy(initialPages+4096,vegetationPage,4096);
    memcpy(initialPages+8192,guardDefinitionPage,4096);
    expectedGlobal=(DWORD)g_EcoSystem;
}
static void NoWrites() {
    if ((DWORD)g_EcoSystem!=expectedGlobal || memcmp(initialPages,ecoPage,4096) ||
        memcmp(initialPages+4096,vegetationPage,4096) ||
        memcmp(initialPages+8192,guardDefinitionPage,4096) ||
        memcmp(initialDefinitions,definitions,sizeof(definitions))) Fail(20);
}
static int Filter(EXCEPTION_POINTERS* p) {
    if (p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;
    result.exception=p->ExceptionRecord->ExceptionCode;
    result.address=p->ExceptionRecord->ExceptionInformation[1];
    result.access=p->ExceptionRecord->ExceptionInformation[0];
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Run(int radius,Vegetation* self) {
    memset(&result,0,sizeof(result));
    __try {
        if (radius) { float value=self->GetRadius(); memcpy(&result.bits,&value,4); }
        else result.bits=(DWORD)self->GetCollisionCount();
        result.complete=1;
    } __except(Filter(GetExceptionInformation())) {}
    ++checks;
    NoWrites();
}
static void Success(int radius,DWORD bits) {
    Snapshot();Run(radius,vegetation);
    if (!result.complete || result.exception || result.bits!=bits) Fail(21);
}
static void Fault(int radius,Vegetation* self,DWORD address) {
    Snapshot();Run(radius,self);++faults;
    if (result.complete || result.exception!=EXCEPTION_ACCESS_VIOLATION ||
        result.access!=0 || result.address!=address) Fail(22);
    // A guarded second x87 operand can fault after FLD; this is test cleanup.
    _fpreset();
    _control87(_PC_53|_RC_NEAR|_MCW_EM,_MCW_PC|_MCW_RC|_MCW_EM);
}
static void Setup() {
    memset(ecoPage,0xa5,4096);memset(vegetationPage,0x5a,4096);
    memset(guardDefinitionPage,0,4096);
    eco=(EcoSystem*)(ecoPage+128);vegetation=(Vegetation*)(vegetationPage+128);
    g_EcoSystem=eco;
    for (int i=0;i<256;++i) eco->definitionTable[i]=(UnknownEcoDefinition*)definitions[i];
    vegetation->definitionIndex=0;vegetation->radiusParam=0;
}
int main() {
    if (g_EcoSystem) return 1; // Actual source definition has zero-initialized storage.
    BYTE** pages[]={&ecoPage,&vegetationPage,&guardDefinitionPage};DWORD old;
    for(int p=0;p<3;++p) {
        *pages[p]=(BYTE*)VirtualAlloc(0,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
        if(!*pages[p]||!VirtualProtect(*pages[p]+4096,4096,PAGE_NOACCESS,&old)) return 2;
    }
    _control87(_PC_53|_RC_NEAR|_MCW_EM,_MCW_PC|_MCW_RC|_MCW_EM);
    memset(definitions,0x69,sizeof(definitions));
    const DWORD counts[]={0,1,0xffffffff,0x7fffffff,0x80000000,0x12345678,0x80000001,0x2468ace0};
    for(int i=0;i<256;++i) {
        UnknownEcoDefinition* d=(UnknownEcoDefinition*)definitions[i];
        int mean=i%31-15;
        d->meanRadius=(float)mean;
        d->minRadius=(float)(mean-(i%7+1));
        d->maxRadius=(float)(mean+(i%11+1));
        memcpy(&d->collisionCount,&counts[i%8],4);
    }
    memcpy(initialDefinitions,definitions,sizeof(definitions));
    Setup();
    for(int index=0;index<256;++index) {
        vegetation->definitionIndex=(unsigned char)index;
        Success(0,counts[index%8]);
        for(int parameter=0;parameter<256;++parameter) {
            vegetation->radiusParam=(unsigned char)parameter;
            // Independent integer numerator; all these rational results are exact binary floats.
            int numerator=128*(index%31-15);
            if(parameter<128) numerator-=parameter*(index%7+1);
            else numerator+=(255-parameter)*(index%11+1);
            float value=(float)((double)numerator/128.0);DWORD bits;memcpy(&bits,&value,4);
            Success(1,bits);
        }
    }
    Setup();Fault(0,0,0x12);
    Setup();Fault(1,0,0x15);
    Setup();g_EcoSystem=0;Fault(0,vegetation,0x58);
    Setup();g_EcoSystem=0;vegetation->definitionIndex=255;Fault(1,vegetation,0x454);
    Setup();eco->definitionTable[0]=0;Fault(0,vegetation,0x204);
    Setup();eco->definitionTable[0]=0;Fault(1,vegetation,0x18c);
    Setup();eco->definitionTable[0]=0;vegetation->radiusParam=128;Fault(1,vegetation,0x194);
    Setup();Fault(0,(Vegetation*)(vegetationPage+4096-0x12),(DWORD)(vegetationPage+4096));
    Setup();Fault(1,(Vegetation*)(vegetationPage+4096-0x15),(DWORD)(vegetationPage+4096));
    Setup();g_EcoSystem=(EcoSystem*)(ecoPage+4096-0x58);Fault(0,vegetation,(DWORD)(ecoPage+4096));
    Setup();g_EcoSystem=(EcoSystem*)(ecoPage+4096-0x58);Fault(1,vegetation,(DWORD)(ecoPage+4096));
    Setup();eco->definitionTable[0]=(UnknownEcoDefinition*)(guardDefinitionPage+4096-0x204);Fault(0,vegetation,(DWORD)(guardDefinitionPage+4096));
    Setup();eco->definitionTable[0]=(UnknownEcoDefinition*)(guardDefinitionPage+4096-0x18c);Fault(1,vegetation,(DWORD)(guardDefinitionPage+4096));
    Setup();eco->definitionTable[0]=(UnknownEcoDefinition*)(guardDefinitionPage+4096-0x194);vegetation->radiusParam=128;Fault(1,vegetation,(DWORD)(guardDefinitionPage+4096));
    Setup();eco->definitionTable[0]=(UnknownEcoDefinition*)(guardDefinitionPage+4096-0x190);Fault(1,vegetation,(DWORD)(guardDefinitionPage+4096));
    if(checks!=65807 || faults!=15) return 3;
    printf("Vegetation getters: %lu cases, %lu independent read AV\n",checks,faults);
    return 0;
}
