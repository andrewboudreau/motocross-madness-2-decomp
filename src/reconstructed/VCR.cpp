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
            slots[i].field_0x08 = 0;
            slots[i].field_0x0c = 0;
            slots[i].field_0x0d = 0;
        }
    }
    field_0x1000 = -1;
    field_0x1004 = -1;
    field_0x1008 = -1;
    field_0x100c = (UnknownVcrBlock*)DebugMalloc(sizeof(UnknownVcrBlock), __FILE__, 44);
    field_0x100c->next = 0;
    field_0x1010 = 0;
    field_0x1014 = 0;
}

// Frees field_0x1014 blocks of the chain, as 0x00524640 does.
UnknownVcr::~UnknownVcr() {
    for (int i = 0; i < 256; i++) {
        if (slots[i].data) operator delete(slots[i].data, __FILE__, 54);
    }
    if (field_0x100c) {
        UnknownVcrBlock* block = field_0x100c;
        int count = field_0x1014;
        while (count--) {
            UnknownVcrBlock* old = block;
            block = block->next;
            operator delete(old, __FILE__, 62);
        }
    }
}

// Claims the slot after the write cursor for a `size`-byte payload behind a
// size header, unless that slot is still in state 1.
int UnknownVcr::UnknownFunction524220(unsigned int size, int* index, void** data) {
    if (slots[UnknownFunction524a30(field_0x1008)].field_0x0d == 1) return 0;
    UnknownFunction524a00();
    unsigned int needed = size + 4;
    void* buffer = slots[field_0x1008].data;
    if (needed > slots[field_0x1008].size || !slots[field_0x1008].size) {
        if (buffer) operator delete(buffer, __FILE__, 82);
        buffer = DebugMalloc(needed, __FILE__, 83);
        if (!buffer) return 0;
        slots[field_0x1008].size = size;
        slots[field_0x1008].data = buffer;
    }
    *(unsigned int*)buffer = size;
    *data = (char*)buffer + 4;
    *index = field_0x1008;
    slots[field_0x1008].field_0x0c = 1;
    return 1;
}

void UnknownVcr::UnknownFunction5242e0(int index) {
    slots[index].field_0x0c = 0;
    slots[index].field_0x0d = 1;
}

// The size written back includes the four-byte header.
void UnknownVcr::UnknownFunction524300(void** data, int* size, int* index) {
    UnknownFunction5249a0();
    slots[field_0x1000].field_0x0c = 1;
    *data = slots[field_0x1000].data;
    *size = *(int*)slots[field_0x1000].data + 4;
    *index = field_0x1000;
}

int UnknownVcr::UnknownFunction524350() {
    if (field_0x1000 != field_0x1008) return slots[UnknownFunction524a30(field_0x1000)].field_0x0d == 1;
    return 0;
}

int UnknownVcr::UnknownFunction524390(unsigned int size, int* index, void** data) {
    UnknownFunction524a00();
    if (size > slots[field_0x1008].size || !slots[field_0x1008].size) {
        if (slots[field_0x1008].data) operator delete(slots[field_0x1008].data, __FILE__, 149);
        slots[field_0x1008].data = DebugMalloc(size, __FILE__, 150);
        if (!slots[field_0x1008].data) return 0;
        slots[field_0x1008].size = size;
    }
    *data = slots[field_0x1008].data;
    *index = field_0x1008;
    slots[field_0x1008].field_0x0c = 1;
    return 1;
}

void UnknownVcr::UnknownFunction524440(int value, int index) {
    slots[index].field_0x0c = 0;
    slots[index].field_0x08 = value;
    slots[index].field_0x0d = 1;
}

int UnknownVcr::UnknownFunction524460(void** data, unsigned int* size, int* value, int* index) {
    if (field_0x1004 >= 0) {
        if (slots[field_0x1004].field_0x0d == 2) return 1;
        if (slots[field_0x1004].field_0x0d == 3) return 2;
    }
    char state = slots[UnknownFunction524a30(field_0x1004)].field_0x0d;
    if (state != 1 && state != 2 && state != 3) return 4;
    UnknownFunction5249d0();
    if (slots[field_0x1004].field_0x0d == 2) return 1;
    if (slots[field_0x1004].field_0x0d == 3) return 2;
    slots[field_0x1004].field_0x0c = 1;
    *data = slots[field_0x1004].data;
    *size = slots[field_0x1004].size;
    *value = slots[field_0x1004].field_0x08;
    *index = field_0x1004;
    return 0;
}

void UnknownVcr::UnknownFunction524540(int index) {
    slots[index].field_0x0d = slots[index].field_0x0c = 0;
}

int UnknownVcr::UnknownFunction524560() {
    return slots[UnknownFunction524a30(UnknownFunction524a30(field_0x1008))].field_0x0d != 1;
}

