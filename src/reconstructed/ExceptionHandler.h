#pragma once

// The crash reporter at 0x0045ff80..0x00460acb (no __FILE__, no RTTI). Its
// literals ("errorlog.txt", "Error creating exception report", the
// "a Control-C".."a Microsoft C++ Exception" table, " - file date is ")
// are those of Bruce Dawson's published ExceptionHandler.cpp (Game
// Developer, 1999), and the retail code follows that listing function by
// function; the file name is borrowed from it (it also fits the link-order
// bracket EventManager.cpp .. FollowCam.cpp). The names are that listing's,
// which makes them external context, not retail evidence.
#include <windows.h>

// 0x0045ff80: the __except filter of the program's thread wrappers
// (0x004a0c19 passes "main thread"). Not reconstructed: its stack dump
// reads the stack top with inline assembly (mov eax, fs:[4]).
int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, const char* Message);

void hprintf(HANDLE LogFile, char* Format, ...);               // 0x00460490
void RecordModuleList(HANDLE LogFile);                         // 0x004604e0
void ShowModuleInfo(HANDLE LogFile, HINSTANCE ModuleHandle);   // 0x00460580
void PrintTime(char* output, FILETIME TimeToPrint);            // 0x00460700
void RecordSystemInformation(HANDLE LogFile);                  // 0x004607a0
const char* GetExceptionDescription(DWORD ExceptionCode);      // 0x004608c0
char* GetFilePart(char* source);                               // 0x00460ab0
