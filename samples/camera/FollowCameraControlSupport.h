#pragma once
// Slot 45's input queries go through the game's ControlInterface each time.
// Macros, not inline functions: with inline helpers VC6 runs out of inline
// budget in slot 45 and calls the Vector3 constructor out of line.
#define FOLLOWCAMERA_CONTROLS() (g_TrackGame->controlInterface)
static inline UnknownJoystickBoolView* FollowCameraJoystick(ControlInterface* controls) {
    return (UnknownJoystickBoolView*)controls->activeJoystick;
}
#define FOLLOWCAMERA_KEY(control) (FOLLOWCAMERA_CONTROLS()->IsDeviceButtonDown((control), 0, 0x3f, 0))

// Inlined by slot 38: the yaw turns and wraps to [-pi, pi]; the pitch tilts
// and is clamped to +-1.55. Retail negates the step before adding it, which
// an argument expression reproduces and `-=` does not.
inline void FollowCameraTurn(float& angle, float step) {
    angle += step;
    if (angle > 3.1415927f)
        angle -= 6.2831855f;
    else if (angle < -3.1415927f)
        angle += 6.2831855f;
}

inline void FollowCameraTilt(float& angle, float step) {
    angle += step;
    if (angle > 1.55f)
        angle = 1.55f;
    else if (angle < -1.55f)
        angle = -1.55f;
}

// Slot 45's reverse keys subtract the step instead (fsubr).
inline void FollowCameraTurnBack(float& angle, float step) {
    angle -= step;
    if (angle > 3.1415927f)
        angle -= 6.2831855f;
    else if (angle < -3.1415927f)
        angle += 6.2831855f;
}

inline void FollowCameraTiltBack(float& angle, float step) {
    angle -= step;
    if (angle > 1.55f)
        angle = 1.55f;
    else if (angle < -1.55f)
        angle = -1.55f;
}
