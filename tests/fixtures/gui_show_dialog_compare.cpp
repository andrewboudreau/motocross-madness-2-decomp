// Private test only: executes one validated retail body with scripted callees.
// No reference code or dependency substitutes enter the production game.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/GUIManagerCore.h"
#include "../../src/reconstructed/RenderTarget.h"

struct Scenario { int flags, parent, lookup, returns, wait, palette, mutate, seed; };
struct Event {
    int operation, receiver, arguments[16], hasRect, rect[4];
    char text[2][256];
};
struct Observation {
    int result, count;
    Event events[16];
    unsigned char manager[sizeof(GUIManager)];
    unsigned char dialogs[3 * sizeof(UIDialog)];
    unsigned char target[sizeof(RenderTarget)];
    CameraRect callerRect;
};
static Scenario scenario;
static Observation observed, expected;
static union { double align; unsigned char data[sizeof(GUIManager)]; } managerSpace;
static union { double align; unsigned char data[3 * sizeof(UIDialog)]; } dialogSpace;
static union { double align; unsigned char data[sizeof(RenderTarget)]; } targetSpace;
static unsigned char pointerTokens[128];
static CameraRect callerRect;
static DWORD dialogVtable[28];
static int lookupCount, checks;
static DWORD referenceAddress;

static GUIManager* Manager() { return (GUIManager*)managerSpace.data; }
static UIDialog* Dialog(int n) { return (UIDialog*)(dialogSpace.data + n * sizeof(UIDialog)); }
static RenderTarget* Target() { return (RenderTarget*)targetSpace.data; }
static Event& Record(int operation, const void* receiver) {
    if (observed.count >= 16) ExitProcess(90);
    Event& e = observed.events[observed.count++];
    e.operation = operation; e.receiver = (int)receiver;
    return e;
}
static void Rect(Event& e, const CameraRect* rect) {
    e.hasRect = rect != 0;
    if (rect) { e.rect[0] = rect->left; e.rect[1] = rect->top;
                e.rect[2] = rect->right; e.rect[3] = rect->bottom; }
}
static void Text(Event& e, int n, const char* text) {
    if (!text || strlen(text) >= sizeof(e.text[n])) ExitProcess(91);
    strcpy(e.text[n], text);
}
static void Touch(int operation) {
    if (!scenario.mutate) return;
    Manager()->field_0x1f8 ^= operation * 0x010101;
    Manager()->paletteChanged = operation & 1;
    Manager()->dialogFontSize = (short)(-32768 + operation);
    Manager()->guiBackground = (BackgroundImage*)(pointerTokens + operation);
    Dialog(0)->field_0x150 ^= operation * 0x10101;
}

UIDialog* GUIManager::FindInputDialog() {
    Event& e = Record(1, this);
    int index = -1;
    switch (scenario.lookup) {
    case 0: break; // first lookup null, so no second lookup
    case 1: index = 1; break; // same nonmodal current dialog
    case 2: index = lookupCount ? 2 : 1; break; // second differs and is modal
    case 3: index = 0; break; // current and requested dialog alias
    case 4: index = lookupCount ? 1 : 2; break; // second differs and is nonmodal
    }
    e.arguments[0] = lookupCount++;
    e.arguments[1] = index;
    Touch(1);
    return index < 0 ? 0 : Dialog(index);
}
void GUIManager::ReleaseBackgroundGrab() { Record(2, this); Touch(2); }
void UIDialog::SetModalPause(int a, int b, CameraRect* rect) {
    Event& e = Record(3, this); e.arguments[0] = a; e.arguments[1] = b; Rect(e, rect);
    if (scenario.mutate) showFlags ^= 0x11223344;
    Touch(3);
}
void GUIManager::GrabBackground(int dim) {
    Record(4, this).arguments[0] = dim; Touch(4);
}
void GUIManager::RecreateCursors(const char* image, int redraw) {
    Event& e = Record(5, this); e.arguments[0] = (int)image; e.arguments[1] = redraw;
    Text(e, 0, image); Touch(5);
}
void GUIManager::AttachDialog(UIDialog* dialog, int a, int wait) {
    Event& e = Record(7, this); e.arguments[0] = (int)dialog;
    e.arguments[1] = a; e.arguments[2] = wait; e.arguments[3] = dialog->showFlags;
    if (scenario.mutate) dialog->showFlags = 0x24681357;
    Touch(7);
}

