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
KrustyBike::KrustyBike(int flags) : GameObject(1), Bike(flags)
{
    field_0x73c = 0x65;
    field_0x1540 = kVec3Zero;
    field_0x11b8 = 0;
    field_0x604 = 0;
    field_0x734 = 0;
    field_0x735 = 0;
    field_0x768 = 0;
    field_0x1604 = 0;
    field_0x740 = 0;
    field_0x7b8 = 0;
    field_0x7c0 = 0;
    field_0x7bc = 1;
    field_0x748 = 0;
    field_0x11bc = 0;
    field_0x141c = 0;
    field_0x4a4 = 1.0f;
    field_0x788 = 0;
    field_0x758 = 0;
    field_0x760 = 0;
    field_0x75c = 0;
    field_0x764 = 0;
    field_0x15e8 = 0;
    field_0x1410 = -1;
    field_0x15e6 = 0;
    field_0x7a5 = 0;
    field_0x736 = 0;
    field_0x7ac = 0;
    field_0x7b0 = 0;
    field_0x7b4 = 0;
    field_0x1358 = kVec3Zero;
    field_0x1364 = kVec3Zero;
    field_0x1370 = kVec3Zero;
    field_0x137c = 0;
    field_0x1380 = 0;
    field_0x1384 = 0;
    field_0x1388 = 0;
    field_0x1558 = 0;
    field_0x15d4 = 0;
    lastCollisionType = 0;
    altBodyB = 0;
    altBodyA = 0;
}

// Back-camera swap: like slot 101 with the two scene objects exchanged.
void KrustyBike::Fn_00496DA0()
{
    altBodyB->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Remove(altBodyB, altBodyB->field_0x84);
    collisionObject = (CollisionObject*)altBodyA;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
    else
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot5();
    ((KbObj128*)collisionObject)->Fn_00435FE0();
}

// owner: bracket only (0x496d20 sits between slot 101's stub and Fn_00496DA0)
// Real body of slot 101: switch the active scene object from altBodyA to altBodyB.  Retail's
// slot 101 entry (0x496d10) is a 16-byte stub, `jmp 0x496d20` plus 11 nops, in front of it.
void KrustyBike::Fn_00496D20()
{
    altBodyA->field_0xc.UnknownVirtualSlot4();
    g_kbDirector->Remove(altBodyA, altBodyA->field_0x84);
    collisionObject = (CollisionObject*)altBodyB;
    if (g_kbGame->field_0x18 > 1 && !g_kbGame->field_0x2d84)
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot4();
    else
        ((KbObj128*)collisionObject)->field_0xc.UnknownVirtualSlot5();
    ((KbObj128*)collisionObject)->Fn_00435FE0();
}

// The 5-byte `jmp` plus 11 nops of retail's stub cannot be reproduced from C++ (no inline asm
// or padding tricks), so this stays partial.
void KrustyBike::UnknownVirtualSlot101()
{
    Fn_00496D20();
}

// Slot 14: apply an impulse. field_0x13c behaves as an inverse mass (dv = k * impulse).
void KrustyBike::UnknownVirtualSlot14(Vec3* a, const Vec3* b, const Vec3* c)
{
    if (field_0x740->field_0x18a) {
        Bike::UnknownVirtualSlot14(a, b, c);
        return;
    }
    angularAcceleration = UnknownVirtualSlot16(b);
    UnknownVirtualSlot15(c, &angularAcceleration);
    angularVelocity += stepTime * angularAcceleration;
    angularVelocity *= 0.999f;
    a->x = 0;
    a->z = 0;
    acceleration = Vec3(0.0f, invMass * a->y, 0.0f);
    velocity += stepTime * acceleration;
    linearSpeed = KbLength(velocity);
}

// Slot 70: threshold test against a per-mode table; a[] and b[][] are static-init'd locals.
int KrustyBike::UnknownVirtualSlot70(float arg)
{
    float b[2][3] = { { 4.0f, 3.25f, 2.5f }, { 3.75f, 3.25f, 2.5f } };
    float a[3] = { 1.15f, 1.05f, 1.0f };
    if (field_0x734) {
        float speedGain = linearSpeed - prevSpeed;
        if (speedGain * b[g_kbGame->field_0x2d74 != 3][g_kbGame->field_0x60c - 1] < arg * speedGainLimit)
            return 1;
        return 0;
    }
    if (linearSpeed - prevSpeed < a[g_kbGame->field_0x60c - 1] * speedGainLimit * arg)
        return 1;
    return 0;
}