void UnknownVcr::UnknownFunction524590(int value, int extra) {
    UnknownVcrBlock* block = field_0x100c;
    int count = field_0x1014;
    if (!block) return;
    while (count--) {
        block = block->next;
        if (!block) return;
    }
    if (field_0x1010 > 255) {
        block->next = (UnknownVcrBlock*)DebugMalloc(sizeof(UnknownVcrBlock), __FILE__, 237);
        if (!block->next) return;
        block = block->next;
        block->next = 0;
        field_0x1010 = 0;
        field_0x1014++;
    }
    block->pairs[field_0x1010][0] = value;
    block->pairs[field_0x1010][1] = extra;
    field_0x1010++;
}

void UnknownVcr::UnknownFunction524640() {
    UnknownVcrBlock* block = field_0x100c;
    if (block) {
        int count = field_0x1014;
        while (count--) {
            UnknownVcrBlock* old = block;
            block = block->next;
            operator delete(old, __FILE__, 258);
        }
    }
    field_0x100c = (UnknownVcrBlock*)DebugMalloc(sizeof(UnknownVcrBlock), __FILE__, 261);
    field_0x100c->next = 0;
    field_0x1010 = 0;
    field_0x1014 = 0;
}

// Steps the pair cursor back one pair and returns its value (extra through
// `extra`). The null-chain case is the else branch so that its store shares
// the final return.
int UnknownVcr::UnknownFunction5246c0(int* extra) {
    UnknownVcrBlock* block = field_0x100c;
    UnknownVcrBlock* previous;
    int count = field_0x1014;
    if (block) {
        if (count || field_0x1010) {
            while (count--) {
                previous = block;
                block = block->next;
                if (!block) {
                    *extra = 0;
                    return 0;
                }
            }
            if (field_0x1010 > 0) {
                *extra = block->pairs[field_0x1010 - 1][1];
                field_0x1010--;
                return block->pairs[field_0x1010][0];
            }
            if (field_0x1014) {
                field_0x1010 = 256;
                field_0x1014--;
                *extra = previous->pairs[255][1];
                field_0x1010--;
                return previous->pairs[field_0x1010][0];
            }
            *extra = field_0x100c->pairs[field_0x1010][1];
            return field_0x100c->pairs[field_0x1010][0];
        }
    } else {
        *extra = 0;
    }
    return 0;
}

// Returns the pair at the cursor, skipping one -1 marker. The tail is written
// out in each branch; retail keeps three copies (one with index 0 folded).
int UnknownVcr::UnknownFunction5247c0(int* extra) {
    UnknownVcrBlock* block = field_0x100c;
    int count = field_0x1014;
    if (block) {
        while (count--) {
            block = block->next;
            if (!block) {
                *extra = 0;
                return 0;
            }
        }
        if (block->pairs[field_0x1010][0] == -1) {
            field_0x1010++;
            if (field_0x1010 > 255) {
                block = block->next;
                field_0x1010 = 0;
                if (!block) return 0;
                *extra = block->pairs[field_0x1010][1];
                return block->pairs[field_0x1010][0];
            }
            *extra = block->pairs[field_0x1010][1];
            return block->pairs[field_0x1010][0];
        }
        *extra = block->pairs[field_0x1010][1];
        return block->pairs[field_0x1010][0];
    } else {
        *extra = 0;
    }
    return 0;
}

// Advances the pair cursor until a pair's value equals `value`.
void UnknownVcr::UnknownFunction524870(int value) {
    UnknownVcrBlock* block = field_0x100c;
    int count = field_0x1014;
    if (!block) return;
    while (count--) {
        block = block->next;
        if (!block) return;
    }
    do {
        field_0x1010++;
        if (field_0x1010 > 255) {
            block = block->next;
            field_0x1010 = 0;
            field_0x1014++;
            if (!block) return;
        }
    } while (block->pairs[field_0x1010][0] != value);
}

void UnknownVcr::UnknownFunction5248f0(int value) {
    field_0x1018 = value;
}

int UnknownVcr::UnknownFunction524900() {
    return field_0x1018;
}

void UnknownVcr::UnknownFunction524910() {
    for (int i = 0; i < 256; i++) {
        slots[i].field_0x0c = 0;
        slots[i].field_0x0d = 0;
    }
    field_0x1000 = -1;
    field_0x1004 = -1;
    field_0x1008 = -1;
}

void UnknownVcr::UnknownFunction524940() {
    UnknownFunction524a00();
    if (slots[field_0x1008].field_0x0d != 2) slots[field_0x1008].field_0x0d = 2;
}

void UnknownVcr::UnknownFunction524970() {
    UnknownFunction524a00();
    if (slots[field_0x1008].field_0x0d != 3) slots[field_0x1008].field_0x0d = 3;
}

int UnknownVcr::UnknownFunction5249a0() {
    field_0x1000++;
    if (field_0x1000 == 256) field_0x1000 = 0;
    return 1;
}

int UnknownVcr::UnknownFunction5249d0() {
    field_0x1004++;
    if (field_0x1004 == 256) field_0x1004 = 0;
    return 1;
}

int UnknownVcr::UnknownFunction524a00() {
    field_0x1008++;
    if (field_0x1008 == 256) field_0x1008 = 0;
    return 1;
}

// The ring successor of `index`.
int UnknownVcr::UnknownFunction524a30(int index) {
    index++;
    if (index == 256) index = 0;
    return index;
}
