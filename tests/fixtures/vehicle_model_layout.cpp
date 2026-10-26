#include <stddef.h>
#include "../../src/reconstructed/D3DIMSoulTree.h"
#include "../../src/reconstructed/BikeCamera.h"
#include "../../src/reconstructed/AuralScape.h"
#include "../../src/reconstructed/RaceView.h"
#include "../../src/krusty2/math/Vec3.h"

// The caller's Vector3 buffers cross the existing Vec3 pointer interface.
typedef char vector_size[sizeof(Vector3) == 12 && sizeof(Vec3) == 12 ? 1 : -1];
typedef char vector_x[offsetof(Vector3, x) == offsetof(Vec3, x) ? 1 : -1];
typedef char vector_y[offsetof(Vector3, y) == offsetof(Vec3, y) ? 1 : -1];
typedef char vector_z[offsetof(Vector3, z) == offsetof(Vec3, z) ? 1 : -1];
typedef char node_size[sizeof(SoultreeObject) == 0x1a4 ? 1 : -1];
typedef char render_node_size[sizeof(D3DIMSoultreeObject) == 0x2d8 ? 1 : -1];
typedef char local_matrix[offsetof(SoultreeObject, localMatrix) == 0xb8 ? 1 : -1];
typedef char world_matrix[offsetof(SoultreeObject, worldMatrix) == 0xf8 ? 1 : -1];
typedef char node_child[offsetof(SoultreeObject, firstChild) == 0x140 ? 1 : -1];
typedef char node_flag[offsetof(SoultreeObject, hidden) == 0x14c ? 1 : -1];
typedef char screen_rect[offsetof(D3DIMSoultreeObject, screenRect) == 0x298 ? 1 : -1];
typedef char second_rect[offsetof(D3DIMSoultreeObject, field_0x2a8) == 0x2a8 ? 1 : -1];
typedef char vehicle_node[offsetof(UnknownCameraVehicle, modelNode) == 0x3bc ? 1 : -1];
typedef char character_node[offsetof(UnknownCameraTargetA, modelNode) == 0x1a0 ? 1 : -1];
typedef char car_node[offsetof(UnknownCameraTargetB, carModel) == 0x34 ? 1 : -1];
typedef char racer_node[offsetof(UnknownEventRacer, vehicleModel) == 0x3bc ? 1 : -1];
typedef char followed_node[offsetof(SoultreeSoundEmitter, followedPart) == 0x1a4 ? 1 : -1];
typedef char followed_child[offsetof(SoultreeSoundEmitter, childPart) == 0x1a8 ? 1 : -1];

// Compile-time check of the corrected explicit output-pointer return, not a
// hidden by-value vector return. These are declarations, not test providers.
Vec3* (SoultreeObject::*PositionMethod)(Vec3*) = &SoultreeObject::GetPosition;
void (SoultreeObject::*PositionInMethod)(SoultreeObject*, Vec3*) = &SoultreeObject::GetPositionIn;
