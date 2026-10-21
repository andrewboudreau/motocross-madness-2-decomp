// Validation only: actual out-of-line constructors and precise guarded stores.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <new>
#include "Parser.h"
#include "DirectoryList.h"
typedef char Layout[(sizeof(UnknownParserList)==4 && offsetof(UnknownParserList,head)==0 &&
    sizeof(UnknownParser)==8 && offsetof(UnknownParser,pairs)==4 &&
    sizeof(UnknownDriveList)==8 && offsetof(UnknownDriveList,drives)==0 &&
    offsetof(UnknownDriveList,count)==4) ? 1 : -1];
static BYTE* page;
static BYTE initial[4096],expected[4096];
static DWORD cases,faults;
struct Result { DWORD complete,returned,exception,address,access; };
static Result result;
static int Filter(EXCEPTION_POINTERS* p) {
    if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;
    result.exception=p->ExceptionRecord->ExceptionCode;
    result.address=p->ExceptionRecord->ExceptionInformation[1];
    result.access=p->ExceptionRecord->ExceptionInformation[0];
    return EXCEPTION_EXECUTE_HANDLER;
}
static void* Construct(int drive,void* storage) {
    if(drive)return new(storage) UnknownDriveList;
    return new(storage) UnknownParserList;
}
static void Run(int drive,void* storage) {
    memset(&result,0,sizeof(result));
    __try {
        result.returned=(DWORD)Construct(drive,storage);
        result.complete=1;
    } __except(Filter(GetExceptionInformation())) {}
}
static void Check(int drive,int offset,int prefix,DWORD fault) {
    memcpy(initial,page,4096);memcpy(expected,initial,4096);
    memset(expected+offset,0,prefix);
    Run(drive,page+offset);++cases;
    if(memcmp(page,expected,4096)) ExitProcess(10);
    if(fault) {
        ++faults;
        if(result.complete || result.returned || result.exception!=EXCEPTION_ACCESS_VIOLATION ||
            result.address!=fault || result.access!=1) ExitProcess(11);
    } else if(!result.complete || result.exception || result.returned!=(DWORD)(page+offset))
        ExitProcess(12);
}
static void Fill(int seed) {
    for(unsigned i=0;i<4096;++i)page[i]=(BYTE)(seed+i*37);
}
int main() {
    DWORD old;
    page=(BYTE*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!page || !VirtualProtect(page+4096,4096,PAGE_NOACCESS,&old))return 1;
    for(int seed=0;seed<64;++seed) {
        for(int offset=128;offset<136;++offset) {
            Fill(seed);Check(0,offset,4,0);
            Fill(seed);Check(1,offset,8,0);
        }
        // Aligned guard boundaries prove the two fields' store order without
        // assuming partial effects of an x86 store straddling a page boundary.
        Fill(seed);Check(0,4096,0,(DWORD)(page+4096));
        Fill(seed);Check(0,4092,4,0);
        Fill(seed);Check(1,4096,0,(DWORD)(page+4096));
        Fill(seed);Check(1,4092,4,(DWORD)(page+4096));
        Fill(seed);Check(1,4088,8,0);
    }
    if(cases!=1344 || faults!=192)return 2;
    printf("Startup list constructors: %lu cases, %lu exact write AV\n",cases,faults);
    return 0;
}
