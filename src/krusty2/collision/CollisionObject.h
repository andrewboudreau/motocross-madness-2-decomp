// CollisionObject -- reconstructed class layout (CollisionObject.cpp).
//
// Evidence (tier 1 unless noted), all from analysis/*.json and target bytes:
//  * RTTI direct bases: QuadTreeObject (offset 0) and GraphicsTest (offset 12);
//    GraphicsTest : GameObject with plain (non-virtual) inheritance
//    (BCD mdisp=12 pdisp=-1).  vtables: 0x005511b8 @0 (2 slots, QuadTreeObject's),
//    0x00551148 @12 (27 slots, GraphicsTest/GameObject shape).
//  * ctor 0x00431e70 writes vptr 0x5511c4 (QuadTreeObject), then the GraphicsTest
//    base ctor 0x0047bc70 on this+12, then 0x5511b8 / 0x551148.
//  * sizeof == 0xb8: operator new(0xb8, "CollisionCharacter.cpp", 27) at 0x00431a1d.
//  * Overrides of the secondary vtable are compiled with `this` == the GraphicsTest
//    subobject (this+12); VC6 does this natively for overrides of a second base
//    (verified with a scratch MI class), so the natural source shape matches.
//
// Member names are field_0xNN (object-relative offsets) unless the arithmetic
// clearly shows the meaning; semantic names are tier 3 (provisional).
#ifndef COLLISION_OBJECT_H
#define COLLISION_OBJECT_H

#include "CollisionTypes.h"
#include "core/GraphicsTest.h"   // canonical BaseObject, GameObject, GraphicsTest

class CollisionFileStream;     // .col file stream (CollisionShapeSetup.cpp, stand-in)
class CollisionModelSource;   // scene-graph node source used by shape setup 0x004324b0
struct CollisionHullBody;      // CollisionShapeTests.h
struct CollisionModelBody;
struct CollisionSweepQuery;
struct CollisionTreeNode;      // CollisionDebugDraw.cpp
struct Vec3;                   // ../common/Math3D.h
struct Matrix4;

// GraphicsTest (: GameObject, non-virtual) is declared once in core/GraphicsTest.h.
// No vbptr/vtordisp here; GameObject members are at their plain offsets from the GraphicsTest
// subobject at +12.

class QuadTreeObject {
public:
    virtual void UnknownVirtualSlot0();         // 0x004dc610 (ret)
    virtual int UnknownVirtualSlot1();          // 0x00434ce0 (returns 15)

    QuadTreeObject() { queryStamp = 0; objectTypeId = (char)0xff; }
    short queryStamp;  // +0x04 Quadtree query code (NextObject) skips an object whose short at +4 equals the query counter (field_0x6c) and otherwise stores the counter there (Quadtree.cpp lines 568/612/676); Insert zeroes it
    short sortKey;                           // +0x06 not touched by the inlined ctor; keeps objectTypeId at +8
    char objectTypeId;  // +0x08 ctor 0x00431e70 stores TypeRegistry::FindTypeId(CollisionObject) (0x00521ea0) here; 0x00438e70 compares [candidate+8] with the cached CollisionObject (0x568610) and Vegetation (0x568611) ids
};

// Shape payloads hung off CollisionObject::field_0x54, discriminated by field_0x50.
// Sizes come from the allocation sizes in the shape setup functions; member names
// are provisional (tier 3).
struct CollisionHullShape {                // type 0, 0x198 bytes (0x004328b0, 0x00432720)
    int swept;  // +0x00 same payload as CollisionHullBody: HullVsHull 0x00436720 forwards to HullVsHullSwept when nonzero
    int sceneNode;  // +0x04 scene node whose world position 0x004fc9a0 reads (CollisionHullBody view)
    char field_0x08[0x80];
    char field_0x88[0xc0];                 // filled by 0x0042cc60 / 0x0042c8c0
    CollisionMatrix4 relativeFrame;          // +0x148 copy of the matrix returned by 0x004a1410
    void* triangleTree;                     // +0x188 freed with 0x0042a160
    void* pointTree;                     // +0x18c freed with 0x0042a160
    CollisionVec3* vertices;            // +0x190 vertex array (0x24-byte tagged block)
};

struct CollisionCapsuleShape {             // type 3, 0x68 bytes (0x00432a20)
    CollisionVec3 p0;                      // arguments 0..2
    CollisionVec3 p1;                      // arguments 3..5
    float radius;                          // argument 6
    float radiusSquared;
    float radiusScale;                      // +0x20 1.0f
    float endpointScale;                      // +0x24 1.0f
    CollisionMatrix4 transform;           // +0x28 identity
};

