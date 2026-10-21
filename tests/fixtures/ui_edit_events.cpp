#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "GameUi.h"
#include "DialogProc.h"
#include "DialogEventKind.h"

#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
CHECK(edit_size, sizeof(UIEditBox) == 0x230);
CHECK(base_size, sizeof(UIControl) == 0x1ec);
CHECK(state_offset, offsetof(UIControl, currentState) == 0x60);
CHECK(owner_offset, offsetof(UIControl, ownerDialog) == 0xb8);
CHECK(anchor_offset, offsetof(UIControl, anchorControl) == 0x18c);
CHECK(frames_offset, offsetof(UIControl, redrawFrames) == 0x1bc);
CHECK(redraw_offset, offsetof(UIControl, needsRedraw) == 0x1c0);
CHECK(button_offset, offsetof(UIControl, heldButton) == 0x1d4);

union EditStorage { double align; unsigned char bytes[sizeof(UIEditBox) + 32]; };
union AnchorStorage { double align; unsigned char bytes[sizeof(UIControl) + 32]; };
union DialogStorage { double align; unsigned char bytes[sizeof(UIDialog) + 32]; };
union UserStorage { double align; unsigned char bytes[sizeof(GUIUser) + 32]; };
struct World {
    EditStorage edit;
    AnchorStorage anchors[3];
    DialogStorage dialogs[2];
    UserStorage users[2];
    int position[2];
};
static World world, expected;
static DWORD controlTable[65], dialogTable[32];
static int handled, mutation, throwStage, swapOwner, calls[4], soundArgument;
static UIEditBox* Edit(World& w) { return reinterpret_cast<UIEditBox*>(w.edit.bytes + 16); }
static UIControl* Anchor(World& w, int n) { return reinterpret_cast<UIControl*>(w.anchors[n].bytes + 16); }
static UIDialog* Dialog(World& w, int n) { return reinterpret_cast<UIDialog*>(w.dialogs[n].bytes + 16); }
static GUIUser* User(World& w, int n) { return reinterpret_cast<GUIUser*>(w.users[n].bytes + 16); }
static void Fail(int n) { printf("UI edit invariant %d\n", n); ExitProcess(n); }

