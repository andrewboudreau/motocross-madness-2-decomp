#include <stddef.h>
#define CHECK(name, expression) typedef char name[(expression) ? 1 : -1]

// Compile the actual local motion view, not a copy of its declaration.
#ifdef MOTION_VIEW
#include "../../src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp"
CHECK(model_size, sizeof(ModelObject) == 0x2d8);
CHECK(texture_manager, offsetof(ModelObject, textureManager) == 0x240);
CHECK(lod_count, offsetof(ModelObject, lodCount) == 0x274);
CHECK(lod_table, offsetof(ModelObject, lodTable) == 0x28c);
extern "C" int LocalModelSecondary() {
    return (int)static_cast<GameObject*>(reinterpret_cast<ModelObject*>(0x1000)) - 0x1000;
}
#else
#include "../../src/reconstructed/CarProcedural.h"
#include "../../src/reconstructed/D3DIMSoulTree.h"
CHECK(model_size, sizeof(CarProceduralModel) == 0x2d8);
CHECK(node_size, sizeof(CarProceduralNode) == 12);
CHECK(vector_size, sizeof(Vector3) == 12);
CHECK(vector_y, offsetof(Vector3, y) == 4);
CHECK(vector_z, offsetof(Vector3, z) == 8);
extern "C" int LocalModelSecondary() {
    return (int)static_cast<GameObject*>(reinterpret_cast<CarProceduralModel*>(0x1000)) - 0x1000;
}
#endif
CHECK(canonical_size, sizeof(D3DIMSoultreeObject) == 0x2d8);
extern "C" int CanonicalModelSecondary() {
    return (int)static_cast<GameObject*>(reinterpret_cast<D3DIMSoultreeObject*>(0x1000)) - 0x1000;
}
