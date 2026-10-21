#pragma once

#include "EcoQuadTreeObject.h"
#include "EcoCoordinates.h"

class TextureMapManager;
class UnknownEcoRenderTarget;
class CollisionObject;
struct Vector3;

class Vegetation : public UnknownEcoQuadTreeObject {
public:
    Vegetation();                             // 0x00456650
    virtual unsigned short UnknownVirtualSlot0(); // 0x00456690: view depth as a sort key
    virtual unsigned int UnknownVirtualSlot1(UnknownEcoQuadTree* tree); // 0x00456720: cell code

    // 0x004567a0 / 0x004567e0: place the object (quantised or world
    // coordinates); the first argument (EcoSystem::textureManager) is not used.
    void PlaceQuantized(TextureMapManager* textures, unsigned char definition,
                               const UnknownEcoCoordinates* coordinates, unsigned char heightParameter,
                               unsigned char radiusParameter);
    void Place(TextureMapManager* textures, unsigned char definition,
                               const Vector3* position, unsigned char heightParameter,
                               unsigned char radiusParameter);
    // 0x00456890: distance test against the current detail band.
    void TestDistance(int* billboard, int* fade);
    // 0x00456a10: switches between the 3D geometry and the billboard.
    void SetBillboard(int billboard, int fade);
    void DrawGeometry(UnknownEcoRenderTarget* target);          // 0x00457000: draws the geometry
    int GetCollisionCount();                  // 0x00457080: the definition's collision count
    // 0x004570a0: the definition's collision object `index`, placed here.
    CollisionObject* GetCollisionObject(int index);
    float GetRadius();                        // 0x00457230: the radius

    UnknownEcoCoordinates quantizedPosition;  // in EcoSystem::unitsPerCoordinate units
    unsigned char definitionIndex;            // definition index
    unsigned char fadeLevel;                  // fade (0..255)
    unsigned char heightParam;                // height parameter
    unsigned char radiusParam;                // radius parameter
    unsigned char isBillboard : 1;            // drawn as a billboard
    void* geometryBlock;                      // geometry block (vertices, indices, AgeEntry)
};
