// KrustyBikeSelectRiderPose.cpp -- KrustyBike::SelectRiderPose (0x0048e3e0, 2707 B plus the
// 4- and 5-entry jump tables to 0x0048ee98, ret 4), slot 89, overriding Bike 0x00406ae0.
// Status: best-guess-unreviewed.  The game build links it with /QIfist (config/game_build.json
// candidate_sources); it used to live in src/krusty2/vehicle/KrustyBike.cpp, whose other casts
// truncate through __ftol as retail does, so that unit cannot take the flag.  The function sits
// among KrustyBike.cpp's methods (0x0048d77c..0x00497cb0), but this file makes no claim about
// the original translation unit beyond that.
//
// Decoded from retail (tier 2 shape; names tier 3).  Bike's version with four differences:
//  - pad_0x5B8 set (Vehicle.cpp clears it; slot 102 tests it with crashState): fixed steer
//    pose (poseIndex 3, poseState 4, poseBlend 1, poseLeanBlend 0) and the state-1 mapping
//    of controlInput.x to poseParam, without touching steerPoseState;
//  - a local (not AI, remote or ghost) rider airborne and uncrashed with both wheels more
//    than 2.5 clear of the ground starts the head turn 0x0048e280 (control 9 while the
//    crossover pose runs) or the knocked-off animation 0x0048d910 picked by controls 6/7
//    and the stick sector (slot 88), and returns;
//  - no lean pose (leanPoseMin/leanPoseMax): backingUp selects pose 16 (BackUp) instead;
//  - the crossover smoother chases crossupScale * controlInput.x clamped to [-1, 1]
//    (0x48e828..0x48e85f: `fld [esi+0x63c] ; fmul [esi+0x504]`, then the -1/1 clamp).
// The rest (crossover pose, slip/steer pose, both steerPoseState machines) is Bike's.  Both
// jump tables (0x0048ee74: cases 1..4, 0x0048ee84: cases 1..5) were checked case by case
// against the source, as were every threshold, comparison sense and stored field.
//
// Rounding: retail converts the pose position with its inline fistp helper at 0x0048e61f,
// 0x0048e6f0 and 0x0048e7c5 (`fstp [t]; fld [t]; mov eax, [out]; fistp dword ptr [eax]`):
// the argument is rounded to binary32, then rounded to an integer under the current x87
// mode, so fistp(p - 0.5f) is floor(p) apart from exact ties.  FistpFloatToIntChecked gives
// that under /QIfist, and 0x80000000 for a result outside int as the 32-bit store does.
// steerValue is not bounded anywhere this file can show (AI writes (lean + 1) * 0.25, the
// player axis is smoothed toward its target), so the checked form is used.  The old
// canonical body corrected a truncating __ftol to round-to-nearest-even (three __ftol calls
// retail does not make) and ignored the rounding mode.  Without /QIfist (the physics
// runner, which scores shape only) the casts truncate through __ftol.
//
// Shape: retail's ebp frame comes from the inline asm; the frame pragma below keeps it
// (linked 0.682 -> 0.729).  Session 23 (linked masked 0.729 -> 0.789, strict 0.686 -> 0.745):
//  - KbAtLeast as `x > lo ? x : lo`: retail branches to the shared lower-bound exit
//    (`fcomp lo; test ah,0x41; jne LO`); KbAtMost's spelling is inert;
//  - `(t * -0.25f) + 1.0f` keeps retail's `fmul -0.25; fadd 1.0` (unparenthesised, VC6 folds it
//    to `1.0 - t*0.25`);
//  - the crossover state machine's case 1 stores poseParam before the state (as case 3 does);
//  - the smoother step through KbAtMost (`fld time; fld t; fcomp st(1)`);
//  - steer case 1 written with the state store inside each arm, as retail lays it out.
// What remains comes from retail's inline asm: retail cross-jumps none of its identical clamp
// arms (VC6 merges them here, 2312 bytes against 2708), keeps 1 in edi where this body caches
// the steer axis across the conversion, and stores 32 bits where /QIfist needs the 64-bit
// store plus the range check.  Ruled out: a `const float&` conversion helper (drops the
// binary32 store, a behaviour change at 53-bit precision); `1.0f - t * 0.25f`,
// `1.0f + t * -0.25f`, `-(t * 0.25f) + 1.0f` (all fold); both clamp helpers flipped (same as
// KbAtLeast alone); `smoothTime > t ? t : smoothTime`.
#include <float.h>

