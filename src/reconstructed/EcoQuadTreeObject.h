#pragma once

class UnknownEcoQuadTreeObject {
public:
    virtual unsigned short UnknownVirtualSlot0();
    virtual unsigned int UnknownVirtualSlot1(class UnknownEcoQuadTree* tree);
    UnknownEcoQuadTreeObject() { queryStamp = 0; typeId = (char)0xff; }
    short queryStamp;                         // +0x04: query stamp (ctor 0)
    short sortKey;                            // +0x06: sort key, slot 0's result
    char typeId;                              // +0x08: object type id (ctor -1)
};
