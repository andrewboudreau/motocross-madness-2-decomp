// Complete functional candidate for 0x0045ff80..0x0046048a (1290 bytes).
// The original stack-bound FS load is replaced by the paired OS backend.
#include <windows.h>
#include <string.h>
#include "../../src/reconstructed/ExceptionHandler.h"
#include "ThreadStackBase.h"

static int exceptionReportStarted;

int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, const char* message) {
    if (exceptionReportStarted)
        return EXCEPTION_CONTINUE_SEARCH;
    exceptionReportStarted = 1;

    char application[MAX_PATH] = "Unknown";
    char path[MAX_PATH];
    if (!GetModuleFileNameA(0, path, sizeof(path)))
        path[0] = 0;
    char* filePart = GetFilePart(path);
    lstrcpyA(application, filePart);
    char* extension = strrchr(application, '.');
    if (extension)
        *extension = 0;
    lstrcpyA(filePart, "errorlog.txt");
    HANDLE log = CreateFileA(path, GENERIC_WRITE, 0, 0, OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
    if (log == INVALID_HANDLE_VALUE) {
        OutputDebugStringA("Error creating exception report");
        return EXCEPTION_CONTINUE_SEARCH;
    }
    SetFilePointer(log, 0, 0, FILE_END);
    hprintf(log, "\r\n\r\n\r\n\r\n");
    EXCEPTION_RECORD* record = data->ExceptionRecord;
    CONTEXT* context = data->ContextRecord;
    const char* moduleName = "Unknown";
    MEMORY_BASIC_INFORMATION memory;
    char modulePath[MAX_PATH];
    if (VirtualQuery((void*)context->Eip, &memory, sizeof(memory)) &&
        GetModuleFileNameA((HMODULE)memory.AllocationBase, modulePath, sizeof(modulePath)))
        moduleName = GetFilePart(modulePath);
    hprintf(log, "%s caused %s in module %s at %04x:%08x.\r\n",
            application, GetExceptionDescription(record->ExceptionCode),
            moduleName, context->SegCs, context->Eip);
    hprintf(log, "Exception handler called in %s.\r\n", message);
    RecordSystemInformation(log);
    if (record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && record->NumberParameters >= 2) {
        char description[1000];
        wsprintfA(description, "%s location %08x caused an access violation.\r\n",
                  record->ExceptionInformation[0] ? "Write to" : "Read from",
                  record->ExceptionInformation[1]);
        hprintf(log, "%s", description);
    }
    hprintf(log, "\r\n");
    hprintf(log, "Registers:\r\n");
    hprintf(log, "EAX=%08x CS=%04x EIP=%08x EFLGS=%08x\r\n",
            context->Eax, context->SegCs, context->Eip, context->EFlags);
    hprintf(log, "EBX=%08x SS=%04x ESP=%08x EBP=%08x\r\n",
            context->Ebx, context->SegSs, context->Esp, context->Ebp);
    hprintf(log, "ECX=%08x DS=%04x ESI=%08x FS=%04x\r\n",
            context->Ecx, context->SegDs, context->Esi, context->SegFs);
    hprintf(log, "EDX=%08x ES=%04x EDI=%08x GS=%04x\r\n",
            context->Edx, context->SegEs, context->Edi, context->SegGs);
    hprintf(log, "Bytes at CS:EIP:\r\n");
    unsigned char* code = (unsigned char*)context->Eip;
    for (int i = 0; i < 16; ++i) {
        __try {
            hprintf(log, "%02x ", code[i]);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            hprintf(log, "?? ");
        }
    }
    hprintf(log, "\r\nStack dump:\r\n");
    __try {
        DWORD* stack = (DWORD*)context->Esp;
        DWORD* top = ReadCurrentThreadStackBase();
        if (top > stack + 2048)
            top = stack + 2048;
        int words = 0;
        char buffer[1000] = "";
        char* output = buffer;
        while (stack + 1 <= top) {
            if (words % 8 == 0)
                output += wsprintfA(output, "%08x: ", stack);
            ++words;
            const char* separator = " ";
            if (words % 8 == 0 || stack + 2 > top)
                separator = "\r\n";
            output += wsprintfA(output, "%08x%s", *stack, separator);
            ++stack;
            if (output > buffer + sizeof(buffer) - 50) {
                hprintf(log, "%s", buffer);
                buffer[0] = 0;
                output = buffer;
            }
        }
        hprintf(log, "%s", buffer);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        hprintf(log, "Exception encountered during stack dump.\r\n");
    }
    RecordModuleList(log);
    CloseHandle(log);
    return EXCEPTION_CONTINUE_SEARCH;
}