#include "vehicle/KrustyBike.h"
#include "../../common/FistpFloatToInt.h"

static inline float KbAtLeast(float x, float lo)
{
    return x > lo ? x : lo;
}

static inline float KbAtMost(float x, float hi)
{
    return x >= hi ? hi : x;
}

static inline float KbRange(float x, float lo, float hi)
{
    return KbAtMost(KbAtLeast(x, lo), hi);
}

// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
void KrustyBike::SelectRiderPose(float t)
{
    if (IsRiderPoseLocked())
        return;

    float x;
    if (*(int*)pad_0x5B8) {
        poseBlend = 1.0f;
        poseState = RiderPose_Turn1U;
        poseIndex = RiderPose_Turn1D;
        poseLeanBlend = 0.0f;
        if (controlInput.x < 0.0f) {
            x = controlInput.x * -0.25f;
            poseParam = KbRange(x, 0.0f, 0.25f);
        } else {
            x = controlInput.x * 0.25f + 0.5f;
            poseParam = KbRange(x, 0.5f, 0.75f);
        }
        return;
    }

    if (!aiKind && !isRemote && !isGhost && airborne && !crashState
        && frontWheel->penetration < -2.5f && rearWheel->penetration < -2.5f) {
        if (QueryControlInput(9, 0x3f) && crossoverPose) {
            LookBackAtNearestRival();
            return;
        }
        int first = QueryControlInput(6, 0x3f);
        int second = QueryControlInput(7, 0x3f);
        if (first || second) {
            int direction = ClassifyInputDirection();
            if (direction != -1) {
                StartTrick(direction + (first ? 0 : 8));
                return;
            }
        }
    }

    float lean;
    if (airborne || rearWheel->inContact)
        lean = -frontWheel->penetration;
    else
        lean = 0.0f;

    if (airborne && lean > 2.0f && rearWheel->penetration < -1.0f) {
        FistpFloatToIntChecked(&poseIndex, (0.75f - steerAxis->steerValue) * 3.99f - 0.5f);
        int idx = poseIndex;
        poseIndex = idx + RiderPose_CrosOv1;
        poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - idx;
        if (crossoverPose == 0) {
            BikeA644* smoother = poseSmoother;
            float initialTime = poseSmoothTime;
            *(float*)&smoother->smoothedValue = 0.0f;
            if (initialTime != FLT_MAX) {
                smoother->smoothTime = initialTime;
                smoother->smoothRatio = 1.0f;
            }
            smoother->maxRise = 1.0f;
            smoother->maxFall = -1.0f;
            steerPoseState = 1;
        }
        crossoverPose = 1;
    } else {
        EndMotion();
        if (rearWheel->slipSpeed > 14.0f) {
            FistpFloatToIntChecked(&poseIndex, (0.75f - steerAxis->steerValue) * 3.99f - 0.5f);
            int idx = poseIndex;
            if (idx == 2) {
                if (rearWheel->slipSpeed > 32.0f)
                    poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - 2.0f;
                else
                    poseBlend = (rearWheel->slipSpeed - 14.0f) * 0.0555f;
            } else {
                poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - idx;
            }
            poseState = idx * 2 + RiderPose_Turn1U;
            poseIndex = poseState - 1;
        } else if (backingUp) {
            poseIndex = RiderPose_BackUpTop;
            poseState = 5;
        } else {
            FistpFloatToIntChecked(&poseIndex, (0.75f - steerAxis->steerValue) * 3.99f - 0.5f);
            int idx = poseIndex;
            poseState = idx * 2 + RiderPose_Turn1D;
            poseBlend = (0.75f - steerAxis->steerValue) * 3.99f - idx;
            poseLeanBlend = (14.0f - rearWheel->slipSpeed) * 0.0714285746f;
        }
    }

    if (crossoverPose) {
        // advance the smoothed value toward the scaled input, limited by its rate
        float target = KbRange(crossupScale * controlInput.x, -1.0f, 1.0f);
        BikeA644* smoother = poseSmoother;
        float change = target - *(float*)&smoother->smoothedValue;
        if (change < 0.0f)
            change = KbAtLeast(change, smoother->maxFall);
        else
            change = KbAtMost(change, smoother->maxRise);
        float step = KbAtMost(t, smoother->smoothTime);
        smoother->smoothRatio = step / smoother->smoothTime;
        *(float*)&smoother->smoothedValue = change * smoother->smoothRatio + *(float*)&smoother->smoothedValue;
        t = *(float*)&smoother->smoothedValue;
        switch (steerPoseState) {
        case 1:
            x = t * -0.25f;
            poseParam = x;
            if (x < 0.0f) {
                poseParam = 0.5f - x;
                steerPoseState = 3;
            } else if (x > 0.225f) {
                poseParam = 0.5f - x;
                steerPoseState = 2;
            }
            break;
        case 2:
            x = t * 0.25f + 0.5f;
            poseParam = x;
            if (x > 0.5f)
                steerPoseState = 3;
            break;
        case 3:
            x = t * 0.25f + 0.5f;
            poseParam = x;
            if (x < 0.5f) {
                poseParam = 0.5f - x;
                steerPoseState = 1;
            } else if (x > 0.725f) {
                poseParam = 1.5f - x;
                steerPoseState = 4;
            }
            break;
        case 4:
            x = (t * -0.25f) + 1.0f;
            poseParam = x;
            if (x > 1.0f) {
                poseParam = x - 1.0f;
                steerPoseState = 1;
            }
            break;
        }
    } else {
        switch (steerPoseState) {
        case 1:
            if (controlInput.x < 0.0f) {
                if (controlInput.x < -0.9f) {
                    steerPoseState = 2;
                    x = controlInput.x * -0.25f;
                    poseParam = KbRange(x, 0.0f, 0.25f);
                } else {
                    x = controlInput.x * -0.25f;
                    poseParam = KbRange(x, 0.0f, 0.25f);
                }
            } else {
                if (controlInput.x > 0.9f) {
                    steerPoseState = 4;
                    x = controlInput.x * 0.25f + 0.5f;
                    poseParam = KbRange(x, 0.5f, 0.75f);
                } else {
                    x = controlInput.x * 0.25f + 0.5f;
                    poseParam = KbRange(x, 0.5f, 0.75f);
                }
            }
            break;
        case 2:
            if (controlInput.x > prevControlInput.x) {
                steerPoseState = 3;
                x = controlInput.x * 0.25f + 0.5f;
                poseParam = KbRange(x, 0.25f, 0.5f);
            } else {
                x = controlInput.x * -0.25f;
                poseParam = KbRange(x, 0.0f, 0.25f);
            }
            break;
        case 3:
            if (controlInput.x > -0.032f) {
                steerPoseState = 1;
                if (controlInput.x < 0.0f) {
                    x = controlInput.x * -0.25f;
                    poseParam = KbRange(x, 0.0f, 0.25f);
                } else {
                    x = controlInput.x * 0.25f + 0.5f;
                    poseParam = KbRange(x, 0.5f, 0.75f);
                }
            } else {
                x = controlInput.x * 0.25f + 0.5f;
                poseParam = KbRange(x, 0.25f, 0.5f);
            }
            break;
        case 4:
            if (controlInput.x < prevControlInput.x) {
                steerPoseState = 5;
                x = (controlInput.x * -0.25f) + 1.0f;
                poseParam = KbRange(x, 0.75f, 1.0f);
            } else {
                x = controlInput.x * 0.25f + 0.5f;
                poseParam = KbRange(x, 0.5f, 0.75f);
            }
            break;
        case 5:
            if (controlInput.x < 0.032f) {
                steerPoseState = 1;
                if (controlInput.x < 0.0f) {
                    x = controlInput.x * 0.25f + 0.5f;
                    poseParam = KbRange(x, 0.5f, 0.75f);
                } else {
                    x = controlInput.x * -0.25f;
                    poseParam = KbRange(x, 0.0f, 0.25f);
                }
            } else {
                x = (controlInput.x * -0.25f) + 1.0f;
                poseParam = KbRange(x, 0.75f, 1.0f);
            }
            break;
        }
    }
}
#pragma optimize("", on)
