#ifndef FILE_PRINTF_H
#define FILE_PRINTF_H

#include <stdio.h>

// 0x00461d40: cdecl varargs; returns the CRT vfprintf result.
int FilePrintf(FILE* file, const char* format, ...);

#endif
