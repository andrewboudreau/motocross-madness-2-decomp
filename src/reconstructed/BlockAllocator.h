#pragma once

// Nonpolymorphic 40-byte fixed-size pool attributed by the retail
// BlockAllocator.cpp literal. The descriptive member names are inferred from
// repeated accesses; they are not recovered original identifiers.
class BlockAllocator {
public:
    BlockAllocator(unsigned int elementSize, unsigned int blockSize);
    ~BlockAllocator();
    void* Alloc();
    void Free(void* p);
    void Reset();
private:
    struct Block {
        Block* next;
    };

    struct FreeElement {
        FreeElement* next;
    };

    int AllocateBlock();
    void Clear();
    int elementsPerBlock;             // +0x00
    Block* firstBlock;                // +0x04, active block-chain head
    Block* lastBlock;                 // +0x08, active block-chain tail
    Block* spareBlocks;               // +0x0c, blocks retained by Reset
    int nextElementIndex;             // +0x10, sequential index in lastBlock
    unsigned int elementSize;         // +0x14
    unsigned int blockSize;           // +0x18, including the next-block link
    FreeElement* freeElements;        // +0x1c, intrusive free-list head
    unsigned int reservedBytes;       // +0x20, blockSize charged per new block
    unsigned int allocatedBytes;      // +0x24, elementSize charged per Alloc
};
