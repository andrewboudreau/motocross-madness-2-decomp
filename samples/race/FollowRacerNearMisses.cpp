// Near-miss candidates for the followed-racer helpers, kept out of
// src/reconstructed until they match. See docs/FOLLOWRACER.md. The canonical
// file is included first so the TU-local declarations are the same.
//
// 0x004a9d20 is included from FollowedRacerPicker.cpp. Its complete body
// remains nonexact (342 vs 340 bytes). The saved race-view pointer across
// rand follows decoded retail and a native mutation regression; see
// docs/FOLLOWED_RACER_PICKER.md.
//
// 0x004a9e80 (272 bytes, 95%) now lives in src/reconstructed/FollowRacer.cpp
// (included above), which keeps its status comment; the forms tried there are
// recorded here. One scheduling difference. After the best-run
// ternary retail pushes the three arguments of the virtual call before the
// `fstp` into +0x760; VC6 here stores after the first push. The if/else,
// max-helper, local-result, int-zero, double-literal and statement-order
// forms tried move the compare or the stores instead, as do const-reference
// and by-value inline max helpers, the ternary on the locals or on
// const-reference aliases of the fields, and the fields read directly in the
// ternary; a `keep` local for the selected value, block-scoping the two
// locals and the /G6 and /ML profiles leave the fstp where it is.
// Session 10 also left it in place with an inline StartRole wrapper, a
// slots pointer local, a by-pointer BankRun helper, a reference alias of
// the racer, the zero store folded into an argument or as an int store,
// and bool/char/short/pointer/float types for the slot 50 arguments.
// Session 23: not a count effect. 64 extra file-scope typedefs, and 0..39
// extra statements (diagnostic only) at the top, before the best-run locals,
// after the announcement, or as dead locals, all leave the fstp after the
// first push; statements between the call and the ball test, or removing the
// zero store, the ball test or the tail, move `previous` to another register.

#include <stdlib.h>

#include "../../src/reconstructed/FollowRacer.cpp"

// rand() scaled to [0, 1), as ProCircuit.cpp.
#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768.0f))

#include "FollowedRacerPicker.cpp"