struct CollisionSphereShape {              // type 4, 0x5c bytes (0x004329a0)
    CollisionVec3 center;
    float radius;
    float radiusSquared;
    float radiusScale;                      // +0x14 1.0f
    float centerScale;                      // +0x18 1.0f
    CollisionMatrix4 transform;           // +0x1c identity
};

// Code pointers at CollisionObject+0x88/+0x8c (tier 1: 0x0043b9a0 stores the addresses
// 0x0043b800 and 0x00464e90 there).  The argument list comes from the one known target,
// 0x0043b800 (cdecl: this object, the other object); tier 3.
class CollisionObject;
typedef void (*CollisionCallback)(CollisionObject* self, CollisionObject* other);

class CollisionObject : public QuadTreeObject, public GraphicsTest {
public:
    CollisionObject(int a);                     // 0x00431e70
    virtual ~CollisionObject();                 // slot 0 @12: 0x00431fd0 -> core 0x00432000
    virtual int GameObjectVirtualSlot10(float dt);    // 0x00499ae0
    virtual int GameObjectVirtualSlot14();            // 0x00434540 (draw, then returns GameObject slot 14; ConstraintMethodCollisionModel overrides it again)
    virtual int GameObjectVirtualSlot23(int a, int b);  // 0x00434970 (ret 8, returns GameObject slot 23)

    // Non-virtual members (this == complete object).
    void FreeShape();                                        // 0x00432430
    // 0x004320f0 (ret 0x10): GameObject slot 8 body (0x004692f0, render context at +0x18) on
    // the GraphicsTest base with `context`, then stores the three flags (decoded).
    void Configure(int context, int useBroadphase, int collisionEnabled, int collidable);
    void SetUseBroadphase(int enable);                         // 0x00432120, (re)inserts into or removes from the quadtree
    // Shape setters (type in shapeType, payload in shape; every allocation passes __FILE__).
    void SetSphereShape(CollisionVec3 center, float radius); // 0x004329a0 (type 4)
    void SetCapsuleShape(CollisionVec3 p0, CollisionVec3 p1, float radius);  // 0x00432a20 (type 3)
    void SetModelShape(void* node, int buildPointTrees, int swept, int filter, int d);  // 0x004324b0 (type 1, one hull per accepted node)
    void SetHullShape(void* node, int buildPointTree, int swept, int c, int d);         // 0x00432720 (ret 0x14, type 0 from a scene node)
    void LoadShape(void* node, const char* path);               // 0x00432800, opens `path` "rb" and calls ReadShape
    void SetTriangleMeshHullShape(const CollisionVec3* verts, const int* indices, int triCount, int vertCount);  // 0x004328b0 (ret 0x10, type 0 from arrays)
    void ReadHullShape(CollisionFileStream* stream, CollisionHullBody* hull);                          // 0x00439ed0 (ret 8), reads a hull payload
    void ReadModelShape(void* node, CollisionFileStream* stream, CollisionModelBody* model);            // 0x0043a050 (ret 0xc), reads a model payload
    void ReadShape(void* node, CollisionFileStream* stream);    // 0x00439e10 (ret 8), reads the shape type, then the payload
    void SetMeshShape(int count, void* points);                 // 0x00432ab0 (ret 8, type 2, lines 0x183/0x18a)
    // 0x00436000 (ret 4): copies the translation row of the shape's current transform into *out:
    // hull +0xf8, model +0xb8, mesh +0x38; types 3/4 leave *out untouched (tier 1 body).
    void GetShapePosition(CollisionVec3* out);
    void ResetHitState();            // 0x00434a10, resets the hit state and per-object query record
    void StoreHitResult();           // 0x00434bb0, publishes hitPoint/hitNormal after a hull/model test
    void StoreHitNormal();           // 0x00434cf0, normal-only variant of StoreHitResult
    // 0x00435f10: SetTransform with the scene node's world matrix (0x004fca80 on the node of
    // shape types 0..2; types 3/4 pass a null matrix).
    void SyncTransformFromNode();
    void UpdatePlacement();          // 0x00435fb0, SyncTransformFromNode, then UpdateQuadtreeCell when in the broadphase
    void UpdateQuadtreeCell();       // 0x00436080, refreshes the quadtree cell from the world bounds
    void Fn_00435fe0();                                       // 0x00435fe0
    // 0x00438e70: broad-phase candidates -> TestAgainst -> hit callbacks; returns hasContact.
    int QueryCollisions();
    inline void ReportHit();                                  // inlined three times in 0x00438e70
    // 0x00435830, thiscall, ret 4 (tier 1): switches on the shape type shapeType (0..4,
    // jump table 0x435ea8), copies the 16 floats of *m into the shape payload at
    // shape and derives values from them. Name tier 3. Callers: ObjectPlacement
    // 0x004b0df0 and Tire 0x00514550.
    void SetTransform(const Matrix4* m);
    // 0x00439400, thiscall, ret 4: ignoreListMode = mode (tier 1).
    void SetIgnoreListMode(int mode);
    // 0x00439410, thiscall, ret 4 (tier 1 body): unique-add of owner into the growable
    // pointer array ignoreList (data) / ignoreCount (count): returns if already present,
    // reuses a NULL slot, else reallocs through 0x004a2ec0 (line 0x843) and appends.
    // Name tier 3.
    void AddIgnoredOwner(void* owner);
    // 0x00439490 (ret 4): finds owner in the ignoreList array and NULLs that slot (tier 1 body).
    void RemoveIgnoredOwner(void* owner);
    // 0x004394d0 (ret 8): stores ownerObject and ownerType (tier 1); name tier 3.
    void SetOwner(void* owner, int tag);

