#pragma once

#include "ContainerList.h"
#include "ControlInterface.h"
#include "PCInputDevice.h"

// RTTI: JoystickDevice : PCInputDevice. It overrides slot 0, introduces
// slot 2 (0x00489980) and pure slots 3-20 that PCJoystickDevice implements.
// Signatures of slots not yet reconstructed are placeholders. TU not
// established.
class JoystickDevice : public PCInputDevice {
public:
    explicit JoystickDevice(int index); // 0x00489800

    virtual void RemoveBindings(int id); // 0x00489b70
    virtual ~JoystickDevice();          // 0x00489920 (deleting wrapper 0x00489900)
    // 0x00489980: copies the state of a pressed button into `entry`.
    virtual int IsButtonDown(int button, int modifier, UnknownInputEntry* entry);
    virtual int GetPovAngle(int index, float* angle) = 0;
    virtual int GetPovDirection(int index, float* x, float* y) = 0;
    virtual int SetAutoCenter(int enable) = 0;
    virtual int CreateConstantForce(int effect, long magnitude, unsigned long duration) = 0;
    virtual int SetConstantForce(int effect, long* direction, long magnitude) = 0;
    virtual int SetDirection(int effect, long* direction) = 0;
    virtual int CreateEnvelopedForce(int effect, unsigned long duration, long direction,
                                     long magnitude, unsigned long attackTime,
                                     unsigned long attackLevel, unsigned long fadeTime,
                                     unsigned long fadeLevel, int button) = 0;
    virtual int CreateSquareWave(int effect, unsigned long duration, unsigned long period,
                                 unsigned long magnitude, unsigned long attackTime,
                                 unsigned long attackLevel, unsigned long fadeTime,
                                 unsigned long fadeLevel, int button) = 0;
    virtual int StopEffect(int effect) = 0;
    virtual int StartEffect(int effect, unsigned long iterations, unsigned long flags) = 0;
    virtual int IsEffectPlaying(int effect) = 0;
    virtual int EnumEffects(UnknownEffectInfo* effects, int* count) = 0;
    virtual void ReleaseEffects() = 0;
    virtual int ResetForceFeedback() = 0;
    virtual int SendForceFeedbackCommand(int command) = 0;
    virtual int PauseForceFeedback(int paused) = 0;
    virtual void DetectSubtype() = 0;
    virtual int Update(int value) = 0;

    // 0x00489a20: attaches `binding` to the list for its axis.
    int AttachAxisBinding(UnknownControlBinding* binding, float step, float interval);
    // 0x00489c00: feeds an axis value to every binding on that axis.
    void FeedAxis(int axis, float value, float range);
    // 0x00489c60: state of `control` (negative: an axis direction).
    int GetControlState(int control, int modifier, UnknownInputEntry* entry);
    float GetAxisValue(int axis);    // 0x00489e20: axis value (OptionProcs.cpp)

    friend class ControlInterface; // reads the input entries

protected:
    int joystickIndex;                     // constructor argument
    UnknownInputEntry buttonStates[32];
    float axisValues[6];                // axis values
    ContainerList<UnknownControlBinding*> axisBindings[6]; // bindings per axis
    unsigned char directionFlipped : 1;  // "JoyDirectionFlipped" setting
};
