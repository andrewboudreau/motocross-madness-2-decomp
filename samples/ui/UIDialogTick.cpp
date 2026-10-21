// Complete nonexact UIDialog tick candidate, retail 0x0046ef00..0x0046f11e.
// The final cold joystick branches are included; 0x0046f11f is alignment.
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/DialogProc.h"
#include "../../src/reconstructed/DialogEventKind.h"
#include "../../src/reconstructed/GUIManager.h"
#include "../../src/reconstructed/InputDevice.h"
#include "../../src/reconstructed/UIDialog.h"

// 0x0046ef00: runs the timers and the joystick focus moves.
int UIDialog::Tick(float frameTime) {
    if (lastFrameTime != 0.0f)
        lastFrameTime = frameTime;
    else
        lastFrameTime = 0.001f;
    if (sendFrameEvent) {
        UnknownDialogEvent event;
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
            timer->elapsedTime += (int)(frameTime * 1000.0f);
            if ((unsigned int)timer->elapsedTime >= (unsigned int)timer->periodTime) {
                timer->elapsedTime = 0;
                UIControl* control;
                if ((control = timer->targetControl) != 0) {
                    control->HandleTimer((int)timer);
                } else {
                    UnknownDialogEvent event;
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
        user->SetCursorAnimation(focus && focus->mouseAnim ? (UnknownCursorAnimation*)focus->mouseAnim
                                                                 : cursorAnimation);
        GUIInputDevice* device;
        int j = 0;
        while ((device = guiUser->GetAcceptedDevice(j++)) != 0) {
            if (device->inputDevice->deviceKind == 2) {
                int x = device->pointerPosition.x;
                int y = device->pointerPosition.y;
                if (focus) {
                    if (x <= -1) {
                        if (joystickCentred) {
                            UIControl* next = focus->PreviousFocusableControl();
                            joystickCentred = 0;
                            if (next)
                                RequestHover(next, 0, 0);
                        }
                    } else if (x >= 1) {
                        if (joystickCentred) {
                            UIControl* next = focus->NextFocusableControl();
                            joystickCentred = 0;
                            if (next)
                                RequestHover(next, 0, 0);
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
                }
            }
        }
        UpdatePointerHover();
        GameObject::Tick(lastFrameTime);
    }
    return 1;
}
