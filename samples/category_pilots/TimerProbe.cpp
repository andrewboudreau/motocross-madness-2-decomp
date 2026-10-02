// Reconstructed helper at 0x004bfa80, used twice by EcoSystem slot 12.
// Declarations only; no Windows SDK headers or import libraries are included.
extern "C" {
__declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int);
__declspec(dllimport) unsigned int __stdcall timeEndPeriod(unsigned int);
__declspec(dllimport) unsigned int __stdcall timeGetTime(void);
}
extern "C" unsigned int ProbeSampleMilliseconds() {
    timeBeginPeriod(1);
    unsigned int result = timeGetTime();
    timeEndPeriod(1);
    return result;
}
