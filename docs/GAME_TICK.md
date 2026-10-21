# Game tick and profiling calibration

`samples/game/GameTick.cpp` supplies complete functional candidates for
`Game::Tick` (0x004685d0, 683 retail bytes) and
`HiResMeter::SetRenderContext` (0x00488440, 196 retail bytes). Authentic VC6 SP3
with `vc6_o2_mt` emits 716 and 204 bytes respectively. Neither is a strict match.

## Evidence and behavior

The complete decoded bodies establish the timer stride (0x48), raw low/high
counter words, signed 64-bit counter subtraction, and the scale at meter +0x30:
**milliseconds per counter tick**, calculated as 1000 divided by the signed
counter difference. The old float stamp declarations and inverse scale name
were incorrect. The corrected declarations are in the narrow `HiResMeter.h`;
GUIManager retains that include at the original declaration position. All 101
GUIManager calibration cases remain strict.

Tick registers `Game::Tick` once, reuses an existing timer or appends a name with
the retail 31-byte copy, and falls back to timer 49 when full without changing
the selected-timer field. It accumulates profiling statistics and starts the
next measurement. Gameplay time comes independently from `ReadClock`, including
its unsigned 32-bit subtraction. It stores unclamped `frameTime`, overwrites the
timer peak with that frame duration, and clamps only the callback argument to
0.2 seconds. The existing opaque integer input/network parameters receive the
float's bits; they do not receive a numeric integer conversion.

Input is dispatched before testing suspension. An active tick times network,
root update, render and present, then releases detached objects and advances the
render target. It reloads callback-mutated pointers and flags at the observed
boundaries. Finally it updates the previous timestamp and handles single-frame
stepping. Calibration sets the base render context before testing QPF success,
waits for a signed accumulated QPC interval of at least one frequency unit, and
stores the scale from the two profiling-counter readings.

## Functional build review

All four retail RDTSC instructions occur in these two bodies. Ordinary VC6 C++
cannot emit RDTSC; inline assembly and copied instructions are not admissible
production reconstructions. `samples/game/ProfilingCounter.cpp` therefore uses
QueryPerformanceCounter for **both** profiling stamps and their calibration.
It is a deliberate platform adaptation, not a recovered byte-identical counter
implementation. Profiling overhead and measured values can differ. The retail
calibration algorithm, including its roughly one-second wait, is retained;
simulation timing still uses the original `ReadClock` provider.

Independent source review compared every branch and dependency in the complete
retail bodies with the actual VC6 output. The x87 elapsed/seconds intermediates
remain in registers until the original float-store boundaries. This review
supports functional admission only, not a new strict-byte claim or completed
startup/racing validation.

`tests/test_game_tick_behavior.py` compiles the standalone candidate and a native
fixture including that same source. The fixture scripts only dependency calls,
QPC/QPF, and four RDTSC boundaries. A private mapped reference executes both
complete retail bodies; vectored breakpoint handling supplies deterministic
counter words without replacing their surrounding arithmetic. No reference
instructions or dependency substitutes enter the production game.
The test also compares the entire emitted Tick body and relocation destinations
between standalone compilation and fixture inclusion, so exposing its private
cache to the fixture cannot silently change the code being tested.

39,376 scenarios compare full Game and two HiResMeter byte states, timer-cache
state, phase durations, counter reads, results and ordered calls. A second-stamp
snapshot checks profiling updates before gameplay overwrites the peak; every
dependency event also captures phase counters and frame timestamps. Cases include
empty/full/found/cached timer registration, negative/wrapped 64-bit profiling
differences, zero/negative/NaN scale, unsigned gameplay-clock wrap, clamp
boundaries, first-tick reset, absent input/network, suspension and step flags,
callback pointer replacement, QPF failure, ignored QPC status, backward QPC
steps, carry/overshoot, zero/negative frequency and zero/negative calibration
delta. These finite scenarios do not prove all possible inputs or a functioning
game. Failed-QPC-status scenarios still supply written counter values; they test
ignored status, not behavior with uninitialized API output.
`tests/test_profiling_counter.py` independently compiles the actual backend
and checks 4,096 calls bracketed by real native QPC readings.

Reproduce on Windows with private inputs:

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_game_tick_behavior.py -v
python -m unittest discover -s tests -p test_profiling_counter.py -v
python tools/vc6_gate.py --vc6-root work/vc6sp3-disc
python tools/build_game.py -j6
```