// Nonvirtual test entry with the exact slot-27 thiscall argument ABI. The real
// vtable call in both bodies remains intact; only the supplied object's slot is
// scripted. Other slots are null so accidental dispatches fail loudly.
class DialogProbe {
public:
    UIDialog* Create(void* target, CameraRect* area, int a, int flags,
                    SoundGroup* sound, void* textures, const char* directory,
                    UIDialog* parent, BackgroundImage* background, const char* font,
                    int fontSize, GUIManager* gui, GUIUser* user, const char* resource) {
        Event& e = Record(6, this);
        e.arguments[0]=(int)target; e.arguments[1]=(int)area;
        e.arguments[2]=a; e.arguments[3]=flags; e.arguments[4]=(int)sound;
        e.arguments[5]=(int)textures; e.arguments[6]=(int)directory;
        e.arguments[7]=(int)parent; e.arguments[8]=(int)background;
        e.arguments[9]=(int)font; e.arguments[10]=fontSize;
        e.arguments[11]=(int)gui; e.arguments[12]=(int)user;
        e.arguments[13]=(int)resource;
        Rect(e, area); Text(e, 0, directory); Text(e, 1, font);
        if (scenario.mutate) ((UIDialog*)this)->showFlags = 0x13572468;
        Touch(6);
        return scenario.returns == 0 ? 0 : Dialog(scenario.returns == 1 ? 0 : 2);
    }
};
template <class T> DWORD Address(T method) {
    typedef char MemberPointerMustBeOneWord[sizeof(T) == sizeof(DWORD) ? 1 : -1];
    union { T method; DWORD address; } p; p.method = method; return p.address;
}
static DWORD Replacement(DWORD target) {
    switch (target) {
    case 0x485df0: return Address(&GUIManager::FindInputDialog);
    case 0x4860f0: return Address(&GUIManager::ReleaseBackgroundGrab);
    case 0x470070: return Address(&UIDialog::SetModalPause);
    case 0x4860a0: return Address(&GUIManager::GrabBackground);
    case 0x4865e0: return Address(&GUIManager::RecreateCursors);
    case 0x485bd0: return Address(&GUIManager::AttachDialog);
    }
    return 0;
}
static int LoadReference(const char* path, const char* patches) {
    FILE* file = fopen(path, "rb");
    if (!file) return 0;
    unsigned char* code = (unsigned char*)VirtualAlloc(0, 341, MEM_COMMIT | MEM_RESERVE,
                                                      PAGE_EXECUTE_READWRITE);
    if (!code || fread(code, 1, 341, file) != 341 || fgetc(file) != EOF) return 0;
    fclose(file);
    file = fopen(patches, "rb");
    if (!file) return 0;
    DWORD patch[2]; int count = 0;
    while (fread(patch, sizeof(patch), 1, file) == 1) {
        DWORD offset=patch[0], target=patch[1], replacement=Replacement(target);
        if (offset + 5 > 341 || !replacement || code[offset] != 0xe8 ||
            0x485a70 + offset + 5 + *(DWORD*)(code + offset + 1) != target) return 0;
        *(DWORD*)(code + offset + 1) = replacement - (DWORD)(code + offset + 5);
        ++count;
    }
    fclose(file);
    if (count != 8) return 0;
    FlushInstructionCache(GetCurrentProcess(), code, 341);
    referenceAddress = (DWORD)code;
    return 1;
}
static void Run(int reference) {
    memset(&observed, 0, sizeof(observed));
    memset(managerSpace.data, 0xa5, sizeof(managerSpace.data));
    memset(dialogSpace.data, 0x5a, sizeof(dialogSpace.data));
    memset(targetSpace.data, 0x69, sizeof(targetSpace.data));
    memset(dialogVtable, 0, sizeof(dialogVtable));
    dialogVtable[27] = Address(&DialogProbe::Create);
    for (int i=0; i<3; ++i) *(DWORD*)Dialog(i) = (DWORD)dialogVtable;
    Dialog(0)->isModal = -1; Dialog(1)->isModal = 0; Dialog(2)->isModal = 1;
    // GameObject render-context storage is protected in the shared declaration.
    *(void**)(managerSpace.data + 0x18) = Target();
    Target()->targetWidth = scenario.seed % 3 == 0 ? 0 : scenario.seed % 3 == 1 ? 640 : 2048;
    Target()->targetHeight = scenario.seed % 3 == 0 ? 0 : scenario.seed % 3 == 1 ? 480 : 17;
    Manager()->paletteChanged = scenario.palette ? -7 : 0;
    Manager()->guiSoundGroup = (SoundGroup*)(pointerTokens + 32);
    Manager()->guiTextures = (TextureMapManager*)(pointerTokens + 40);
    Manager()->guiBackground = (BackgroundImage*)(pointerTokens + 48);
    Manager()->users[0] = (GUIUser*)(pointerTokens + 56);
    Manager()->dialogFontSize = scenario.seed & 1 ? -19 : 32767;
    strcpy(Manager()->dialogDirectory, "dialogs\\distinct-directory");
    strcpy(Manager()->dialogFontName, "Distinct Font Name");
    strcpy(Manager()->cursorImage, "normal-cursor.tga");
    strcpy(Manager()->waitCursorImage, "wait-cursor.tga");
    callerRect.left = -17; callerRect.top = 23; callerRect.right = 317; callerRect.bottom = 409;
    lookupCount = 0;
    UIDialog* parent = scenario.parent == 0 ? 0 : Dialog(scenario.parent == 1 ? 0 : 1);
    union {
        UIDialog* (GUIManager::*method)(UIDialog*,int,int,int,UIDialog*,int,int,int);
        DWORD address;
    } call;
    call.method = &GUIManager::ShowDialog;
    if (reference) call.address = referenceAddress;
    observed.result = (int)(Manager()->*call.method)(Dialog(0), scenario.seed * 37 - 911,
        scenario.flags, scenario.seed & 1 ? (int)&callerRect : 0, parent,
        scenario.seed * -131, (int)(pointerTokens + 72), scenario.wait);
    memcpy(observed.manager, managerSpace.data, sizeof(managerSpace.data));
    memcpy(observed.dialogs, dialogSpace.data, sizeof(dialogSpace.data));
    memcpy(observed.target, targetSpace.data, sizeof(targetSpace.data));
    observed.callerRect = callerRect;
}
static void Compare() {
    Run(1); expected = observed;
    Run(0);
    if (memcmp(&expected, &observed, sizeof(observed))) {
        printf("Mismatch case %d flags=%x parent=%d lookup=%d returns=%d wait=%d palette=%d mutate=%d\n",
            checks,scenario.flags,scenario.parent,scenario.lookup,scenario.returns,
            scenario.wait,scenario.palette,scenario.mutate);
        printf("Events reference=%d candidate=%d returns=%x/%x\n",expected.count,observed.count,
               expected.result,observed.result);
        for (int i=0; i<(int)sizeof(observed); ++i)
            if (((unsigned char*)&observed)[i] != ((unsigned char*)&expected)[i]) {
                printf("First different observation byte %d: %02x/%02x\n",i,
                    ((unsigned char*)&expected)[i],((unsigned char*)&observed)[i]); break;
            }
        fflush(stdout);
        ExitProcess(2);
    }
    ++checks;
}
int main(int argc, char** argv) {
    if (argc != 3 || !LoadReference(argv[1], argv[2])) { puts("Reference mapping failed"); return 1; }
    const int waits[] = {0, 1, -3};
    for (scenario.flags=0; scenario.flags<64; ++scenario.flags)
    for (scenario.parent=0; scenario.parent<3; ++scenario.parent)
    for (scenario.lookup=0; scenario.lookup<5; ++scenario.lookup)
    for (scenario.returns=0; scenario.returns<3; ++scenario.returns)
    for (int w=0; w<3; ++w)
    for (scenario.palette=0; scenario.palette<2; ++scenario.palette)
    for (scenario.mutate=0; scenario.mutate<2; ++scenario.mutate) {
        scenario.wait = waits[w];
        scenario.seed = scenario.flags * 13 + scenario.parent * 7 +
                        scenario.lookup * 5 + scenario.returns * 3 + w + scenario.palette;
        Compare();
    }
    printf("ShowDialog: %d scenarios match return, complete state and ordered calls\n", checks);
    VirtualFree((void*)referenceAddress, 0, MEM_RELEASE);
    return 0;
}
