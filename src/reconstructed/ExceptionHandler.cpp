// Crash reporter, 0x0045ff80..0x00460acb (provisional file name, see
// ExceptionHandler.h). It sits between EventManager.cpp's vector set
// (ends 0x0045ff7b) and the FastMath table initializer 0x00460ad0. Every
// Win32 call binds to the KERNEL32/USER32/ADVAPI32 IAT; strrchr is LIBCMT's
// (0x00534a60).

#include <windows.h>
#include <string.h>

#include "ExceptionHandler.h"

#define ONEK        1024
#define SIXTYFOURK  (64 * ONEK)
#define ONEM        (ONEK * ONEK)
#define ONEG        (ONEK * ONEK * ONEK)

// 0x00460490: printf to the log handle through wvsprintf and WriteFile, so
// the reporter never touches the C runtime's stdio.
void hprintf(HANDLE LogFile, char* Format, ...)
{
    char buffer[2000];

    va_list arglist;
    va_start(arglist, Format);
    wvsprintf(buffer, Format, arglist);
    va_end(arglist);

    DWORD NumBytes;
    WriteFile(LogFile, buffer, lstrlen(buffer), &NumBytes, 0);
}

// 0x00460700: "m/d/yyyy hh:mm:ss" in local time, or an empty string.
void PrintTime(char* output, FILETIME TimeToPrint)
{
    WORD Date, Time;
    if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) &&
        FileTimeToDosDateTime(&TimeToPrint, &Date, &Time)) {
        wsprintf(output, "%d/%d/%d %02d:%02d:%02d",
                 (Date / 32) & 15, Date & 31, (Date / 512) + 1980,
                 (Time / 2048), (Time / 32) & 63, (Time & 31) * 2);
    } else
        output[0] = 0;
}

