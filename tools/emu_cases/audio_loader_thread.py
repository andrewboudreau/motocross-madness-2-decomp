"""UnknownPCAudioObject::UnknownThreadProc 0x004bdc00, the streamed-sound loader thread
(tools/emu_diff.py case).

The linked body is samples/audio/AudioLoaderThread.cpp. Offsets from the retail body (see
docs/AUDIO_LOADER_THREAD.md): the audio object's stop event +0x04, queued event +0x08, queue lock
+0x0c and queued sounds (count +0x2c, data +0x30); a sound's pending buffers +0x18/+0x1c, buffer
lock +0x48, byte count +0x198, flags +0x1e8 (4: streamed), state bytes +0x1f4 (bit 7 cleared on
load) and +0x1f5 (bit 0 buffer ready, bit 1 stopped). WaitForMultipleObjects (IAT 0x005500d8)
answers a scripted run of 0 (queued), 2 (neither) and ends with 1 (stop) or WAIT_FAILED; the
queue holds 0..5 sounds (repeats included) in any flag state, and more are queued between waits.
Enter/LeaveCriticalSection (IAT 0x0055025c / 0x00550284) note their lock, so the lock order is
compared. MakeRoomForSound, CreatePendingBuffer (answers per sound), FillBufferFromFile, Play,
RecordLoadedSound and _endthreadex are stubs; the sounds' fields are compared as memory.
"""
RETAIL_VA = 0x004bdc00
SYMBOL = '?UnknownThreadProc@UnknownPCAudioObject@@SGIPAX@Z'
SOURCE = 'samples/audio/AudioLoaderThread.cpp'
PROBES = {0x004bdcb1: 'streamed', 0x004bdcdb: 'not ready', 0x004bdd21: 'created', 0x004bdd6f: 'play',
          0x004bdd89: 'record', 0x004bddb3: 'other wait result'}

WAIT, ENTER, LEAVE = 0x005500d8, 0x0055025c, 0x00550284


def cases(rng, n):
    for _ in range(n):
        waits = [rng.choice([0, 0, 0, 2]) for _ in range(rng.randrange(0, 5))]
        waits.append(rng.choice([1, 1, 0xffffffff]))
        batches = [[(rng.choice([0, 4, 4, 4, 5]), rng.choice([0, 0, 1, 2, 3]), rng.choice([0, 1, 1]),
                     rng.choice([0, 0x1000, 0x23456]), rng.getrandbits(8))
                    for _ in range(rng.randrange(0, 6))] for _ in waits]
        yield dict(waits=waits, batches=batches, repeat=rng.random() < 0.2)


def setup(ctx, c):
    audio = ctx.alloc(0x100, fill=0x41)
    ctx.pack(audio + 4, 'II', 0xe0000001, 0xe0000002)
    queue = ctx.alloc(4 * 64, fill=0x51)
    ctx.pack(audio + 0x2c, 'iI', 0, queue)
    waits = list(c['waits'])
    batches = list(c['batches'])
    created = {}

    def make_sound(spec):
        flags, state, create_ok, size, top = spec
        sound = ctx.alloc(0x200, fill=0x53)
        ctx.pack(sound + 0x1e8, 'I', flags)
        ctx.pack(sound + 0x1f4, 'BB', top, state)
        ctx.pack(sound + 0x198, 'I', size)
        ctx.pack(sound + 0x18, 'II', 0xb0000000 + sound, 0xb1000000 + sound)
        created[sound] = create_ok
        return sound

    def wait(cx, e, a):
        cx.note('WaitForMultipleObjects', a[0], cx.unpack(a[1], '2I'), a[2], a[3])
        if batches:
            count = cx.unpack(audio + 0x2c, 'i')[0]
            sounds = [make_sound(s) for s in batches.pop(0)]
            if c['repeat'] and sounds:
                sounds.append(sounds[0])
            for k, s in enumerate(sounds):
                cx.pack(queue + 4 * (count + k), 'I', s)
            cx.pack(audio + 0x2c, 'i', count + len(sounds))
        return waits.pop(0) if waits else 1

    def lock(name):
        def f(cx, e, a):
            cx.note(name, a[0])
        return f

    ctx.pack(WAIT, 'I', ctx.stub('WaitForMultipleObjects', pop=16, nargs=4, ret=wait, log=False))
    ctx.pack(ENTER, 'I', ctx.stub('EnterCriticalSection', pop=4, ret=lock('Enter'), log=False))
    ctx.pack(LEAVE, 'I', ctx.stub('LeaveCriticalSection', pop=4, ret=lock('Leave'), log=False))
    ctx.stub_at(0x004be130, 'UnknownPCAudioObject::MakeRoomForSound', pop=4)
    ctx.stub_at(0x004bd4b0, 'Sound::CreatePendingBuffer', pop=0, ret=lambda cx, e, a: created.get(e, 0))
    ctx.stub_at(0x004bd0c0, 'Sound::FillBufferFromFile', pop=4)
    ctx.stub_at(0x004bbcd0, 'Sound::Play', pop=0)
    ctx.stub_at(0x004be0a0, 'UnknownPCAudioObject::RecordLoadedSound', pop=8)
    ctx.stub_at(0x00537c9f, '_endthreadex', nargs=1, ret=lambda cx, e, a: cx.note('_endthreadex', a[0]),
                log=False)
    return dict(args=[audio])
