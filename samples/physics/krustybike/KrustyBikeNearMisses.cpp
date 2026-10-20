// Near-miss KrustyBike.cpp candidates (D:\aardvark\VC\krusty2\KrustyBike.cpp), kept out
// of src/krusty2/vehicle/KrustyBike.cpp until they match.  The canonical file is
// included first so the TU-local statics, helpers and the Math3D vector set are the
// same.  Each function's note records what still differs; the scores are in
// targets.json and docs/NEAR_MISS_INDEX.md.
#include "vehicle/KrustyBike.cpp"

// 0x0048fa60 (near miss, 86.36%, 532 of 532 bytes): the GameObject virtual base is
// built only for the most-derived object (GameObject(1)), then Bike(flags). Every
// store matches; the sole divergence (0xb7..0xfa) is which two stores VC6 hoists
// into the load delay of the +0x1540 kVec3Zero copy: ours picks +0x7c0/+0x7bc,
// retail +0x11b8/+0x604 (the next two statements).  Measured: the pair is chosen at
// a fixed distance from the END of the block, counted in statements (one added
// store shifts it by one, regardless of size; lea-based copy stores, duplicate
// stores, type punning and folded inline guards do not count).  Retail's pick is
// reproduced exactly by dropping the last eight stores, so retail's stream has
// eight fewer scheduling units after the copy for a reason not yet found.  Other
// orders of the first four statements, init-list members, component-wise or
// chained copies and an inline helper for the +0x7b8/+0x7c0/+0x7bc triple (also
// written together by slot 97) do not move the pick.  Re-measured: the pick index
// is (units after the copy) - 32 and is end-anchored only; units added before the
// copy, `a = b = 0` chains, `0.0f` vs `0` literals and moving +0x11b8/+0x604 ahead
// of the copy leave it in place (those two then simply stay before the copy), and a
// redundant store to one field is eliminated.  Adding many stores instead makes the
// scheduler hoist copy 2's lea and the EH-state reload far up.
// 0..15 dead initialised int locals at the top of the function do not move the pick either.
KrustyBike::KrustyBike(int flags) : GameObject(1), Bike(flags)
{
    plateNumber = 0x65;
    lastFramePosition = kVec3Zero;
    hasFullState = 0;
    wrecker = 0;
    aiKind = 0;
    isRemote = 0;
    score = 0;
    pingTimer = 0;
    race = 0;
    lapGate = 0;
    forceGatePass = 0;
    nextLapGate = 1;
    finishTimeStamp = 0;
    netId = 0;
    trickLandTimer = 0;
    driveForceScale = 1.0f;
    largestStuntPoints = 0;
    totalTrickTime = 0;
    bestTagRun = 0;
    tagRun = 0;
    totalTagRun = 0;
    ghostMod = 0;
    debugPage = -1;
    spawnProtectForced = 0;
    spawnProtectEndPending = 0;
    isGhost = 0;
    repairDamage = 0;
    injuryDamage = 0;
    stuntEarnings = 0;
    deltaBaseVelocity = kVec3Zero;
    deltaBasePosition = kVec3Zero;
    deltaBaseAngularVelocity = kVec3Zero;
    deltaBaseRoll = 0;
    deltaBasePitch = 0;
    deltaBaseYaw = 0;
    deltaBaseTime = 0;
    rivalBearing = 0;
    field_0x15d4 = 0;
    lastCollisionType = 0;
    bikeCollision = 0;
    bikeAndRiderCollision = 0;
}

// Slot 70: threshold test against a per-mode table; a[] and b[][] are static-init'd locals.
// Same bytes (151/230): level/row int locals, a flat
// (&b[0][0])[row * 3 + level - 1] index, swapped products, an else arm. A KbGame*
// local for both arms drops to 22.
// Measured (s14): the else arm gets retail's registers (g in eax, index in edx) when the
// true arm binds `KbGame&` (100/229), uses `i += eventType != 3 ? 3 : 0`, or comes second
// (`if (!aiKind)`, wrong layout); the true arm then still loads +0x60c after the cmp into
// the eventType register, where retail loads it into the g register before the xor/cmp.
// Index spellings (level-first sums, pointer rows, inline accessors, top-level locals)
// all compile to the base bytes.
int KrustyBike::UnknownVirtualSlot70(float arg)
{
    float b[2][3] = { { 4.0f, 3.25f, 2.5f }, { 3.75f, 3.25f, 2.5f } };
    float a[3] = { 1.15f, 1.05f, 1.0f };
    if (aiKind) {
        float speedGain = linearSpeed - prevSpeed;
        if (speedGain * b[g_kbGame->eventType != 3][g_kbGame->difficulty - 1] < arg * speedGainLimit)
            return 1;
        return 0;
    }
    if (linearSpeed - prevSpeed < a[g_kbGame->difficulty - 1] * speedGainLimit * arg)
        return 1;
    return 0;
}

// Slot 11 (0x00491d10): tier 3 -- camera / target ray query.  Fills a2 (origin) and a3
// (unit direction) according to the game mode (g->0x2d74: 0..5), then issues the same
// collision query as the base class with per-mode ranges.  The two "copy the stored
// vectors" paths fall off the end without setting a result (retail leaves eax as is).
// Near miss (523/1296 strict, 1300 bytes): flag and lim are assigned in both arms (an
// initialiser plus a conditional store keeps 0 in edi and spills lim), and the mode 2/3
// copy path ends with `break` (retail returns there; it does not fall into case 0) and
// copies position from *a2.  Left: the 2/3 normalise places its join after the invsqrt
// arm (retail: after the zero arm, the invsqrt arm jumps back) and its query call is not
// cross-jumped into the shared case 0/4 call; case 0 jumps to a call homed in case 4 in
// retail (here case 4 jumps into case 0's).  `d != 0` first, a Vec3 ternary, `*a3 * inv`
// and an inv local do not help (494-523).
int KrustyBike::UnknownVirtualSlot11(int a1, Vec3* a2, Vec3* a3, Vec3* a4, int* a5)
{
    int flag;
    if (g_kbGame->mode.GetSelectedSeries() == 3 || g_kbGame->mode.GetSelectedSeries() == 4)
        flag = 1;
    else
        flag = 0;
    int lim;
    if (g_kbGame->mode.GetSelectedSeries() == 5 || g_kbGame->mode.GetSelectedSeries() == 4)
        lim = 0x7fffffff;
    else
        lim = 0x66;
    unsigned char hit = 0;
    if (g_kbGame->mode.GetSelectedSeries() == 5)
        hit = Kb_004B0AC0(&position, 2.0f, 2.0f, 2.0f, terrainScale, 0, 0, 0, 0);
    KbGame* g = g_kbGame;
    switch (g->eventType) {
    case 2:
    case 3:
        if (g->raceMode || onTrack || aiKind || isGhost || a1 == 2 ||
            g->field_0xc50) {
            *a2 = respawnPosition;
            *a3 = respawnHeading;
            position = *a2;
            break;
        } else {
            Vec3 v = respawnPosition;
            *a2 = position;
            *a3 = Vec3(v.x - a2->x, -a2->y, v.z - a2->z);
            float d = a3->x * a3->x;
            d += a3->y * a3->y;
            d += a3->z * a3->z;
            if (d == 0.0f) *a3 = g_kbZeroVec;
            else *a3 *= FastInvSqrt(d);
            return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, 0x64,
                               0x65, hit, 3.0f, 8, 0, &respawnPosition, !respawnPending, a2, a3, a4, a5);
        }
    case 0: {
        const Vec3* p = respawnPending ? &respawnHeading : &bodyForward;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, 6.0f, 8, p, 0, !respawnPending, a2, a3, a4, a5);
    }
    case 4: {
        KbPlayer* r = (KbPlayer*)g->tagEvent;
        const Vec3* l = 0;
        const Vec3* k = 0;
        if (r->field_0xdc) l = &r->field_0xdc->position;
        else if (r->field_0xa8 == this) { *a3 = bodyForward; k = &bodyForward; }
        else l = &r->field_0xa8->position;
        float i = 9.0f;
        if (((KbPlayer*)g->tagEvent)->field_0xa8 != this) i = 3.0f;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, i, 8, k, l, !respawnPending, a2, a3, a4, a5);
    }
    case 1:
    case 5: {
        if (respawnPending) scratchVector = respawnPosition + respawnHeading;
        else scratchVector = g->bajaEnduroEvent[lapGate + 9].field_0x00;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim,
                           respawnPending ? 0x64 : 0x7fffffff, respawnPending ? 0x65 : 0x7fffffff,
                           3.0f, 8, 0, &scratchVector, !respawnPending, a2, a3, a4, a5);
    }
    default:
        *a2 = position;
        *a3 = bodyForward;
    }
}

