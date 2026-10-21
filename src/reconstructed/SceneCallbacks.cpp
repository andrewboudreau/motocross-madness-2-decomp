// Scene reader callbacks: complete direct-call RET bodies. The names,
// overload grouping, and original translation unit remain provisional.
#include "SceneManager.h"

// 0x004de580, complete RET 12. Keep the existing distinct declarations:
// a shared return address does not establish that these were one method.
void Scene::UnknownFunction4de580(const char*, const char*, const char*) {}
void Scene::UnknownFunction4de580(const char*, const char*, float) {}
void Scene::UnknownFunction4de580(const char*, const char*, int) {}
void Scene::UnknownFunction4de580(const char*, const char*, const Vector3*) {}
void Scene::UnknownFolded4de580(const char*, const char*, const char*) {}

// 0x00464e80, complete RET 4. Retail does not inspect the formatted message.
void Scene::UnknownFunction464e80(const char*) {}
