// Near-miss Parser.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PARSER.md.
//
// UnknownParser::Parse (0x004b87f0, 408 bytes): parses
// `<Tag>value</Tag>` pairs. Each pair needs a non-empty tag, end tag and value
// and matching tags. It then counts the pair and appends it to the list.
// The loop body, register assignment, stack layout and call sequence are
// identical to retail. The only difference is the prologue. Retail pushes
// ebx, ebp, esi and edi before the null test on `text`. VC6 here pushes esi
// and edi, returns E_FAIL, and only then pushes ebx and ebp (406 bytes). The
// shift drops the positional score to 8.1%.
//
// Forms tried:
// - `for (;;)` (1.5%): ebp loaded late, and lstrlenA cached in ebx.
// - `while (text)` (7.8%): adds a loop-exit test.
// - Also: wrapping in `if (text)`, a local cursor, top-declared pointers,
//   `== NULL`, a continue chain and an explicit 0x80004005. None moves the
//   push placement.
// - Swept later: an HRESULT local returned early, a `goto fail` single exit,
//   an else block around `pairCount = 0` and loop pointers declared at the
//   top, in every combination: all 33/408. The /G6 profile scores 39 with 19
//   instruction diffs and /O1 scores 9, so the push placement is not a
//   profile difference either.
#include "../../src/reconstructed/Parser.h"

HRESULT UnknownParser::Parse(char* text, int length)
{
    char tag[0x200];
    char endTag[0x200];
    char value[0x200];

    if (!text)
        return E_FAIL;
    pairCount = 0;
    while (1) {
        char* p = SkipBlanks(text, 0);
        if (!p)
            break;
        char* open = FindChar(p, '<', 0);
        if (!open)
            break;
        char* close = FindChar(open, '>', 0);
        if (!close)
            break;
        lstrcpyn(tag, open + 1, close - open);
        p = SkipBlanks(close + 1, 0);
        if (!p)
            break;
        open = FindChar(p, '<', 0);
        if (!open)
            break;
        lstrcpyn(value, p, open - p + 1);
        p = SkipBlanks(open, 0);
        if (!p)
            break;
        open = FindChar(p, '<', 0);
        if (!open)
            break;
        close = FindChar(open, '>', 0);
        if (!close)
            break;
        lstrcpyn(endTag, open + 2, close - open - 1);
        text = close + 1;
        if (lstrlen(tag) && lstrlen(endTag) && lstrlen(value) && lstrcmp(tag, endTag) == 0) {
            pairCount++;
            pairs.Append(tag, value);
        }
    }
    return S_OK;
}