// |v| < limit, the range a message 13 delta may carry (written out at every use in retail):
// 30 for velocity and position, 2 pi for angular velocity and the three angles.
#define KB_DELTA_FITS(v, limit) (((v) < 0.0f ? -(v) : (v)) < (limit))

static inline void KbCopy(Vec3* out, const Vec3& a)
{
    out->x = a.x;
    out->y = a.y;
    out->z = a.z;
}

static inline void KbSub(Vec3* out, const Vec3& a, const Vec3& b)
{
    out->x = a.x - b.x;
    out->y = a.y - b.y;
    out->z = a.z - b.z;
}

// 0x00492ad0: sends (or, with `record`, records) message 13, this bike's change since the
// last state as byte deltas plus the pose and flags, carrying what the bytes lose over to the
// next message. A full message 1 (0x00492670) goes instead when a delta does not fit, 1 s
// has passed, or the full interval is due; peers otherwise get one every short interval.
// Near miss (2312 of 2315 bytes, 631 of 641 instructions aligned; frame 0x34 exact): all
// that differs is the record-interval branch and what it drags along. Retail keeps the full
// message block at the end, entered by `jne deltas; jmp sendFull` from the record branch and
// by `push 0; jmp sendFull+1` (record known zero) from the network branch, and the register
// choice in the network branch follows. With the shared `sendFull` label VC6 places the block
// at the end too but inverts the record test (`je sendFull; jmp deltas`) and makes the
// network branch jump there conditionally instead of duplicating the push. Inline calls,
// a literal 0, a boolean flag, explicit gotos to `deltas`, swapped branches, an inverted
// test with an empty then-block and an explicit final return were tried.  Retail has three
// physical full-send copies (two early ones with their own epilogues, the end block the
// `je` early returns use); a physical `{ Fn_00492670(..); return; }` in both branches gives
// VC6's cross-jumping retail's local shape (`jne deltas` and `push 0; jmp home+1`) but it
// keeps the first copy (record branch, inline) as the home and drops the end block
// (2295 B, 78 differing instructions); `goto deltas` + inline calls move the block inline
// after the record branch and the network copy to the end (2337-2348 B).
// Measured (s14): with the network copy passing 0, a record copy passing a constant (`1`)
// leaves one differing instruction (`push 1; jmp end+1` for retail's `jmp end`, 2317 B);
// with `record` the record copy and the end block are identical and VC6 keeps the record
// copy as home, early returns included (2295 B). Equal-value spellings (`record + 0`, casts,
// a copied local, `record != 0 ? record : 1`), per-copy inline helpers, `if (0) goto`,
// dead statements, an explicit end return, `if (!record)` order and `goto deltas` forms
// leave the home there.  So retail's end block was home at the merge; the cause is open.
// One end call taking an `int arg` that each path sets (record, or 0 on the network
// path) is worse: 30-127 differing instructions against 11.
// Shapes that mattered: `KbSub` through an out-pointer keeps `diff` a 12-byte slot (the
// operator form is scalar-replaced and repacks the frame); one `ping` next to `message`
// overlays the dead `d` slots; the angular-velocity and angle limits are 2 pi, not 30;
// `scratchVector[1] += turnRate` and `KbCopy` before `state->angularVelocity.y += turnRate`
// make VC6 load turnRate first (a plain aggregate copy before `.y +=` yields the
// read-modify-write order instead).
void KrustyBike::Fn_00492AD0(float dt, int record)
{
    if (!g_kbGame->network && (!record || !netRecorder))
        return;
    KbBikeState* state;
    if (record) {
        state = &recordState;
        state->timer += g_kbGame->frameTime;
        if (state->timer > g_kbGame->fullRecordPacketIntervalSec)
            goto sendFull;
    } else {
        state = &netState;
        state->timer += g_kbGame->frameTime;
        shortMessageTimer += g_kbGame->frameTime;
        if (shortMessageTimer < g_kbGame->shortNetPacketIntervalSec)
            return;
        shortMessageTimer = 0;
        if (state->timer > g_kbGame->fullNetPacketIntervalSec)
            goto sendFull;
    }
    {
        KbBikeDeltaMessage message;
        KbBikePing ping;
        Vec3 d;
        Vec3 diff;
        message.field_0x16 = loaderKind;
        KbSub(&diff, velocity, state->velocity);
        d = diff + state->velocityError;
        if (KB_DELTA_FITS(d.x, 30.0f) && KB_DELTA_FITS(d.y, 30.0f) && KB_DELTA_FITS(d.z, 30.0f)) {
            message.velocity[0] = (signed char)(d.x * 4.2666669f);
            state->velocityError.x = d.x - message.velocity[0] * 0.234375f;
            message.velocity[1] = (signed char)(d.y * 4.2666669f);
            state->velocityError.y = d.y - message.velocity[1] * 0.234375f;
            message.velocity[2] = (signed char)(d.z * 4.2666669f);
            state->velocityError.z = d.z - message.velocity[2] * 0.234375f;
            KbSub(&diff, position, state->position);
            d = diff + state->positionError;
            if (KB_DELTA_FITS(d.x, 30.0f) && KB_DELTA_FITS(d.y, 30.0f) && KB_DELTA_FITS(d.z, 30.0f)) {
                message.position[0] = (signed char)(d.x * 4.2666669f);
                state->positionError.x = d.x - message.position[0] * 0.234375f;
                message.position[1] = (signed char)(d.y * 4.2666669f);
                state->positionError.y = d.y - message.position[1] * 0.234375f;
                message.position[2] = (signed char)(d.z * 4.2666669f);
                state->positionError.z = d.z - message.position[2] * 0.234375f;
                scratchVector = angularVelocity;
                scratchVector[1] += turnRate;
                KbSub(&diff, scratchVector, state->angularVelocity);
                d = diff + state->angularVelocityError;
                if (KB_DELTA_FITS(d.x, 6.28318548f) && KB_DELTA_FITS(d.y, 6.28318548f) && KB_DELTA_FITS(d.z, 6.28318548f)) {
                    message.angularVelocity[0] = (signed char)(d.x * 20.371832f);
                    state->angularVelocityError.x = d.x - message.angularVelocity[0] * 0.049087387f;
                    message.angularVelocity[1] = (signed char)(d.y * 20.371832f);
                    state->angularVelocityError.y = d.y - message.angularVelocity[1] * 0.049087387f;
                    message.angularVelocity[2] = (signed char)(d.z * 20.371832f);
                    state->angularVelocityError.z = d.z - message.angularVelocity[2] * 0.049087387f;
                    float a = bodyYaw - state->yaw + state->yawError;
                    if (KB_DELTA_FITS(a, 6.28318548f)) {
                        message.yaw = (signed char)(a * 20.371832f);
                        state->yawError = a - message.yaw * 0.049087387f;
                        a = bodyRoll - state->roll + state->rollError;
                        if (KB_DELTA_FITS(a, 6.28318548f)) {
                            message.roll = (signed char)(a * 20.371832f);
                            state->rollError = a - message.roll * 0.049087387f;
                            a = bodyPitch - state->pitch + state->pitchError;
                            if (KB_DELTA_FITS(a, 6.28318548f)) {
                                message.pitch = (signed char)(a * 20.371832f);
                                state->pitchError = a - message.pitch * 0.049087387f;
                                unsigned int now = ReadClock();
                                unsigned int elapsed = now - state->time;
                                if (elapsed < 1000) {
                                    if (elapsed < 0x80) {
                                        message.coarse = 0;
                                        message.count = elapsed;
                                    } else {
                                        message.coarse = 1;
                                        message.count = elapsed >> 3;
                                    }
                                    message.poseIndex = poseIndex;
                                    message.poseState = poseState;
                                    message.poseParam = poseParam * 255.0f;
                                    message.poseLeanBlend = poseLeanBlend * 255.0f;
                                    message.poseBlend = poseBlend * 255.0f;
                                    if (motionActive) {
                                        if (motionIndex >= 0)
                                            message.motion = motionIndex + 1;
                                        else
                                            message.motion = 17;
                                    } else {
                                        message.motion = 0;
                                    }
                                    message.flag7 = throttleOn;
                                    message.flag8 = driveDisengaged;
                                    message.flag9 = trickPoseHeld;
                                    message.crashDirection = crashDirection;
                                    message.crashed = crashState;
                                    message.flag4 = UnknownVirtualSlot51();
                                    message.flag5 = hasFinished != 0;
                                    message.flag6 = onTrack;
                                    if (g_kbGame->eventType == 2 || g_kbGame->eventType == 3)
                                        message.field_0x11 = (unsigned char)lapCount;
                                    else
                                        message.field_0x11 = lapGate;
                                    if (!record) {
                                        if (g_kbGame->network)
                                            g_kbGame->network->Send(13, &message, sizeof(message), netId, record);
                                    } else if (netRecorder) {
                                        netRecorder->QueueRecord(13, netId, &message, sizeof(message));
                                    }
                                    state->position = position;
                                    state->roll = bodyRoll;
                                    state->pitch = bodyPitch;
                                    state->yaw = bodyYaw;
                                    state->velocity = velocity;
                                    KbCopy(&state->angularVelocity, angularVelocity);
                                    state->angularVelocity.y += turnRate;
                                    state->time = now;
                                    if (!record) {
                                        if (g_kbGame->network &&
                                            (!g_kbGame->eventType || (g_kbGame->eventType == 4 && g_kbGame->stuntMode))) {
                                            pingTimer -= g_kbGame->frameTime;
                                            if (pingTimer < 0.0f) {
                                                ping.field_0x04 = score;
                                                ping.field_0x01 = loaderKind;
                                                g_kbGame->network->Send(10, &ping, sizeof(ping), netId, 0);
                                                pingTimer = 2.0f;
                                            }
                                        }
                                    } else if (netRecorder && race->scoreRecordDue) {
                                        ping.field_0x04 = score;
                                        ping.field_0x01 = loaderKind;
                                        netRecorder->QueueRecord(10, netId, &ping, sizeof(ping));
                                    }
                                    return;
                                }
                            }
                        }
                    }
                }
                Fn_00492670(state, g_kbGame->frameTime, record);
                return;
            }
            Fn_00492670(state, g_kbGame->frameTime, record);
            return;
        }
    }
sendFull:
    Fn_00492670(state, g_kbGame->frameTime, record);
}

