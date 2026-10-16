#pragma once

#include "GameObject.h"

// gameobj.cpp's iterator over the objects GameObject::FindByClassName
// searches (0x94 bytes; the same modes): those whose name list or RTTI class
// name matches "<filter>,", or every one for a null filter. While it runs it
// counts in g_ActiveGameObjectIterators, which blocks releases and relinking.
class GameObjectIterator {
public:
    GameObjectIterator(GameObject* root, int mode, const char* filter); // 0x00469950
    ~GameObjectIterator();                                                // 0x00469a40
    GameObject* Next();                                                   // 0x00469a50
    void Stop();                                         // 0x00469a20: stops early

private:
    GameObject* rootObject;    // +0x00 root
    GameObject* currentObject; // +0x04 current object, 0 when done
    int walkMode;              // +0x08 mode (see GameObject::FindByClassName)
    char filterText[0x80];     // +0x0c "<filter>,"
    int filterLength;          // +0x8c compared length (the filter without the comma)
    int exhausted;             // +0x90 set when the walk ran out (the global was released)
};
