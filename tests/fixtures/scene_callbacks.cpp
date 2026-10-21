#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "SceneManager.h"

typedef char SceneSize[(sizeof(Scene) == 0x8cc) ? 1 : -1];

int main()
{
    union { double align; unsigned char bytes[sizeof(Scene) + 32]; } actual, expected;
    unsigned char* inaccessible = (unsigned char*)VirtualAlloc(0, 4096,
        MEM_RESERVE | MEM_COMMIT, PAGE_NOACCESS);
    if (!inaccessible) return 1;
    const unsigned bits[] = {0, 0x80000000, 1, 0x3f800000, 0xbf800000,
        0x7f7fffff, 0x00800000, 0x12345678};
    unsigned count = 0;
    for (unsigned seed = 0; seed < 256; ++seed) {
        for (unsigned value = 0; value < 8; ++value) {
            for (unsigned receiver = 0; receiver < 3; ++receiver) {
                for (unsigned i = 0; i < sizeof(actual.bytes); ++i)
                    actual.bytes[i] = (unsigned char)(seed + 37 * i);
                memcpy(expected.bytes, actual.bytes, sizeof(actual.bytes));
                Scene* scene = reinterpret_cast<Scene*>(receiver == 0 ? actual.bytes + 16 :
                    receiver == 1 ? inaccessible : 0);
                // Prepared raw storage and deliberately unreadable arguments,
                // not constructed scenes or a simulated loader. The compiled
                // standalone bodies must not access any of these pointers.
                const char* section = value & 1 ? (const char*)inaccessible : 0;
                const char* key = value & 2 ? (const char*)actual.bytes : (const char*)1;
                const char* text = value & 4 ? (const char*)inaccessible : (const char*)3;
                const Vector3* vector = reinterpret_cast<const Vector3*>(text);
                float number;
                memcpy(&number, &bits[value], 4);
                if ((char*)(GameObject*)reinterpret_cast<Scene*>(actual.bytes + 16) !=
                    (char*)actual.bytes + 16) return 2;
                __try {
                    scene->UnknownFunction4de580(section, key, text);
                    if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 3;
                    scene->UnknownFunction4de580(section, key, number);
                    if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 4;
                    scene->UnknownFunction4de580(section, key, (int)bits[value]);
                    if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 5;
                    scene->UnknownFunction4de580(section, key, vector);
                    if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 6;
                    scene->UnknownFolded4de580(section, key, text);
                    if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 7;
                    scene->UnknownFunction464e80(text);
                    if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 8;
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    return 9;
                }
                count += 6;
            }
        }
    }
    VirtualFree(inaccessible, 0, MEM_RELEASE);
    printf("Scene callbacks: %u complete-storage calls passed\n", count);
    return 0;
}
