// Near-miss bikerace.cpp candidates, kept out of src/reconstructed until
// they match. The canonical file is included first so the TU-local
// declarations, statics and inline helpers are the same.
//
// 0x0041eb20 (1708 bytes; camera target): 1699 of 1710 positions match.
// Retail loads the scene-table index (+0x150) before the table pointer when
// it addresses the entry (edx/ecx swapped at the branch head as well); a
// local index, a reference, pointer arithmetic and store orders did not
// reproduce it; neither do an entry reference, a long/unsigned index,
// index[array] or the /G, /O, /Zp flag sweep, nor a table local before the
// stores, a scene local, a by-value index/scene helper, an index local after
// the 0x004dab90 call, the entry indexed directly at every use, char-pointer
// arithmetic, an explicit `!= 0` bit test, the two branches swapped
// (`if (!caster)`), or 0..63 prepended typedefs (no arena window).
// Redundant parentheses on the head's `targetEntry` copy, the table
// pointer test and the entry's index, table and whole address (162
// combinations) give identical bytes, as do inline helpers for the entry
// (scene and index by value, either order; table and index; entry base and
// index), the scene, the table or the index read through a `BikeRace*`
// (25 forms).
// Cycle 3: `const int& index = targetEntry` or `int& index` aliases for the
// head copy and the entry index (1685), and `Scene& scene = *raceScene` for
// the table reads (1492) are worse than the form above, which keeps the
// edx/ecx swap.
//
// 0x0041f1d0 (881 bytes; next/previous camera target): control flow, the
// three branches and the shared tail call 0x0041eb20(caster) (the caster
// flag lives in the dead third-argument slot) are reconstructed. Retail
// keeps the constant 0 in edx from the first instruction (cmp [mem], edx,
// stores of edx) and so orders registers differently; the candidate tests
// with `test reg, reg` until the first call. Retail has three copies of the
// 0x0041eb20 call, each reloading the caster slot; the found path of the
// objects scan calling it and returning (instead of `break`) gives the
// second copy (230 -> 266 of 881). A third explicit call after the racers
// loop makes VC6 push a constant 0 (worse); explicit `!= 0` tests, an
// `int caster = 0` at the top, a function-scope `old` and the `objects`
// parameter for the caster flag leave the zero register unchanged.
//
// 0x004210f0 (2981 bytes with its jump table; the start grid): every case
// is decoded and the arithmetic matches where VC6 makes the same inlining
// choices. Retail calls the out-of-line Vector3 constructor (0x00404e60)
// for the last two expansions of the mode 2/3 case and inlines
// Scale(field_0x170, 20.0f) in the mode 4 case; this candidate does the
// opposite (VC6's inline budget). Call sequence (D dot, I FastInvSqrt,
// S scale, C constructor): retail DIS..CCDISCCCCCCDISCCCCCCDISCC-CSS+,
// candidate DIS..DISCCCCCCDISCCCCCCDISCS-CSS+ (the mode 2/3 slot step
// scaled with `gridSlotStep = Scale(...)` instead of an inline `*=`
// gives mode 4's middle constructor call, 975 -> 1018 of 3008; spelling
// the 4.66 offset `*a = *a - Scale(...)` in any case is worse).
// Statement-count and helper
// spelling variations moved either end but never both: Normalize written
// `return Scale(v, FastInvSqrt(length))` gives retail's mode 4 sequence
// (DISCC-CSS+) but still expands the mode 2/3 tail (aligned ratio 0.85 ->
// 0.90, strict 450 because the code grows by 4 bytes); trivial inline
// expansions added at the top cost the mode 2/3 normalisation's nested
// constructor first (DIC), never the tail's; a by-value grid-slot helper is
// not expanded at all; Vector3's member operator* instead of Scale, a
// per-case temporary, `gridFirstSlot + ...` and a merged scale factor make
// it worse. The side test is `p1.x * p0.z - p0.x * p1.z` (retail's load
// order and p0/p1 slots).
//
// 0x00417ed0 (6745 bytes with its jump table at 0x0041992c; the setup,
// ret 0x24): every branch, all 166 calls in retail order, EH states 0..0x10
// and the 0x4e8-byte frame with its buffers at retail's offsets
// (0x0a8/0x1ac/0x2b0/0x3b4). 2828 of 6768 positions match with the
// length helper's `xx` and `yy` locals and the z square left in the sum
// (the shape that squares each component as retail does in EventManager
// 0x0045d480's PodiumLength; `(x*x) + y*y + z*z` gives 2804); aligned
// instruction ratio 0.990. What differs: retail spills z*z of each vector
// length to a stack temporary (`fstp [t]; faddp; fadd [t]`; the only other
// copies in the binary are EventManager 0x0045d480's squared lengths at
// 0x0045dc05 and 0x0045dc99) and keeps the youind axis in the slot it
// shares with the girl block's temporaries above the angle, so the girl
// block's temporaries (and with them most esp offsets below +0x44) sit 4
// to 12 bytes apart; one copy store of the girl offset is scheduled after
// instead of before `fadd 1.2`. Helper-function, declaration-scope and
// /G5, /G6, /Ox flag variations did not move either. The three squares as
// separate locals (any sum order or parenthesisation, a `float[3]`, a
// named result) put all three products first as retail does but keep the
// third on the x87 stack (`fadd st(1); faddp st(2); fstp st(0)` or fxch
// forms) instead of storing it; eight other spellings of the sum and a
// by-value parameter give the candidate's two-then-one order. Of the 144
// bracketings with redundant parentheses (docs/VC6_OPERAND_ORDER.md section
// 3), `(x*x) + y*y + z*z` is best (+4 bytes) and none stores z*z. Also
// without effect (2799-2804): an inline square helper (by value or const
// reference, for z or all three), `(float)` casts on the products, a double
// result variable, and accumulating per statement (`l = x*x; l += y*y;
// l += z*z`, `l = (x*x) + y*y; l += z*z`, z first, from 0.0f, `l = l + ...`);
// promoting z*z to double (`(float)(... + (double)v.z * v.z)`, 2819) keeps
// all three products on the stack (`fld st(1); fmul st(2)` tail). An
// all-double sum (`double l = (double)v.x * v.x + ...`) scores 6625 only
// because its longer x87 code (fld st(n) squares, `fcom qword`) restores
// the 12 bytes the two missing spills cost, so the code after the length
// sites realigns; it is not retail's float code and is not used.
// Session 14 probe: `float xx = x*x, yy = y*y; volatile float zz = z*z;
// l = xx + yy + zz` gives 6768 bytes and 5578 of 5596 masked: zz then takes
// retail's shared slot (+0x34, +0x30) and every later frame offset lines up,
// so retail's z*z is a frame local while x*x and y*y stay on the x87 stack.
// Only the two length sites (`fst [zz]; fadd st(1); faddp st(2); fstp
// st(0)` against retail's `fstp [zz]; faddp; fadd [zz]`) and the girl-copy
// store order remain; volatile is not retail's code and is not used. Plain
// named squares (any subset, any sum bracketing, `+=` forms, float[3], a
// struct or constructor of squares, a by-value vector, `const float&`
// squares or a `float&` out helper) stay on the x87 stack. In probes VC6
// only stops substituting such locals into the sum inside a function with an
// EH object (as here), and no /G3-/G6 setting moves a product.
//
// 0x00419970 (13428 bytes; the loader, ret 0x14): decoded completely (the
// pro circuit branch, the network player and AI racers, the offline player,
// ghost and AI racers, the collision pairing and the 10 EH states); 194 of
// retail's 195 calls in the same order (call-sequence ratio 0.967: retail
// keeps one more stream destructor tail and lays the failure tail out
// between the AI loops). The frame is retail's 0xab0 bytes and every buffer
// from 0xcc up sits at retail's offset except bikeFile/riderFile (swapped;
// 20 and 21 accesses each). Three retail facts fixed it: the 0x40-byte own
// texture names are function-scope (they never share with the network
// block's), `ranking` is function-scope, and the `used` memset (rep stosd at
// 0x41a456) sits inside the multiplayer block. UnknownRandomSkill takes
// `const float&` because retail reads base and range from memory after the
// rand() calls (fmul [ebp+0x94]; by-value parameters are copied to temps).
// The pro circuit block caches g_TrackGame in a local (retail holds it in
// ebx across the garage copy and spills `circuit`, which then shares
// riders' slot) and takes each computer racer as &circuit->racers[n] (retail
// strength-reduces circuit+0x6dd); the network loops index
// g_TrackGame->mode.racers[j] directly (retail recomputes it from the
// global, so there is no `slots` local), which pulls setup to retail's 0x6c.
// The grid entries are read the same way (lea esi, [edx+0x2ec1] from the
// global, so no `players` local either). The offline player branch comes
// first in the source: its new carries line 0x4ea against the network
// block's 0x53b and EH state 3 against their 4-6, although VC6 lays the
// network block out first. The network player's body sits inside its
// search loop before a break (retail jumps past the block when no player is
// found). The event AI records and the front end's bike, manufacturer and
// rider tables are read through g_TrackGame at each use too (EventAi,
// UiBike, UiManufacturer, UiRider); with cached locals VC6 merges the
// failure tails differently (retail's MP player tail is inline with the
// vtable in eax and the "No Net Bike MCF" tail is the shared one, edx). Each
// network, ghost and AI racer is held in a local from its new until Load
// (retail calls Load with mov ecx, eax or edi instead of reloading the slot),
// and the remote player's setup.weight is stored before its path sprintf.
// The two network player loops share one block-level `k` for their preset
// curve copies; with a `for (int k` in each, VC6 allocates the later event
// AI copy differently (retail [base + n*0xcc], ours was [n*0xcc + base], and
// its UI else branch loaded bikes[n] before ui->bikes). That makes the
// candidate retail's code apart from three register choices near the start
// (0x419b0d, 0x419b39, 0x419bf0). Left are frame slots: the scalars below
// 0x6c, bikeTexture/riderTexture and ownBikeTexture/ownRiderTexture (each
// pair swapped; their use counts tie), so 13100 of 13428 bytes match.
// Measured but not kept: one function-level `i` for both network loops puts
// i, recordIndex, position, direction and bike/riderTexture in retail's
// slots but moves up/plate/fromEvent/riders (12792), and a char[2][0x40]
// for the own textures fixes bike/riderTexture but swaps the recorded pair.
// The shared-`i` form fails because retail homes the pro circuit's inlined
// UnknownRandomSkill `offset` temporary (fstp/fld at 0x41a212/0x41a231) in
// skill's slot, while ours puts it in riders/circuit's, which then ranks
// too high. That home choice is the lead for the remaining slots.

