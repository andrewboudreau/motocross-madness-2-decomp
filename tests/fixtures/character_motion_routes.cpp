// Execute unchanged, independently strict-checked Character/MotionManager bodies.
// Raw storage avoids constructing an unrelated scene or starting resource I/O.
#include "motion/Motnctrl.h"
#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

MotionManager* g_pMotionManager;

typedef char PointerWidth[sizeof(Motion*) == sizeof(int) ? 1 : -1];
typedef char StoppedOffset[offsetof(Character, motionStopped) == 0xc ? 1 : -1];
typedef char MotionOffset[offsetof(Character, currentMotion) == 0x18 ? 1 : -1];
typedef char FrameOffset[offsetof(Character, currentFrame) == 0x1c ? 1 : -1];
typedef char BlendOffset[offsetof(Character, blendActive) == 0x34 ? 1 : -1];
typedef char FrameSourceOffset[offsetof(Motion, frames) == 0xa4 ? 1 : -1];

// The complete primary Character base has no pointer adjustment. Its virtual
// GameObject base is separate and is never used as a motion-method receiver.
Character* CharacterBase(D3DIMSoultreeCharacter* value) { return value; }

static void Put(unsigned char* bytes, int offset, unsigned value) {
    memcpy(bytes + offset, &value, sizeof(value));
}

int main() {
    union { unsigned words[0x240 / 4]; unsigned char bytes[0x240]; } storage;
    unsigned char expected[sizeof(storage)];
    unsigned char managerStorage[sizeof(MotionManager)];
    MotionManager* manager = (MotionManager*)managerStorage;
    Motion motions[3];
    Motion* entries[3] = {&motions[0], &motions[1], &motions[2]};
    MotionPoseList frames[3][2];
    const char* queries[5] = {"Stand", "FLAGSTART", "flagloop", "missing", "sTaNd"};
    unsigned cases = 0;
    memset(managerStorage, 0, sizeof(managerStorage));
    memset(motions, 0, sizeof(motions));
    strcpy(motions[0].name, "Stand");
    strcpy(motions[1].name, "FlagStart");
    strcpy(motions[2].name, "FlagLoop");
    manager->motions = entries;
    g_pMotionManager = manager;
    for (int seed = 0; seed < 64; ++seed) {
        for (int count = 0; count <= 3; ++count) {
            manager->count = count;
            for (int query = 0; query < 5; ++query) {
                int index = query == 4 ? 0 : query;
                Motion* found = index < count ? entries[index] : 0;
                for (int mode = 0; mode < 3; ++mode) {
                    unsigned value = seed + 1;
                    for (unsigned i = 0; i < sizeof(storage); ++i) {
                        value = value * 1664525UL + 1013904223UL;
                        storage.bytes[i] = (unsigned char)(value >> 24);
                    }
                    for (int j = 0; j < 3; ++j)
                        motions[j].frames = mode == 0 ? 0 : &frames[j][mode - 1];
                    Character* character = (Character*)&storage;
                    memcpy(expected, storage.bytes, sizeof(storage));
                    if (character->FindMotion(queries[query], (seed - 32) * (mode + 1)) != found ||
                        memcmp(expected, storage.bytes, sizeof(storage))) return 1;
                    Put(expected, 0xc, 0);
                    Put(expected, 0x18, (unsigned)found);
                    if (found) {
                        Put(expected, 0x1c, (unsigned)found->frames);
                        Put(expected, 0x34, 0);
                    }
                    unsigned fault = 0;
                    __try { character->SetMotionByName(queries[query]); }
                    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                        fault = 1;
                    }
                    // The actual SetMotion body first clears stopped and stores
                    // the pointer, then reads motion+0xa4. Missing names fault;
                    // do not invent a null guard or clear the remaining fields.
                    if (fault != (found == 0) || memcmp(expected, storage.bytes, sizeof(storage))) return 2;
                    ++cases;
                }
            }
        }
    }
    printf("Character motion: %u lookup, state and missing-name cases passed\n", cases);
    return 0;
}
