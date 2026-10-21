// Near-miss racesnd.cpp candidates, kept out of src/reconstructed until they
// match. See docs/RACESOUND.md. The canonical file is included first so the
// TU-local declarations and globals are the same.
//
// 0x004e4d10 (2032 bytes, the half-buffer refill; 0x004e39b0 is exact in
// RaceSound.cpp): every block but two matches. (1) First half, next-sample
// path: retail reloads the remainder from its stack slot for the final
// `field_0x18 = rest` (so that store is cross-jumped with the other two
// arms' stores); VC6 here keeps the loaded remainder in eax across the two
// copies. (2) Second half, looping path: retail keeps half in ebp and the
// remainder in edx and spills the available byte count, VC6 here spills a
// temporary and computes the remainder in ebp. The second copy's destination
// reloads 11025 * bytesPerSample as retail does. Unchanged or worse: a `rest`
// local in (2) (before or after the first copy), the destination's
// association (sampleBuffer + avail + 11025 * bytesPerSample, a parenthesised
// offset, `+ half`), and block-scoped `avail`/`rest` in (1). Also 806 or
// worse: in (1) `half - avail` for the second copy and/or the final store
// (799 each; both, or `rest` computed after the switch, give 1231 bytes
// only because the frame shrinks to 0x1c: 42 instructions differ against
// 37); in (2) `rest` set before the first copy with `half - avail` in the
// copy or the store (806), `+ 11025 * bytesPerSample` for the first
// copy's destination (806), and `+ half + avail` for the second (779).

#include "../../src/reconstructed/RaceSound.cpp"

#include "RaceSoundRefill.cpp"