#include "../../src/reconstructed/BikeRace.cpp"

#include <math.h>
#include <stdlib.h>

#include "../../src/reconstructed/CarProcedural.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/Krusty3DObjects.h"
#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/OptionProcs.h"
#include "../../src/reconstructed/RaceSound.h"
#include "../../src/reconstructed/RaceStatus.h"
#include "../../src/reconstructed/SelectGamePicProcs.h"
#include "../../src/reconstructed/TextureMap.h"

// 0x0041eb20. Left: the caster arm loads targetCaster then raceScene as
// retail does, the entry arm loads targetEntry and raceScene in the other
// order. Splitting the two arms' targetKind/targetIndex stores into their
// own `if (caster) ... else ...` ahead of the bodies flips both arms (the
// entry arm then matches, the caster arm does not); splitting one arm,
// `!caster` tests, ternary stores or one store per split leave it or worse.
int BikeRace::AimCamera(int caster) {
    if (UnknownFunctionCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
        if (g_TrackGame->playerCount > 1) {
            UnknownFunctionCameraView()->followedBike->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->UnknownFunctionFollow(racerSlots[targetRacer]);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(racerSlots[targetRacer]->modelNode);
                raceShadow->AddCaster(racerSlots[targetRacer]->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                UnknownBikeRaceRacer* racer = racerSlots[targetRacer];
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        } else if (targetRacer == 0) {
            UnknownFunctionCameraView()->followedBike->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->UnknownFunctionFollow(localRacer);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(localRacer->modelNode);
                raceShadow->AddCaster(localRacer->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                UnknownBikeRaceRacer* racer = localRacer;
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        } else {
            UnknownFunctionCameraView()->followedBike->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            int index = targetRacer - 1;
            UnknownFunctionCameraView()->UnknownFunctionFollow(aiRacers[index]);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(aiRacers[index]->modelNode);
                raceShadow->AddCaster(aiRacers[index]->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                UnknownBikeRaceRacer* racer = aiRacers[index];
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        }
    } else {
    if (caster) {
        targetKind = 0;
        targetIndex = targetCaster;
        if (raceScene->staticModels != 0 && raceScene->staticModels->casterCount > 0) {
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
            }
            UnknownFunctionCameraView()->UnknownFunctionTargetCaster(
                raceScene->staticModels->casters[targetCaster].model);
        } else {
            UnknownFunctionCameraView()->followingRacer = 1;
        }
    } else {
        targetKind = 1;
        targetIndex = targetEntry;
        if (raceScene->animationTable != 0 && raceScene->animationTable->entryCount != 0) {
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
            }
            UnknownSceneEntry* entry = &raceScene->animationTable->entries[targetEntry];
            if (entry->keyFramed) {
                UnknownBikeRaceSceneCharacter* character =
                    (UnknownBikeRaceSceneCharacter*)entry->character;
                if (character->isEnabled) {
                    UnknownFunctionCameraView()->UnknownFunctionTargetCharacter(character);
                } else {
                    return 0;
                }
            } else {
                GameObject* object = (GameObject*)entry->proceduralObject;
                if (((UnknownBikeRaceObjectFlags*)object)->isEnabled) {
                    UnknownFunctionCameraView()->UnknownFunctionTargetObject(object);
                } else {
                    return 0;
                }
            }
        } else {
            UnknownFunctionCameraView()->followingRacer = 1;
        }
    }
    if (UnknownFunctionCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
    }
    }
    return 1;
}

// Inline: racer `index` in camera order (own racer first, then the AI or
// remote racers).
inline UnknownBikeRaceRacer* BikeRace::UnknownFunctionRacerAt(int index) {
    int players = g_TrackGame->playerCount;
    if (players == 1 && index == 0) {
        return localRacer;
    }
    if (players > 1 || index < players) {
        return racerSlots[index];
    }
    return aiRacers[index - players];
}

// 0x0041f1d0
void BikeRace::NextCameraTarget(int forward, int racers, int objects) {
    int caster;
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    UnknownBikeRaceCameraView* camera = UnknownFunctionCameraView();
    if ((camera->overrideActive || camera->cameraState == 7) &&
        (camera->cameraState != 7 || racers || objects)) {
        return;
    }
    if (camera->followingRacer != racers) {
        camera->followingRacer = racers;
        if (AimCamera(objects == 0)) {
            return;
        }
    }
    if (racers) {
        caster = 0;
        int others;
        if (g_TrackGame->playerCount > 1) {
            others = g_TrackGame->networkAiRacerCount;
        } else if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            others = 1;
        } else {
            others = g_TrackGame->mode.raceSettings.aiRacerCount;
        }
        if (g_TrackGame->playerCount == 1 && others == 0) {
            return;
        }
        int old = targetRacer;
        do {
            if (forward) {
                targetRacer++;
            } else {
                targetRacer--;
            }
            if (targetRacer < 0) {
                targetRacer = g_TrackGame->playerCount + others - 1;
            }
            if (targetRacer >= g_TrackGame->playerCount + others) {
                targetRacer = 0;
            }
            if (old == targetRacer) {
                UnknownFunctionCameraView()->followingRacer = 1;
                return;
            }
        } while (UnknownFunctionRacerAt(targetRacer)->outOfRace != 0);
    } else if (objects) {
        caster = 0;
        Scene* scene = raceScene;
        if (scene->animationTable == 0 || scene->animationTable->entryCount <= 0) {
            UnknownFunctionCameraView()->followingRacer = 1;
            return;
        }
        int hidden = 1;
        int old = targetEntry;
        do {
            if (forward) {
                targetEntry++;
            } else {
                targetEntry--;
            }
            if (targetEntry < 0) {
                targetEntry = scene->animationTable->entryCount - 1;
            }
            if (targetEntry >= scene->animationTable->entryCount) {
                targetEntry = 0;
            }
            UnknownSceneEntry* entry = &scene->animationTable->entries[targetEntry];
            if (entry->keyFramed) {
                if (((UnknownBikeRaceSceneCharacter*)entry->character)->isEnabled) {
                    hidden = entry->excludeFromCameraIteration;
                } else {
                    hidden = 1;
                }
            } else if (((UnknownBikeRaceObjectFlags*)entry->proceduralObject)->isEnabled) {
                hidden = entry->excludeFromCameraIteration;
            } else {
                hidden = 1;
            }
            if (old == targetEntry) {
                UnknownBikeRaceCameraView* view = UnknownFunctionCameraView();
                if (view->targetCharacter != 0 || view->targetCar != 0) {
                    return;
                }
                entry = &scene->animationTable->entries[targetEntry];
                if (entry->keyFramed) {
                    if (!((UnknownBikeRaceSceneCharacter*)entry->character)->isEnabled) {
                        view->followingRacer = 1;
                        return;
                    }
                } else if (!((UnknownBikeRaceObjectFlags*)entry->proceduralObject)->isEnabled) {
                    view->followingRacer = 1;
                    return;
                }
                AimCamera(caster);
                return;
            }
        } while (hidden);
    } else {
        caster = 1;
        UnknownSceneBuffer* casters = raceScene->staticModels;
        if (casters == 0 || casters->casterCount <= 0) {
            UnknownFunctionCameraView()->followingRacer = 1;
            return;
        }
        int old = targetCaster;
        targetCaster = forward ? old + 1 : old - 1;
        if (targetCaster < 0) {
            targetCaster = raceScene->staticModels->casterCount - 1;
        }
        if (targetCaster >= raceScene->staticModels->casterCount) {
            targetCaster = 0;
        }
        if (old == targetCaster && UnknownFunctionCameraView()->shadowCaster != 0) {
            return;
        }
    }
    AimCamera(caster);
}