// 0x00460580: one line per code module (path, base, file size, link time
// stamp, file date). Bad module headers fault inside the __try.
void ShowModuleInfo(HANDLE LogFile, HINSTANCE ModuleHandle)
{
    char ModName[MAX_PATH];
    __try {
        if (GetModuleFileName(ModuleHandle, ModName, sizeof(ModName)) > 0) {
            IMAGE_DOS_HEADER* DosHeader = (IMAGE_DOS_HEADER*)ModuleHandle;
            if (IMAGE_DOS_SIGNATURE != DosHeader->e_magic)
                return;
            IMAGE_NT_HEADERS* NTHeader = (IMAGE_NT_HEADERS*)((char*)DosHeader + DosHeader->e_lfanew);
            if (IMAGE_NT_SIGNATURE != NTHeader->Signature)
                return;
            HANDLE ModuleFile = CreateFile(ModName, GENERIC_READ, FILE_SHARE_READ, 0,
                                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            char TimeBuffer[100] = "";
            DWORD FileSize = 0;
            if (ModuleFile != INVALID_HANDLE_VALUE) {
                FileSize = GetFileSize(ModuleFile, 0);
                FILETIME LastWriteTime;
                if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime)) {
                    wsprintf(TimeBuffer, " - file date is ");
                    PrintTime(TimeBuffer + lstrlen(TimeBuffer), LastWriteTime);
                }
                CloseHandle(ModuleFile);
            }
            hprintf(LogFile, "%s, loaded at 0x%08x - %d bytes - %08x%s\r\n",
                    ModName, ModuleHandle, FileSize,
                    NTHeader->FileHeader.TimeDateStamp, TimeBuffer);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// 0x004604e0: walks the address space with VirtualQuery and reports every
// new committed allocation base as a module.
void RecordModuleList(HANDLE LogFile)
{
    hprintf(LogFile, "\r\n"
                     "\tModule list: names, addresses, sizes, time stamps "
                     "and file times:\r\n");
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    const size_t PageSize = SystemInfo.dwPageSize;
    const size_t NumPages = 4 * size_t(ONEG / PageSize);
    size_t pageNum = 0;
    void* LastAllocationBase = 0;
    while (pageNum < NumPages) {
        MEMORY_BASIC_INFORMATION MemInfo;
        if (VirtualQuery((void*)(pageNum * PageSize), &MemInfo, sizeof(MemInfo))) {
            if (MemInfo.RegionSize > 0) {
                pageNum += MemInfo.RegionSize / PageSize;
                if (MemInfo.State == MEM_COMMIT && MemInfo.AllocationBase > LastAllocationBase) {
                    LastAllocationBase = MemInfo.AllocationBase;
                    ShowModuleInfo(LogFile, (HINSTANCE)LastAllocationBase);
                }
            } else
                pageNum += SIXTYFOURK / PageSize;
        } else
            pageNum += SIXTYFOURK / PageSize;
    }
}

// 0x004607a0: time, executable, user, processor count/type and memory.
void RecordSystemInformation(HANDLE LogFile)
{
    FILETIME CurrentTime;
    GetSystemTimeAsFileTime(&CurrentTime);
    char TimeBuffer[100];
    PrintTime(TimeBuffer, CurrentTime);
    hprintf(LogFile, "Error occurred at %s.\r\n", TimeBuffer);
    char ModuleName[MAX_PATH];
    if (GetModuleFileName(0, ModuleName, sizeof(ModuleName)) <= 0)
        lstrcpy(ModuleName, "Unknown");
    char UserName[200];
    DWORD UserNameSize = sizeof(UserName);
    if (!GetUserName(UserName, &UserNameSize))
        lstrcpy(UserName, "Unknown");
    hprintf(LogFile, "%s, run by %s.\r\n", ModuleName, UserName);

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    hprintf(LogFile, "%d processor(s), type %d.\r\n",
            SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);

    MEMORYSTATUS MemInfo;
    MemInfo.dwLength = sizeof(MemInfo);
    GlobalMemoryStatus(&MemInfo);
    hprintf(LogFile, "%d MBytes physical memory.\r\n", (MemInfo.dwTotalPhys + ONEM - 1) / ONEM);
}

// 0x004608c0: the exception code's name, from a table built on the stack.
const char* GetExceptionDescription(DWORD ExceptionCode)
{
    struct ExceptionNames {
        DWORD ExceptionCode;
        char* ExceptionName;
    };

    ExceptionNames ExceptionMap[] = {
        {0x40010005, "a Control-C"},
        {0x40010008, "a Control-Break"},
        {0x80000002, "a Datatype Misalignment"},
        {0x80000003, "a Breakpoint"},
        {0xc0000005, "an Access Violation"},
        {0xc0000006, "an In Page Error"},
        {0xc0000017, "a No Memory"},
        {0xc000001d, "an Illegal Instruction"},
        {0xc0000025, "a Noncontinuable Exception"},
        {0xc0000026, "an Invalid Disposition"},
        {0xc000008c, "a Array Bounds Exceeded"},
        {0xc000008d, "a Float Denormal Operand"},
        {0xc000008e, "a Float Divide by Zero"},
        {0xc000008f, "a Float Inexact Result"},
        {0xc0000090, "a Float Invalid Operation"},
        {0xc0000091, "a Float Overflow"},
        {0xc0000092, "a Float Stack Check"},
        {0xc0000093, "a Float Underflow"},
        {0xc0000094, "an Integer Divide by Zero"},
        {0xc0000095, "an Integer Overflow"},
        {0xc0000096, "a Privileged Instruction"},
        {0xc00000fd, "a Stack Overflow"},
        {0xc0000142, "a DLL Initialization Failed"},
        {0xe06d7363, "a Microsoft C++ Exception"},
    };

    for (int i = 0; i < sizeof(ExceptionMap) / sizeof(ExceptionMap[0]); i++)
        if (ExceptionCode == ExceptionMap[i].ExceptionCode)
            return ExceptionMap[i].ExceptionName;

    return "Unknown exception type";
}

// 0x00460ab0: the file name part of a path.
char* GetFilePart(char* source)
{
    char* result = strrchr(source, '\\');
    if (result)
        result++;
    else
        result = source;
    return result;
}
