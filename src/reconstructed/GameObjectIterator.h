#pragma once

#include "GameObject.h"

// gameobj.cpp's iterator over the objects GameObject::FindByClassName
// searches (0x94 bytes; the same modes): those whose name list or RTTI class
// name matches "<filter>,", or every one for a null filter. While it runs it
// holds g_UnknownGlobal65b548, which blocks releases and relinking.
class GameObjectIterator {
public:
    GameObjectIterator(GameObject* root, int mode, const char* filter); // 0x00469950
    ~GameObjectIterator();                                                // 0x00469a40
    GameObject* Next();                                                   // 0x00469a50
    void UnknownFunction469a20();                                         // 0x00469a20: stops early

private:
    GameObject* field_0x00;    // root
    GameObject* field_0x04;    // current object, 0 when done
    int field_0x08;            // mode (see GameObject::FindByClassName)
    char field_0x0c[0x80];     // "<filter>,"
    int field_0x8c;            // compared length (the filter without the comma)
    int field_0x90;            // set when the walk ran out (the global was released)
};
