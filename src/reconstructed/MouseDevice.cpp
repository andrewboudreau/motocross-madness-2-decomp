#include "MouseDevice.h"

// 0x0048a2d0
MouseDevice::MouseDevice() : PCInputDevice(1) {
    for (int j = 0; j < 2; j++)
        axisBindings[j].Init(1, 1);
    unsigned int stamp = ReadClock();
    for (int i = 0; i < 4; i++) {
        buttonStates[i].state = 0;
        buttonStates[i].releaseTime = stamp;
        buttonStates[i].pressTime = stamp;
        buttonStates[i].previousReleaseTime = stamp;
        buttonStates[i].previousPressTime = stamp;
    }
}

// 0x0048a420: attaches `binding` to this device's list for its axis.
int MouseDevice::AttachBinding(UnknownControlBinding* binding) {
    binding->owner = this;
    axisBindings[binding->axisIndex].Add(binding);
    return 1;
}

// 0x0048a4c0: drops every binding with id `id`.
void MouseDevice::RemoveBindings(int id) {
    for (int list = 0; list < 2; list++) {
        for (int i = 0; i < axisBindings[list].m_count; i++) {
            UnknownControlBinding* binding = axisBindings[list].Get(i);
            if (binding->bindingId == id)
                axisBindings[list].Remove(binding);
        }
    }
}

// 0x0048a550
void MouseDevice::MoveAxis(int axis, float amount) {
    for (int i = 0; i < axisBindings[axis].m_count; i++) {
        UnknownControlBinding* binding = axisBindings[axis].Get(i);
        binding->Adjust(amount * binding->maxValue * (1.0f / 375.0f));
    }
}