// Byte-coded pose values of message 1 are 0..255 (tier 3: 1/255 constant at 0x005511e4).
#define KB_BYTE_SCALE (1.0f / 255.0f)

// Retail tests the byte deltas with a copying conditional negate (`mov edx, eax; jns; neg
// edx`), not the `abs` intrinsic (`cdq; xor; sub`).
#define KB_ABS(x) ((x) < 0 ? -(x) : (x))

// Provisional: `dist2` is a squared change in units of the rate limit; the change is
// smoothed (rate limited) only when it is more than one frame's worth and below the warp
// threshold (seconds), and warped otherwise.
static inline int KbWithinWarpBand(float dist2, float frame, float warp)
{
    return dist2 > frame * frame && dist2 < warp * warp;
}

// The same band for a change against a squared limit; dividing inside the helper (rather
// than passing the quotient) keeps limitSq a stored local ahead of the AllowWarping test.
static inline int KbWithinWarpBandOf(float dist2, float limitSq, float frame, float warp)
{
    return dist2 / limitSq > frame * frame && dist2 / limitSq < warp * warp;
}

// 0x00493660: the network update of a remote bike. Places the bike from the received
// states (states[0] newest): either interpolating between the two states around the
// current time (g_kbInterpolate) or extrapolating the newest one, with optional latency
// hiding, rate limiting and warping of the velocity and position; then applies the pose,
// lap and crash flags of the state and runs the physics bookkeeping of a step.
// `dt` is unused; `a` is set for the live game (lap and race fields are taken over).
//
// Partial (8545 bytes retail; 2688 strict, 249 differing instructions): the frame (0x58)
// and stack slots match retail (tools/frame_layout.py). What fixed the shape so far: 4-byte
// locals are declared at function scope in retail slot order (ratio, dist2, len, reset)
// ahead of the 12-byte vectors (step, delta, savedPos, localFwd, localUp); an `int k`
// state index (not a `cur` pointer) gives retail's registers through the live-game
// interval code; `float clock` hoists the race clock read above the replay-mode test;
// dividing by limitSq inside KbWithinWarpBandOf stores limitSq before the AllowWarping
// test as retail does; both orientation calls written in each branch (VC6 cross-jumps the
// second, retail's single deferred `add esp, 0x28`), with the angles passed yaw, pitch,
// roll (+0x1c pushed first); `float ff = f * f` keeps VC6 from refactoring
// `f*f*50 + len*f`; the named `scale` stays FPU resident across the three `delta * scale`
// products; the pose clamps follow both modes; KB_ABS on a variable gives the copying
// `jns; neg`; `states[0]->` is re-read after every call and store (no alias); the
// lap/pose flags are 1/0 if-else stores; the crash-flip test reads bodyUp.y; `char m`
// keeps the motion compare byte sized.
// Still different: retail keeps 1 in ebp from the second SetAxesPtr call on and 0 in edi
// up to the pose section (candidate: 0 in ebp, 1 in ebx, and a zero register through the
// tail where retail uses `test`/`push 0`), which also forces `c` through a stack byte.
// Writing the rider placement in both orientation-save branches, extra `= 1` stores
// before PlaceWheels, moving `movingForward = 1` or an int `settled` do not move it; a
// probe third SetAxesPtr call removes the middle zero register and the tail then matches
// (233). Also: the replay-branch entry registers (g/race in edx/ecx for retail's ecx/edx),
// the Vec3 copy registers from the interpolated velocity on (retail takes edx for the
// destination before the expression, the candidate ecx after it, which rotates eax/ecx/edx
// through the position block; KbCopy, a Vec3 reference, pointer or named temporary and
// either operand order do not move it), the velocity
// AllowWarping load (`cmp mem` for retail's `mov eax`; a local, `!= 0` and an inline test
// do not change it), the velocity y/z `fld s; fmul` order, and retail recomputing `c - b`
// for the pose lerps while the candidate reuses `d`. The 100.0f here is 0x5505ec;
// 0x492670/0x492ad0 scale the message-1 pose bytes by 255.0f (0x551420), which the
// bindings used to give the 100.0f key.
void KrustyBike::Fn_00493660(float dt, int a)
{
    Vec3 step;
    Vec3 delta;
    Vec3 savedPos;
    Vec3 localFwd;
    Vec3 localUp;
    float ratio;
    float dist2;
    float len;
    int reset;

    justLanded = 0;
    justReset = 0;
    reset = 0;
    if (g_kbGame->frameTime <= 0.0f)
        return;
    // the newest state that is old enough to be complete (3 in a replay)
    int k = g_kbGame->playingReplay ? 3 : 2;
    if (states[k]->position.x != 0.0f && states[k]->position.y != 0.0f && states[k]->position.z != 0.0f) {
        if (g_kbGame->playingReplay == 0 && g_kbGame->raceMode != 4) {
            if (lastTimeReceived != states[0]->timeReceived) {
                lastTimeReceived = states[0]->timeReceived;
                if (firstPacketLatch == 0) {
                    firstPacketLatch = 1;
                } else {
                    stateIntervalLength = (float)(states[1]->time - states[2]->time) * 0.001f;
                    interpTarget = states[1];
                    interpSource = states[2];
                    while (stateIntervalElapsed > stateIntervalLength && stateIntervalLength > 0.0f)
                        stateIntervalElapsed -= stateIntervalLength;
                    if (stateIntervalElapsed < 0.0f)
                        stateIntervalElapsed = 0.0f;
                }
            }
            if (stateIntervalElapsed > stateIntervalLength) {
                if (firstPacketLatch == 1) {
                    firstPacketLatch = 0;
                    while (stateIntervalElapsed > stateIntervalLength && stateIntervalLength > 0.0f)
                        stateIntervalElapsed -= stateIntervalLength;
                    stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
                    interpTarget = states[0];
                    interpSource = states[1];
                } else {
                    stateIntervalElapsed = stateIntervalLength;
                }
            }
        } else {
            // replay: the race clock, less a fixed lag, picks the state interval
            float clock = this->race->replayTime;
            if (g_kbGame->raceMode == 4) {
                float adj;
                stateIntervalElapsed = (clock - 0.17f) - (float)states[2]->time * 0.001f;
                adj = this->race->replayTime - 0.17f;
                if (adj > (float)states[0]->time * 0.001f && this->race->lastRecordKind == -2) {
                    stateIntervalElapsed = stateIntervalLength;
                } else if (adj > (float)states[1]->time * 0.001f) {
                    stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
                    stateIntervalElapsed -= (float)(states[1]->time - states[2]->time) * 0.001f;
                    interpTarget = states[0];
                    interpSource = states[1];
                } else {
                    stateIntervalLength = (float)(states[1]->time - states[2]->time) * 0.001f;
                    interpTarget = states[1];
                    interpSource = states[2];
                }
            } else {
                float adj;
                stateIntervalElapsed = (clock - 0.27f) - (float)states[3]->time * 0.001f;
                adj = this->race->replayTime - 0.27f;
                if (adj > (float)states[0]->time * 0.001f && this->race->lastRecordKind == -2) {
                    stateIntervalElapsed = stateIntervalLength;
                } else if (adj < (float)states[2]->time * 0.001f) {
                    stateIntervalLength = (float)(states[2]->time - states[3]->time) * 0.001f;
                    interpTarget = states[2];
                    interpSource = states[3];
                } else if (adj < (float)states[1]->time * 0.001f) {
                    stateIntervalLength = (float)(states[1]->time - states[2]->time) * 0.001f;
                    stateIntervalElapsed -= (float)(states[2]->time - states[3]->time) * 0.001f;
                    interpTarget = states[1];
                    interpSource = states[2];
                } else {
                    stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
                    stateIntervalElapsed -= (float)(states[1]->time - states[3]->time) * 0.001f;
                    if (stateIntervalElapsed > stateIntervalLength)
                        stateIntervalElapsed = stateIntervalLength;
                    interpTarget = states[0];
                    interpSource = states[1];
                }
                if (stateIntervalElapsed < 0.0f)
                    stateIntervalElapsed = 0.0f;
            }
        }
        if (!g_kbInterpolate) {
            unsigned int now = ReadClock();
            if (!g_kbUseTimeReceived)
                stateIntervalElapsed = ((float)now - (float)states[0]->time - clockOffset) * 0.001f;
            else
                stateIntervalElapsed = ((float)now - (float)states[0]->timeReceived) * 0.001f;
            if (!g_kbAllowNegative && stateIntervalElapsed < 0.0f)
                stateIntervalElapsed = 0.0f;
            if (g_kbUseExtrapLimit && stateIntervalElapsed > g_kbExtrapLimit)
                stateIntervalElapsed = g_kbExtrapLimit;
            stateIntervalLength = (float)(states[0]->time - states[1]->time) * 0.001f;
        }
        if (stateIntervalLength > 0.0f)
            ratio = stateIntervalElapsed / stateIntervalLength;
        else
            ratio = 0.0f;

        // velocity
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold)
                    delta = states[0]->velocity - velocity;
                else
                    delta = (states[0]->velocity - states[1]->velocity) * ratio + states[0]->velocity - velocity;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // warp when the change (in units of the 100/s rate) is below a frame's
                    // worth or beyond the warp threshold; otherwise move at the rate
                    if (g_kbAllowWarping
                        && !KbWithinWarpBand(dist2 * 0.0001f, g_kbGame->frameTime, g_kbWarpThreshold)) {
                        velocity += delta;
                    } else {
                        step = delta * 100.0f * g_kbGame->frameTime;
                        velocity += step * FastInvSqrtEstimate(dist2);
                    }
                } else {
                    velocity += delta;
                }
            } else {
                velocity = states[0]->velocity;
            }
        } else {
            velocity = (interpTarget->velocity - interpSource->velocity) * ratio + interpSource->velocity;
        }
        smoothedVerticalAccel = (velocity.y - prevVelocity.y) / g_kbGame->frameTime;
        prevSpeed = linearSpeed;
        len = KbLength(velocity);
        linearSpeed = len;

        // position
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold)
                    delta = states[0]->position - position;
                else
                    delta = states[0]->velocity * stateIntervalElapsed + states[0]->position - position;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // the position may move at speed + 50 per second
                    float limitSq = (len + 50.0f) * (len + 50.0f);
                    if (g_kbAllowWarping
                        && !KbWithinWarpBandOf(dist2, limitSq, g_kbGame->frameTime, g_kbWarpThreshold)) {
                        position += delta;
                    } else {
                        float ff = g_kbGame->frameTime * g_kbGame->frameTime;
                        float scale = ff * 50.0f + len * g_kbGame->frameTime;
                        step = delta * scale;
                        position += step * FastInvSqrtEstimate(dist2);
                    }
                } else {
                    position += delta;
                }
            } else {
                position = states[0]->position;
            }
        } else {
            position = (interpTarget->position - interpSource->position) * ratio + interpSource->position;
        }

        // orientation
        if (!g_kbInterpolate) {
            OrientationAxesFromAngles(&bodyForward, &bodyUp, states[0]->yaw, states[0]->pitch, states[0]->roll);
            OrientationAxesFromAngles(&localFwd, &localUp, states[1]->yaw, states[1]->pitch, states[1]->roll);
        } else {
            OrientationAxesFromAngles(&bodyForward, &bodyUp, interpTarget->yaw, interpTarget->pitch, interpTarget->roll);
            OrientationAxesFromAngles(&localFwd, &localUp, interpSource->yaw, interpSource->pitch, interpSource->roll);
        }
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold) {
                    angularVelocity = states[0]->angularVelocity;
                } else {
                    bodyForward = (bodyForward - localFwd) * ratio + localFwd;
                    bodyUp = (bodyUp - localUp) * ratio + localUp;
                    angularVelocity = (states[0]->angularVelocity - states[1]->angularVelocity) * ratio + states[0]->angularVelocity;
                }
            } else {
                angularVelocity = states[0]->angularVelocity;
            }
        } else {
            bodyForward = (bodyForward - localFwd) * ratio + localFwd;
            bodyUp = (bodyUp - localUp) * ratio + localUp;
            angularVelocity = (interpTarget->angularVelocity - interpSource->angularVelocity) * ratio + interpSource->angularVelocity;
        }

        savedPos = position;
        modelNode->SetPosition(position);
        modelNode->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        UnknownVirtualSlot34();
        OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                                     &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
        if (!crossoverPose && !motionActive) {
            savedForward = bodyForward;
            savedUp = bodyUp;
            savedYaw = bodyYaw;
            savedPitch = bodyPitch;
            savedRoll = bodyRoll;
            savedSinRoll = bodySinRoll;
            savedCosRoll = bodyCosRoll;
            savedCosPitch = bodyCosPitch;
            savedSinPitch = bodySinPitch;
        } else {
            poseNode->GetAxesIn(0, &savedForward, &savedUp);
            OrientationAnglesFromVectors(savedForward, savedUp, &savedYaw, &savedPitch, &savedRoll,
                                         &savedSinRoll, &savedCosRoll, &savedCosPitch, &savedSinPitch);
        }
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetPosition(position);
        ((SoultreeObject*)riderCharacter->c_0x1a0)->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        centerNode->GetPositionIn(0, &centerOfMass);
        worldAngularVelocity = modelNode->LocalToWorldDirection(angularVelocity);
        ((KbSink*)frontWheel->primaryAux)->UnknownVirtualSlot0();
        ((KbSink*)rearWheel->secondaryAux)->UnknownVirtualSlot0();
        PlaceWheels();
        allWheelsInContact = wheelsInContact == wheelCount;
        anyWheelInContact = wheelsInContact != 0;
        UpdateWheelsInContact();
        {
            bool settled = !anyWheelInContact;
            UnknownVirtualSlot71(settled);
            airborne = settled;
            if (settled)
                landingLatched = 0;
        }
        movingForward = 1;

        // wheel roll from the horizontal speed
        if (!(this->race->replayPaused && this->race->lastRecordKind != -2)) {
            float d;
            float roll;
            scratchVector = velocity;
            scratchVector.y = 0.0f;
            d = SquareMagnitude(scratchVector);
            if (d == 1.0f)
                roll = 1.0f;
            else
                roll = FastSqrt(d);
            ((KbWheel*)frontWheel)->SetRollDistance(roll * g_kbGame->frameTime);
            ((KbWheel*)rearWheel)->SetRollDistance(roll * g_kbGame->frameTime);
        }
        if (!g_kbInterpolate)
            turnRate = states[0]->angularVelocity.y;
        else
            turnRate = (interpTarget->angularVelocity.y - interpSource->angularVelocity.y) * ratio + interpSource->angularVelocity.y;

        // pose parameter; the clamp follows both modes
        if (!g_kbInterpolate) {
            int d = interpTarget->poseParam - interpSource->poseParam;
            if (KB_ABS(d) >= 0x40) {
                poseParam = (float)states[0]->poseParam * KB_BYTE_SCALE;
            } else {
                float p = (float)states[0]->poseParam * KB_BYTE_SCALE;
                poseParam = p;
                if (!(g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold))
                    poseParam = (p - (float)states[1]->poseParam * KB_BYTE_SCALE) * ratio + p;
            }
        } else {
            int b = interpSource->poseParam;
            unsigned char c = interpTarget->poseParam;
            int d = c - b;
            if (KB_ABS(d) >= 0x40 && KB_ABS(d) <= 0x7c) {
                if (!poseParamLatched) {
                    poseParamLatched = 1;
                    latchedPoseParam = interpSource->poseParam;
                }
                poseParam = (float)latchedPoseParam * KB_BYTE_SCALE;
            } else if (poseParamLatched) {
                if (c <= 3 || (c >= 0x7e && c <= 0x82))
                    poseParamLatched = 0;
                poseParam = (float)latchedPoseParam * KB_BYTE_SCALE;
            } else if (KB_ABS(d) <= 0x18) {
                poseParam = (float)((int)((float)(c - b) * ratio) + b) * KB_BYTE_SCALE;
            } else {
                poseParam = (float)b * KB_BYTE_SCALE;
            }
        }
        if (poseParam > 1.0f)
            poseParam = 1.0f;
        else if (poseParam < 0.0f)
            poseParam = 0.0f;

        // pose lean blend
        if (!g_kbInterpolate) {
            int a = states[0]->poseLeanBlend;
            int d = a - states[1]->poseLeanBlend;
            float v;
            d = KB_ABS(d);
            v = (float)a * KB_BYTE_SCALE;
            if (d >= 0x40) {
                poseLeanBlend = v;
            } else {
                poseLeanBlend = v;
                if (!(g_kbUseLatencyThreshold && stateIntervalElapsed > g_kbLatencyThreshold))
                    poseLeanBlend = (v - (float)states[1]->poseLeanBlend * KB_BYTE_SCALE) * ratio + v;
            }
        } else {
            int b = interpSource->poseLeanBlend;
            unsigned char c = interpTarget->poseLeanBlend;
            int d = c - b;
            if (KB_ABS(d) >= 0x40 && KB_ABS(d) <= 0x7c) {
                if (!poseLeanBlendLatched) {
                    poseLeanBlendLatched = 1;
                    latchedPoseParam = interpSource->poseLeanBlend;   // retail stores +0x1408 but reads +0x140c
                }
                poseLeanBlend = (float)latchedPoseLeanBlend * KB_BYTE_SCALE;
            } else if (poseLeanBlendLatched) {
                if (c <= 2 || (c >= 0x7f && c <= 0x81))
                    poseLeanBlendLatched = 0;
                poseLeanBlend = (float)latchedPoseLeanBlend * KB_BYTE_SCALE;
            } else if (KB_ABS(d) <= 0x20) {
                poseLeanBlend = (float)((int)((float)(c - b) * ratio) + b) * KB_BYTE_SCALE;
            } else {
                poseLeanBlend = (float)b * KB_BYTE_SCALE;
            }
        }
        if (poseLeanBlend > 1.0f)
            poseLeanBlend = 1.0f;
        else if (poseLeanBlend < 0.0f)
            poseLeanBlend = 0.0f;

        // pose blend; only the interpolated value is smoothed through the steer axis
        if (!g_kbInterpolate) {
            int b1 = states[1]->poseBlend;
            int b0 = states[0]->poseBlend;
            int d = b0 - b1;
            if (KB_ABS(d) >= 0x40)
                poseBlend = (float)b1 * KB_BYTE_SCALE;
            else
                poseBlend = (float)b0 * KB_BYTE_SCALE;
        } else {
            int b = interpSource->poseBlend;
            int c = interpTarget->poseBlend;
            int d = c - b;
            float v;
            float lim;
            float t;
            float r;
            if (KB_ABS(d) >= 0x40)
                v = (float)b;
            else
                v = (float)((int)((float)(c - b) * ratio) + b);
            v *= KB_BYTE_SCALE;
            lim = steerAxis->l_0x4;
            t = g_kbGame->frameTime;
            if (t >= lim)
                t = lim;
            r = t / steerAxis->l_0x4;
            steerAxis->l_0x8 = r;
            v = (v - steerAxis->steerValue) * r + steerAxis->steerValue;
            steerAxis->steerValue = v;
            poseBlend = v;
        }
        if (poseBlend > 1.0f)
            poseBlend = 1.0f;
        else if (poseBlend < 0.0f)
            poseBlend = 0.0f;

        // pose, lap and crash flags (retail keeps two copies of this block, one per mode;
        // they differ in the +0x478 flag only)
        if (!g_kbInterpolate) {
            int lean;
            int prevCrash;
            poseIndex = states[0]->poseIndex;
            lean = poseIndex == 11 || poseIndex == 12;
            if (crossoverPose && !lean) {
                UnknownVirtualSlot41();
                motionCompleted = 0;
            }
            crossoverPose = lean;
            poseState = states[0]->poseState;
            if (g_kbGame->bajaEnduroEvent) {
                if (lapGate == states[0]->lapGate - 1 || (lapGate == ((KbTrack*)g_kbGame->bajaEnduroEvent)->field_0xac - 1 && states[0]->lapGate == 0))
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (a) {
                    int prev = lapGate;
                    lapGate = states[0]->lapGate;
                    if (prev != lapGate) {
                        if (lapGate == 0)
                            lapCount++;
                        nextLapGate = (lapGate + 1) % ((KbTrack*)g_kbGame->bajaEnduroEvent)->field_0xac;
                        if (this == this->race->raceCamera->followedRacer)
                            ((KbTrack*)g_kbGame->bajaEnduroEvent)->SelectLapGates(lapGate, nextLapGate, this);
                    }
                }
            } else {
                if (!states[0]->field_0x5c)
                    states[0]->lapCount = (short)((lapCount & 0xff00) | states[0]->lapGate);
                if ((unsigned short)lapCount == (unsigned short)states[0]->lapCount - 1)
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (a)
                    lapCount = states[0]->lapCount;
            }
            if (states[0]->field_0x5c) {
                lastLapTime = states[0]->lastLapTime;
                fastestLap = states[0]->fastestLap;
                if (a) {
                    timeBehindLeader = states[0]->timeBehindLeader;
                    finishPosition = states[0]->finishPosition;
                }
            }
            if (!isGhost) {
                UnknownVirtualSlot50(states[0]->flag4, 5.0f, 0);
                if (spawnProtectEndPending) {
                    Fn_00496E30(0);
                    spawnProtectEndPending = 0;
                }
            }
            if (states[0]->flag6)
                onTrack = 1;
            else
                onTrack = 0;
            prevCrash = crashState;
            crashState = states[0]->crashed;
            throttleOn = states[0]->flag7;
            driveDisengaged = states[0]->flag8;
            trickPoseHeld = states[0]->flag9;
            if (crashState && !prevCrash) {
                field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                crashedUpsideDown = bodyUp.y < 0.0f;
                crashYaw = savedYaw;
                if (motionActive)
                    wrecker->StartTumble();
                else
                    wrecker->StartAnimatedWreck(states[0]->crashDirection);
                motionIndex = 0;
                UnknownVirtualSlot41();
                motionCompleted = 0;
                Fn_00496D20();
            } else if (!crashState && prevCrash) {
                wrecker->EndWreck();
                Fn_00496DA0();
                motionActive = 0;
                motionIndex = 0;
                reset = 1;
            } else if (UnknownVirtualSlot42()) {
                motionActive = 0;
                motionIndex = 0;
            } else if (!motionActive) {
                char m = states[0]->motion;
                if (m) {
                    if (m < 0x11)
                        Fn_0048D910(m - 1);
                    else if (m == 0x11)
                        Fn_0048E280();
                }
            } else if (!trickChained) {
                char m = interpSource->motion;
                if (m > 0x11 && m - 0x11 == motionIndex)
                    Fn_0048D990(motionIndex);
            }
        } else {
            int lean;
            int prevCrash;
            poseIndex = interpSource->poseIndex;
            lean = poseIndex == 11 || poseIndex == 12;
            if (crossoverPose && !lean) {
                UnknownVirtualSlot41();
                motionCompleted = 0;
            }
            crossoverPose = lean;
            poseState = interpSource->poseState;
            if (g_kbGame->bajaEnduroEvent) {
                if (lapGate == interpSource->lapGate - 1 || (lapGate == ((KbTrack*)g_kbGame->bajaEnduroEvent)->field_0xac - 1 && interpSource->lapGate == 0))
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (a) {
                    int prev = lapGate;
                    lapGate = interpSource->lapGate;
                    if (prev != lapGate) {
                        if (lapGate == 0)
                            lapCount++;
                        nextLapGate = (lapGate + 1) % ((KbTrack*)g_kbGame->bajaEnduroEvent)->field_0xac;
                        if (this == this->race->raceCamera->followedRacer)
                            ((KbTrack*)g_kbGame->bajaEnduroEvent)->SelectLapGates(lapGate, nextLapGate, this);
                    }
                }
            } else {
                if (!interpSource->field_0x5c)
                    interpSource->lapCount = (short)((lapCount & 0xff00) | interpSource->lapGate);
                if ((unsigned short)lapCount == (unsigned short)interpSource->lapCount - 1)
                    forceGatePass = 1;
                else
                    forceGatePass = 0;
                if (a)
                    lapCount = interpSource->lapCount;
            }
            if (interpSource->field_0x5c) {
                lastLapTime = interpSource->lastLapTime;
                fastestLap = interpSource->fastestLap;
                if (a) {
                    timeBehindLeader = interpSource->timeBehindLeader;
                    finishPosition = interpSource->finishPosition;
                }
            }
            if (!isGhost) {
                UnknownVirtualSlot50(interpSource->flag4, 5.0f, 0);
                if (spawnProtectEndPending) {
                    Fn_00496E30(0);
                    spawnProtectEndPending = 0;
                }
            }
            if (interpSource->flag6)
                onTrack = 1;
            else
                onTrack = 0;
            prevCrash = crashState;
            crashState = interpSource->crashed;
            if (!this->race->replayPaused && this->race->lastRecordKind != -2)
                throttleOn = interpSource->flag7;
            else
                throttleOn = 0;
            driveDisengaged = interpSource->flag8;
            trickPoseHeld = interpSource->flag9;
            if (crashState && !prevCrash) {
                field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                crashedUpsideDown = bodyUp.y < 0.0f;
                crashYaw = savedYaw;
                if (motionActive)
                    wrecker->StartTumble();
                else
                    wrecker->StartAnimatedWreck(interpSource->crashDirection);
                motionIndex = 0;
                UnknownVirtualSlot41();
                motionCompleted = 0;
                Fn_00496D20();
            } else if (!crashState && prevCrash) {
                wrecker->EndWreck();
                Fn_00496DA0();
                motionActive = 0;
                motionIndex = 0;
                reset = 1;
            } else if (UnknownVirtualSlot42()) {
                motionActive = 0;
                motionIndex = 0;
            } else if (!motionActive) {
                char m = interpSource->motion;
                if (m) {
                    if (m < 0x11)
                        Fn_0048D910(m - 1);
                    else if (m == 0x11)
                        Fn_0048E280();
                }
            } else if (!trickChained) {
                char m = interpSource->motion;
                if (m > 0x11 && m - 0x11 == motionIndex)
                    Fn_0048D990(motionIndex);
            }
        }

        UnknownVirtualSlot30();
        justReset = reset;
        attachmentResetPending = reset;
        UnknownVirtualSlot21();
        prevCrashState = crashState;
        UnknownVirtualSlot102(g_kbGame->frameTime);
    } else {
        savedPos = position;
    }

    if (g_kbInterpolate)
        stateIntervalElapsed = g_kbGame->frameTime + stateIntervalElapsed;
    if (!a && !hasFinished) {
        if (stateIntervalElapsed >= g_kbStallThreshold) {
            stallHoldTimer = g_kbStallHoldSec;
            UnknownVirtualSlot50(1, 0.0f, 0);
        } else if (stallHoldTimer > 0.0f) {
            stallHoldTimer -= g_kbGame->frameTime;
            if (stallHoldTimer < 0.0f)
                stallHoldTimer = 0.0f;
        } else if (!isGhost) {
            UnknownVirtualSlot50(0, 0.0f, 0);
        }
    }
    if (!g_kbGame->playingReplay) {
        collisionObject->UpdatePlacement();
        if (g_kbGame->eventType == 4)
            UnknownVirtualSlot28(1);
    }
    prevVelocity = velocity;
    position = savedPos;
    lastStepTime = g_kbGame->frameTime;
}

