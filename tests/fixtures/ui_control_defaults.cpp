#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "GameUi.h"

#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
CHECK(control_size, sizeof(UIControl) == 0x1ec);
CHECK(static_size, sizeof(UIStaticText) == 0x1ec);
CHECK(dropdown_size, sizeof(UIDropDownList) == 0x21c);
CHECK(state_offset, offsetof(UIControl, currentState) == 0x60);
CHECK(redraw_offset, offsetof(UIControl, needsRedraw) == 0x1c0);

int main() {
    const int values[] = {0, 1, -1, 2, 0x7fffffff, (int)0x80000000, 0x12345678};
    union { double align; unsigned char bytes[sizeof(UIDropDownList) + 32]; } actual, expected;
    unsigned calls = 0;
    for (int seed = 0; seed < 256; ++seed) {
        for (unsigned v = 0; v < sizeof(values) / sizeof(values[0]); ++v) {
            for (unsigned i = 0; i < sizeof(actual.bytes); ++i)
                actual.bytes[i] = (unsigned char)(seed + 37 * i);
            memcpy(expected.bytes, actual.bytes, sizeof(actual.bytes));
            UIControl* control = reinterpret_cast<UIControl*>(actual.bytes + 16);
            UIStaticText* text = reinterpret_cast<UIStaticText*>(actual.bytes + 16);
            UIDropDownList* dropdown = reinterpret_cast<UIDropDownList*>(actual.bytes + 16);
            // Qualified calls exercise the actual out-of-line methods. This
            // fixture does not claim a constructed GUI or a working lifecycle.
            control->UIControl::KillFocus(values[v]);
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 1;
            control->UIControl::SyncBoundValue(values[v]);
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 2;
            control->UIControl::HandleTimer(values[v]);
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 3;
            if (control->UIControl::IsInteractive() != 1) return 4;
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 5;
            if (text->UIStaticText::IsInteractive() != 0) return 6;
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 7;
            if (dropdown->UIDropDownList::IsInteractive() != 1) return 8;
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 9;
            int one = 1;
            memcpy(expected.bytes + 16 + 0x1c0, &one, 4);
            memcpy(expected.bytes + 16 + 0x60, &values[v], 4);
            text->UIStaticText::SetState(values[v]);
            if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 10;
            calls += 7;
        }
    }
    printf("UI defaults: %u complete-storage calls passed\n", calls);
    return 0;
}