// Slot 11 (0x00491d10): tier 3 -- camera / target ray query.  Fills a2 (origin) and a3
// (unit direction) according to the game mode (g->0x2d74: 0..5), then issues the same
// collision query as the base class with per-mode ranges.  The two "copy the stored
// vectors" paths fall off the end without setting a result (retail leaves eax as is).
int KrustyBike::UnknownVirtualSlot11(int a1, Vec3* a2, Vec3* a3, Vec3* a4, int* a5)
{
    int flag = 0;
    if (g_kbGame->field_0x578.Fn_00524100() == 3 || g_kbGame->field_0x578.Fn_00524100() == 4)
        flag = 1;
    int lim = 0x66;
    if (g_kbGame->field_0x578.Fn_00524100() == 5 || g_kbGame->field_0x578.Fn_00524100() == 4)
        lim = 0x7fffffff;
    unsigned char hit = 0;
    if (g_kbGame->field_0x578.Fn_00524100() == 5)
        hit = Kb_004B0AC0(&position, 2.0f, 2.0f, 2.0f, terrainScale, 0, 0, 0, 0);
    KbGame* g = g_kbGame;
    switch (g->field_0x2d74) {
    case 2:
    case 3:
        if (g->field_0x2d70 || field_0x78c || field_0x734 || field_0x736 || a1 == 2 ||
            g->field_0xc50) {
            *a2 = respawnPosition;
            *a3 = respawnHeading;
            position = respawnPosition;
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
        KbPlayer* r = (KbPlayer*)g->field_0x568;
        const Vec3* l = 0;
        const Vec3* k = 0;
        if (r->field_0xdc) l = &r->field_0xdc->position;
        else if (r->field_0xa8 == this) { *a3 = bodyForward; k = &bodyForward; }
        else l = &r->field_0xa8->position;
        float i = 9.0f;
        if (((KbPlayer*)g->field_0x568)->field_0xa8 != this) i = 3.0f;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim, lim,
                           hit, i, 8, k, l, !respawnPending, a2, a3, a4, a5);
    }
    case 1:
    case 5: {
        if (respawnPending) scratchVector = respawnHeading + respawnPosition;
        else scratchVector = g->field_0x560[field_0x7b8 + 9].field_0x00;
        return FindObjectPlacement(collisionObject, terrain, &position, terrainScale, flag, lim,
                           respawnPending ? 0x64 : 0x7fffffff, respawnPending ? 0x65 : 0x7fffffff,
                           3.0f, 8, 0, &scratchVector, !respawnPending, a2, a3, a4, a5);
    }
    default:
        *a2 = position;
        *a3 = bodyForward;
    }
}

// owner: bracket only (0x48e280; callers 0x48e533, 0x495288)
// Look at the nearest rival: compute the bearing to it relative to the heading (savedYaw +0x50), wrap to
// +-pi and, when it is more than 0.698 rad off-axis, start the head-turn pose (clamped at +-2.7).
// Near miss: the double subtraction after atan2 gives retail's duplicated x argument around fpatan,
// but VC6 pops the duplicate after the yaw fsub (retail pops it first).
void KrustyBike::Fn_0048E280()
{
    nearestRival = FindNearestRival(0);
    if (!nearestRival)
        return;
    Vec3 d = nearestRival->position - position;
    float angle = (float)(atan2(d.x, d.z) - savedYaw);
    if (angle < -3.14159274f)
        angle += 6.28318548f;
    else if (angle > 3.14159274f)
        angle -= 6.28318548f;
    field_0x1558 = angle;
    if (angle < 0)
        angle = -angle;
    if (angle <= 0.698f)
        return;
    UnknownVirtualSlot41();
    field_0x431 = 0;
    field_0x430 = 1;
    field_0x432 = 1;
    field_0x154d = 1;
    field_0x433 = (char)0xff;
    if (field_0x1558 > 2.7f) {
        field_0x1558 = 2.7f;
        field_0x1550 = 0.7f;
    } else if (field_0x1558 < -2.7f) {
        field_0x1558 = -2.7f;
        field_0x1550 = -0.7f;
    } else {
        field_0x1550 = (field_0x1558 * 0.36963f) * 0.7f;
    }
    field_0x1554 = 0;
    ((KbA5C4*)riderCharacter)->Fn_004A8BF0(riderPoseHandles[14], 0.5f);
    D3DIMSoultreeCharacter::Method_0x004a8bf0(bikePoseHandles[14], 0.5f);
}

