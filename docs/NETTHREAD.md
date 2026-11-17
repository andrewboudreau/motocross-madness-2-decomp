# NetThread.cpp

`src/reconstructed/NetThread.h` / `NetThread.cpp`. Evidence: the
`__FILE__` literal for NetThread.cpp at `0x0056ec6c`. Its xrefs fall in
`0x004af6a0`, `0x004af8e0` and `0x004afa90`, and the unwind funclets
`0x0054c626`/`0x0054c63d` belong to `0x004afa90`. The file statics
`0x006886d0..0x006886e4` are used only inside this range.

Extent: `0x004af680..0x004aff73`. The dialog procs `0x004af470` and
`0x004af590` call FindControl and are left to NetProcs.cpp. `0x004aff80`
starts NormalDistribution.cpp, which is already matched in
`src/krusty2/effects/NormalDistribution.cpp`. `0x004af680` is placed here
by adjacency only: its sole caller is `0x004af6a0`.

The receive thread `0x004af6a0` is the `_beginthreadex` target in Net.cpp
`0x004ab6b0`. It runs the receive loop `0x004af8e0` and the message
dispatcher `0x004afa90`, and it drops duplicate messages through
`0x004afa40`.

Exact: 4 calibration cases (`0x004af680`, `0x004af6a0`, `0x004afa40`,
`0x004afa90`).
In `0x004af6a0` the second 0.001 factor must be written `/ 1000.0f`. VC6
then multiplies by the reciprocal, which is retail's separate constant at
`0x005507d4`.

The dispatcher `0x004afa90` needs two locals for VC6's eax/ecx/edx rotation
of expression temporaries: the acknowledgement reads the local player into a
local before `SendRaw`, and the keep-alive holds the data pointer in a local
across `EnterCriticalSection`. With only the second, every later block was
one register off.

Not exact, in retail order in NetThread.cpp: the receive loop
`0x004af8e0`, 274 of 338 bytes (339 compiled): the buffer pointer is a local the
reallocation assigns, and only the place of its reload differs (retail has
one load at the loop head, VC6 here one before the loop and one after the
dispatch).

Open question: `0x004af6a0` passes a float (`fstp [esp]`) to
NetworkInterface `0x004acc50`, but Net.cpp's body uses that argument as an
int, as the initial value of the lost player id. The real parameter type
is unresolved. NetThread.h therefore calls the method through
`NetKeepAliveView`. A float overload in Net.h was tried and
rejected: it makes VC6 swap the operands of `availPhys + availPageFile` in
TrackGame slot 1 (`0x00520ab0`). Equal-cost commutative operand order can
depend on unrelated declarations in included headers.

## Forms tried

Near-miss NetThread.cpp candidates (D:\aardvark\VC\krusty2\NetThread.cpp),
kept out of src/reconstructed until they match. The exact functions of the
file are in src/reconstructed/NetThread.cpp.

ReceiveMessages (0x004af8e0, 338 bytes): 274 of 339 bytes, every instruction
retail's but one. The buffer pointer is a local that the reallocation
assigns, which gives retail's registers (buffer in eax, &size in ebx,
Receive through ecx; 234 before, with the buffer read from the global in
the argument list). Retail reloads the local once at the loop head and the
reallocation jumps past that load; VC6 here keeps one load before the loop
and one after the dispatch.
Tried for the last instruction: the reload at the loop head with a `goto`
or a nested retry loop past it (VC6 clones the Receive call for the inner
back edge, 385 bytes), `for (init;;)`, `while (1)`, a `continue` after the
reallocation and the reload after the if/else (all 274). Tried before the
local: a buffer or &size local that is only read (propagated away, 234),
the buffer passed to the dispatch (106), inline Receive wrappers taking the
buffer, the size or the interface (234, 194), a local interface pointer, a
function-pointer local and inline accessors for the buffer and the size.
Swept after that (1800 renderings, none above 274): the local declared
without or with a null initialiser, `do`/`while (1)`, the reload as an
assignment inside the Receive argument list or as the first statement of
the body, the allocation chained into the global and the local in either
order or tested inside the `if`, the local in the DebugFree test, and the
dispatch written as an early `continue` for size < 4 (271 of 349). The loop
written with labels only (reload/retry gotos, no loop statement) loses the
local altogether (130 of 385). The two loads VC6 emits are byte-identical
to retail's one and both sit directly before the join, so this is a loop
head retail has and the candidate does not, not a missed tail merge.
Wave 32: the arms swapped (`!= BUFFERTOOSMALL` first, with or without a
`continue`) follow the source order and do not merge the join (179, 335
bytes); the reload written in both arms of the size test 274, ahead of the
size test 272 of 342.

HandleReceivedMessage (0x004afa90) is exact in src/reconstructed/NetThread.cpp:
the acknowledgement reads the local player into a local before SendRaw and
the keep-alive holds data in a local across EnterCriticalSection. Tried
first: a data-pointer or short-pointer local in the acknowledgement (no
change), inline acknowledgement helpers taking data, the interface or the
sequence (668 to 697), a sequence local (767) and an interface local (no
change).