    // Narrow phase (CollisionShapeTests.cpp, names tier 3).  These are thiscall members that
    // mostly ignore `this`: the 0x00438b90 dispatchers forward ecx unchanged down the chain.
    void GetWorldBounds(CollisionVec3* outMin, CollisionVec3* outMax);   // 0x00436100
    int TestAgainst(CollisionObject* other);                             // 0x00436430
    int HullVsHull(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q);        // 0x00436720
    int HullVsHullSwept(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q);   // 0x00436af0
    int HullVsModel(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q);      // 0x00436e50
    int HullVsModelSwept(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q); // 0x004376f0
    int HullVsSphere(CollisionHullBody* hull, CollisionSphereShape* sphere);                   // 0x004379c0
    int HullVsCapsule(CollisionHullBody* hull, CollisionCapsuleShape* capsule);                // 0x00437aa0
    int ModelVsHull(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q);      // 0x00437c20
    int ModelVsHullSwept(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q); // 0x00437ef0
    int ModelVsModel(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q);    // 0x00438280
    int ModelVsModelSwept(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q); // 0x00438550
    int ModelVsSphere(CollisionModelBody* model, CollisionSphereShape* sphere);                // 0x00438860
    int ModelVsCapsule(CollisionModelBody* model, CollisionCapsuleShape* capsule);             // 0x004389b0
    int TestHullAgainst(CollisionObject* other);                         // 0x00438b90 (this is type 0)
    int TestModelAgainst(CollisionObject* other);                        // 0x00438c10 (this is type 1)
    int TestMeshBounds(CollisionObject* other);                             // 0x004392c0 (bounds test used by TestMeshAgainst)
    int TestMeshAgainst(CollisionObject* other);                         // 0x00438c90 (this is type 2)

    // Debug drawing of the bounding-volume trees (CollisionDebugDraw.cpp, names tier 3).
    // DrawHull / DrawModel pass a hull's tree (+0x188 triangles or +0x18c points), its
    // transform (+0x48), the second matrix (+0x08) and its vertex array (+0x190).
    void DrawTree(CollisionTreeNode* node, int depth, const Matrix4* xf,
                  const Vec3* verts);                                    // 0x00433240
    void DrawTreeMotion(CollisionTreeNode* node, int depth, const Matrix4* xf,
                        const Matrix4* motion, const Vec3* verts);       // 0x004334c0
    void DrawPointTree(CollisionTreeNode* node, const Matrix4* xf,
                       const Matrix4* motion);                           // 0x00433930
    void DrawTreeNormals(CollisionTreeNode* node, const Vec3* verts,
                         const Matrix4* xf);                             // 0x00433be0
    void DrawBoxTree(CollisionTreeNode* node, int depth, const Matrix4* xf); // 0x00434040
    void DrawSegmentTree(CollisionTreeNode* node, const Matrix4* xf);       // 0x004341b0
    void DrawSegmentTreeLevel(CollisionTreeNode* node, int depth, const Matrix4* xf); // 0x00434340
    void DrawHull(CollisionHullBody* hull, int depth, int mode);         // 0x00432d30
    void DrawModel(CollisionModelBody* model, int depth, int mode);      // 0x00432b30

