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
// fistp pick all agree). Retail re-reads the entry as animationTable->entries + i * 0x44
// after every store (no cached entry pointer), keeps sequenceLength in a stack byte, and its
// inline-assembly fistp gives the body an EBP frame.
// Session 23 (linked 0.701 -> ~0.93): entries[i] and randomSets[j] written per use; the
// character advance ("UseBlendedKeys" entries through Character slot 7 0x004a70c0, the others
// through AdvanceMotion 0x004a6bb0) written out at both sites, as retail expands it (a static
// helper stayed out of line); the random-set object's isEnabled/Disable/Enable used through
// the object (a GameObject* local adds a null-checked virtual-base conversion) and the same
// object advanced afterwards; (rand() * (1 / 32768)) * count parenthesised, as retail
// multiplies by the constant first (exact either way: the constant is a power of two).
// Remaining: esi/edi swapped in the random-set loop, the isEnabled address CSE'd into the
// Disable call, and the fistp helper's pointer argument.
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
            if (!animationTable->entries[i].keyFramed) {
                continue;
            }
            if (holdAnimations && !animationTable->entries[i].animatesWhileHeld) {
                continue;
            }
            Character* character = reinterpret_cast<Character*>(animationTable->entries[i].character);
            if (animationTable->entries[i].inRandomSet || !animationTable->entries[i].automaticSequence) {
                continue;
            }

            if (animationTable->entries[i].sequenceLength) {
                Motion* current = character->currentMotion;
                if (current) {
                    // Seconds left in the playing motion: blend into the next
                    // motion of the sequence once less than half its
                    // transition time remains.
                    float remaining = (float)(current->frameCount - animationTable->entries[i].motionFrame) / current->fps;
                    if (remaining < current->transitionTime * 0.5f) {
                        animationTable->entries[i].sequenceIndex++;
                        if (animationTable->entries[i].sequenceIndex >= animationTable->entries[i].sequenceLength) {
                            animationTable->entries[i].sequenceIndex = 0;
                        }
                        animationTable->entries[i].currentMotion = animationTable->entries[i].motionSequence[animationTable->entries[i].sequenceIndex] - 1;
                        character->BlendToMotion((Motion*)animationTable->entries[i].motions[animationTable->entries[i].currentMotion],
                                                 remaining + remaining);
                    }
                } else if (animationTable->entries[i].sequenceLength == 1 &&
                           !((Motion*)animationTable->entries[i].motions[animationTable->entries[i].currentMotion])->looping) {
                    // A one-motion sequence of a non-looping motion stops.
                    animationTable->entries[i].automaticSequence = 0;
                } else {
                    animationTable->entries[i].sequenceIndex++;
                    if (animationTable->entries[i].sequenceIndex >= animationTable->entries[i].sequenceLength) {
                        animationTable->entries[i].sequenceIndex = 0;
                    }
                    animationTable->entries[i].currentMotion = animationTable->entries[i].motionSequence[animationTable->entries[i].sequenceIndex] - 1;
                    // best guess: retail passes the motion index (converted to
                    // float) as RestartEntry's time argument.
                    RestartEntry(i, (float)animationTable->entries[i].currentMotion, 0, 0);
                }
            } else if (character->motionStopped) {
                character->SetMotion((Motion*)animationTable->entries[i].motions[animationTable->entries[i].currentMotion]);
            }
            if (animationTable->entries[i].useBlendedKeys) {
                animationTable->entries[i].motionFrame = character->CharacterVirtualSlot7(dt, 0, 0);
            } else {
                animationTable->entries[i].motionFrame = character->AdvanceMotion(dt, 0, 0);
            }
        }

        if (holdAnimations) {
            return 1;  // retail skips GameObject::Tick here
        }

        for (int j = 0; j < animationTable->randomSetCount; j++) {
            int count = animationTable->randomSets[j].count;
            if (!count) {
                continue;
            }
            UnknownSceneEntry* entry = animationTable->randomSets[j].entry;
            // best guess: retail tests the key-framed bit of entries[j] (the
            // set's index into the entry table), not of the set's entry.
            if (!animationTable->entries[j].keyFramed) {
                continue;
            }
            UnknownSceneObject* object = entry->character;
            if (!object->isEnabled || reinterpret_cast<Character*>(object)->motionStopped) {
                // Swap in a random member of the set.
                object->Disable();
                // Retail's fistp helper (0x004eadf1): floor() of the scaled rand except at ties.
                int pick;
                FistpFloatToInt(&pick, (rand() * (1.0f / 32768.0f)) * count - 0.5f);
                entry = &animationTable->entries[animationTable->randomSets[j].animationIndices[pick]];
                entry->currentMotion = animationTable->randomSets[j].motionIndices[pick];
                object = entry->character;
                object->Enable();
                animationTable->randomSets[j].entry = entry;
                reinterpret_cast<Character*>(object)->SetMotion(
                    (Motion*)entry->motions[entry->currentMotion]);
            }
            Character* character = reinterpret_cast<Character*>(object);
            if (entry->useBlendedKeys) {
                entry->motionFrame = character->CharacterVirtualSlot7(dt, 0, 0);
            } else {
                entry->motionFrame = character->AdvanceMotion(dt, 0, 0);
            }
        }
    }
    return GameObject::Tick(dt);
}
#pragma optimize("", on)