// 0x0048eea0, KrustyBike slot 102 (overrides Bike 0x00407440): the rider/bike pose update.
// Crashed (or +0x5b8 set): Bike's steer pose (pose 1) unless remote.  Otherwise, with +0x430
// set, either a trick clip runs (+0x433 = 16 + trick) or the head turns towards the nearest
// rival (+0x433 = -1, started by 0x0048e280); without it, the +0x7a4 == 2 end pose or Bike's
// lean-pose blend with the KrustyBike extras (poses 16/17 and the four-pose blend 0x004a9050).
// Tables (tier 1 data, tier 3 names): 0x0056cc60 the trick clip lengths in seconds (the clip
// time +0x1520 is clamped to them), 0x0056d060 the frame after which a trick's landing window
// (three frames) opens.
extern float s_kbTrickLength[16];   // 0x0056cc60
extern int s_kbTrickFrame[16];      // 0x0056d060

// Whether trick `trick` may chain from the current input direction (0..7): not when the
// input is the trick's own direction or one of its two neighbours in the table.
static inline int KbTrickChains(int trick, int input)
{
    switch (trick % 8) {
    case 0:
        return input != 0 && input != 4 && input != 5;
    case 1:
        return input != 1 && input != 5 && input != 6;
    case 2:
        return input != 2 && input != 6 && input != 7;
    case 3:
        return input != 3 && input != 4 && input != 7;
    case 4:
        return input != 4 && input != 0 && input != 3;
    case 5:
        return input != 5 && input != 0 && input != 1;
    case 6:
        return input != 6 && input != 1 && input != 2;
    case 7:
        return input != 7 && input != 2 && input != 3;
    }
    return 0;
}