float FastInvSqrt(float x); // 0x00460c00 (FastMath)

static inline float DotProduct(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vector3& operator-=(Vector3& v, const Vector3& o) {
    v.x -= o.x;
    v.y -= o.y;
    v.z -= o.z;
    return v;
}

static inline float SquareMagnitude(const Vector3& v) {
    return DotProduct(v, v);
}

static inline Vector3 Scale(const Vector3& v, float s) {
    return Vector3(s * v.x, s * v.y, s * v.z);
}

float Vec3Dot(const Vector3* a, const Vector3* b); // 0x0040ae30: a.b

static inline Vector3 Normalize(const Vector3& v) {
    float length = SquareMagnitude(v);
    if (length == 1.0f) {
        return v;
    }
    float scale = FastInvSqrt(length);
    return Scale(v, scale);
}

// 0x004210f0
void BikeRace::WriteRelativePositions(Vector3* a, Vector3* b, void* reference, int count) {
    if (gridReady && count != 0) {
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        *b = gridDirection;
        return;
    }
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 2:
    case 3: {
        if (hasStartProbe) {
            a->x = startProbe.center.x;
            a->z = startProbe.center.z;
            a->y = 0.0f;
            b->x = startProbe.direction.x;
            b->z = startProbe.direction.z;
            b->y = 0.0f;
        } else {
            *a = raceScene->defaultStartPosition;
            *b = Normalize(raceScene->defaultStartDirection);
        }
        if (count <= 0) {
            return;
        }
        float side;
        if (raceTrack->startNode != 0 && raceTrack->startNode->firstSegment != 0 &&
            raceTrack->startNode->firstSegment->next != 0 &&
            raceTrack->startNode->firstSegment->next->next != 0) {
            TrackVec3 p0;
            TrackVec3 p1;
            raceTrack->SegmentDirection(raceTrack->startNode->firstSegment, &p0);
            raceTrack->SegmentDirection(raceTrack->startNode->firstSegment->next, &p1);
            if (p1.x * p0.z - p0.x * p1.z <= 0.0f) {
                side = 1.0f;
            } else {
                side = -1.0f;
            }
        } else {
            side = -1.0f;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(Scale(Scale(gridSlotStep, 6.75f), raceTerrain->gridCellSize), side);
        gridSlotStep = Scale(gridSlotStep, raceTerrain->gridCellSize * 15.5f / (racerCount + 1) * side);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    }
    case 0:
        *a = raceScene->defaultStartPosition;
        *b = Normalize(raceScene->defaultStartDirection);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, 20.0f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    case 1:
    case 5: {
        UnknownBikeRaceNodeOwner* owner = (UnknownBikeRaceNodeOwner*)reference;
        Vector3 direction = owner->gateSources[owner->gateCount - 1].direction;
        direction.y = 0.0f;
        *b = Normalize(direction);
        *a = owner->gateSources[owner->gateCount - 1].center;
        *a += Scale(*b, 8.0f);
        a->y = 0.0f;
        if (count <= 0) {
            return;
        }
        float width = owner->gateEntries[owner->gateCount - 1]->width * 1.05f;
        float spacing = width / (racerCount + 1);
        if (3.4f > spacing) {
            spacing = 3.4f;
        }
        gridDirection = *b;
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, width * 0.48f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), spacing);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    }
    case 4:
        *a = raceScene->defaultStartPosition;
        *b = Normalize(raceScene->defaultStartDirection);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, 20.0f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = gridFirstSlot + Scale(gridSlotStep, (float)count);
        break;
    default:
        return;
    }
    gridReady = true;
}

// DlgProcs.h's DemoDlg (vtable 0x00550e18, 0x7f58 bytes) with the inline
// constructor the setup's `new` expands (UIDialog(1, "credits.dtm"), then
// the vtable); DlgProcs.h declares no constructor for it.
class UnknownBikeRaceDemoDlg : public UIDialog {
public:
    UnknownBikeRaceDemoDlg(int flags, const char* resource) : UIDialog(flags, resource) {}
    virtual int Tick(float frameTime);                    // 0x00455b20
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x00455ae0
};

// The length of `v` (1 without a square root for unit vectors); the setup
// restores the lengths D3DRMVectorRotate's unit results lose.
static inline float UnknownVectorLength(const Vector3& v) {
    float xx = v.x * v.x;
    float yy = v.y * v.y;
    float lengthSquared = xx + yy + v.z * v.z;
    if (lengthSquared == 1.0f)
        return 1.0f;
    return (float)sqrt(lengthSquared);
}

