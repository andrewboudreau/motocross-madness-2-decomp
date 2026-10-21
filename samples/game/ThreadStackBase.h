#pragma once

#include <windows.h>

// Ordinary-C++ access to the current thread's NT_TIB::StackBase. The x86
// functional backend resolves the OS's NtCurrentTeb export; it is nonexact.
DWORD* ReadCurrentThreadStackBase();