static inline float KbMinF(float a, float b) { return a < b ? a : b; }

// Near miss (0x0048eea0..0x0048fa08, 2920 bytes with the 8-entry switch table at 0x0048f9e8;
// sdiff ratio 0.955, 69 differing instruction lines): calls, the 0x50 frame, every branch,
// constant and store match.  Measured: `int remote = a || b;` (not if/assign) gives retail's
// `cmp [g+0x3428], ebp`; the rider pose-16 test on the member poseIndex (the bike-side test
// reloads it) stops VC6 threading the idx == 16 path past the 11..13 test; the head-turn
// look value is computed after each join (a per-arm `field_0x1554 * 0.71f` folds 0.24 * 0.71)
// and the turning value is a local (retail's dt-slot temporary); `(x * 0.36963f) * 0.7f`
// keeps the two multiplies.  Left: (1) VC6 cross-jumps the head-turn arms differently: retail
// keeps the + side hold block (the - side jumps into it after `fld head`) and the - side
// `1558 = angle; 1550 = target` tail; the store order of the hold block moves the merge
// (1554/154d/1558 gives retail's per-arm schedule without the merge, 1554/1558/154d merges
// into the - copy); (2) the trick-chain switch ends each case with `jne call; jmp skip`
// where retail has `je skip; jmp call` (an inline predicate, a blocked-predicate, per-case
// if/return and a goto-skip switch compile identically); (3) the pending-trick else block
// (`+0x153c = 0`) sits before the shared advance instead of after it (inverted, nested and
// goto forms, and the advance copied into both arms, do not move it); (4) the crash path keeps
// t on the x87 stack where retail spills it to the dead dt slot (Bike's own slot 102 spills
// with the same text; t as a separate statement, the dt parameter itself, if/else and an
// inline max do not).
void KrustyBike::UnknownVirtualSlot102(float dt)
{
    if (crashState || *(int*)pad_0x5B8) {
        if (!isRemote && !g_kbGame->playingReplay) {
            float t = -(steerState->steerAngle / steerState->maxSteerAngle);
            t = (t + 1.0f) * 0.25f;
            t += 0.25f;
            poseParam = (t > 0.25f) ? KbMinF(t, 0.75f) : 0.25f;
        }
        D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[1], poseParam);
        return;
    }
    if (motionActive) {
        if (motionIndex >= 0) {
            // Trick clip.
            if (field_0x432) {
                field_0x432 = 0;
                ((Character*)riderCharacter)->CharacterVirtualSlot7(0, 0, 0);
                CharacterVirtualSlot7(0, 0, 0);
            } else {
                int remote = isRemote || g_kbGame->playingReplay;
                int held;
                if (!aiKind && !remote && (UnknownVirtualSlot84(6, 0x3f) || UnknownVirtualSlot84(7, 0x3f)))
                    held = 1;
                else
                    held = 0;
                trickClipTime += dt;
                if (motionIndex < 16 && trickClipTime > s_kbTrickLength[motionIndex])
                    trickClipTime = s_kbTrickLength[motionIndex];
                if (!held && field_0x152c == 0.0f) {
                    if (remote) {
                        if (!trickPoseHeld) {
                            if (trickPoseHeldPrev && motionIndex > 15)
                                Fn_0048D990(motionIndex - 16);
                            ((Character*)riderCharacter)->CharacterVirtualSlot7(dt, 0, 0);
                            CharacterVirtualSlot7(dt, 0, 0);
                        }
                    } else {
                        if (trickPoseHeld && motionIndex < 16 && animSetC[motionIndex]) {
                            trickPoseHeld = 0;
                            int input = UnknownVirtualSlot88();
                            if (input != -1 && KbTrickChains(motionIndex, input))
                                Fn_0048D990(motionIndex);
                        } else {
                            trickPoseHeld = 0;
                        }
                        ((Character*)riderCharacter)->CharacterVirtualSlot7(dt, 0, 0);
                        CharacterVirtualSlot7(dt, 0, 0);
                    }
                } else {
                    int frame = D3DIMSoultreeCharacter::currentFrame->frameIndex;
                    if (frame > s_kbTrickFrame[motionIndex] && frame < s_kbTrickFrame[motionIndex] + 3) {
                        trickPoseHeld = 1;
                        if (aiKind) {
                            field_0x152c -= dt;
                            if (field_0x152c < 0.0) {
                                field_0x152c = 0;
                                trickPoseHeld = 0;
                            }
                        } else {
                            trickHoldTime += dt;
                            trickClipTime -= dt;
                        }
                    } else {
                        ((Character*)riderCharacter)->CharacterVirtualSlot7(dt, 0, 0);
                        CharacterVirtualSlot7(dt, 0, 0);
                    }
                }
            }
        } else {
            // Head turn towards the nearest rival: +0x1554 is the current head angle, +0x1550
            // its target (bearing +0x1558 scaled into the pose range), +0x154d the turn phase,
            // +0x154e the return to straight ahead.
            float look;
            if (headTurningBack) {
                // Turning back to straight ahead.
                if (rivalBearingScaled < 0.0f) {
                    headTurnAngle += dt;
                    if (headTurnAngle > 0.0f) {
                        headTurnAngle = 0;
                        motionActive = 0;
                        motionIndex = 0;
                        headTurningBack = 0;
                    }
                } else {
                    headTurnAngle -= dt;
                    if (headTurnAngle < 0.0f) {
                        headTurnAngle = 0;
                        motionActive = 0;
                        motionIndex = 0;
                        headTurningBack = 0;
                    }
                }
                look = headTurnAngle * 0.71f;
            } else if (headTurningToRival) {
                // Turning towards the target; near it, the rival's bearing is taken again.
                if (rivalBearingScaled > 0.0f) {
                    float head = headTurnAngle + dt;
                    headTurnAngle = head;
                    if (head > rivalBearingScaled) {
                        headTurningToRival = 0;
                        headTurnAngle = 0.24f;
                        look = rivalBearing * 0.1837f;
                    } else if (rivalBearingScaled - 0.1f < head) {
                        Vec3 d = nearestRival->position - position;
                        float angle = (float)(atan2(d.x, d.z)) - savedYaw;
                        if (angle < -3.14159274f)
                            angle += 6.28318548f;
                        else if (angle > 3.14159274f)
                            angle -= 6.28318548f;
                        float target = (angle * 0.36963f) * 0.7f;
                        if (angle < rivalBearing) {
                            if (head > target) {
                                // Passed the new target: hold the head where it is.
                                headTurnAngle = 0.24f;
                                headTurningToRival = 0;
                                rivalBearing = head * 3.864687f;
                            } else {
                                rivalBearing = angle;
                                rivalBearingScaled = target;
                            }
                        } else if (angle > 2.7f) {
                            rivalBearing = 2.7f;
                            rivalBearingScaled = 0.7f;
                        } else {
                            rivalBearing = angle;
                            rivalBearingScaled = target;
                        }
                        look = headTurnAngle * 0.71f;
                    } else {
                        look = head * 0.71f;
                    }
                } else if (rivalBearingScaled < 0.0f) {
                    float head = headTurnAngle - dt;
                    headTurnAngle = head;
                    if (head < rivalBearingScaled) {
                        headTurningToRival = 0;
                        headTurnAngle = 0.24f;
                        look = rivalBearing * 0.1837f;
                    } else if (rivalBearingScaled + 0.1f > head) {
                        Vec3 d = nearestRival->position - position;
                        float angle = (float)(atan2(d.x, d.z)) - savedYaw;
                        if (angle < -3.14159274f)
                            angle += 6.28318548f;
                        else if (angle > 3.14159274f)
                            angle -= 6.28318548f;
                        float target = (angle * 0.36963f) * 0.7f;
                        if (angle > rivalBearing) {
                            if (head < target) {
                                // Passed the new target: hold the head where it is.
                                headTurnAngle = 0.24f;
                                headTurningToRival = 0;
                                rivalBearing = head * 3.864687f;
                            } else {
                                rivalBearing = angle;
                                rivalBearingScaled = target;
                            }
                        } else if (angle < -2.7f) {
                            rivalBearing = -2.7f;
                            rivalBearingScaled = -0.7f;
                        } else {
                            rivalBearing = angle;
                            rivalBearingScaled = target;
                        }
                        look = headTurnAngle * 0.71f;
                    } else {
                        look = head * 0.71f;
                    }
                } else {
                    headTurningToRival = 0;
                    headTurnAngle = 0.24f;
                    look = rivalBearing * 0.1837f;
                }
            } else {
                // Holding: once the hold time runs out, turn back from the current bearing.
                headTurnAngle -= dt;
                if (headTurnAngle < 0.0f) {
                    headTurningBack = 1;
                    rivalBearingScaled = headTurnAngle = (rivalBearing * 0.36963f) * 0.7f;
                    look = headTurnAngle * 0.71f;
                } else {
                    look = rivalBearing * 0.1837f;
                }
            }
            float pose = look + 0.5f;
            ((KbA5C4*)riderCharacter)->Fn_004A8BF0(riderPoseHandles[14], pose);
            D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[14], pose);
        }
    } else if (hasFinished == 2) {
        // The end pose (riderPoseHandles[15]) once the rider character is ready.
        if (riderCharacter->c_0xc) {
            ((KbA5C4*)riderCharacter)->SetMotion(riderPoseHandles[15]);
            ((KbA5C4*)riderCharacter)->field_0x10 = 0;
            hasFinished++;
        } else {
            ((Character*)riderCharacter)->CharacterVirtualSlot7(dt, 0, 0);
        }
        if (hasFinished > 2) {
            if (crossoverPose)
                UnknownVirtualSlot41();
            motionCompleted = 0;
        }
    } else {
        int cnt = poseState;
        int idx = poseIndex;
        int next;
        int other;
        int blend;
        if (crossoverPose) {
            next = idx + 1;
            if (next > 13)
                next = 13;
            blend = 0;
        } else if (idx > 2) {
            next = idx + 2;
            if (idx == 10) {
                next = idx;
                blend = 0;
            } else {
                other = cnt + 2;
                if (other > 10)
                    other = 10;
                blend = 1;
            }
        } else {
            other = cnt + 2;
            next = idx + 1;
            if (other > 8)
                other = 8;
            blend = 1;
        }
        float w = 1.0f - poseBlend;
        if (idx > 2)
            blend = 0;
        if (poseIndex == 16) {
            if (backUpPhase < 0.45f) {
                float lean = forwardSpeed;
                if (lean < 0.0f)
                    lean = -lean;
                float minimum = dt * 0.55371f;
                float rate = lean * dt * 0.55371f;
                backUpPhaseRate = (rate > minimum) ? rate : minimum;
            }
            backUpPhase += backUpPhaseRate;
            if (backUpPhase > 1.0f)
                backUpPhase -= 1.0f;
            ((KbA5C4*)riderCharacter)->Fn_004A8BF0(riderPoseHandles[16], poseParam);
            ((KbA5C4*)riderCharacter)->Fn_004A8BF0(riderPoseHandles[17], backUpPhase);
        } else if (blend) {
            ((Character*)riderCharacter)->Method_0x004a9050(riderPoseHandles[idx], riderPoseHandles[next],
                riderPoseHandles[cnt], riderPoseHandles[other], poseParam, w, poseLeanBlend);
        } else {
            riderCharacter->Method_0x004a8c50(riderPoseHandles[idx], riderPoseHandles[next], poseParam, w);
        }
        if (idx >= 11 && idx <= 13) {
            D3DIMSoultreeCharacter::Method_0x004a8c50(bikePoseHandles[idx], bikePoseHandles[next], poseParam, w);
        } else if (poseIndex == 16) {
            D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[16], poseParam);
        } else if (blend) {
            D3DIMSoultreeCharacter::Method_0x004a9050(bikePoseHandles[idx], bikePoseHandles[next],
                bikePoseHandles[cnt], bikePoseHandles[other], poseParam, w, poseLeanBlend);
        } else {
            D3DIMSoultreeCharacter::Method_0x004a8c50(bikePoseHandles[idx], bikePoseHandles[next], poseParam, w);
        }
    }
    int mode = wrecker->a_0x44;
    if (mode == 0 || mode == 1) {
        Matrix4 tmp;
        modelNode->GetMatrixIn(0, &tmp);
        riderCharacter->c_0x1a0->SetMatrixIn(0, &tmp);
    }
}
