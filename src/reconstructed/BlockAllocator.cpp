#include "BlockAllocator.h"

#include "DebugAlloc.h"

BlockAllocator::BlockAllocator(unsigned int elementSize, unsigned int blockSize) {
    elementsPerBlock = (blockSize - 4) / elementSize;
    firstBlock = 0;
    lastBlock = 0;
    this->elementSize = elementSize;
    this->blockSize = blockSize;
    freeElements = 0;
    reservedBytes = 0;
    spareBlocks = 0;
    nextElementIndex = elementsPerBlock;
}

void* BlockAllocator::Alloc() {
    void* result;

    // Prefer untouched space in the current block, then an individually freed
    // element, then a whole block retained by Reset. Allocate a block only when
    // all three sources are exhausted.
    if (nextElementIndex < elementsPerBlock) {
        result = (char*)(lastBlock + 1) + elementSize * nextElementIndex;
        nextElementIndex++;
    } else if (freeElements) {
        result = freeElements;
        freeElements = freeElements->next;
    } else if (spareBlocks) {
        if (!firstBlock) {
            lastBlock = spareBlocks;
            firstBlock = spareBlocks;
        } else {
            lastBlock->next = spareBlocks;
            lastBlock = spareBlocks;
        }
        spareBlocks = lastBlock->next;
        lastBlock->next = 0;
        nextElementIndex = 1;
        result = lastBlock + 1;
    } else {
        if (!AllocateBlock()) return 0;
        nextElementIndex = 1;
        result = lastBlock + 1;
    }
    if (result) allocatedBytes += elementSize;
    return result;
}

void BlockAllocator::Free(void* p) {
    if (elementSize > 4) {
        FreeElement* element = static_cast<FreeElement*>(p);
        element->next = freeElements;
        freeElements = element;
        allocatedBytes -= elementSize;
    }
}

int BlockAllocator::AllocateBlock() {
    Block* block = static_cast<Block*>(DebugMalloc(blockSize, __FILE__, 132));
    if (!block) return 0;
    if (!firstBlock) {
        lastBlock = block;
        firstBlock = block;
    } else {
        lastBlock->next = block;
        lastBlock = block;
    }
    block->next = 0;
    reservedBytes += blockSize;
    return 1;
}

BlockAllocator::~BlockAllocator() {
    Clear();
}

void BlockAllocator::Clear() {
    Block* block = firstBlock;
    while (block) {
        Block* old = block;
        block = block->next;
        DebugFree(old, __FILE__, 156);
    }
    firstBlock = 0;
    lastBlock = 0;
    nextElementIndex = elementsPerBlock;
    freeElements = 0;
    reservedBytes = 0;
    allocatedBytes = 0;
}

void BlockAllocator::Reset() {
    if (spareBlocks && lastBlock) lastBlock->next = spareBlocks;
    spareBlocks = firstBlock;
    firstBlock = 0;
    lastBlock = 0;
    allocatedBytes = 0;
    freeElements = 0;
    nextElementIndex = elementsPerBlock;
}
