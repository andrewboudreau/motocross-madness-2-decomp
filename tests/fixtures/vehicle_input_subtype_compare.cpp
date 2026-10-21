// Actual 17-byte InputDevice getter, prepared storage and bounded access faults.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "InputDevice.h"
struct InputLayout : InputDevice { static void Verify(); };
void InputLayout::Verify() {
    typedef char Verified[(sizeof(InputDevice)==0x18 && offsetof(InputLayout,deviceSubtype)==0x10 &&
        offsetof(InputLayout,deviceKind)==0xc && sizeof(int(InputDevice::*)(int))==4)?1:-1];
};
static BYTE referenceSpace[0x800000]={1};
static BYTE* page;
static BYTE initial[4096];
static DWORD exceptionCode,accessKind,faultAddress;
static int cases,faults;
static int Filter(EXCEPTION_POINTERS* p) {
    if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    exceptionCode=p->ExceptionRecord->ExceptionCode;
    accessKind=p->ExceptionRecord->ExceptionInformation[0];
    faultAddress=p->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
static void Check(InputDevice* receiver,int value,int expectFault,DWORD stored,int readOnly) {
    DWORD old;
    for(int role=0;role<2;role++) {
        if(!VirtualProtect(page,4096,PAGE_READWRITE,&old))ExitProcess(2);
        memcpy(page,initial,4096);
        if(readOnly&&!VirtualProtect(page,4096,PAGE_READONLY,&old))ExitProcess(2);
        union {int(InputDevice::*member)(int);DWORD address;} call;
        call.member=&InputDevice::IsDeviceSubtype;
        if(role)call.address=0x4897e0;
        exceptionCode=accessKind=faultAddress=0;
        int result=0x13579bdf,completed=0;
        __try {result=(receiver->*call.member)(value);completed=1;}
        __except(Filter(GetExceptionInformation())) {}
        if(completed==expectFault || !!exceptionCode!=expectFault ||
           (expectFault&&(accessKind!=0 || faultAddress!=(DWORD)receiver+0x10 || result!=0x13579bdf)) ||
           (!expectFault&&result!=(stored==(DWORD)value)) || memcmp(page,initial,4096)) {
            printf("mismatch %d role%d result%d complete%d fault%08lx addr%08lx\n",cases,role,result,completed,exceptionCode,faultAddress);
            ExitProcess(3);
        }
    }
    cases++;faults+=expectFault;
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
    BYTE* reserve=(BYTE*)VirtualAlloc(0,12288,MEM_RESERVE,PAGE_NOACCESS);
    if(!reserve)return 2;
    page=(BYTE*)VirtualAlloc(reserve+4096,4096,MEM_COMMIT,PAGE_READWRITE);
    if(!page)return 2;
    DWORD state=0x7134abc;
    const DWORD special[]={0,1,4,0xffffffff,0x80000000,0x7fffffff,0x12345678,0x87654321};
    for(int seed=0;seed<256;seed++) {
        for(int i=0;i<4096;i++){state=state*1664525+1013904223;initial[i]=(BYTE)(state>>24);}
        state=state*1664525+1013904223;
        DWORD stored=seed<8?special[seed]:state;
        DWORD values[]={stored,stored^1,4,0xffffffff};
        for(int argument=0;argument<4;argument++) {
            for(int location=0;location<4;location++) {
                int offset=location==0?64:location==1?4096-20:location==2?-16:65;
                memcpy(initial+offset+16,&stored,4);
                for(int ro=0;ro<2;ro++)Check((InputDevice*)(page+offset),(int)values[argument],0,stored,ro);
            }
            Check((InputDevice*)0,(int)values[argument],1,0,0);
            Check((InputDevice*)(page+4096-16),(int)values[argument],1,0,0);
            Check((InputDevice*)(page-20),(int)values[argument],1,0,0);
        }
    }
    printf("Input subtype: %d cases, %d modeled read faults\n",cases,faults);
    return 0;
}