struct Observer {
    void Handle(UnknownDialogEvent* event) {
        ++calls[0];
        if ((void*)this != Dialog(world, 0) || event->handled || event->kind != kDialogCommand ||
            event->code != 0x12345678 || event->controlName != Edit(world)->controlName ||
            event->dialog != Dialog(world, 0) || event->gui != Edit(world)->ownerGui ||
            event->control != Edit(world)) Fail(10);
        // Only the seven initialized fields above are observed; unused event
        // fields remain uninitialized exactly as in the production body.
        if (throwStage == 1) throw 71;
        event->handled = handled;
        if (mutation == 1) User(world, 0)->hoverControl = 0;
        if (mutation == 2) Edit(world)->currentState = 4;
        if (mutation == 3) Edit(world)->ownerDialog = Dialog(world, 1);
    }
    void Set(int state) {
        ++calls[1];
        if ((void*)this != Edit(world) || state != 3) Fail(11);
        if (throwStage == 2) throw 72;
        if (swapOwner) Edit(world)->ownerDialog = Dialog(world, 1);
        Edit(world)->UIControl::SetState(state);
    }
    void Sound(int index, int argument) {
        ++calls[2]; soundArgument = argument;
        if ((void*)this != Edit(world) || index != 3) Fail(12);
        if (throwStage == 3) throw 73;
    }
    void Redraw() {
        ++calls[3];
        if ((void*)this != Edit(world)) Fail(16);
        Edit(world)->UIControl::RequestRedraw();
    }
};
template<class T> static DWORD Address(T pointer) {
    typedef char Fits[sizeof(T) == 4 ? 1 : -1];
    DWORD address; memcpy(&address, &pointer, 4); return address;
}
static void Setup(int seed) {
    memset(&world, seed, sizeof(world));
    memset(calls, 0, sizeof(calls)); soundArgument = -1;
    UIEditBox* edit = Edit(world);
    *(DWORD**)edit = controlTable;
    edit->ownerDialog = Dialog(world, 0);
    edit->ownerGui = reinterpret_cast<GUIManager*>(0x12340000);
    strcpy(edit->controlName, "event-control");
    edit->eventCode = 0x12345678;
    for (int i = 0; i < 2; ++i) {
        *(DWORD**)Dialog(world, i) = dialogTable;
        Dialog(world, i)->guiUser = User(world, i);
        Dialog(world, i)->field_0x180 = 100 + i;
        User(world, i)->hoverControl = reinterpret_cast<UnknownGuiControl*>(edit);
    }
}
static int TestRelease() {
    const int arguments[] = {0, 1, -1, (int)0x80000000};
    const int states[] = {-1, 0, 1, 2, 3};
    unsigned cases = 0;
    for (int a = 0; a < 4; ++a) for (int s = 0; s < 5; ++s)
    for (int hover = 0; hover < 2; ++hover) for (handled = -1; handled <= 1; ++handled)
    for (mutation = 0; mutation < 4; ++mutation) for (throwStage = 0; throwStage < 4; ++throwStage)
    for (swapOwner = 0; swapOwner < 2; ++swapOwner) {
        Setup((cases % 193) + 17);
        Edit(world)->currentState = states[s];
        if (!hover) User(world, 0)->hoverControl = 0;
        expected = world;
        int result = -99, exception = 0;
        try { result = Edit(world)->UIEditBox::ButtonUp(arguments[a], cases & 1 ? 0 : world.position); }
        catch (int code) { exception = code; }
        int event = hover && !arguments[a] && states[s] == 2;
        int base = event && !handled && mutation != 1 && mutation != 2;
        int wantedException = event && throwStage == 1 ? 71 :
            base && throwStage == 2 ? 72 : base && throwStage == 3 ? 73 : 0;
        int stateCalls = base && !(event && throwStage == 1);
        int soundCalls = stateCalls && throwStage != 2;
        if (event && throwStage != 1) {
            if (mutation == 1) User(expected, 0)->hoverControl = 0;
            if (mutation == 2) Edit(expected)->currentState = 4;
            if (mutation == 3) Edit(expected)->ownerDialog = Dialog(world, 1);
        }
        if (soundCalls) {
            if (swapOwner) Edit(expected)->ownerDialog = Dialog(world, 1);
            Edit(expected)->currentState = 3;
            Edit(expected)->needsRedraw = 0;
            Edit(expected)->redrawFrames = 3;
            if (soundArgument != ((swapOwner || mutation == 3) ? 101 : 100)) Fail(13);
        }
        if (!wantedException && !(event && handled)) Edit(expected)->heldButton = -1;
        if (calls[0] != event || calls[1] != stateCalls || calls[2] != soundCalls || calls[3] != soundCalls ||
            exception != wantedException || result != (wantedException ? -99 : event && handled ? 1 : 0) ||
            memcmp(&world, &expected, sizeof(world))) Fail(14);
        ++cases;
    }
    printf("UI edit release: %u event, helper, mutation and exception cases passed\n", cases);
    return 0;
}
static int TestRedraw() {
    const int frames[] = {(int)0x80000000, -1, 0, 1, 2, 0x7fffffff};
    unsigned cases = 0;
    for (int text = 0; text < 4; ++text) for (int shown = 0; shown < 2; ++shown)
    for (int depth = 0; depth <= 3; ++depth) for (int lastShown = 0; lastShown < 2; ++lastShown)
    for (int transition = -1; transition <= 1; ++transition) for (int frame = 0; frame < 6; ++frame) {
        Setup((cases % 193) + 17);
        UIEditBox* edit = Edit(world);
        edit->controlText = text ? reinterpret_cast<char*>(1) : 0;
        edit->controlTextLength = text == 1 ? 0 : text == 2 ? -1 : 5;
        edit->shown = shown;
        edit->redrawFrames = frames[frame];
        edit->anchorControl = depth ? Anchor(world, 0) : 0;
        for (int i = 0; i < depth; ++i) {
            Anchor(world, i)->anchorControl = i + 1 < depth ? Anchor(world, i + 1) : 0;
            Anchor(world, i)->shown = lastShown;
            Anchor(world, i)->transitionState = transition;
        }
        expected = world;
        if (!shown || text < 2 || (depth && (!lastShown || transition))) {
            int value = (int)((unsigned)frames[frame] - 1);
            Edit(expected)->redrawFrames = value < 0 ? 0 : value;
            if (value < 0) Edit(expected)->needsRedraw = 1;
        }
        edit->UIEditBox::UpdateNeedsRedraw();
        if (memcmp(&world, &expected, sizeof(world))) Fail(15);
        ++cases;
    }
    printf("UI edit redraw: %u text, anchor, counter and whole-storage cases passed\n", cases);
    return 0;
}
int main() {
    controlTable[29] = Address(&Observer::Set);
    controlTable[50] = Address(&Observer::Redraw);
    controlTable[58] = Address(&Observer::Sound);
    dialogTable[29] = Address(&Observer::Handle);
    int result = TestRelease();
    return result ? result : TestRedraw();
}
