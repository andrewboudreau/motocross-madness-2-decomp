// Native integration only: link the three production objects unchanged.
// Real access violations pass through the reporter's filter to an outer SEH
// handler. All file, stack-bound, system-information and module APIs are real.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/ExceptionHandler.h"

static int filterCalls;
static int innerHandlers;
static int outerHandlers;
static CONTEXT firstContext;
static DWORD firstStack[8];
static DWORD firstFaultAddress;

static int ReportFilter(EXCEPTION_POINTERS* info, const char* message) {
    ++filterCalls;
    if (filterCalls == 1) {
        firstContext = *info->ContextRecord;
        firstFaultAddress = info->ExceptionRecord->ExceptionInformation[1];
        for (int i = 0; i < 8; ++i)
            firstStack[i] = ((DWORD*)firstContext.Esp)[i];
    }
    int result = RecordExceptionInfo(info, message);
    if (result != EXCEPTION_CONTINUE_SEARCH)
        ExitProcess(10);
    return result;
}

static void FaultWithOuterHandler(void* inaccessible, const char* message) {
    __try {
        __try {
            volatile DWORD value = *(volatile DWORD*)inaccessible;
            (void)value;
        } __except (ReportFilter(GetExceptionInformation(), message)) {
            ++innerHandlers;
        }
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                    ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        ++outerHandlers;
    }
}

static char* ReadReport(const char* path, DWORD* length) {
    HANDLE file = CreateFileA(path, GENERIC_READ, 0, 0, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    *length = GetFileSize(file, 0);
    if (*length == INVALID_FILE_SIZE || *length > 1024 * 1024) {
        CloseHandle(file);
        return 0;
    }
    char* text = (char*)malloc(*length + 1);
    DWORD read = 0;
    BOOL ok = text && ReadFile(file, text, *length, &read, 0);
    CloseHandle(file);
    if (!ok || read != *length) {
        free(text);
        return 0;
    }
    text[*length] = 0;
    return text;
}

int main(int argc, char** argv) {
    if (argc != 2)
        return 1;
    void* inaccessible = VirtualAlloc(0, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_NOACCESS);
    if (!inaccessible)
        return 2;
    FaultWithOuterHandler(inaccessible, "native-first-report");
    if (filterCalls != 1 || innerHandlers != 0 || outerHandlers != 1)
        return 3;
    DWORD firstLength, secondLength;
    char* first = ReadReport(argv[1], &firstLength);
    if (!first || !strstr(first, "native-first-report"))
        return 4;
    FaultWithOuterHandler(inaccessible, "native-second-report-must-not-appear");
    if (filterCalls != 2 || innerHandlers != 0 || outerHandlers != 2)
        return 5;
    char* second = ReadReport(argv[1], &secondLength);
    if (!second || firstLength != secondLength || memcmp(first, second, firstLength))
        return 6;
    if (firstFaultAddress != (DWORD)inaccessible)
        return 7;

    // Expected fields come from the actual first exception context, not from
    // a constructed context or a substitute reporter/helper implementation.
    printf("REGISTER EAX=%08x CS=%04x EIP=%08x EFLGS=%08x\n",
           firstContext.Eax, firstContext.SegCs, firstContext.Eip, firstContext.EFlags);
    printf("REGISTER EBX=%08x SS=%04x ESP=%08x EBP=%08x\n",
           firstContext.Ebx, firstContext.SegSs, firstContext.Esp, firstContext.Ebp);
    printf("REGISTER ECX=%08x DS=%04x ESI=%08x FS=%04x\n",
           firstContext.Ecx, firstContext.SegDs, firstContext.Esi, firstContext.SegFs);
    printf("REGISTER EDX=%08x ES=%04x EDI=%08x GS=%04x\n",
           firstContext.Edx, firstContext.SegEs, firstContext.Edi, firstContext.SegGs);
    printf("FAULT %08x\n", firstFaultAddress);
    printf("CODE ");
    for (int i = 0; i < 16; ++i)
        printf("%02x ", ((BYTE*)firstContext.Eip)[i]);
    printf("\nSTACK %08x: ", firstContext.Esp);
    for (i = 0; i < 8; ++i)
        printf(i == 7 ? "%08x\n" : "%08x ", firstStack[i]);
    printf("Two native exceptions continued to outer handlers; one report, %lu bytes unchanged\n",
           firstLength);
    free(second);
    free(first);
    VirtualFree(inaccessible, 0, MEM_RELEASE);
    return 0;
}
