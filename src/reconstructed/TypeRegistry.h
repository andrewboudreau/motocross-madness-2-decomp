#pragma once

// Reconstruction of D:\aardvark\VC\krusty2\TypeRegistry.cpp (literal
// __FILE__ at 0x00575748; code 0x00521d00..0x00521f2b). A registry of type
// names; an id is a name's index. CollisionObject's constructor asks it for
// "CollisionObject" and "Vegetation" (src/krusty2/collision). The method
// name follows that krusty2 declaration; the field names are provisional.
class TypeRegistry {
public:
    TypeRegistry();                       // folded body 0x004676a0
    ~TypeRegistry();                      // 0x00521e80
    // 0x00521ea0: the id of `name`, appending it when it is new.
    char FindTypeId(const char* name);

    int count;
    char (*names)[0x80];
};

// 0x0068a490, and the pointer to it at 0x00575744.
extern TypeRegistry g_TypeRegistryInstance;
extern TypeRegistry* g_TypeRegistry;
