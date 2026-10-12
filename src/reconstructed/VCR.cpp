#include "VCR.h"

#include "DebugAlloc.h"

// recorder.cpp passes the slot payload size; each slot gets four extra bytes
// for the size header written by 0x00524220.
UnknownVcr::UnknownVcr(unsigned int size) {
    for (int i = 0; i < 256; i++) {
        void* data = DebugMalloc(size + 4, __FILE__, 29);
        if (data) {
            slots[i].size = size;
            slots[i].data = data;
            slots[i].filePosition = 0;
            slots[i].inUse = 0;
            slots[i].state = 0;
        }
    }
    writeCursor = -1;
    playCursor = -1;
    fillCursor = -1;
    positionBlocks = (UnknownVcrBlock*)DebugMalloc(sizeof(UnknownVcrBlock), __FILE__, 44);
    positionBlocks->next = 0;
    positionIndex = 0;
    positionBlock = 0;
}

// Frees positionBlock blocks of the chain, as 0x00524640 does.
UnknownVcr::~UnknownVcr() {
    for (int i = 0; i < 256; i++) {
        if (slots[i].data) DebugFree(slots[i].data, __FILE__, 54);
    }
    if (positionBlocks) {
        UnknownVcrBlock* block = positionBlocks;
        int count = positionBlock;
        while (count--) {
            UnknownVcrBlock* old = block;
            block = block->next;
            DebugFree(old, __FILE__, 62);
        }
    }
}

// Claims the slot after the write cursor for a `size`-byte payload behind a
// size header, unless that slot is still in state 1.
int UnknownVcr::ClaimRecordSlot(unsigned int size, int* index, void** data) {
    if (slots[NextSlot(fillCursor)].state == 1) return 0;
    AdvanceFillCursor();
    unsigned int needed = size + 4;
    void* buffer = slots[fillCursor].data;
    if (needed > slots[fillCursor].size || !slots[fillCursor].size) {
        if (buffer) DebugFree(buffer, __FILE__, 82);
        buffer = DebugMalloc(needed, __FILE__, 83);
        if (!buffer) return 0;
        slots[fillCursor].size = size;
        slots[fillCursor].data = buffer;
    }
    *(unsigned int*)buffer = size;
    *data = (char*)buffer + 4;
    *index = fillCursor;
    slots[fillCursor].inUse = 1;
    return 1;
}

void UnknownVcr::CommitRecordSlot(int index) {
    slots[index].inUse = 0;
    slots[index].state = 1;
}

// The size written back includes the four-byte header.
void UnknownVcr::TakeSlotToWrite(void** data, int* size, int* index) {
    AdvanceWriteCursor();
    slots[writeCursor].inUse = 1;
    *data = slots[writeCursor].data;
    *size = *(int*)slots[writeCursor].data + 4;
    *index = writeCursor;
}

int UnknownVcr::HasSlotToWrite() {
    if (writeCursor != fillCursor) return slots[NextSlot(writeCursor)].state == 1;
    return 0;
}

int UnknownVcr::ClaimReadSlot(unsigned int size, int* index, void** data) {
    AdvanceFillCursor();
    if (size > slots[fillCursor].size || !slots[fillCursor].size) {
        if (slots[fillCursor].data) DebugFree(slots[fillCursor].data, __FILE__, 149);
        slots[fillCursor].data = DebugMalloc(size, __FILE__, 150);
        if (!slots[fillCursor].data) return 0;
        slots[fillCursor].size = size;
    }
    *data = slots[fillCursor].data;
    *index = fillCursor;
    slots[fillCursor].inUse = 1;
    return 1;
}

void UnknownVcr::CommitReadSlot(int position, int index) {
    slots[index].inUse = 0;
    slots[index].filePosition = position;
    slots[index].state = 1;
}

int UnknownVcr::TakePlaybackSlot(void** data, unsigned int* size, int* position, int* index) {
    if (playCursor >= 0) {
        if (slots[playCursor].state == 2) return 1;
        if (slots[playCursor].state == 3) return 2;
    }
    char state = slots[NextSlot(playCursor)].state;
    if (state != 1 && state != 2 && state != 3) return 4;
    AdvancePlayCursor();
    if (slots[playCursor].state == 2) return 1;
    if (slots[playCursor].state == 3) return 2;
    slots[playCursor].inUse = 1;
    *data = slots[playCursor].data;
    *size = slots[playCursor].size;
    *position = slots[playCursor].filePosition;
    *index = playCursor;
    return 0;
}

void UnknownVcr::ReleaseSlot(int index) {
    slots[index].state = slots[index].inUse = 0;
}

int UnknownVcr::HasRoomToRead() {
    return slots[NextSlot(NextSlot(fillCursor))].state != 1;
}

