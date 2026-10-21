#include "core/GameObject.h"
#include "../../src/reconstructed/GameObject.h"
#include "../../src/reconstructed/GameObjectIterator.h"
#include <stdio.h>

typedef char CheckBaseObjectSize[sizeof(BaseObject) == 8 ? 1 : -1];
typedef char CheckGameObjectSize[sizeof(GameObject) == 0x2c ? 1 : -1];

class PhysicsObject : public GameObject {
public:
    PhysicsObject() : GameObject(1), timeStep(0), event(0), entry(0) {}
    virtual int UnknownVirtualSlot11(float step) { timeStep = step; return 7; }
    virtual int OnControlPress(UnknownControlEvent* value, UnknownInputEntry* owner) {
        event = value;
        entry = owner;
        return 1;
    }
    void* Context() const { return renderContext; }
    void DisableNamed(const char* name) { DisableByClassName(name); }
    void EnableNamed(const char* name) { EnableByClassName(name); }
    float timeStep;
    UnknownControlEvent* event;
    UnknownInputEntry* entry;
};

int main() {
    if (g_ActiveGameObjectIterators != 0) return 1;
    GameObject root(1);
    PhysicsObject child;
    GameObject* object = &child;
    if (object->AddRef() != 2 || object->Release() != 1 ||
        object->GetRefCount() != 1 || !object->isEnabled) return 2;
    if (object->UnknownVirtualSlot11(0.125f) != 7 || child.timeStep != 0.125f) return 3;
    int context, event, entry;
    if (object->SetRenderContext(&context) != object || child.Context() != &context) return 4;
    if (root.AppendChild(object, -1) != (int)object) return 5;
    if (!root.OnControlPress((UnknownControlEvent*)&event, (UnknownInputEntry*)&entry) ||
        child.event != (UnknownControlEvent*)&event || child.entry != (UnknownInputEntry*)&entry) return 6;
    if (!root.AttachRenderContext(&root) || child.Context() != &root) return 7;
    {
        GameObjectIterator iterator(&root, 0, 0);
        if (g_ActiveGameObjectIterators != 1 || object->Unlink() != 0) return 8;
        if (iterator.Next() != object) return 9;
        iterator.Stop();
        if (g_ActiveGameObjectIterators != 0) return 10;
    }
    if (g_ActiveGameObjectIterators != 0 || object->Unlink() != 1) return 11;
    PhysicsObject namedRoot, namedChild, namedGrandchild, skipped;
    GameObject other(1);
    namedRoot.AppendChild(&namedChild, -1);
    namedRoot.AppendChild(&other, -1);
    namedRoot.AppendChild(&skipped, -1);
    namedChild.AppendChild(&namedGrandchild, -1);
    skipped.MarkPendingRelease();
    namedRoot.DisableNamed("PhysicsObject");
    if (namedChild.isEnabled || namedGrandchild.isEnabled || !other.isEnabled ||
        !namedRoot.isEnabled || !skipped.isEnabled) return 12;
    namedRoot.EnableNamed("PhysicsObject");
    if (!namedChild.isEnabled || !namedGrandchild.isEnabled || !other.isEnabled) return 13;
    namedRoot.DisableNamed("MissingClass");
    if (!namedChild.isEnabled || !namedGrandchild.isEnabled || !other.isEnabled) return 14;
    // Alias-list matching takes priority over RTTI matching.
    other.AppendClassName(&namedChild);
    namedRoot.DisableNamed("PhysicsObject");
    if (namedChild.isEnabled || namedGrandchild.isEnabled || other.isEnabled || !skipped.isEnabled) return 15;
    namedRoot.EnableNamed("PhysicsObject");
    if (!namedChild.isEnabled || !namedGrandchild.isEnabled || !other.isEnabled) return 16;
    namedGrandchild.Unlink();namedChild.Unlink();other.Unlink();skipped.Unlink();
    puts("GameObject: shared headers, virtual dispatch, child callbacks and iterator storage and named descendant switches passed");
    return 0;
}