static inline Vector3 UnknownScaled(const Vector3& v, float scale) {
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

// 0x00417ed0
BikeRace* BikeRace::Create(void* owner, void* particles, LightManager* lights,
                                          UnknownTextureFormatChoice* textures,
                                          UnknownBikeRaceProjector* terrain, KrustyBikeCamera* camera,
                                          Scene* scene, UnknownBikeRaceNodeOwner* event,
                                          UnknownBikeRaceShadow* shadow) {
    char name[0x104];
    char path[0x104];
    char modelName[0x104];
    char message[0x144];
    Vector3 position;
    Vector3 direction;
    Vector3 normal;
    Vector3 point;

    GameObject::SetRenderContext(owner);
    field_0x25_bit1 = 1;
    statusList = 0;
    aiSettings[0] = 50.0f;
    aiSettings[1] = 10.0f;
    aiSettings[2] = 40.0f;
    aiSettings[3] = 40.0f;
    aiSettings[4] = 60.0f;
    aiSettings[5] = 0.0f;
    aiSettings[6] = 100.0f;
    aiSettings[7] = 10.0f;
    aiSettings[8] = 10.0f;
    aiSettings[9] = 50.0f;
    aiSettings[10] = 100.0f;
    forceAiStep = 0;
    particleManager = (int)particles;
    raceScene = scene;
    raceShadow = shadow;
    if (!g_TrackGame->workingDirectory) {
        Release();
        return 0;
    }
    raceTerrain = terrain;
    raceCamera = camera;
    replayPaused = false;
    replayStepFrame = false;
    replayTime = 0;
    replayLength = 0;
    replayFrames = 0;
    replayLengthFrames = 0;
    g_TrackGame->ui->StepProgressBar();

    int mode = g_TrackGame->mode.raceSettings.eventType;
    switch (mode) {
    case 2:
    case 3: {
        COPY_TEXT(path, raceScene->environment->terrainFile, 0x104);
        strcpy(strrchr(path, '.'), ".tdf");
        UnknownTextureStream* stream =
            new(__FILE__, 0x193) UnknownTextureStream((int)g_ResourceManager);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)modelName)) {
            sprintf(message, "No Track file found in env file.  Abort.\n");
            delete stream;
            Release();
            return 0;
        }
        if (stream) {
            raceTrack = new(__FILE__, 0x1a5) Track;
            trackLoaded = raceTrack->LoadTrack(stream, &startProbe, &finishProbe, &hasStartProbe,
                                                 &hasFinishProbe);
            delete stream;
        }
        if (hasFinishProbe)
            raceTerrain->QueryGround(&finishProbe.center, 0, 0, 0);
        if (hasStartProbe)
            raceTerrain->QueryGround(&startProbe.center, 0, 0, 0);
        if (hasFinishProbe && hasStartProbe)
            UpdateTrackSeparation();
        break;
    }
    case 1:
    case 5:
        if (mode != 1 || !g_TrackGame->mode.raceSettings.randomGates) {
            sprintf(path, "%s\\%s%02d.tdf", g_TrackGame->sceneObject->sceneDirectory,
                    g_TrackGame->mode.raceSettings.trackName,
                    g_TrackGame->mode.raceSettings.trackNumber);
            UnknownTextureStream* stream =
                new(__FILE__, 0x1be) UnknownTextureStream((int)g_ResourceManager);
            if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)modelName)) {
                delete stream;
            } else if (stream) {
                {
                    raceTrack = new(__FILE__, 0x1d0) Track;
                    trackLoaded = raceTrack->LoadTrack(stream, &startProbe, &finishProbe,
                                                         &hasStartProbe, &hasFinishProbe);
                    if (!trackLoaded) {
                        Track* track = raceTrack;
                        if (track != 0) {
                            if (track->startNode != 0)
                                track->FreeReachableNodes(&track->startNode);
                            delete track;
                        }
                    }
                    delete stream;
                }
            }
        }
        trackLoaded = BuildNodeList(event);
        break;
    }
    g_TrackGame->ui->StepProgressBar();

    WriteRelativePositions(&position, &direction, event, 0);
    mode = g_TrackGame->mode.raceSettings.eventType;
    if (mode == 3 || mode == 2) {
        if (raceTrack) {
            if (!g_TrackGame->playingReplay) {
                if (hasStartProbe) {
                    direction.x = -startProbe.direction.x;
                    direction.z = -startProbe.direction.z;
                    point.x = startProbe.center.x - direction.x * 15.0f;
                    point.z = startProbe.center.z - direction.z * 15.0f;
                } else {
                    point = position;
                }
                terrain->QueryGround(&point, &normal, 0, 0);
                thirtySecondMan = (UnknownBikeRaceView6c*)raceScene->FindEntryByFileName("30SecMan.slt", &thirtySecondManEntry);
                if (thirtySecondMan) {
                    raceScene->RestartEntry(thirtySecondManEntry, 0, 0, 0);
                    raceScene->animationTable->entries[thirtySecondManEntry].animatesWhileHeld = 1;
                    if (thirtySecondMan->collisionObject) {
                        thirtySecondMan->collisionObject->Release();
                        thirtySecondMan->collisionObject = 0;
                    }
                }
                if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                    g_TrackGame->mode.raceSettings.raceMode != 4 && thirtySecondMan) {
                    thirtySecondMan->model->SetPositionVec3(point);
                    thirtySecondMan->model->SetAxesPtr(&direction, &kVec3YAxis, 1, 0);
                }
                if (thirtySecondMan)
                    thirtySecondMan->Disable();
                if (hasStartProbe) {
                    point.x = startProbe.center.x;
                    point.z = startProbe.center.z;
                    direction.x = startProbe.direction.x;
                    direction.z = startProbe.direction.z;
                } else {
                    point = position;
                }
                terrain->QueryGround(&point, &normal, 0, 0);
                if (g_TrackGame->mode.raceSettings.eventType == 3) {
                    startGate =
                        (UnknownBikeRaceView6c*)raceScene->FindEntryByFileName("StartGate.slb", &startGateEntry);
                    if (startGate) {
                        raceScene->RestartEntry(startGateEntry, 0, 0, 0);
                        if (startGate->collisionObject) {
                            startGate->collisionObject->Release();
                            startGate->collisionObject = 0;
                        }
                    }
                    if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                        g_TrackGame->mode.raceSettings.raceMode != 4) {
                        if (startGate) {
                            startGate->model->SetPositionVec3(point);
                            startGate->model->SetAxesPtr(&direction, &kVec3YAxis, 1, 0);
                        }
                    } else if (startGate) {
                        startGate->Disable();
                    }
                }
            }
            if (hasFinishProbe) {
                point.x = finishProbe.center.x;
                point.z = finishProbe.center.z;
                terrain->QueryGround(&point, 0, 0, 0);
                direction.x = finishProbe.direction.x;
                direction.y = 0.0f;
                direction.z = finishProbe.direction.z;
            }

            sprintf(name, "%s\\%s", "Res", "MetalArch.slt");
            ArcadeObject* arch = new(__FILE__, 0x247) ArcadeObject(1);
            arch = arch->Create(renderContext, (int)lights, (int)textures, name, kVec3Zero, 0, 0,
                                               2.0f, 0.5f, 0.5f, 0);
            AppendChild(arch, -1);
            arch->arcadeModel->SetUseLighting(1);
            float scale = finishProbe.width / arch->modelWidth * 1.111f;
            arch->arcadeModel->Scale(scale, scale, scale);
            arch->arcadeModel->SoultreeVirtualSlot6();
            arch->SetPosition(&point);
            arch->SetAxes(&direction, &kVec3YAxis, 1, 0);
            arch->BuildCollision(name, 0x6a, 0, 0, 0, 1);

            Vector3 girlOffset;
            Vector3 boxOffset;
            Vector3 turned;
            Vector3 ground;
            Vector3 girlPosition;
            Vector3 boxPosition;
            girlOffset = Vector3(scale * 9.2f, 8.0f, scale * -1.8f);
            boxOffset = Vector3(girlOffset.x - 1.22f, 4.0f, girlOffset.z + 1.2f);
            float angle = (float)atan2(direction.x, direction.z);
            D3DRMVectorRotate(&turned, &girlOffset, (Vector3*)&kVec3YAxis, angle);
            girlOffset = UnknownScaled(turned, UnknownVectorLength(girlOffset));
            ground = point + girlOffset;
            terrain->QueryGround(&ground, 0, 0, 0);
            float height = ground.y - point.y;
            girlPosition = point + girlOffset;
            D3DRMVectorRotate(&turned, &boxOffset, (Vector3*)&kVec3YAxis, angle);
            boxOffset = UnknownScaled(turned, UnknownVectorLength(boxOffset));
            boxPosition = point + boxOffset;
            girlPosition.y += height;
            boxPosition.y += height;

            if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                g_TrackGame->mode.raceSettings.raceMode != 4) {
                sprintf(modelName, "%s\\%s", "Res", "FlagGirl.mcf");
                flagGirl = (UnknownBikeRaceCharacter*)new(__FILE__, 0x26f) D3DIMSoultreeCharacter(1);
                AppendChild(flagGirl->UnknownVirtualSlot11(renderContext, modelName, lights, textures, 1, 1),
                                      -1);
                ReplaceMeshId(flagGirl, 1, 2);
                flagGirl->modelNode->SetUseLighting(1);
                flagGirl->modelNode->SetPositionVec3(girlPosition);
                flagGirl->modelNode->SetAxesPtr(&direction, &kVec3YAxis, 1, 0);
                flagGirl->SetMotionByName("Stand");
                flagGirl->UnknownVirtualSlot7(0.001f, 0, 0);
            }

            sprintf(name, "%s\\%s", "Res", "GirlBox.slt");
            ArcadeObject* box = new(__FILE__, 0x27c) ArcadeObject(1);
            box = box->Create(renderContext, (int)lights, (int)textures, name, kVec3Zero, 0, 0, 2.0f,
                                             0.5f, 0.5f, 0);
            AppendChild(box, -1);
            box->arcadeModel->SetUseLighting(1);
            box->SetPosition(&boxPosition);
            box->SetAxes(&direction, &kVec3YAxis, 1, 0);
            box->BuildCollision(name, 0x6a, 0, 0, 0, 1);
            g_TrackGame->eventManager->RemoveVegetationInRect(girlPosition.x - 4.0f, girlPosition.z - 4.0f,
                                                                       girlPosition.x + 4.0f, girlPosition.z + 4.0f);
        }
    } else if (mode == 0 || (mode == 4 && g_TrackGame->mode.raceSettings.stuntMode)) {
        BonusObjectManager* bonus = new(__FILE__, 0x28e) BonusObjectManager(1);
        scoreBoard = (int)bonus->SetupCounter(renderContext, (int)lights, (int)textures,
                                               (UnknownBonusRacer*)this, (UnknownBonusCamera*)raceCamera);
        if (!AppendChild((GameObject*)scoreBoard, -1))
            return 0;
    }
    g_TrackGame->ui->StepProgressBar();

    SetSceneTime(0.0f);
    raceScene->holdAnimations = 1;
    if (g_TrackGame->mode.raceSettings.raceMode) {
        NumberObjectManager* numbers = new(__FILE__, 0x29b) NumberObjectManager(1);
        digitCounter = (int)numbers->SetupDigits(renderContext, (int)textures, (UnknownNumberRacer*)this, 0x1e,
                                                (UnknownNumberCamera*)raceCamera, 3.0f);
        if (!AppendChild((GameObject*)digitCounter, -1))
            return 0;
    }
    if (g_TrackGame->mode.raceSettings.eventType != 4 &&
        g_TrackGame->mode.raceSettings.raceMode != 4 &&
        (g_TrackGame->playerCount > 1 || g_TrackGame->mode.raceSettings.aiRacerCount > 0)) {
        sprintf(name, "%s\\%s", "Res", "youind.slt");
        ArcadeObject* marker = new(__FILE__, 0x2a9) ArcadeObject(g_TrackGame->mode.playerIndicator);
        youIndicator = marker->Create(renderContext, (int)lights, (int)textures, name, position, 0, 0,
                                                    2.0f, 0.5f, 0.5f, 0);
        AppendChild(youIndicator, -1);
        Vector3 axis(0.0f, 1.0f, 0.0f);
        ((ArcadeObject*)youIndicator)->StartSpin(axis, 75.0f);
        ((ArcadeObject*)youIndicator)->arcadeModel->Scale(0.6f, 0.6f, 0.6f);
    }
    if (g_TrackGame->mode.raceSettings.raceMode == 4) {
        vcrFile = new(__FILE__, 0x2b6) UnknownVcrFile(1, 2);
        if (g_TrackGame->mode.ghostFile[0]) {
            vcrFile->Load(g_TrackGame->mode.ghostFile, "VCRgtemp.dat");
            ghostVcr = new(__FILE__, 0x2b9) KrustyVCR;
        }
        replayVcr = new(__FILE__, 0x2bd) KrustyVCR;
        ghostLap = 0;
    } else if (g_TrackGame->useReplayTape) {
        vcrFile = new(__FILE__, 0x2c2) UnknownVcrFile(0, 1);
        replayVcr = new(__FILE__, 0x2c3) KrustyVCR;
        if (g_TrackGame->playingReplay) {
            if (g_TrackGame->ui->playingDemo) {
                replayVcr->OpenRecording(RecorderCallback, 1, "ui\\demo.rpl", vcrFile);
                AppendChild(replayVcr, -1);
            } else {
                replayVcr->OpenRecording(RecorderCallback, 1, g_TrackGame->mode.replayFile,
                                                   vcrFile);
                AppendChild(replayVcr, -1);
            }
            savedCameraState = UnknownFunctionCameraView()->cameraState;
            raceCamera->UnknownVirtualSlot71(1);
        }
    }
    g_TrackGame->ui->StepProgressBar();

    if (!CreateRacers(owner, lights, textures, terrain, event))
        return 0;
    g_TrackGame->ui->StepProgressBar();

    if (thirtySecondMan) {
        if (hasStartProbe) {
            localRacer->modelNode->GetPosition(&point);
            point.x += localRacer->heading.x * 15.0f;
            point.z += localRacer->heading.z * 15.0f;
        }
        terrain->QueryGround(&point, 0, 0, 0);
        thirtySecondMan->model->SetPositionVec3(point);
    }
    if (g_TrackGame->playingReplay && g_TrackGame->mode.raceSettings.raceMode != 4 &&
        g_TrackGame->useReplayTape) {
        if (g_TrackGame->ui->playingDemo) {
            UnknownBikeRaceDemoDlg* dialog = new(__FILE__, 0x2fb) UnknownBikeRaceDemoDlg(1, "credits.dtm");
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 1, 0, 0, 0, 0, 1);
        } else {
            g_TrackGame->ui->gui->ShowCursors(1);
            VCRDlg* dialog = new(__FILE__, 0x2ff) VCRDlg(1, "VCR.dtm");
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 1, 0, 0, 0, 0, 1);
        }
    }
    if (replayVcr && !g_TrackGame->playingReplay) {
        g_TrackGame->ui->StepProgressBar();
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            COPY_TEXT(ghostFileRecorded, "VCRghost.dat", 0x104);
            replayVcr->OpenRecording(RecorderCallback, 0, ghostFileRecorded, vcrFile);
            if (ghostVcr) {
                COPY_TEXT(ghostFilePlayed, "VCRgtemp.dat", 0x104);
                ghostVcr->OpenRecording(RecorderCallback, 1, ghostFilePlayed, vcrFile);
                bestGhostLength = ghostVcr->RecordedLength();
                aiRacers[0]->UnknownFunction496e20(ghostVcr);
                if (startBeforeFinish)
                    aiRacers[0]->Disable();
                else
                    aiRacers[0]->Enable();
                AppendChild(ghostVcr, -1);
            }
        } else {
            replayVcr->OpenRecording(RecorderCallback, 0, "VCRtape.dat", vcrFile);
        }
        AppendChild(replayVcr, -1);
        isRecording = 1;
    }
    if (g_TrackGame->playerCount == 1 && g_TrackGame->mode.raceSettings.raceMode != 4) {
        int count = g_TrackGame->mode.raceSettings.aiRacerCount;
        localRacer->UnknownRacerVirtualSlot50(1, localRacer->UnknownRacerVirtualSlot45(), 0);
        for (int i = 0; i < count; i++)
            aiRacers[i]->UnknownRacerVirtualSlot50(1, aiRacers[i]->UnknownRacerVirtualSlot45(), 0);
    }
    g_TrackGame->ui->StepProgressBar();

    g_MemTagStack->Push("Audio");
    RaceSound* sound = new(__FILE__, 0x347) RaceSound(1);
    raceSound = (UnknownBikeRaceViews*)sound->Create(renderContext, (UnknownKrustyBikeView*)this,
                                                                      (UnknownRaceSoundCamera*)raceCamera, 6);
    AppendChild((GameObject*)raceSound, -1);
    g_MemTagStack->Push("BikeRace");
    if (g_TrackGame->mode.joystickOn) {
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(0);
        g_TrackGame->controlInterface->activeJoystick->SetAutoCenter(g_TrackGame->mode.joystickOn);
        localRacer->UnknownRacerVirtualSlot93();
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(1);
    }
    if (g_TrackGame->mode.raceSettings.eventType)
        BuildStatusList(&statusList, (UnknownKrustyBikeView*)this);
    if (ghostVcr)
        localRacer->fastestLap = bestGhostLength;
    g_TrackGame->ui->StepProgressBar();

    UnknownBikeNumberPainter* painter = new(__FILE__, 0x367) UnknownBikeNumberPainter(textures->manager);
    if (g_TrackGame->playingReplay) {
        painter->PaintNumber(localRacer->modelNode, localRacer->plateNumber);
        for (int i = 0; i < UnknownFunctionRacerCount() - 1; i++)
            painter->PaintNumber(aiRacers[i]->modelNode, aiRacers[i]->plateNumber);
    } else if (g_TrackGame->playerCount > 1) {
        for (int i = 0; i < UnknownFunctionRacerCount(); i++) {
            for (int j = 0; j < UnknownFunctionRacerCount(); j++) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                UnknownTrackGameRacerSlot* slot = &g_TrackGame->mode.racers[j];
                if (racer->recorderId == slot->networkPlayerId && racer->aiMessageIndex == slot->racerIndex)
                    painter->PaintNumber(racer->modelNode, slot->plateNumber);
            }
        }
    } else {
        painter->PaintNumber(localRacer->modelNode, g_TrackGame->mode.plateNumber);
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            painter->PaintNumber(aiRacers[0]->modelNode, g_TrackGame->mode.plateNumber);
        } else {
            for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++)
                painter->PaintNumber(aiRacers[i]->modelNode, aiRacers[i]->plateNumber);
        }
    }
    delete painter;
    g_TrackGame->ui->StepProgressBar();
    return this;
}

