// D3DIMSoultreeModifier.cpp -- see D3DIMSoultreeModifier.h for the evidence.

#include "D3DIMSoultreeModifier.h"
#include "DebugAlloc.h"

D3DIMSoultreeModifier::D3DIMSoultreeModifier(int flags)
    : GraphicsTest(flags)
{
    field_0x38 = 0;
    field_0x34 = 0;
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

void D3DIMSoultreeModifier::UnknownFunction4452f0(D3DIMSoultreeObject* object)
{
    D3DIMSoultreeObject** objects =
        (D3DIMSoultreeObject**)DebugMalloc(field_0x38 * 4 + 4, __FILE__, 26);
    for (int i = 0; i < field_0x38; i++)
        objects[i] = field_0x34[i];
    objects[field_0x38] = object;
    field_0x38++;
    operator delete(field_0x34, __FILE__, 33);
    field_0x34 = objects;
}

void D3DIMSoultreeModifier::UnknownFunction445360(D3DIMSoultreeObject* object)
{
    D3DIMSoultreeObject** objects;
    if (field_0x38 > 1)
        objects = (D3DIMSoultreeObject**)DebugMalloc(field_0x38 * 4 + 4, __FILE__, 41);
    else
        objects = 0;
    int n = 0;
    for (int i = 0; i < field_0x38; i++) {
        if (field_0x34[i] != object)
            objects[n++] = field_0x34[i];
    }
    operator delete(field_0x34, __FILE__, 53);
    field_0x34 = objects;
    field_0x38--;
}

void D3DIMSoultreeModifier::UnknownFunction4453e0()
{
    if (field_0x38) {
        D3DIMSoultreeObject** objects =
            (D3DIMSoultreeObject**)DebugMalloc(field_0x38 * 4, __FILE__, 64);
        for (int i = 0; i < field_0x38; i++)
            objects[i] = field_0x34[i];
        int count = field_0x38;
        for (int j = 0; j < count; j++) {
            if (field_0x3c)
                objects[j]->UnknownFunction444de0(this);
            else
                objects[j]->UnknownFunction444f10(this);
        }
        operator delete(objects, __FILE__, 80);
    }
}
