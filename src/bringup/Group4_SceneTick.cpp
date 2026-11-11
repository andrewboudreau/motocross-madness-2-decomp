// Bring-up (best-guess-unreviewed): Scene slot 10, 0x004eab00 (958 bytes,
// ret 4), written from the retail disassembly. Not byte matched.
//
// Scene::Tick advances every "Animation<n>" entry of the scene's animation
// table, then (unless holdAnimations is set) the "RandomSet<n>" picks, and
// finally runs GameObject::Tick. The scene entries keep their characters as
// UnknownSceneObject (SceneManager.h); like SceneManager.cpp, this unit uses
// the real Character methods on them through reinterpret_cast.
//
// Audited against the retail disassembly on 2026-11-11 for the linked game: no
// behavioural difference (flag bits, the fild/fidiv remaining-time test, the
// sequence wrap, RestartEntry's arguments, the random-set swap condition and the
// fistp pick all agree). The gap is code shape: retail re-reads the entry as
// animationTable->entries + i * 0x44 after every store (it does not cache the
// pointer), keeps sequenceLength in a stack byte, and its inline-assembly fistp
// gives the body an EBP frame.
// Rounding: the game build compiles this unit with /QIfist (config/game_build.json
// bringup_flags), so FistpFloatToInt's cast is a FISTP under the current rounding
// mode, as retail's helper at 0x004eadf1; the pick is the unit's only float-to-int
// conversion and lies in [-0.5, count - 0.5), inside int.
#include <stdlib.h>

#include "../reconstructed/TrackGame.h"
#include "../reconstructed/SceneManager.h"
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"
#include "../krusty2/motion/Motnctrl.h"
#include "../../samples/common/FistpFloatToInt.h"

// Advances an entry's character by dt; "UseBlendedKeys" entries go through
// Character slot 7 (0x004a70c0), the others through AdvanceMotion (0x004a6bb0).
static void Group4Scene_AdvanceEntry(UnknownSceneEntry* entry, float dt)
{
    Character* character = reinterpret_cast<Character*>(entry->character);
    if (entry->useBlendedKeys) {
        entry->motionFrame = character->CharacterVirtualSlot7(dt, 0, 0);
    } else {
        entry->motionFrame = character->AdvanceMotion(dt, 0, 0);
    }
}

// 0x004eab00
// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
int Scene::Tick(float frameTime)
{
    if (g_TrackGame->paused) {
        return 1;
    }
    // Retail ignores the argument: it scales the game's own frame time by the
    // replay speed and hands that to GameObject::Tick as well.
    float dt = g_TrackGame->frameTime * g_TrackGame->replaySpeed;

    if (animationTable) {
        for (int i = 0; i < animationTable->entryCount; i++) {
            UnknownSceneEntry* entry = &animationTable->entries[i];
            if (!entry->keyFramed) {
                continue;
            }
            if (holdAnimations && !entry->animatesWhileHeld) {
                continue;
            }
            Character* character = reinterpret_cast<Character*>(entry->character);
            if (entry->inRandomSet || !entry->automaticSequence) {
                continue;
            }

            if (entry->sequenceLength) {
                Motion* current = character->currentMotion;
                if (current) {
                    // Seconds left in the playing motion: blend into the next
                    // motion of the sequence once less than half its
                    // transition time remains.
                    float remaining = (float)(current->frameCount - entry->motionFrame) / current->fps;
                    if (remaining < current->transitionTime * 0.5f) {
                        entry->sequenceIndex++;
                        if (entry->sequenceIndex >= entry->sequenceLength) {
                            entry->sequenceIndex = 0;
                        }
                        entry->currentMotion = entry->motionSequence[entry->sequenceIndex] - 1;
                        character->BlendToMotion((Motion*)entry->motions[entry->currentMotion],
                                                 remaining + remaining);
                    }
                } else if (entry->sequenceLength == 1 &&
                           !((Motion*)entry->motions[entry->currentMotion])->looping) {
                    // A one-motion sequence of a non-looping motion stops.
                    entry->automaticSequence = 0;
                } else {
                    entry->sequenceIndex++;
                    if (entry->sequenceIndex >= entry->sequenceLength) {
                        entry->sequenceIndex = 0;
                    }
                    entry->currentMotion = entry->motionSequence[entry->sequenceIndex] - 1;
                    // best guess: retail passes the motion index (converted to
                    // float) as RestartEntry's time argument.
                    RestartEntry(i, (float)entry->currentMotion, 0, 0);
                }
            } else if (character->motionStopped) {
                character->SetMotion((Motion*)entry->motions[entry->currentMotion]);
            }
            Group4Scene_AdvanceEntry(entry, dt);
        }

        if (holdAnimations) {
            return 1;  // retail skips GameObject::Tick here
        }

        for (int j = 0; j < animationTable->randomSetCount; j++) {
            UnknownSceneEntry2* set = &animationTable->randomSets[j];
            int count = set->count;
            if (!count) {
                continue;
            }
            UnknownSceneEntry* entry = set->entry;
            // best guess: retail tests the key-framed bit of entries[j] (the
            // set's index into the entry table), not of the set's entry.
            if (!animationTable->entries[j].keyFramed) {
                continue;
            }
            UnknownSceneObject* object = entry->character;
            GameObject* gameObject = object;
            if (!gameObject->isEnabled || reinterpret_cast<Character*>(object)->motionStopped) {
                // Swap in a random member of the set.
                gameObject->Disable();
                // Retail's fistp helper (0x004eadf1): floor() of the scaled rand except at ties.
                int pick;
                FistpFloatToInt(&pick, rand() * (1.0f / 32768.0f) * count - 0.5f);
                entry = &animationTable->entries[set->animationIndices[pick]];
                entry->currentMotion = set->motionIndices[pick];
                object = entry->character;
                static_cast<GameObject*>(object)->Enable();
                animationTable->randomSets[j].entry = entry;
                reinterpret_cast<Character*>(object)->SetMotion(
                    (Motion*)entry->motions[entry->currentMotion]);
            }
            Group4Scene_AdvanceEntry(entry, dt);
        }
    }
    return GameObject::Tick(dt);
}
#pragma optimize("", on)
