"""KbPlayer::SetFollowedRacer 0x004a9e80 = TrackGameViewOwner::SetFollowedRacer(racer, previous, tell) (emu_diff case).

The linked body is the bring-up copy (src/bringup/Group3_FollowedRacer.cpp). Offsets from the
retail body: the owner's race view +0x34 (re-read at every use; its localRacer +0x38,
raceSound +0x44, scoreBoard +0xbc), followedRacer +0xa8, the 5.0f store +0xac, ball +0xdc
(its +0x10a byte cleared); on the bikes tagRun +0x75c, bestTagRun +0x760, totalTagRun +0x764,
score +0x768 (+2500.0f, 0x00550bb8) and vtable slot 50 (+0xc8) SetSpawnProtection(1, 5.0f, 0).

Stubbed and logged (thiscall): slot 50 on each bike, the local racer's 0x004925a0(racer, false),
RaceSound::PlayItSound 0x004e5860, the owner's AnnounceFollowedRacer 0x004a9f90(racer,
previous) and KbScoreBoard::ShowValue 0x0048d1e0(2500.0f, 1.0f). Random: racer and previous
(null, distinct bikes, the local racer, the same bike), the tell flag, ball and scoreboard
present or null, and the run floats (best above, below or equal to the run, a NaN run now and
then, which the retail fcompp/C0 test keeps on the best value).
"""
import math
import random

RETAIL_VA = 0x004a9e80
SYMBOL = '?SetFollowedRacer@KbPlayer@@QAEXPAVVehicle@@0H@Z'
SOURCE = 'src/bringup/Group3_FollowedRacer.cpp'
PROBES = {0x004a9e8d: 'previous', 0x004a9ebc: 'run is best', 0x004a9ef0: 'ball', 0x004a9f0d: 'tell',
          0x004a9f1f: 'racer', 0x004a9f56: 'score bonus', 0x004a9f75: 'scoreboard'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), racer=rng.choice(['none', 'a', 'local']),
                   previous=rng.choice(['none', 'b', 'local', 'same']), tell=rng.choice([0, 1, 7]),
                   ball=rng.random() < 0.5, board=rng.random() < 0.7,
                   best=rng.choice(['above', 'below', 'equal', 'nan']))


def setup(ctx, c):
    r = random.Random(c['seed'])
    spawn = ctx.stub('KrustyBike::SetSpawnProtection', pop=12)
    vt = ctx.vtable([0] * 50 + [spawn] + [0] * 10)

    def bike():
        b = ctx.alloc(0x800, bytes(r.randrange(256) for _ in range(0x800)))
        ctx.pack(b, 'I', vt)
        run = r.uniform(0, 100)
        best = {'above': run + r.uniform(0.1, 50), 'below': run - r.uniform(0.1, 50), 'equal': run,
                'nan': run}[c['best']]
        if c['best'] == 'nan':
            run = math.nan
        ctx.pack(b + 0x75c, '4f', run, best, r.uniform(0, 1000), r.uniform(0, 1e5))
        return b
    local, a, b = bike(), bike(), bike()
    racer = {'none': 0, 'a': a, 'local': local}[c['racer']]
    previous = {'none': 0, 'b': b, 'local': local, 'same': racer}[c['previous']]

    view = ctx.alloc(0x100)
    sound = ctx.alloc(0x40)
    board = ctx.alloc(0x40)
    ctx.pack(view + 0x38, 'I', local)
    ctx.pack(view + 0x44, 'I', sound)
    ctx.pack(view + 0xbc, 'I', board if c['board'] else 0)
    owner = ctx.alloc(0x100, bytes(r.randrange(256) for _ in range(0x100)))
    ctx.pack(owner + 0x34, 'I', view)
    ball = ctx.alloc(0x200, fill=0x77)
    ctx.pack(owner + 0xdc, 'I', ball if c['ball'] else 0)

    ctx.stub_at(0x004925a0, 'KrustyBike::SendFollowedRacer', pop=8)
    ctx.stub_at(0x004e5860, 'RaceSound::PlayItSound', pop=0)
    ctx.stub_at(0x004a9f90, 'TrackGameViewOwner::AnnounceFollowedRacer', pop=8)
    ctx.stub_at(0x0048d1e0, 'KbScoreBoard::ShowValue', pop=8)
    return dict(ecx=owner, args=[racer, previous, c['tell']], ret_eax=False)
