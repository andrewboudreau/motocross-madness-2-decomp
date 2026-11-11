"""QuerySegmentObstacles 0x00415640, the AI obstacle query along a segment (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBikeAI.cpp), cdecl
(seg, radius, skipVegetation, ignoreCount, ignoreList, push, outPush, outPoint, outDist,
nearOnly). The collision quadtree (0x0068aba4) is a stub object in most runs: BeginQuery
0x004dd270 logs the query box, NextObject 0x004dd540 hands out 0..7 random objects and
EndQuery 0x004dd770 is logged. Objects (offsets from the retail body: type id byte +0x08,
boundCenter +0x40, owner +0x60, ownerType +0x64, queryTouched +0x98) are CollisionObjects of
owner type 0x64 / 0x65 (segment owner: start +0x0c, dir +0x64), 0x69 (dir +0x40) or other,
some on the ignore list, plus Vegetation patches (GetObjectCount 0x00457080 / GetObject
0x004570a0 stubs, GetShapePosition 0x00436000 writes a random point). The capsule test
0x00439600 and SegmentTouchesObject 0x00439820 are stubs that note the segment they are
handed (bytes) and answer from the run's random stream; the segment test writes a point and
a normal whose y straddles 0.70755. Without the quadtree, the iterator path runs (debug new
0x004a3010, iterator ctor/Next/dtor 0x00469950/0x00469a50/0x00469a40, delete 0x004a30c0, all
stubs). FindTypeId 0x00521ea0 is a stub; the two cached type ids are bound to retail's .data
bytes (0x00566f70/71) so both bodies cache into the same place. IntersectLines 0x0047b800, the
dot product, FastInvSqrt, the Vec3 constructor and operators run as retail code; the debug
lines and hit flags are retail globals. Page 0 is mapped for the SEH registration at fs:[0].
"""
import struct

RETAIL_VA = 0x00415640
SYMBOL = '?QuerySegmentObstacles@@YAHPAUVec3@@MHHPAPAVCollisionObject@@M00PAMH@Z'
SOURCE = 'src/bringup/Group3_KrustyBikeAI.cpp'
BINDINGS = {'_s_BikeAICollisionObjectTypeId': 0x00566f70, '_s_BikeAIVegetationTypeId': 0x00566f71}
QUADTREE = 0x0068aba4
TRACK_GAME = 0x0056e26c
COLLISION_ID, VEGETATION_ID = 3, 7
PROBES = {0x00415692: 'find id', 0x00415eb7: 'owner 0x69', 0x00416833: 'owner 0x64', 0x00416105: 'capsule',
          0x0041653d: 'right segment', 0x00416a9c: 'vegetation capsule', 0x00416b2a: 'shape position',
          0x00416c69: 'iterator', 0x00416db0: 'push normalise'}


def vec(rng, s):
    return [rng.uniform(-s, s) for _ in range(3)]


def cases(rng, n):
    for _ in range(n):
        s = rng.choice([3.0, 10.0, 40.0])
        a = vec(rng, s)
        b = [a[0] + rng.uniform(-s, s), a[1] + rng.uniform(-1, 1), a[2] + rng.uniform(-s, s)]
        if rng.random() < 0.05:
            b = [a[0], a[1] + 1.0, a[2]]                     # vertical: zero side vector
        objects = []
        for _ in range(rng.choice([0, 1, 2, 3, 5, 7])):
            kind = rng.choice(['seg', 'seg', 'point', 'other', 'other', 'veg', 'foreign'])
            mid = [a[i] + (b[i] - a[i]) * rng.uniform(-0.2, 1.2) + rng.uniform(-s / 4, s / 4) for i in range(3)]
            objects.append(dict(kind=kind, owner_type=rng.choice([0x64, 0x65]) if kind == 'seg' else 0x69,
                                center=mid, start=mid, dir=vec(rng, s), ignored=rng.random() < 0.15,
                                veg=rng.randrange(4)))
        yield dict(seed=rng.randrange(1 << 30), seg=a + b, radius=rng.choice([0.5, 2.0, rng.uniform(0, 5)]),
                   skip_veg=rng.choice([0, 0, 1]), push=rng.choice([0.0, 5.0, 10.0, 12.0, rng.uniform(0, 20)]),
                   near_only=rng.choice([0, 1]), quadtree=rng.random() < 0.85, objects=objects,
                   cached=(rng.random() < 0.5, rng.random() < 0.5), outs=rng.choice([7, 7, 7, 0, 1, 2, 4]),
                   hit=rng.choice([0.2, 0.5, 0.8]), s=s)