    // +0x40 is a world bounding-sphere centre (Vec3, written by SetTransform 0x00435830 via
    // 0x00428db0/0x0042a510 and read by BoundingSpheresOverlap 0x0043a270); its first float
    // overlaps GraphicsTest::field_0x34 in the stand-in GraphicsTest layout.
    char field_0x44[8];                         // bounding-sphere centre y, z
    float boundRadius;                           // bounding-sphere radius (fstp [this+0x4c] in 0x00435830)
    int shapeType;                             // +0x50 shape type 0..4
    void* shape;                           // +0x54 shape payload (see shape structs)
    int hasContact;  // +0x58 cleared at entry of 0x00434a10 and 0x00438e70, set to 1 right after a successful TestAgainst (0x00438f6d), returned by 0x00438e70 and tested by ObjectPlacement and ConstraintMethodCollisionModel slot 11
    void* contactRecord;                           // +0x5c contact record pointer (constraint solver)
    void* ownerObject;                           // +0x60 owner: SoulTreePhysics slot 40 loaders store their `this`,
                                                // ConstraintMethodCollisionModel stores itself (tier 2)
    int ownerType;                             // +0x64 type tag; 0x3ea (1002) = has a body at +0xc4
    int ignoreVegetation;  // +0x68 0x00438fef: the branch for a candidate whose objectTypeId == Vegetation (0x568611) is taken only when this is 0; ObjectPlacement temp object and the tire collision character set it to 1
    int collisionEnabled;  // +0x6c 0x00438e8b: when 0, 0x00438e70 clears hasContact and returns 0 without querying; Configure stores its 3rd argument here; ctor sets 1
    int collidable;  // +0x70 TestAgainst 0x00436430 rejects other when other+0x70 == 0; Configure stores its 4th argument; ctor sets 1
    int ignoreListMode;  // +0x74 TestAgainst: with a listed partner mode 1 rejects, with an unlisted partner mode 0 rejects (exclusion vs inclusion list); SetIgnoreListMode 0x00439400 writes it; ctor sets 1
    int ignoreList;  // +0x78 growable pointer array searched by TestAgainst; AddIgnoredOwner 0x00439410 appends (realloc through 0x004a2ec0)
    int ignoreCount;  // +0x7c element count of the array at +0x78 (loop bound in TestAgainst and AddIgnoredOwner)
    int useBroadphase;  // +0x80 0x00435fb0 (-> 0x00436080) and 0x00438e70 only touch the global quadtree 0x68aba4 when it is nonzero, otherwise 0x00438e70 takes the non-quadtree path; Configure stores its 2nd argument
    int quadtreeCell;  // +0x84 0x00436080: cell code from the world bounds (0x4dc8c0); when it differs from this field the object is removed from the old cell (0x4dcf20) and reinserted (0x4dcac0), then the new code is stored; ctor sets 15
    CollisionCallback onHitCallback;               // +0x88 0x0043b9a0 stores 0x0043b800
    CollisionCallback onHitByCallback;               // +0x8c 0x0043b9a0 stores 0x00464e90 (empty function)
    int debugTreeDepth;  // +0x90 slot 23 (0x00434970) decrements it (clamped at 0) on key 10 and increments it on key 11; slot 14 (0x00434540) passes it as the depth argument of DrawHull 0x00432d30 / DrawModel 0x00432b30 and DrawBoxTree
    int debugDrawMode;  // +0x94 slot 23 (0x00434970) increments it on key 0x2e and wraps to 0 above 7; slot 14 (0x00434540) passes it as the mode argument of DrawHull / DrawModel
    int field_0x98;
    CollisionObject* hitObject;  // +0x9c 0x00438e70 stores each broad-phase candidate here before TestAgainst(candidate) and then passes it to the callbacks
    CollisionVec3 hitPoint;  // +0xa0 0x00434bb0 fills it from the contact record +0x18, 0x00438e70 copies it into the hit object +0xa0; SoultreePhysics slots 28/38 read it as contact point
    CollisionVec3 hitNormal;  // +0xac 0x00434bb0 fills it from the normalised accumulated scratch points; 0x00438e70 copies it into the hit object +0xac; read as contact normal by SoultreePhysics
};

typedef char collision_object_assert_sizeof[(sizeof(CollisionObject) == 0xb8) ? 1 : -1];

#endif