void UnknownVcr::AppendPosition(int position, int timeMs) {
    UnknownVcrBlock* block = positionBlocks;
    int count = positionBlock;
    if (!block) return;
    while (count--) {
        block = block->next;
        if (!block) return;
    }
    if (positionIndex > 255) {
        block->next = (UnknownVcrBlock*)DebugMalloc(sizeof(UnknownVcrBlock), __FILE__, 237);
        if (!block->next) return;
        block = block->next;
        block->next = 0;
        positionIndex = 0;
        positionBlock++;
    }
    block->pairs[positionIndex][0] = position;
    block->pairs[positionIndex][1] = timeMs;
    positionIndex++;
}

void UnknownVcr::ClearPositions() {
    UnknownVcrBlock* block = positionBlocks;
    if (block) {
        int count = positionBlock;
        while (count--) {
            UnknownVcrBlock* old = block;
            block = block->next;
            DebugFree(old, __FILE__, 258);
        }
    }
    positionBlocks = (UnknownVcrBlock*)DebugMalloc(sizeof(UnknownVcrBlock), __FILE__, 261);
    positionBlocks->next = 0;
    positionIndex = 0;
    positionBlock = 0;
}

// Steps the pair cursor back one pair and returns its position (the time
// through `timeMs`). The null-chain case is the else branch so that its store shares
// the final return.
int UnknownVcr::StepPositionBack(int* timeMs) {
    UnknownVcrBlock* block = positionBlocks;
    UnknownVcrBlock* previous;
    int count = positionBlock;
    if (block) {
        if (count || positionIndex) {
            while (count--) {
                previous = block;
                block = block->next;
                if (!block) {
                    *timeMs = 0;
                    return 0;
                }
            }
            if (positionIndex > 0) {
                *timeMs = block->pairs[positionIndex - 1][1];
                positionIndex--;
                return block->pairs[positionIndex][0];
            }
            if (positionBlock) {
                positionIndex = 256;
                positionBlock--;
                *timeMs = previous->pairs[255][1];
                positionIndex--;
                return previous->pairs[positionIndex][0];
            }
            *timeMs = positionBlocks->pairs[positionIndex][1];
            return positionBlocks->pairs[positionIndex][0];
        }
    } else {
        *timeMs = 0;
    }
    return 0;
}

// Returns the pair at the cursor, skipping one -1 marker. The tail is written
// out in each branch; retail keeps three copies (one with index 0 folded).
int UnknownVcr::PeekPosition(int* timeMs) {
    UnknownVcrBlock* block = positionBlocks;
    int count = positionBlock;
    if (block) {
        while (count--) {
            block = block->next;
            if (!block) {
                *timeMs = 0;
                return 0;
            }
        }
        if (block->pairs[positionIndex][0] == -1) {
            positionIndex++;
            if (positionIndex > 255) {
                block = block->next;
                positionIndex = 0;
                if (!block) return 0;
                *timeMs = block->pairs[positionIndex][1];
                return block->pairs[positionIndex][0];
            }
            *timeMs = block->pairs[positionIndex][1];
            return block->pairs[positionIndex][0];
        }
        *timeMs = block->pairs[positionIndex][1];
        return block->pairs[positionIndex][0];
    } else {
        *timeMs = 0;
    }
    return 0;
}

// Advances the pair cursor until a pair's position equals `position`.
void UnknownVcr::SeekPosition(int position) {
    UnknownVcrBlock* block = positionBlocks;
    int count = positionBlock;
    if (!block) return;
    while (count--) {
        block = block->next;
        if (!block) return;
    }
    do {
        positionIndex++;
        if (positionIndex > 255) {
            block = block->next;
            positionIndex = 0;
            positionBlock++;
            if (!block) return;
        }
    } while (block->pairs[positionIndex][0] != position);
}

void UnknownVcr::SetFirstPosition(int position) {
    firstPosition = position;
}

int UnknownVcr::GetFirstPosition() {
    return firstPosition;
}

void UnknownVcr::ResetSlots() {
    for (int i = 0; i < 256; i++) {
        slots[i].inUse = 0;
        slots[i].state = 0;
    }
    writeCursor = -1;
    playCursor = -1;
    fillCursor = -1;
}

void UnknownVcr::MarkForwardEnd() {
    AdvanceFillCursor();
    if (slots[fillCursor].state != 2) slots[fillCursor].state = 2;
}

void UnknownVcr::MarkReverseEnd() {
    AdvanceFillCursor();
    if (slots[fillCursor].state != 3) slots[fillCursor].state = 3;
}

int UnknownVcr::AdvanceWriteCursor() {
    writeCursor++;
    if (writeCursor == 256) writeCursor = 0;
    return 1;
}

int UnknownVcr::AdvancePlayCursor() {
    playCursor++;
    if (playCursor == 256) playCursor = 0;
    return 1;
}

int UnknownVcr::AdvanceFillCursor() {
    fillCursor++;
    if (fillCursor == 256) fillCursor = 0;
    return 1;
}

// The ring successor of `index`.
int UnknownVcr::NextSlot(int index) {
    index++;
    if (index == 256) index = 0;
    return index;
}