def setup(ctx, c):
    s = c['s']
    seg0 = c['seg'][:3]
    ctx.uc.mem_map(0, 0x1000)          # fs:[0], the SEH registration of the debug new
    ctx.pack(0, 'I', 0x0012ff00)
    ctx.write(0x00566f70, bytes([COLLISION_ID if c['cached'][0] else 0xff,
                                 VEGETATION_ID if c['cached'][1] else 0xff]))

    def find_type(cx, ecx, a):
        name = cx.read(a[0], 16).split(b'\0')[0]
        cx.note('FindTypeId', name)
        return {b'CollisionObject': COLLISION_ID, b'Vegetation': VEGETATION_ID}.get(name, 0x7e)
    registry = ctx.alloc(0x10)
    ctx.pack(0x00575744, 'I', registry)
    ctx.stub_at(0x00521ea0, 'FindTypeId', pop=4, ret=find_type)

    def make_object(o):
        obj = ctx.alloc(0xa0)
        ctx.write(obj, struct.pack('<40f', *[ctx.rng.uniform(-s, s) for _ in range(40)]))
        ctx.pack(obj + 0x08, 'B', COLLISION_ID)
        ctx.pack(obj + 0x40, '3f', *o['center'])
        ctx.pack(obj + 0x54, 'I', 0x5a5a0000 if ctx.rng.random() < 0.8 else 0)
        owner = ctx.alloc(0x80)
        ctx.write(owner, struct.pack('<32f', *[ctx.rng.uniform(-s, s) for _ in range(32)]))
        ctx.pack(owner + 0x0c, '3f', *o['start'])
        ctx.pack(owner + 0x64 if o['owner_type'] != 0x69 else owner + 0x40, '3f', *o['dir'])
        ctx.pack(obj + 0x60, 'Ii', owner,
                 o['owner_type'] if o['kind'] in ('seg', 'point') else ctx.rng.choice([0, 0x3ea, 0x66]))
        ctx.pack(obj + 0x98, 'i', 0)
        return obj

    listed, ignore, vegs = [], [], {}
    for o in c['objects']:
        if o['kind'] == 'veg':
            veg = ctx.alloc(0x40, fill=0x33)
            ctx.pack(veg + 0x08, 'B', VEGETATION_ID)
            vegs[veg] = [make_object(dict(o, kind='other')) for _ in range(o['veg'])]
            listed.append(veg)
        elif o['kind'] == 'foreign':
            other = ctx.alloc(0x40, fill=0x33)
            ctx.pack(other + 0x08, 'B', 0x21)
            listed.append(other)
        else:
            obj = make_object(o)
            listed.append(obj)
            if o['ignored']:
                ignore.append(obj)
    ignore_list = ctx.alloc(4 * (len(ignore) + 1))
    ctx.pack(ignore_list, '%dI' % (len(ignore) + 1), *(ignore + [0x0bad0bad]))
    ignore_count = len(ignore) + (1 if ignore and ctx.rng.random() < 0.3 else 0)
    hit = c['hit']

    def capsule(cx, ecx, a):           # cdecl (CollisionVec3* ends, radius, radiusSq, obj)
        cx.note('capsule', cx.read(a[0], 24).hex(), a[1], a[2], a[3])
        return 1 if cx.rng.random() < hit else 0

    def touches(cx, ecx, a):           # cdecl (ends, obj, outPoint, outNormal)
        cx.note('touches', cx.read(a[0], 24).hex(), a[1])
        if cx.rng.random() >= hit:
            return 0
        t = cx.rng.uniform(0, 1)
        ends = struct.unpack('<6f', cx.read(a[0], 24))
        cx.pack(a[2], '3f', *[ends[i] + t * (ends[3 + i] - ends[i]) for i in range(3)])
        cx.pack(a[3], '3f', cx.rng.uniform(-1, 1), cx.rng.choice([0.70755, 0.7075, 0.71, cx.rng.uniform(-1, 1)]),
                cx.rng.uniform(-1, 1))
        return 1
    ctx.stub_at(0x00439600, 'CapsuleTouches', nargs=4, ret=capsule, log=False)
    ctx.stub_at(0x00439820, 'SegmentTouchesObject', nargs=4, ret=touches, log=False)
    ctx.stub_at(0x00457080, 'Vegetation::GetObjectCount', ret=lambda cx, e, a: len(vegs.get(e, [])))
    ctx.stub_at(0x004570a0, 'Vegetation::GetObject', pop=4, ret=lambda cx, e, a: vegs[e][a[0]])

    def shape_position(cx, ecx, a):
        cx.pack(a[0], '3f', *[seg0[i] + cx.rng.uniform(-s, s) for i in range(3)])
    ctx.stub_at(0x00436000, 'CollisionObject::GetShapePosition', pop=4, ret=shape_position)

    queue = list(listed)

    def next_object(cx, ecx, a):
        return queue.pop(0) if queue else 0
    if c['quadtree']:
        tree = ctx.alloc(0x40)
        ctx.pack(QUADTREE, 'I', tree)
    else:
        ctx.pack(QUADTREE, 'I', 0)
    ctx.stub_at(0x004dd270, 'QuadTree::BeginQuery', pop=16, ret=1)
    ctx.stub_at(0x004dd540, 'QuadTree::NextObject', ret=next_object)
    ctx.stub_at(0x004dd770, 'QuadTree::EndQuery')

    # The iterator path visits only collision objects (the iterator filters by name).
    iter_queue = [o for o in listed if ctx.unpack(o + 8, 'B')[0] == COLLISION_ID]
    game = ctx.alloc(0x3000)
    ctx.pack(TRACK_GAME, 'I', game)
    def debug_new(cx, ecx, a):          # cdecl (size, __FILE__, line); the file name is not compared
        cx.note('operator new', a[0], a[2])
        return cx.alloc(a[0])
    ctx.stub_at(0x004a3010, 'operator new', nargs=3, ret=debug_new, log=False)
    ctx.stub_at(0x00469950, 'GameObjectIterator::ctor', pop=12, nargs=2, ret=lambda cx, e, a: e)
    ctx.stub_at(0x00469a50, 'GameObjectIterator::Next',
                ret=lambda cx, e, a: iter_queue.pop(0) if iter_queue else 0)
    ctx.stub_at(0x00469a40, 'GameObjectIterator::dtor')
    ctx.stub_at(0x004a30c0, 'operator delete', nargs=1)

    seg = ctx.alloc(24)
    ctx.pack(seg, '6f', *c['seg'])
    outs = [ctx.alloc(12, fill=0x77) if c['outs'] & (1 << i) else 0 for i in range(3)]
    return dict(args=[seg, float(c['radius']), c['skip_veg'], ignore_count, ignore_list, float(c['push']),
                      outs[0], outs[1], outs[2], c['near_only']])