// A racer's skill: `base` moved up or down by at most `range`, closer to
// `base` more often.
static inline float UnknownRandomSkill(const float& base, const float& range) {
    float r = rand() * (1.0f / 32768.0f);
    float offset = (float)((1.0 - exp(-r * r)) * range);
    if (rand() * (1.0f / 32768.0f) > 0.5f)
        offset = -offset;
    return base + offset;
}

// The network view of start grid entry i (BikeRace reads +0x5 as the AI
// flag); the loader reads it through g_TrackGame each time.
static inline UnknownBikeRaceNetPlayer& GridPlayer(int i) {
    return ((UnknownBikeRaceNetPlayer*)g_TrackGame->mode.raceSettings.gridOrder)[i];
}

// Event AI racer n's record in the event manager; read through g_TrackGame
// each time (retail keeps only the byte offset n * 0x50 live).
static inline UnknownBikeRaceEventAi& EventAi(int n) {
    return ((UnknownBikeRaceEventAi*)g_TrackGame->eventManager->field_0x444)[n];
}

// The front end's bike, manufacturer and rider tables, read through
// g_TrackGame at each use as retail does.
static inline UnknownBikeRaceUiChoice& UiBike(int i) {
    return ((UnknownBikeRaceUiChoice*)g_TrackGame->ui->bikes)[i];
}

static inline UnknownBikeRaceUiModel& UiManufacturer(int i) {
    return ((UnknownBikeRaceUiModel*)g_TrackGame->ui->manufacturers)[i];
}

static inline UnknownBikeRaceUiModel& UiRider(int i) {
    return ((UnknownBikeRaceUiModel*)g_TrackGame->ui->riders)[i];
}