// 0x00495ff0: ends a trick. The angle (+0x1530) drops its part below 100, the
// score is angle x multiplier (+0x1538, +0.5 for a +0x153f landing); the race
// handler hears of it. In game mode 0 (or 4 with +0x2eb8 for the +0x568 racer's
// bike) the score is kept in +0x788 and returned; otherwise the bonus rules
// (+0x3444) turn it into a capped bonus, shown to the +0x50 racer's bike.
// Near miss (99.17%, 895 of 895 bytes): the stack slots now match once the float
// bonus is not a named local (the (float)points conversion is a CSE temp that
// retail spills over the dead integer slot; naming it costs a fourth slot).  The
// one remaining divergence is the order of "mov al,[+0x153f]" and the fsubr after
// the fmod call: retail loads the flag first.  Reading the flag into a local before
// the fmod call anchors it before the call (mov bl); reading it after the call, in
// any position, is forward-substituted to the test and scheduled after the fsubr.
// Also tried: a named `float`/`double` remainder temp (forward-substituted, no
// change) and dropping the (float) cast (double fsubr from a spill slot, 11%).
float KrustyBike::Fn_00495FF0()
{
    field_0x1530 -= (float)fmod(field_0x1530, 100.0);
    if (field_0x153f)
        field_0x1538 += 0.5f;
    float score = field_0x1530 * field_0x1538;
    if (field_0x740->field_0x38 == this && field_0x740->handler)
        field_0x740->handler->ReportTrickScore(this, score);
    if (!g_kbGame->field_0x2d74 ||
        (g_kbGame->field_0x2d74 == 4 && g_kbGame->field_0x2eb8 && g_kbGame->field_0x568->field_0xa8 == this)) {
        if (field_0x740->field_0x38 == this) {
            if (field_0x740->field_0xbc)
                field_0x740->field_0xbc->Fn_0048D1E0(field_0x1530, field_0x1538);
            int points = (int)score;
            if (field_0x788 > points)
                points = field_0x788;
            field_0x788 = points;
        }
        Fn_0048D8B0();
        return score;
    }
    KbBonusTable* rules = g_kbGame->field_0x3444;
    if (rules) {
        float step = field_0x1530 * 0.000016f;
        if (!(step < 1.0f))
            step = 1.0f;
        int base = rules->base[rules->index];
        int points = (int)(base * rules->baseScale);
        float gain = 0.0f;
        float limit = points * rules->limitScale;
        if (field_0x7b4 * 400.0f < limit) {
            float total = step + field_0x7b4;
            if (total * 400.0f > limit) {
                field_0x7b4 = limit * 0.0025f;
            } else {
                field_0x7b4 = total;
                gain = step * 400.0f;
            }
        }
        if (this == field_0x740->field_0x50->field_0x3b4) {
            KbMsgSink* sink = g_kbGame->field_0x570->Fn_0045D340();
            char text[0x80];
            char line[0x80];
            if (gain > 0.0f) {
                g_kbGame->GetStringText(0x14d8, text, 0x80);
                sprintf(line, "%s %.0f.00", text, gain);
                KbMessage* message = new(__FILE__, 0xa14) KbMessage(line, 3.25f);
                if (message) {
                    sink->Fn_0051B540(message);
                    delete message;
                }
            } else {
                g_kbGame->GetStringText(0x14d9, text, 0x80);
                sprintf(line, "%s %.0f.00)", text, points * rules->limitScale);
                KbMessage* message = new(__FILE__, 0xa1d) KbMessage(line, 3.25f);
                if (message) {
                    sink->Fn_0051B540(message);
                    delete message;
                }
            }
        }
    }
    Fn_0048D8B0();
    return 0.0f;
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
// Shapes that mattered: `KbSub` through an out-pointer keeps `diff` a 12-byte slot (the
// operator form is scalar-replaced and repacks the frame); one `ping` next to `message`
// overlays the dead `d` slots; the angular-velocity and angle limits are 2 pi, not 30;
// `scratchVector[1] += turnRate` and `KbCopy` before `state->angularVelocity.y += turnRate`
// make VC6 load turnRate first (a plain aggregate copy before `.y +=` yields the
// read-modify-write order instead).
void KrustyBike::Fn_00492AD0(float dt, int record)
{
    if (!g_kbGame->field_0x8 && (!record || !netRecorder))
        return;
    KbBikeState* state;
    if (record) {
        state = &recordState;
        state->timer += g_kbGame->field_0x2f0;
        if (state->timer > g_kbGame->fullRecordPacketIntervalSec)
            goto sendFull;
    } else {
        state = &netState;
        state->timer += g_kbGame->field_0x2f0;
        field_0x15d0 += g_kbGame->field_0x2f0;
        if (field_0x15d0 < g_kbGame->shortNetPacketIntervalSec)
            return;
        field_0x15d0 = 0;
        if (state->timer > g_kbGame->fullNetPacketIntervalSec)
            goto sendFull;
    }
    {
        KbBikeDeltaMessage message;
        KbBikePing ping;
        Vec3 d;
        Vec3 diff;
        message.field_0x16 = field_0x11c0;
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
                                unsigned int now = UnknownFunction4bfa80();
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
                                    message.poseParam = poseParam * 100.0f;
                                    message.poseLeanBlend = poseLeanBlend * 100.0f;
                                    message.poseBlend = poseBlend * 100.0f;
                                    if (field_0x430) {
                                        if (field_0x433 >= 0)
                                            message.motion = field_0x433 + 1;
                                        else
                                            message.motion = 17;
                                    } else {
                                        message.motion = 0;
                                    }
                                    message.flag7 = field_0x478;
                                    message.flag8 = field_0x479;
                                    message.flag9 = field_0x153c;
                                    message.crashDirection = crashDirection;
                                    message.crashed = crashState;
                                    message.flag4 = UnknownVirtualSlot51();
                                    message.flag5 = field_0x7a4 != 0;
                                    message.flag6 = field_0x78c;
                                    if (g_kbGame->field_0x2d74 == 2 || g_kbGame->field_0x2d74 == 3)
                                        message.field_0x11 = (unsigned char)field_0x7a0;
                                    else
                                        message.field_0x11 = field_0x7b8;
                                    if (!record) {
                                        if (g_kbGame->field_0x8)
                                            g_kbGame->field_0x8->Send(13, &message, sizeof(message), field_0x11bc, record);
                                    } else if (netRecorder) {
                                        netRecorder->Fn_004E8720(13, field_0x11bc, &message, sizeof(message));
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
                                        if (g_kbGame->field_0x8 &&
                                            (!g_kbGame->field_0x2d74 || (g_kbGame->field_0x2d74 == 4 && g_kbGame->field_0x2eb8))) {
                                            field_0x1604 -= g_kbGame->field_0x2f0;
                                            if (field_0x1604 < 0.0f) {
                                                ping.field_0x04 = field_0x768;
                                                ping.field_0x01 = field_0x11c0;
                                                g_kbGame->field_0x8->Send(10, &ping, sizeof(ping), field_0x11bc, 0);
                                                field_0x1604 = 2.0f;
                                            }
                                        }
                                    } else if (netRecorder && field_0x740->field_0x3fb) {
                                        ping.field_0x04 = field_0x768;
                                        ping.field_0x01 = field_0x11c0;
                                        netRecorder->Fn_004E8720(10, field_0x11bc, &ping, sizeof(ping));
                                    }
                                    return;
                                }
                            }
                        }
                    }
                }
                Fn_00492670(state, g_kbGame->field_0x2f0, record);
                return;
            }
            Fn_00492670(state, g_kbGame->field_0x2f0, record);
            return;
        }
    }
