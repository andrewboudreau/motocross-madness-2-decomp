#pragma once

class UnknownEcoQuadTreeObject {
public:
    virtual unsigned short ComputeSortKey();
    virtual unsigned int ComputeQuadTreeCode(class EcoQuadTreeView* tree);
    UnknownEcoQuadTreeObject() { queryStamp = 0; typeId = (char)0xff; }
    short queryStamp;                         // +0x04: query stamp (ctor 0)
    short sortKey;                            // +0x06: sort key, slot 0's result
    char typeId;                              // +0x08: object type id (ctor -1)
};
