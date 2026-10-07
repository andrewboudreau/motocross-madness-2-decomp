// D3DIMSoultreeModifier.cpp -- see D3DIMSoultreeModifier.h for the evidence.

#include "D3DIMSoultreeModifier.h"
#include "DebugAlloc.h"

D3DIMSoultreeModifier::D3DIMSoultreeModifier(int flags)
    : GraphicsTest(flags)
{
    objectCount = 0;
    modifiedObjects = 0;
    field_0x3c = 1;
}

D3DIMSoultreeModifier::~D3DIMSoultreeModifier()
{
    UnknownFunction4453e0();
}

GameObject* D3DIMSoultreeModifier::UnknownVirtualSlot8(void* value)
{
    return GameObject::UnknownVirtualSlot8(value);
}

void D3DIMSoultreeModifier::AddObject(D3DIMSoultreeObject* object)
{
    D3DIMSoultreeObject** objects =
        (D3DIMSoultreeObject**)DebugMalloc(objectCount * 4 + 4, __FILE__, 26);
    for (int i = 0; i < objectCount; i++)
        objects[i] = modifiedObjects[i];
    objects[objectCount] = object;
    objectCount++;
    DebugFree(modifiedObjects, __FILE__, 33);
    modifiedObjects = objects;
}

void D3DIMSoultreeModifier::RemoveObject(D3DIMSoultreeObject* object)
{
    D3DIMSoultreeObject** objects;
    if (objectCount > 1)
        objects = (D3DIMSoultreeObject**)DebugMalloc(objectCount * 4 + 4, __FILE__, 41);
    else
        objects = 0;
    int n = 0;
    for (int i = 0; i < objectCount; i++) {
        if (modifiedObjects[i] != object)
            objects[n++] = modifiedObjects[i];
    }
    DebugFree(modifiedObjects, __FILE__, 53);
    modifiedObjects = objects;
    objectCount--;
}

void D3DIMSoultreeModifier::UnknownFunction4453e0()
{
    if (objectCount) {
        D3DIMSoultreeObject** objects =
            (D3DIMSoultreeObject**)DebugMalloc(objectCount * 4, __FILE__, 64);
        for (int i = 0; i < objectCount; i++)
            objects[i] = modifiedObjects[i];
        int count = objectCount;
        for (int j = 0; j < count; j++) {
            if (field_0x3c)
                objects[j]->UnknownFunction444de0(this);
            else
                objects[j]->UnknownFunction444f10(this);
        }
        DebugFree(objects, __FILE__, 80);
    }
}