// 0x00419970
int BikeRace::CreateRacers(void* owner, LightManager* lights, UnknownTextureFormatChoice* textures,
                                    UnknownBikeRaceProjector* terrain, UnknownBikeRaceNodeOwner* event) {
    char path[0x104];
    char message[0x184];
    char bikeFile[0x104];
    char riderFile[0x104];
    char bikeTexture[0x40];
    char riderTexture[0x40];
    char name[0x10];
    char ownBikeTexture[0x40];
    char ownRiderTexture[0x40];
    char empty[0x104];
    int order[11];
    int ranking[11];
    Vector3 position;
    Vector3 direction;
    UnknownBikeRaceBikeSetup setup;
    int playerId = 0x7fffffff;
    int recordIndex = 0;
    int id = 0;
    float skill = aiSettings[0];
    Vector3 up(0.0f, 1.0f, 0.0f);
    empty[0] = 0;

    UnknownTextureStream* stream = new(__FILE__, 0x3b6) UnknownTextureStream((int)g_ResourceManager);
    sprintf(path, "%s\\rider.mcf", "Res");
    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)riderFile)) {
        sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
        delete stream;
        Release();
        return 0;
    }

    if (g_TrackGame->playingReplay) {
        if (g_TrackGame->mode.raceSettings.networkPlayerCount) {
            racerCount = g_TrackGame->mode.raceSettings.networkPlayerCount +
                          g_TrackGame->mode.raceSettings.multiplayerAiRacerCount;
            g_TrackGame->mode.raceSettings.aiRacerCount = racerCount - 1;
            g_TrackGame->mode.raceSettings.networkPlayerCount = 0;
        } else {
            racerCount = g_TrackGame->mode.raceSettings.aiRacerCount + g_TrackGame->playerCount;
        }
    } else {
        if (g_TrackGame->mode.raceSettings.raceMode == 4)
            racerCount = 1;
        else
            racerCount = UnknownFunctionRacerCount();
        recordIndex = 0;
        if (replayVcr)
            replayVcr->racerCount = racerCount;
    }
    if (g_TrackGame->playingReplay || g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 2) {
            RankByPoints(ranking);
            for (int i = 0; i < racerCount; i++) {
                for (int j = 0; j < racerCount; j++) {
                    if (ranking[j] == i + 1) {
                        order[i] = j + 1;
                        break;
                    }
                }
            }
        } else {
            for (int i = 0; i < racerCount; i++)
                order[i] = i + 1;
        }
    }

    if (g_TrackGame->InProCircuitCareer()) {
        // The pro circuit: the player's racer, then the computer racers of
        // the career.
        TrackGame* game = g_TrackGame;
        UnknownTrackGameObject3444* circuit = game->proCircuit;
        setup = *(UnknownBikeRaceBikeSetup*)game->mode.garageOptions;
        setup.engineSize = circuit->racers[0].engineSize;
        setup.fourStroke = circuit->racers[0].fourStroke;
        int bikeClass = BikeClassOf(setup.engineSize);
        setup.rpmLowerLimit = game->ui->rpmLowerLimit[bikeClass];
        setup.rpmUpperLimit = game->ui->rpmUpperLimit[bikeClass];
        setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
        setup.weight = game->ui->weight[bikeClass];
        WriteRelativePositions(&position, &direction, event, order[0]);
        sprintf(path, "%s\\%s", "Res", circuit->racers[0].bikeModel);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        char riderPath[0x104];
        char circuitRiderFile[0x104];
        if (circuit->careerFlags & 2) {
            sprintf(riderPath, "%s\\%s", "Res", "riderd.mcf");
        } else {
            COPY_TEXT(riderPath, riderFile, 0x104);
        }
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, riderPath, "rb",
                                                                       (int)circuitRiderFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        localRacer = new(__FILE__, 0x42a) UnknownBikeRaceRacer(1);
        localRacer->Load(owner, "", bikeFile, circuitRiderFile, lights, textures, position,
                                           direction, up, terrain, g_TrackGame->controlInterface, 0, 0, this,
                                           g_TrackGame->mode.profileName, 0, 0, &setup,
                                           g_TrackGame->mode.gasGyro,
                                           g_TrackGame->mode.brakeGyro, particleManager, raceScene,
                                           replayVcr, 0, !(circuit->careerFlags & 2));
        AppendChild(localRacer, -1);
        g_TrackGame->ui->ResolveBikeFile(circuit->racers[0].bikeName, ownBikeTexture, 0x3f);
        localRacer->modelNode->ReplaceMaterialTexture(0, ownBikeTexture, (int)textures);
        if (!(circuit->careerFlags & 2)) {
            g_TrackGame->ui->ResolveRiderFile(circuit->racers[0].riderModel, ownRiderTexture,
                                                             0x3f);
            localRacer->riderCharacter->modelNode->ReplaceMaterialTexture(0, ownRiderTexture, (int)textures);
        }
        if (g_TrackGame->mode.raceSettings.raceMode == 1) {
            id = 1;
            aiRacers = (UnknownBikeRaceRacer**)DebugMalloc(
                (g_TrackGame->proCircuit->racerCount - 1) * sizeof(UnknownBikeRaceRacer*), __FILE__,
                0x44f);
            for (int i = 0; i < g_TrackGame->proCircuit->racerCount - 1; i++)
                aiRacers[i] = 0;
            for (int n = 1; n < g_TrackGame->proCircuit->racerCount; n++) {
                UnknownProCircuitRacer* racer = &circuit->racers[n];
                WriteRelativePositions(&position, &direction, event, order[n]);
                sprintf(path, "%s\\%s", "Res", racer->bikeModel);
                if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
                    sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
                    delete stream;
                    Release();
                    return 0;
                }
                setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                setup.engineSize = racer->engineSize;
                setup.fourStroke = racer->fourStroke;
                bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                skill = UnknownRandomSkill(aiSettings[0], aiSettings[1]);
                aiRacers[n - 1] = new(__FILE__, 0x47c) UnknownBikeRaceRacer(1);
                aiRacers[n - 1]->Load(owner, empty, bikeFile, riderFile, lights, textures,
                                                          position, direction, up, terrain, 0, 1, 0, this,
                                                          racer->racerName, 0, id, &setup, 1, 1, particleManager,
                                                          raceScene, replayVcr, skill, 1);
                AppendChild(aiRacers[n - 1], -1);
                aiRacers[n - 1]->modelNode->ReplaceMaterialTexture(0, racer->bikeName, (int)textures);
                aiRacers[n - 1]->riderCharacter->modelNode->ReplaceMaterialTexture(0, racer->riderModel,
                                                                                     (int)textures);
                id++;
            }
        }
        return 1;
    }

    int used[8];
    if (!g_TrackGame->playingReplay && g_TrackGame->playerCount > 1) {
        racerSlots = new(__FILE__, 0x4a3) UnknownBikeRaceRacer*[racerCount];
        memset(used, 0, sizeof(used));
    }
    g_TrackGame->ui->StepProgressBar();
    char ai = 0;
    int* bikes = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4af);
    int* riders = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4b0);
    const char** names = (const char**)DebugMalloc(racerCount * 4, __FILE__, 0x4b1);
    int plate;
    int engineSize;
    int fourStroke;
    if (g_TrackGame->playingReplay) {
        // A replay: the recorded player.
        char recordedBike[0x40];
        char recordedRider[0x40];
        recordIndex = 1;
        replayVcr->GetRacer(&id, &ai, 0, name, bikeFile, riderFile, recordedBike, recordedRider,
                                           &engineSize, &fourStroke, &plate);
        g_TrackGame->ui->ResolveBikeFile(recordedBike, bikeTexture, 0x3f);
        g_TrackGame->ui->ResolveRiderFile(recordedRider, riderTexture, 0x3f);
    } else {
        sprintf(path, "%s\\%s", "Res", g_TrackGame->mode.racerChoice.bikeModel);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                         bikeTexture, 0x3f);
        g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                         riderTexture, 0x3f);
        if (!g_TrackGame->mode.raceSettings.multiplayerAiRacerCount) {
            g_TrackGame->ui->PickRandomRacers(bikes, racerCount, riders);
            g_TrackGame->ui->MakeRandomNames(names, racerCount);
        }
        strcpy(name, g_TrackGame->mode.profileName);
        plate = g_TrackGame->mode.plateNumber;
    }

    if (g_TrackGame->playingReplay || g_TrackGame->playerCount == 1) {
        // The player's racer offline (or in a replay).
        WriteRelativePositions(&position, &direction, event, order[0]);
        localRacer = new(__FILE__, 0x4ea) UnknownBikeRaceRacer(1);
        if (!AppendChild(
                localRacer->Load(
                    owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                    g_TrackGame->controlInterface, 0, 0, this, name, 0, id,
                    (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions,
                    g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro, particleManager,
                    raceScene, replayVcr, 0, 1),
                -1)) {
            sprintf(message, "Player Bike not loaded\n");
            if (riders)
                DebugFree(riders, __FILE__, 0x4ff);
            if (bikes)
                DebugFree(bikes, __FILE__, 0x500);
            if (names)
                DebugFree(names, __FILE__, 0x501);
            delete stream;
            Release();
            return 0;
        }
        localRacer->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
        localRacer->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
        if (!g_TrackGame->mode.raceSettings.treeCollision) {
            localRacer->collisionObject->ignoreVegetation = 1;
            localRacer->frontWheel->ignoreVegetation = 1;
            localRacer->rearWheel->ignoreVegetation = 1;
        }
        localRacer->engineSize = ((UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions)->engineSize;
        localRacer->fourStroke = (char)((UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions)->fourStroke;
        localRacer->plateNumber = plate;
        if ((replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) ||
            g_TrackGame->mode.raceSettings.raceMode == 4) {
            replayVcr->SetRacer(0, 0, recordIndex++, g_TrackGame->mode.profileName, bikeFile,
                                               riderFile, bikeTexture, riderTexture, localRacer->engineSize,
                                               (unsigned char)localRacer->fourStroke, plate);
        }
    } else {
        // A network race: this player's racer, the other players' and the
        // AI racers of other hosts.
        playerId = g_TrackGame->network->localPlayer;
        int i;
        int k;
        for (i = 0; i < racerCount; i++) {
            if (GridPlayer(i).networkPlayerId == playerId && !GridPlayer(i).isAi) {
                int j;
                for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                    if (GridPlayer(i).networkPlayerId == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex == 0) {
                        used[j] = 1;
                        break;
                    }
                }
                WriteRelativePositions(&position, &direction, event, i + 1);
                UnknownBikeRaceRacer* racer = new(__FILE__, 0x53b) UnknownBikeRaceRacer(1);
                racerSlots[j] = racer;
                if (!AppendChild(
                        racer->Load(
                            owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                            g_TrackGame->controlInterface, 0, 0, this, g_TrackGame->mode.profileName, 0,
                            playerId, (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions,
                            g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro,
                            particleManager, raceScene, replayVcr, 0, 1),
                        -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                    if (riders)
                        DebugFree(riders, __FILE__, 0x550);
                    if (bikes)
                        DebugFree(bikes, __FILE__, 0x551);
                    if (names)
                        DebugFree(names, __FILE__, 0x552);
                    delete stream;
                    Release();
                    return 0;
                }
                g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                                 ownBikeTexture, 0x3f);
                g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                                 ownRiderTexture, 0x3f);
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    racerSlots[j]->collisionObject->ignoreVegetation = 1;
                    racerSlots[j]->frontWheel->ignoreVegetation = 1;
                    racerSlots[j]->rearWheel->ignoreVegetation = 1;
                }
                racerSlots[j]->modelNode->ReplaceMaterialTexture(0, ownBikeTexture, (int)textures);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, ownRiderTexture, (int)textures);
                racerSlots[j]->plateNumber = g_TrackGame->mode.plateNumber;
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(playerId, 0, recordIndex++, g_TrackGame->mode.profileName,
                                                       bikeFile, riderFile, ownBikeTexture, ownRiderTexture,
                                                       racerSlots[j]->engineSize,
                                                       (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
                }
                localRacer = racerSlots[j];
                targetRacer = j;
                break;
            }
        }
        for (i = 0; i < racerCount; i++) {
            if (GridPlayer(i).networkPlayerId == playerId || GridPlayer(i).isAi)
                continue;
            int j;
            for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                if (GridPlayer(i).networkPlayerId == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex == 0) {
                    used[j] = 1;
                    break;
                }
            }
            WriteRelativePositions(&position, &direction, event, i + 1);
            setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
            setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
            setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
            int bikeClass = BikeClassOf(setup.engineSize);
            setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
            setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
            for (k = 0; k < 11; k++)
                setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
            setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
            setup.weight = g_TrackGame->ui->weight[bikeClass];
            sprintf(path, "%s\\%s", "Res", g_TrackGame->mode.racers[j].bikeModelName);
            UnknownBikeRaceRacer* racer = new(__FILE__, 0x5a3) UnknownBikeRaceRacer(1);
            racerSlots[j] = racer;
            if (!AppendChild(racer->Load(
                                           owner, empty, path, riderFile, lights, textures, position, direction,
                                           up, terrain, 0, 0, 1, this, g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId,
                                           &setup, 0, 1, particleManager, raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racers[j].bikeName, bikeTexture, 0x3f);
            racerSlots[j]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
            if (g_TrackGame->GetRegistryInt("MPR", 0) == 123 && g_TrackGame->mode.racers[j].hasMprSkin) {
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, "MPR.tga", (int)textures);
            } else {
                g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racers[j].riderName, riderTexture, 0x3f);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
            }
            if (!g_TrackGame->mode.raceSettings.treeCollision) {
                racerSlots[j]->collisionObject->ignoreVegetation = 1;
                racerSlots[j]->frontWheel->ignoreVegetation = 1;
                racerSlots[j]->rearWheel->ignoreVegetation = 1;
            }
            racerSlots[j]->Disable();
            racerSlots[j]->messageDelay = g_TrackGame->mode.racers[j].lowestMessageDelay;
            if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, 0, recordIndex++, g_TrackGame->mode.racers[j].playerName, path,
                                                   riderFile, bikeTexture, riderTexture, racerSlots[j]->engineSize,
                                                   (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
            }
        }
        for (i = 0; i < racerCount; i++) {
            int other = GridPlayer(i).networkPlayerId;
            if (other == playerId || !GridPlayer(i).isAi)
                continue;
            int j;
            for (j = 0; j < racerCount; j++) {
                if (other == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex != 0 && !used[j]) {
                    used[j] = 1;
                    break;
                }
            }
            WriteRelativePositions(&position, &direction, event, i + 1);
            setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
            setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
            setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
            int bikeClass = BikeClassOf(setup.engineSize);
            setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
            setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
            for (k = 0; k < 11; k++)
                setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
            setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
            setup.weight = g_TrackGame->ui->weight[bikeClass];
            UnknownBikeRaceRacer* racer = new(__FILE__, 0x61f) UnknownBikeRaceRacer(1);
            racerSlots[j] = racer;
            if (!AppendChild(racer->Load(
                                           owner, empty, g_TrackGame->mode.racers[j].bikeModelName, riderFile, lights, textures, position,
                                           direction, up, terrain, 0, (char)g_TrackGame->mode.racers[j].racerIndex, 1, this,
                                           g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId, &setup, 0, 1, particleManager,
                                           raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            racerSlots[j]->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].bikeName, (int)textures);
            racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].riderName, (int)textures);
            if (!g_TrackGame->mode.raceSettings.treeCollision) {
                racerSlots[j]->collisionObject->ignoreVegetation = 1;
                racerSlots[j]->frontWheel->ignoreVegetation = 1;
                racerSlots[j]->rearWheel->ignoreVegetation = 1;
            }
            racerSlots[j]->Disable();
            racerSlots[j]->messageDelay = g_TrackGame->mode.racers[j].lowestMessageDelay;
            if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, (char)g_TrackGame->mode.racers[j].racerIndex, recordIndex++,
                                                   g_TrackGame->mode.racers[j].playerName, g_TrackGame->mode.racers[j].bikeModelName, riderFile,
                                                   g_TrackGame->mode.racers[j].bikeName, g_TrackGame->mode.racers[j].riderName,
                                                   racerSlots[j]->engineSize,
                                                   (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
            }
        }
    }

    ((FollowCamera*)raceCamera)->AppendTableEntry(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);
    ((FollowCamera*)raceCamera)->AppendTableEntry(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);

    int fromEvent = 0;
    if ((g_TrackGame->playingReplay && replayVcr->racerCount > 1) || g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            // The ghost.
            aiRacers = new(__FILE__, 0x674) UnknownBikeRaceRacer*[1];
            WriteRelativePositions(&position, &direction, event, order[0]);
            sprintf(bikeFile, "%s\\%s", "Res", "Ghost.mcf");
            sprintf(riderFile, "%s\\GhostRider.mcf", "Res");
            UnknownBikeRaceRacer* racer = new(__FILE__, 0x67c) UnknownBikeRaceRacer(1);
            aiRacers[0] = racer;
            if (!AppendChild(
                    racer->Load(
                        owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain, 0, -1,
                        0, this, "Ghost", 0, 0, (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions,
                        g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro,
                        particleManager, raceScene, replayVcr, 0, 1),
                    -1)) {
                sprintf(message, "Ghost Bike not loaded\n");
            }
            replayVcr->SetRacer(0, 0, recordIndex, g_TrackGame->mode.profileName, bikeFile,
                                               riderFile, bikeTexture, riderTexture, aiRacers[0]->engineSize,
                                               (unsigned char)aiRacers[0]->fourStroke,
                                               g_TrackGame->mode.plateNumber);
            aiRacers[0]->UnknownRacerVirtualSlot50(1, 0, 1);
            aiRacers[0]->Disable();
        } else if (g_TrackGame->mode.raceSettings.aiRacerCount > 0) {
            // The AI racers.
            id = 1;
            aiRacers = new(__FILE__, 0x6a5) UnknownBikeRaceRacer*[g_TrackGame->mode.raceSettings.aiRacerCount];
            for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++)
                aiRacers[i] = 0;
            for (int n = 0; n < g_TrackGame->mode.raceSettings.aiRacerCount; n++) {
                char aiName[0x10];
                if (g_TrackGame->playingReplay) {
                    char recordedBike[0x40];
                    char recordedRider[0x40];
                    replayVcr->GetRacer(&id, &ai, recordIndex, aiName, bikeFile, riderFile,
                                                       recordedBike, recordedRider, &engineSize, &fourStroke,
                                                       &plate);
                    setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                    g_TrackGame->ui->ResolveBikeFile(recordedBike, bikeTexture, 0x3f);
                    g_TrackGame->ui->ResolveRiderFile(recordedRider, riderTexture, 0x3f);
                } else {
                    if (g_TrackGame->mode.raceSettings.races > 0 &&
                        g_TrackGame->eventManager->racesRun > 0) {
                        fromEvent = 1;
                        COPY_TEXT(aiName, g_TrackGame->eventManager->results[n + 1].name, 0x10);
                        COPY_TEXT(bikeFile, EventAi(n).bikeModelPath, 0x104);
                    } else {
                        COPY_TEXT(aiName, names[n], 0x10);
                        sprintf(bikeFile, "%s\\%s", "Res", UiManufacturer(UiBike(bikes[n]).manufacturer).modelFile);
                    }
                    ai = 1;
                    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, bikeFile, "rb",
                                                                                   (int)bikeFile)) {
                        sprintf(message, "No Net Bike MCF found in resources.  Aborting.\n");
                        delete stream;
                        Release();
                        return 0;
                    }
                    setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                    if (fromEvent) {
                        engineSize = EventAi(n).engineSize;
                        fourStroke = EventAi(n).fourStroke;
                        skill = EventAi(n).skill;
                    } else {
                        engineSize = UiBike(bikes[n]).engineSize;
                        fourStroke = UiBike(bikes[n]).fourStroke;
                        if (g_TrackGame->mode.raceSettings.eventType == 0)
                            skill = UnknownRandomSkill(aiSettings[5], aiSettings[6]);
                        else
                            skill = UnknownRandomSkill(aiSettings[0], aiSettings[1]);
                    }
                    float r = rand() * (1.0f / 32768.0f);
                    plate = 101 - (int)(r * -898.0f);
                }
                WriteRelativePositions(&position, &direction, event, order[n + 1]);
                setup.engineSize = engineSize;
                setup.fourStroke = fourStroke;
                int bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                UnknownBikeRaceRacer* racer = new(__FILE__, 0x718) UnknownBikeRaceRacer(1);
                aiRacers[n] = racer;
                if (!AppendChild(racer->Load(
                                               owner, empty, bikeFile, riderFile, lights, textures, position,
                                               direction, up, terrain, 0, ai, 0, this, aiName, 0, id, &setup, 1, 1,
                                               particleManager, raceScene, replayVcr, skill, 1),
                                           -1)) {
                    sprintf(message, "AI Bike%d not loaded\n", n);
                }
                if (g_TrackGame->playingReplay) {
                    char text[0x40];
                    COPY_TEXT(text, bikeTexture, 0x40);
                    g_TrackGame->ui->ResolveBikeFile(text, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
                    COPY_TEXT(text, riderTexture, 0x40);
                    g_TrackGame->ui->ResolveRiderFile(text, riderTexture, 0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
                    aiRacers[n]->plateNumber = plate;
                    recordIndex++;
                } else if (fromEvent) {
                    g_TrackGame->ui->ResolveBikeFile(EventAi(n).bikeTexture, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
                    g_TrackGame->ui->ResolveRiderFile(EventAi(n).riderTexture, riderTexture, 0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
                    aiRacers[n]->plateNumber = plate;
                } else {
                    g_TrackGame->ui->ResolveBikeFile(UiBike(bikes[n]).bikeName, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
                    g_TrackGame->ui->ResolveRiderFile(UiRider(riders[n]).modelFile, riderTexture,
                                                                     0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
                    aiRacers[n]->plateNumber = plate;
                }
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    aiRacers[n]->collisionObject->ignoreVegetation = 1;
                    aiRacers[n]->frontWheel->ignoreVegetation = 1;
                    aiRacers[n]->rearWheel->ignoreVegetation = 1;
                }
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(id, 1, recordIndex++, aiName, path, riderFile, bikeTexture,
                                                       riderTexture, aiRacers[n]->engineSize,
                                                       (unsigned char)aiRacers[n]->fourStroke, plate);
                }
                id++;
                if (g_TrackGame->mode.raceSettings.raceMode == 2 &&
                    g_TrackGame->mode.raceSettings.races > 0 &&
                    !g_TrackGame->eventManager->racesRun) {
                    strcpy(EventAi(n).bikeTexture, bikeTexture);
                    strcpy(EventAi(n).riderTexture, riderTexture);
                    strcpy(EventAi(n).bikeModelPath, bikeFile);
                    EventAi(n).engineSize = engineSize;
                    EventAi(n).fourStroke = fourStroke;
                    EventAi(n).skill = skill;
                }
            }
        }
    } else {
        // A network race: the AI racers this host runs.
        id = 1;
        if (g_TrackGame->mode.raceSettings.aiRacerCount > 0) {
            aiRacers = new(__FILE__, 0x770) UnknownBikeRaceRacer*[g_TrackGame->mode.raceSettings.aiRacerCount];
            for (int i = 0; i < racerCount; i++) {
                int other = GridPlayer(i).networkPlayerId;
                if (other != playerId || !GridPlayer(i).isAi)
                    continue;
                int j;
                for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                    if (other == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex != 0 && !used[j]) {
                        used[j] = 1;
                        break;
                    }
                }
                WriteRelativePositions(&position, &direction, event, i + 1);
                setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
                setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
                int bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                UnknownBikeRaceRacer* racer = new(__FILE__, 0x78f) UnknownBikeRaceRacer(1);
                racerSlots[j] = racer;
                if (!AppendChild(racer->Load(
                                               owner, empty, g_TrackGame->mode.racers[j].bikeModelName, riderFile, lights, textures,
                                               position, direction, up, terrain, 0, (char)g_TrackGame->mode.racers[j].racerIndex, 0,
                                               this, g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId, &setup, 0, 1,
                                               particleManager, raceScene, replayVcr, 0, 1),
                                           -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                }
                racerSlots[j]->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].bikeName, (int)textures);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].riderName, (int)textures);
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    racerSlots[j]->collisionObject->ignoreVegetation = 1;
                    racerSlots[j]->frontWheel->ignoreVegetation = 1;
                    racerSlots[j]->rearWheel->ignoreVegetation = 1;
                }
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, (char)g_TrackGame->mode.racers[j].racerIndex, recordIndex++,
                                                       g_TrackGame->mode.racers[j].playerName, g_TrackGame->mode.racers[j].bikeModelName, riderFile,
                                                       g_TrackGame->mode.racers[j].bikeName, g_TrackGame->mode.racers[j].riderName,
                                                       racerSlots[j]->engineSize,
                                                       (unsigned char)racerSlots[j]->fourStroke,
                                                       g_TrackGame->mode.racers[j].plateNumber);
                }
                aiRacers[id - 1] = racerSlots[j];
                id++;
            }
        }
        // Every racer's collisions against every other racer's.
        for (int a = 0; a < racerCount; a++) {
            if (!racerSlots[a]->isRemote)
                continue;
            for (int b = 0; b < racerCount; b++) {
                if (a == b)
                    continue;
                if (racerSlots[b]->isRemote) {
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->frontWheel);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->rearWheel);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->frontWheel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->rearWheel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                }
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(
                    racerSlots[b]->wrecker->collisionModel);
            }
        }
    }
    if (riders)
        DebugFree(riders, __FILE__, 0x7f7);
    if (bikes)
        DebugFree(bikes, __FILE__, 0x7f8);
    if (names)
        DebugFree(names, __FILE__, 0x7f9);
    delete stream;
    return 1;
}