sendFull:
    Fn_00492670(state, g_kbGame->field_0x2f0, record);
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

// 0x00493660: the network update of a remote bike. Places the bike from the received
// states (states[0] newest): either interpolating between the two states around the
// current time (g_kbInterpolate) or extrapolating the newest one, with optional latency
// hiding, rate limiting and warping of the velocity and position; then applies the pose,
// lap and crash flags of the state and runs the physics bookkeeping of a step.
// `dt` is unused; `a` is set for the live game (lap and race fields are taken over).
//
// Partial (8545 bytes retail): the instruction streams align block for block with the
// retail frame (0x58) and stack slots. What fixed the shape so far: 4-byte locals are
// declared at function scope in retail slot order (ratio, dist2, len, reset) ahead of the
// 12-byte vectors (step, delta, savedPos, localFwd, localUp); `float clock` hoists the
// race clock read above the replay-mode test; `float ff = f * f` keeps VC6 from
// refactoring `f*f*50 + len*f`; the named `scale` stays FPU resident across the three
// `delta * scale` products; the pose clamps follow both modes; KB_ABS on a variable
// gives the copying `jns; neg`; `states[0]->` is re-read after every call and store
// (no `s` alias); the lap/pose flags are 1/0 if-else stores; the crash-flip test reads
// bodyUp.y; `char m` keeps the motion compare byte sized.
// Still different: retail keeps 1 in ebp from the second SetAxesPtr call on and 0 in edi
// (candidate: 0 in ebp, 1 in ebx, and a zero register through the tail where retail
// uses `test`/`push 0`), which also forces `c` through a stack byte; retail stores
// `(len+50)*(len+50)` before the AllowWarping test (a named `lim` or `limitSq *= limitSq`
// stores the unsquared sum instead and re-squares after the branch, 22.89% vs 23.46%);
// the velocity y/z `fld s; fmul` order;
// retail recomputes `c - b` for the pose lerps while the candidate reuses `d`; the retail
// 100.0f lives at 0x5505ec while the shared bindings bind it to 0x551420 (0x492670).
void KrustyBike::Fn_00493660(float dt, int a)
{
    KbBikeNetState* cur;
    KbBikeNetState* s;
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
    if (g_kbGame->field_0x2f0 <= 0.0f)
        return;
    cur = states[g_kbGame->field_0x3428 ? 3 : 2];
    if (cur->position.x != 0.0f && cur->position.y != 0.0f && cur->position.z != 0.0f) {
        if (g_kbGame->field_0x3428 == 0 && g_kbGame->field_0x2d70 != 4) {
            if (field_0x15f8 != states[0]->timeReceived) {
                field_0x15f8 = states[0]->timeReceived;
                if (field_0x15f4 == 0) {
                    field_0x15f4 = 1;
                } else {
                    field_0x15f0 = (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15fc = states[1];
                    field_0x1600 = states[2];
                    while (field_0x15ec > field_0x15f0 && field_0x15f0 > 0.0f)
                        field_0x15ec -= field_0x15f0;
                    if (field_0x15ec < 0.0f)
                        field_0x15ec = 0.0f;
                }
            }
            if (field_0x15ec > field_0x15f0) {
                if (field_0x15f4 == 1) {
                    field_0x15f4 = 0;
                    while (field_0x15ec > field_0x15f0 && field_0x15f0 > 0.0f)
                        field_0x15ec -= field_0x15f0;
                    field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
                    field_0x15fc = states[0];
                    field_0x1600 = states[1];
                } else {
                    field_0x15ec = field_0x15f0;
                }
            }
        } else {
            // replay: the race clock, less a fixed lag, picks the state interval
            float clock = field_0x740->field_0x1b8;
            if (g_kbGame->field_0x2d70 == 4) {
                float adj;
                field_0x15ec = (clock - 0.17f) - (float)states[2]->time * 0.001f;
                adj = field_0x740->field_0x1b8 - 0.17f;
                if (adj > (float)states[0]->time * 0.001f && field_0x740->field_0x1e4 == -2) {
                    field_0x15ec = field_0x15f0;
                } else if (adj > (float)states[1]->time * 0.001f) {
                    field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
                    field_0x15ec -= (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15fc = states[0];
                    field_0x1600 = states[1];
                } else {
                    field_0x15f0 = (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15fc = states[1];
                    field_0x1600 = states[2];
                }
            } else {
                float adj;
                field_0x15ec = (clock - 0.27f) - (float)states[3]->time * 0.001f;
                adj = field_0x740->field_0x1b8 - 0.27f;
                if (adj > (float)states[0]->time * 0.001f && field_0x740->field_0x1e4 == -2) {
                    field_0x15ec = field_0x15f0;
                } else if (adj < (float)states[2]->time * 0.001f) {
                    field_0x15f0 = (float)(states[2]->time - states[3]->time) * 0.001f;
                    field_0x15fc = states[2];
                    field_0x1600 = states[3];
                } else if (adj < (float)states[1]->time * 0.001f) {
                    field_0x15f0 = (float)(states[1]->time - states[2]->time) * 0.001f;
                    field_0x15ec -= (float)(states[2]->time - states[3]->time) * 0.001f;
                    field_0x15fc = states[1];
                    field_0x1600 = states[2];
                } else {
                    field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
                    field_0x15ec -= (float)(states[1]->time - states[3]->time) * 0.001f;
                    if (field_0x15ec > field_0x15f0)
                        field_0x15ec = field_0x15f0;
                    field_0x15fc = states[0];
                    field_0x1600 = states[1];
                }
                if (field_0x15ec < 0.0f)
                    field_0x15ec = 0.0f;
            }
        }
        if (!g_kbInterpolate) {
            unsigned int now = UnknownFunction4bfa80();
            if (!g_kbUseTimeReceived)
                field_0x15ec = ((float)now - (float)states[0]->time - field_0x11c4) * 0.001f;
            else
                field_0x15ec = ((float)now - (float)states[0]->timeReceived) * 0.001f;
            if (!g_kbAllowNegative && field_0x15ec < 0.0f)
                field_0x15ec = 0.0f;
            if (g_kbUseExtrapLimit && field_0x15ec > g_kbExtrapLimit)
                field_0x15ec = g_kbExtrapLimit;
            field_0x15f0 = (float)(states[0]->time - states[1]->time) * 0.001f;
        }
        if (field_0x15f0 > 0.0f)
            ratio = field_0x15ec / field_0x15f0;
        else
            ratio = 0.0f;

        // velocity
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold)
                    delta = states[0]->velocity - velocity;
                else
                    delta = (states[0]->velocity - states[1]->velocity) * ratio + states[0]->velocity - velocity;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // warp when the change (in units of the 100/s rate) is below a frame's
                    // worth or beyond the warp threshold; otherwise move at the rate
                    if (g_kbAllowWarping
                        && !KbWithinWarpBand(dist2 * 0.0001f, g_kbGame->field_0x2f0, g_kbWarpThreshold)) {
                        velocity += delta;
                    } else {
                        step = delta * 100.0f * g_kbGame->field_0x2f0;
                        velocity += step * FastInvSqrtEstimate(dist2);
                    }
                } else {
                    velocity += delta;
                }
            } else {
                velocity = states[0]->velocity;
            }
        } else {
            velocity = (field_0x15fc->velocity - field_0x1600->velocity) * ratio + field_0x1600->velocity;
        }
        smoothedVerticalAccel = (velocity.y - prevVelocity.y) / g_kbGame->field_0x2f0;
        prevSpeed = linearSpeed;
        len = KbLength(velocity);
        linearSpeed = len;

        // position
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold)
                    delta = states[0]->position - position;
                else
                    delta = states[0]->velocity * field_0x15ec + states[0]->position - position;
                if (g_kbRateLimiting) {
                    dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                    // the position may move at speed + 50 per second; retail stores limitSq
                    // before the AllowWarping test (fld st0; fmul st1; fstp; fstp st0), VC6
                    // forward-substitutes it into the division. `float lim = len + 50; lim*lim`
                    // and `limitSq = len + 50; limitSq *= limitSq` both store lim instead and
                    // re-square it after the branch (22.89% vs 23.46%).
                    float limitSq = (len + 50.0f) * (len + 50.0f);
                    if (g_kbAllowWarping
                        && !KbWithinWarpBand(dist2 / limitSq, g_kbGame->field_0x2f0, g_kbWarpThreshold)) {
                        position += delta;
                    } else {
                        float ff = g_kbGame->field_0x2f0 * g_kbGame->field_0x2f0;
                        float scale = ff * 50.0f + len * g_kbGame->field_0x2f0;
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
            position = (field_0x15fc->position - field_0x1600->position) * ratio + field_0x1600->position;
        }

        // orientation
        if (!g_kbInterpolate) {
            UnknownFunction4b5d00(&bodyForward, &bodyUp, states[0]->roll, states[0]->pitch, states[0]->yaw);
            s = states[1];
        } else {
            UnknownFunction4b5d00(&bodyForward, &bodyUp, field_0x15fc->roll, field_0x15fc->pitch, field_0x15fc->yaw);
            s = field_0x1600;
        }
        UnknownFunction4b5d00(&localFwd, &localUp, s->roll, s->pitch, s->yaw);
        if (!g_kbInterpolate) {
            if (g_kbLatencyHiding) {
                if (g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold) {
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
            angularVelocity = (field_0x15fc->angularVelocity - field_0x1600->angularVelocity) * ratio + field_0x1600->angularVelocity;
        }

        savedPos = position;
        modelNode->SetPosition(position);
        modelNode->SetAxesPtr(&bodyForward, &bodyUp, 1, 1);
        UnknownVirtualSlot34();
        OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                                     &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
        if (!field_0x6fc && !field_0x430) {
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
        ((KbSink*)frontWheel->w_0x2b0)->UnknownVirtualSlot0();
        ((KbSink*)rearWheel->w_0x2ac)->UnknownVirtualSlot0();
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
        if (!(field_0x740->field_0x3f8 && field_0x740->field_0x1e4 != -2)) {
            float d;
            float roll;
            scratchVector = velocity;
            scratchVector.y = 0.0f;
            d = SquareMagnitude(scratchVector);
            if (d == 1.0f)
                roll = 1.0f;
            else
                roll = FastSqrt(d);
            ((KbWheel*)frontWheel)->SetRollDistance(roll * g_kbGame->field_0x2f0);
            ((KbWheel*)rearWheel)->SetRollDistance(roll * g_kbGame->field_0x2f0);
        }
        if (!g_kbInterpolate)
            turnRate = states[0]->angularVelocity.y;
        else
            turnRate = (field_0x15fc->angularVelocity.y - field_0x1600->angularVelocity.y) * ratio + field_0x1600->angularVelocity.y;

        // pose parameter; the clamp follows both modes
        if (!g_kbInterpolate) {
            int d = field_0x15fc->poseParam - field_0x1600->poseParam;
            if (KB_ABS(d) >= 0x40) {
                poseParam = (float)states[0]->poseParam * KB_BYTE_SCALE;
            } else {
                float p = (float)states[0]->poseParam * KB_BYTE_SCALE;
                poseParam = p;
                if (!(g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold))
                    poseParam = (p - (float)states[1]->poseParam * KB_BYTE_SCALE) * ratio + p;
            }
        } else {
            int b = field_0x1600->poseParam;
            unsigned char c = field_0x15fc->poseParam;
            int d = c - b;
            if (KB_ABS(d) >= 0x40 && KB_ABS(d) <= 0x7c) {
                if (!field_0x1400) {
                    field_0x1400 = 1;
                    field_0x1408 = field_0x1600->poseParam;
                }
                poseParam = (float)field_0x1408 * KB_BYTE_SCALE;
            } else if (field_0x1400) {
                if (c <= 3 || (c >= 0x7e && c <= 0x82))
                    field_0x1400 = 0;
                poseParam = (float)field_0x1408 * KB_BYTE_SCALE;
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
                if (!(g_kbUseLatencyThreshold && field_0x15ec > g_kbLatencyThreshold))
                    poseLeanBlend = (v - (float)states[1]->poseLeanBlend * KB_BYTE_SCALE) * ratio + v;
            }
        } else {
            int b = field_0x1600->poseLeanBlend;
            unsigned char c = field_0x15fc->poseLeanBlend;
            int d = c - b;
            if (KB_ABS(d) >= 0x40 && KB_ABS(d) <= 0x7c) {
                if (!field_0x1404) {
                    field_0x1404 = 1;
                    field_0x1408 = field_0x1600->poseLeanBlend;   // retail stores +0x1408 but reads +0x140c
                }
                poseLeanBlend = (float)field_0x140c * KB_BYTE_SCALE;
            } else if (field_0x1404) {
                if (c <= 2 || (c >= 0x7f && c <= 0x81))
                    field_0x1404 = 0;
                poseLeanBlend = (float)field_0x140c * KB_BYTE_SCALE;
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
            int b = field_0x1600->poseBlend;
            int c = field_0x15fc->poseBlend;
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
            t = g_kbGame->field_0x2f0;
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
            if (field_0x6fc && !lean) {
                UnknownVirtualSlot41();
                field_0x431 = 0;
            }
            field_0x6fc = lean;
            poseState = states[0]->poseState;
            if (g_kbGame->field_0x560) {
                if (field_0x7b8 == states[0]->field_0x53 - 1 || (field_0x7b8 == ((KbTrack*)g_kbGame->field_0x560)->field_0xac - 1 && states[0]->field_0x53 == 0))
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a) {
                    int prev = field_0x7b8;
                    field_0x7b8 = states[0]->field_0x53;
                    if (prev != field_0x7b8) {
                        if (field_0x7b8 == 0)
                            field_0x7a0++;
                        field_0x7bc = (field_0x7b8 + 1) % ((KbTrack*)g_kbGame->field_0x560)->field_0xac;
                        if (this == field_0x740->field_0x50->field_0x3b4)
                            ((KbTrack*)g_kbGame->field_0x560)->UnknownFunction404df0(field_0x7b8, field_0x7bc, this);
                    }
                }
            } else {
                if (!states[0]->field_0x5c)
                    states[0]->field_0x04 = (short)((field_0x7a0 & 0xff00) | states[0]->field_0x53);
                if ((unsigned short)field_0x7a0 == (unsigned short)states[0]->field_0x04 - 1)
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a)
                    field_0x7a0 = states[0]->field_0x04;
            }
            if (states[0]->field_0x5c) {
                field_0x74c = states[0]->field_0x20;
                field_0x750 = states[0]->field_0x28;
                if (a) {
                    field_0x770 = states[0]->field_0x2c;
                    field_0x784 = states[0]->field_0x01;
                }
            }
            if (!field_0x736) {
                UnknownVirtualSlot50(states[0]->flag4, 5.0f, 0);
                if (field_0x7a5) {
                    Fn_00496E30(0);
                    field_0x7a5 = 0;
                }
            }
            if (states[0]->flag6)
                field_0x78c = 1;
            else
                field_0x78c = 0;
            prevCrash = crashState;
            crashState = states[0]->crashed;
            field_0x478 = states[0]->flag7;
            field_0x479 = states[0]->flag8;
            field_0x153c = states[0]->flag9;
            if (crashState && !prevCrash) {
                field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                field_0x464 = bodyUp.y < 0.0f;
                field_0x45c = savedYaw;
                if (field_0x430)
                    field_0x604->Method_0x00532310();
                else
                    field_0x604->Method_0x00532220(states[0]->crashDirection);
                field_0x433 = 0;
                UnknownVirtualSlot41();
                field_0x431 = 0;
                Fn_00496D20();
            } else if (!crashState && prevCrash) {
                field_0x604->Method_0x005327c0();
                Fn_00496DA0();
                field_0x430 = 0;
                field_0x433 = 0;
                reset = 1;
            } else if (UnknownVirtualSlot42()) {
                field_0x430 = 0;
                field_0x433 = 0;
            } else if (!field_0x430) {
                char m = states[0]->motion;
                if (m) {
                    if (m < 0x11)
                        Fn_0048D910(m - 1);
                    else if (m == 0x11)
                        Fn_0048E280();
                }
            } else if (!field_0x153f) {
                char m = field_0x1600->motion;
                if (m > 0x11 && m - 0x11 == field_0x433)
                    Fn_0048D990(field_0x433);
            }
        } else {
            int lean;
            int prevCrash;
            poseIndex = field_0x1600->poseIndex;
            lean = poseIndex == 11 || poseIndex == 12;
            if (field_0x6fc && !lean) {
                UnknownVirtualSlot41();
                field_0x431 = 0;
            }
            field_0x6fc = lean;
            poseState = field_0x1600->poseState;
            if (g_kbGame->field_0x560) {
                if (field_0x7b8 == field_0x1600->field_0x53 - 1 || (field_0x7b8 == ((KbTrack*)g_kbGame->field_0x560)->field_0xac - 1 && field_0x1600->field_0x53 == 0))
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a) {
                    int prev = field_0x7b8;
                    field_0x7b8 = field_0x1600->field_0x53;
                    if (prev != field_0x7b8) {
                        if (field_0x7b8 == 0)
                            field_0x7a0++;
                        field_0x7bc = (field_0x7b8 + 1) % ((KbTrack*)g_kbGame->field_0x560)->field_0xac;
                        if (this == field_0x740->field_0x50->field_0x3b4)
                            ((KbTrack*)g_kbGame->field_0x560)->UnknownFunction404df0(field_0x7b8, field_0x7bc, this);
                    }
                }
            } else {
                if (!field_0x1600->field_0x5c)
                    field_0x1600->field_0x04 = (short)((field_0x7a0 & 0xff00) | field_0x1600->field_0x53);
                if ((unsigned short)field_0x7a0 == (unsigned short)field_0x1600->field_0x04 - 1)
                    field_0x7c0 = 1;
                else
                    field_0x7c0 = 0;
                if (a)
                    field_0x7a0 = field_0x1600->field_0x04;
            }
            if (field_0x1600->field_0x5c) {
                field_0x74c = field_0x1600->field_0x20;
                field_0x750 = field_0x1600->field_0x28;
                if (a) {
                    field_0x770 = field_0x1600->field_0x2c;
                    field_0x784 = field_0x1600->field_0x01;
                }
            }
            if (!field_0x736) {
                UnknownVirtualSlot50(field_0x1600->flag4, 5.0f, 0);
                if (field_0x7a5) {
                    Fn_00496E30(0);
                    field_0x7a5 = 0;
                }
            }
            if (field_0x1600->flag6)
                field_0x78c = 1;
            else
                field_0x78c = 0;
            prevCrash = crashState;
            crashState = field_0x1600->crashed;
            if (!field_0x740->field_0x3f8 && field_0x740->field_0x1e4 != -2)
                field_0x478 = field_0x1600->flag7;
            else
                field_0x478 = 0;
            field_0x479 = field_0x1600->flag8;
            field_0x153c = field_0x1600->flag9;
            if (crashState && !prevCrash) {
                field_0x574 = Vec3(bodyForward.x, 0.0f, bodyForward.z);
                field_0x464 = bodyUp.y < 0.0f;
                field_0x45c = savedYaw;
                if (field_0x430)
                    field_0x604->Method_0x00532310();
                else
                    field_0x604->Method_0x00532220(field_0x1600->crashDirection);
                field_0x433 = 0;
                UnknownVirtualSlot41();
                field_0x431 = 0;
                Fn_00496D20();
            } else if (!crashState && prevCrash) {
                field_0x604->Method_0x005327c0();
                Fn_00496DA0();
                field_0x430 = 0;
                field_0x433 = 0;
                reset = 1;
            } else if (UnknownVirtualSlot42()) {
                field_0x430 = 0;
                field_0x433 = 0;
            } else if (!field_0x430) {
                char m = field_0x1600->motion;
                if (m) {
                    if (m < 0x11)
                        Fn_0048D910(m - 1);
                    else if (m == 0x11)
                        Fn_0048E280();
                }
            } else if (!field_0x153f) {
                char m = field_0x1600->motion;
                if (m > 0x11 && m - 0x11 == field_0x433)
                    Fn_0048D990(field_0x433);
            }
        }

        UnknownVirtualSlot30();
        justReset = reset;
        attachmentResetPending = reset;
        UnknownVirtualSlot21();
        prevCrashState = crashState;
        UnknownVirtualSlot102(g_kbGame->field_0x2f0);
    } else {
        savedPos = position;
    }

    if (g_kbInterpolate)
        field_0x15ec = g_kbGame->field_0x2f0 + field_0x15ec;
    if (!a && !field_0x7a4) {
        if (field_0x15ec >= g_kbStallThreshold) {
            field_0x138c = g_kbStallHoldSec;
            UnknownVirtualSlot50(1, 0.0f, 0);
        } else if (field_0x138c > 0.0f) {
            field_0x138c -= g_kbGame->field_0x2f0;
            if (field_0x138c < 0.0f)
                field_0x138c = 0.0f;
        } else if (!field_0x736) {
            UnknownVirtualSlot50(0, 0.0f, 0);
        }
    }
    if (!g_kbGame->field_0x3428) {
        collisionObject->UpdatePlacement();
        if (g_kbGame->field_0x2d74 == 4)
            UnknownVirtualSlot28(1);
    }
    prevVelocity = velocity;
    position = savedPos;
    lastStepTime = g_kbGame->field_0x2f0;
}
