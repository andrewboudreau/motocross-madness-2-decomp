// Near-miss Net.cpp candidates, kept out of src/reconstructed until they
// match. See docs/NET.md.
//
// NetworkInterface::DispatchMessages (0x004aced0, 370 bytes): 361/370.
// Addressing the slot through a byte offset (MessageAt below) fixes the
// SIB base/index order everywhere (was 355/370 with messages[index]). The
// rest is in the system-message block: retail loads [edx+edi+4] (from) and
// then forms the pointer with `lea ebx,[edx+edi]`; VC6 here forms it first
// (`mov ecx,[esi+0x34]; mov ebx,edi; add ebx,ecx`) and pushes system[2]
// from edx instead of eax. Taking the pointer only inside the `if (!from)`
// (through offset or index) gives retail's load+lea but swaps ebx and ebp
// for the index and the pointer (197/370); local declaration order, name
// buffer order, `last != index`, a split increment, removing the `system`
// local or the `message` pointer, per-case `system` locals and a loop-scope
// pointer declared outside the `if` do not change that swap. Using offset in
// the pointer itself (`&MessageAt(..)` or a cast) outside the `if` gives a
// 368-byte form; a reference for the whole loop body is far worse. Sweeps of
// the in-`if` pointer form (144 combinations: pointer and test forms, the
// `system` local, the switch operand, increment forms and the `more` operand
// order) keep the ebx/ebp swap (best 197) and leave the original at 361.
// Also unchanged or worse: `value` reused as `last`, an inverted head test
// (326), a `NetMessage* base` local (83), `++index >= 0x100`, a split
// increment with `== 0x100` (360) and an `== 0xff` reset (318/372).
// Session 15, worse: `last = nextMessage - 1` hoisted above the if/else (16),
// `last` assigned before `index` in both arms (18), `offset` computed before
// `more` (190).
// Session 14: `messages + index`, `&(messages[index])` and `(int*)(..)`
// spellings keep 361. `&MessageAt(messages, offset)` for the pointer with
// `message->from` (or a reference) gives retail's ebx/ebp and the system
// pushes but forms the pointer with `mov; add` (368 bytes, 3 code diffs);
// testing `MessageAt(messages, offset).from` instead gives retail's
// load+lea with the ebx/ebp swap (197), with or without the system local.
// Testing `MessageAt(messages, offset).from` and taking the pointer inside
// the `if` (or using no pointer) is identical to retail once registers are
// masked (0 hunks, 139 aligned): only the ebx/ebp swap is left, so VC6 here
// weights the index above the message web, where retail weights it below.
// Pointer scope, `register`, const, a reference, per-use MessageAt and a
// pointer shared with the 0x40 block do not change the weights.
// Session 21: tools/decl_shift_scan.py on that 197 form (test through
// MessageAt, `&MessageAt(messages, offset)` taken inside the `if`) is 197 at
// every k = 0..63, so the ebx/ebp swap is not the symbol-arena tie-break;
// retail keeps index in ebp and gives ebx to `last`'s loop-top reload, the
// vtable and the message pointer. /ML keeps 361; /G6 gives 72.
// Session 24, on that 197 form (all keep the swap unless noted): a `from`
// local read through MessageAt before the pointer is taken, the pointer taken
// before the test, a `NetMessage&` (232, 368 bytes), an assignment inside the
// test (232), a `NetMessage* base = messages` local (83); the system block
// with or without the `system` local or the pointer (no pointer: 60), an
// `unsigned long*` view; `index++` then the test, a ternary or if/else
// increment (144), `more` after `offset` (190), `last != index` (198) or
// `index - last` (79); all six declaration orders of index/last/more before
// or after the name buffers, `value` reused as `last`, a `next` local,
// `offset` at function scope, `for (;;)` with a break, `while (more != 0)`,
// a `bool more` (76) and `last = index - 1` (57).
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/DirectPlayMessages.h"

// Ring slot at a byte offset into the received-message array. Addressing the
// slot as messages + offset (offset = index * sizeof) gives retail's
// [eax+edi+4] SIB order; messages[index] gives [edi+eax+4].
static inline NetMessage& MessageAt(NetMessage* messages, int offset)
{
    return *(NetMessage*)((char*)messages + offset);
}

// 0x004aced0: dispatches the received messages in ring order.
void NetworkInterface::DispatchMessages(int value)
{
    char name[16];
    char systemName[16];
    int index;
    int last;
    int more;

    ResendPending(value);
    if (nextMessage || wrappedMessages) {
        if (!wrappedMessages) {
            index = 0;
            last = nextMessage - 1;
        } else {
            index = nextMessage;
            last = nextMessage - 1;
            if (last < 0)
                last = 0xff;
        }
        do {
            more = index != last;
            int offset = index * sizeof(NetMessage);
            if (MessageAt(messages, offset).from && !FindPlayer(MessageAt(messages, offset).from) &&
                GetPlayerName(MessageAt(messages, offset).from, name))
                AddPlayer(MessageAt(messages, offset).from, name);
            if (MessageAt(messages, offset).type == 0x40) {
                AcknowledgeMessage(*(short*)(MessageAt(messages, offset).data + 2));
            } else {
                g_TrackGame->OnNetworkMessage(
                    MessageAt(messages, offset).type, MessageAt(messages, offset).data,
                    MessageAt(messages, offset).from, MessageAt(messages, offset).to,
                    MessageAt(messages, offset).flags);
                NetMessage* message = &messages[index];
                if (!message->from) {
                    // DirectPlay system message: type, player type, player id.
                    int* system = (int*)message->data;
                    switch (message->type) {
                    case DPSYS_CREATEPLAYERORGROUP:
                        if (system[2] && system[1] == 1) {
                            GetPlayerName(system[2], systemName);
                            AddPlayer(system[2], systemName);
                        }
                        break;
                    case DPSYS_DESTROYPLAYERORGROUP:
                        RemovePlayer(system[2]);
                        break;
                    }
                }
            }
            MessageAt(messages, offset).type = 0;
            if (++index >= 0x100)
                index = 0;
        } while (more);
        nextMessage = 0;
        wrappedMessages = 0;
    }
    ringFull = 0;
}
