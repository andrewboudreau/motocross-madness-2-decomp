// UIDialog::Tick, retail 0x0046ef00..0x0046f11e (543 bytes, strict exact).
// GameUi.cpp includes this body (config/game_build.json included_sources); it
// also compiles on its own as a calibration case and the native-test provider.
//
// Two source shapes give retail's code:
// - The timer step is an inline helper that clears the elapsed time and
//   reports expiry. Its result test separates the +0x0c clear from the
//   +0x14 target load, so VC6 does not hoist the load above the store.
// - The joystick arms only pick the control to focus; one shared
//   `if (next) RequestHover(next, 0, 0)` follows them. VC6 threads that
//   test out of the arms that leave `next` null, which places every arm after
//   the first beyond the epilogue, as in retail.
#include "GameUi.h"
#include "DialogProc.h"
#include "DialogEventKind.h"
#include "GUIManager.h"
#include "InputDevice.h"
#include "UIDialog.h"

// Adds `elapsed` milliseconds to the timer; when the period has run out it
// restarts the timer and returns 1.
static inline int AdvanceTimer(UITimer* timer, int elapsed) {
    timer->elapsedTime += elapsed;
    if ((unsigned int)timer->elapsedTime >= (unsigned int)timer->periodTime) {
        timer->elapsedTime = 0;
        return 1;
    }
    return 0;
}

// 0x0046ef00: runs the timers and the joystick focus moves.
int UIDialog::Tick(float frameTime) {
    if (lastFrameTime != 0.0f)
        lastFrameTime = frameTime;
    else
        lastFrameTime = 0.001f;
    if (sendFrameEvent) {
        DialogEvent event;
        event.gui = guiManager;
        event.handled = 0;
        event.kind = kDialogFrame;
        event.controlName = 0;
        event.dialog = this;
        event.control = 0;
        HandleEvent(&event);
        sendFrameEvent = 0;
    }
    if (!isPaused && !isClosing) {
        UITimer* timer;
        for (int i = 0; (timer = timerList.Get(i)) != 0; i++) {
            if (AdvanceTimer(timer, (int)(frameTime * 1000.0f))) {
                UIControl* control;
                if ((control = timer->targetControl) != 0) {
                    control->HandleTimer((int)timer);
                } else {
                    DialogEvent event;
                    event.code = timer->timerId;
                    event.gui = guiManager;
                    event.handled = 0;
                    event.kind = kDialogTimer;
                    event.controlName = 0;
                    event.dialog = this;
                    event.control = 0;
                    HandleEvent(&event);
                    if (event.handled)
                        return 1;
                }
            }
        }
        GUIUser* user = guiUser;
        UIControl* focus = (UIControl*)user->hoverControl;
        user->SetCursorAnimation(focus && focus->mouseAnim ? (CursorUIAnimView*)focus->mouseAnim
                                                                 : cursorAnimation);
        GUIInputDevice* device;
        int j = 0;
        while ((device = guiUser->GetAcceptedDevice(j++)) != 0) {
            if (device->inputDevice->deviceKind == 2) {
                int x = device->pointerPosition.x;
                int y = device->pointerPosition.y;
                if (focus) {
                    UIControl* next = 0;
                    if (x <= -1) {
                        if (joystickCentred) {
                            next = focus->PreviousFocusableControl();
                            joystickCentred = 0;
                        }
                    } else if (x >= 1) {
                        if (joystickCentred) {
                            next = focus->NextFocusableControl();
                            joystickCentred = 0;
                        }
                    } else if (y <= -1) {
                        if (joystickCentred)
                            joystickCentred = 0;
                    } else if (y >= 1) {
                        if (joystickCentred)
                            joystickCentred = 0;
                    } else {
                        joystickCentred = 1;
                    }
                    if (next)
                        RequestHover(next, 0, 0);
                }
            }
        }
        UpdatePointerHover();
        GameObject::Tick(lastFrameTime);
    }
    return 1;
}
