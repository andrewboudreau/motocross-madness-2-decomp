# recorder.cpp

`src/reconstructed/Recorder.h` / `Recorder.cpp`. Evidence: the `__FILE__`
literal `D:\aardvark\VC\krusty2\recorder.cpp` (`0x00572b04`; lines 442,
477, 535), RTTI `VCRInterface : GameObject` (vtable `0x0055778c`,
overriding slots 0 and 10) and `KrustyVCR : VCRInterface` (`0x00550e9c`),
and the Win32 imports the code calls. The TU starts at `0x004e6f80`. The
kVec3 initializers just before it belong to RaceStatus.cpp (bss and
`.CRT$XCU` order; see [RACESTATUS](RACESTATUS.md)); the shared
`Rectangle2D`/`RenderTarget` code from `0x004e8ad0` follows. Names other
than the RTTI classes are provisional.

VCRInterface records and plays back replays on a worker thread
(`0x004e6f80`, started with `_beginthreadex`): `0x004e7a90` creates the
VCR ring (VCR.h), eleven auto-reset events and a critical section,
then starts mode 0, 1 or 2; the game queues and takes records
(`0x004e8720`, `0x004e8810`), seeks, rewinds and flushes by signalling the
events, and `0x004e7a00` waits for the worker's acknowledgement. The event
and helper names (Recorder.h) follow the worker case each event selects.

Exact: 12 calibration cases (the constructor and destructors and every
method above). Near misses, in the unit in retail order with `// Not exact` comments (they
moved in from their samples; [Forms tried](#forms-tried) keeps the samples'
notes): the worker thread `0x004e6f80` (an 11-way
`WaitForMultipleObjects` switch over the VCR file), first in the unit. Its
[functional build review](RECORDER_WORKER.md#functional-build-review)
records the full code/table extent, the nonexact code shape and finite native
boundary evidence.
Also slot 10 `0x004e7d60`, which
KrustyVCR shares; its control flow and callback switch order are
reconstructed, but register and stack-slot assignment differ. It sits between
`0x004e7a90` and `0x004e86d0`. Its [functional build review](RECORDER_TICK.md#functional-build-review)
records the complete extent, native differential evidence and remaining limits.

## Forms tried

The notes the aggregate near-miss sample carried before both bodies moved
into the unit (scores are aligned instructions or bytes of their time):

```text
0x004e6f80 (2148 bytes with its jump table, the worker thread): the stack
frame, the prologue, the eleven-way switch, every case body and the error
exits match. Retail case 1 ends in a `jmp` to case 2's strcpy(message,
name); `goto opened` there reproduces it (516 -> 652 of 2180; the real
extent is 2148 bytes, the rest is padding and the next function). With
its own copy of the tail VC6 hoists the first `lea edx, message` of the
strcpy above the header-write test (both successors start with it) and
merges the tail suffixes of cases 1 and 2 into case 4; a shared
`headerWriteError` label for cases 1 and 9 removes the hoist but not the
merge, and `strcpy; goto` (label after case 2's strcpy) keeps the hoist.
Left: case 2's "workerAcknowledged = 1; LeaveCriticalSection" suffix still
cross-jumps into case 4 (retail keeps it, after scheduling `push esi`
above the stores), and with one `name` use fewer `name` and `headerSize`
trade stack slots (0x08/0x10; declaration order does not move them).
break/continue mixes, braces, a comma store and store orders did not
change the case 2 merge.
Session 10: retail keeps `name` in edi from case 2's Open to the shared
strcpy and reloads headerSize into eax for the Seek; VC6 here gives edi
to headerSize and reloads `name` at the join. Unchanged (652): the
opened critical section as an inline helper, a lock pointer local, the
two stores as a comma or chained assignment, `register` on name, an
unsigned headerSize, name declared first or at its assignment.
Session 14: with case 1 given its own copy of the tail (516), case 2 is
retail's instruction for instruction (name in edi from its Open to the
strcpy, headerSize reloaded for the Seek), so the goto is what costs case
2 its registers. Case 1's copy then starts its block with `lea edx,
message`, which VC6 hoists above the header-write `je` (retail's case 1
strcpy is scheduled like case 2's, `lea edx` after `sub edi, ecx`, and
cross-jumps into it at `or ecx, -1`). The success path inline before the
error path and a block-local copy of name give the same 516.
Lead: case 2's two stores written workerAcknowledged-first stop the
cross-jump into case 4 (VC6 then schedules `push esi` above them, as
retail does) and give 2096 strict, but the stores come out in that
order (retail: isStopped first) and case 1's Open/Write block moves
after case 2's; aligned 586 -> 590, 13 hunks either way, so not kept.
`continue` for case 2's or case 4's break changes nothing (652).

0x004e7d60 (2412 bytes with its jump table, VCRInterface slot 10; KrustyVCR
shares it): control flow, call arguments and the callback switch order
(5, 3, 11, 12, 2, 1, 7) are reconstructed, but many blocks differ: retail
keeps the constant 0 in eax from the readingReverse test on (cmp ecx, eax;
push eax for the flag; recordPending = eax) and 1 in ebp in the loops, assigns
the stack slots value 0x18, queued 0x1c, size 0x20 and the readsReset
flag/loop count 0x24, keeps loop counts in ebp/edx, and cross-jumps fewer
`Rewind(); return 1;` exits.
Setting `queued = 1` before the AppendPosition call gives retail's store
among that call's argument pushes (63 -> 56 hunks). Assigning `record`
before the AppendPosition(recordPosition) call keeps the count in ebp as
retail does, but rotates the scratch registers of the switch cases
(aligned 706 -> 530). `int queued = 0`, `queued = 0` before the
readingReverse test and `while (1)` for the reverse loop do not give
retail's `xor eax, eax ; cmp ecx, eax` zero there.
```

