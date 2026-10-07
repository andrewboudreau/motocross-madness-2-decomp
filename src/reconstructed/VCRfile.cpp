#include "VCRfile.h"

#include <string.h>

#include "DebugAlloc.h"

// bikerace.cpp passes (1, 2): two in-memory handles.
UnknownVcrFile::UnknownVcrFile(int memory, int count) {
    this->memory = memory;
    if (count > 2)
        this->count = 2;
    else
        this->count = count;
    if (memory) {
        for (int i = 0; i < this->count; i++) {
            entries[i].field_0x00 = 0;
            entries[i].name[0] = 0;
            entries[i].mode[0] = 0;
            entries[i].position = 0;
            entries[i].size = 0;
            blocks[i] = (UnknownVcrFileBlock*)DebugMalloc(sizeof(UnknownVcrFileBlock), __FILE__, 33);
            blocks[i]->next = 0;
        }
    } else {
        for (int i = 0; i < this->count; i++) files[i] = 0;
    }
}

UnknownVcrFile::~UnknownVcrFile() {
    if (memory) {
        for (int i = 0; i < count; i++) {
            UnknownVcrFileBlock* next = blocks[i]->next;
            DebugFree(blocks[i], __FILE__, 50);
            while (next) {
                UnknownVcrFileBlock* block = next;
                next = next->next;
                DebugFree(block, __FILE__, 54);
            }
        }
    }
}

// Loads `path` into the first unused in-memory handle, naming it `name`
// with mode "rb". The read loop ends inside the found path; a loop with the
// tail after it compiles to a second epilogue (see docs/VCR.md).
void UnknownVcrFile::UnknownFunction524b80(char* path, char* name) {
    if (!memory) return;
    for (int i = 0; i < count; i++) {
        if (!entries[i].field_0x00 && !entries[i].size) {
            int n = strlen(name);
            int length = n > 0x103 ? 0x103 : n;
            strncpy(entries[i].name, name, length);
            entries[i].name[length] = 0;
            n = strlen("rb");
            length = n > 7 ? 7 : n;
            strncpy(entries[i].mode, "rb", length);
            entries[i].mode[length] = 0;
            entries[i].position = 0;
            FILE* file = fopen(path, "rb");
            if (file) {
                UnknownVcrFileBlock* block = blocks[i];
                while (1) {
                    if (!fread(block, 0x19000, 1, file)) {
                        entries[i].size = ftell(file);
                        fclose(file);
                        return;
                    }
                    block->next = (UnknownVcrFileBlock*)DebugMalloc(sizeof(UnknownVcrFileBlock), __FILE__, 81);
                    block->next->next = 0;
                    block = block->next;
                }
            }
        }
    }
}

// Writes the first in-memory handle whose mode starts with 'r' to `path`.
void UnknownVcrFile::UnknownFunction524d00(char* path) {
    if (memory) {
        for (int i = 0; i < count; i++) {
            FILE* file;
            if (entries[i].mode[0] == 'r' && (file = fopen(path, "wb")) != 0) {
                int size = entries[i].size;
                UnknownVcrFileBlock* block = blocks[i];
                while (size >= 0x19000) {
                    fwrite(block, 0x19000, 1, file);
                    size -= 0x19000;
                    if (!block->next) break;
                    block = block->next;
                }
                if (size) fwrite(block, size, 1, file);
                fclose(file);
                return;
            }
        }
    }
}

// fopen-like: returns a handle index or -1. In memory mode an existing name
// is reopened with the new mode ("w..." truncates it); otherwise the first
// unused entry is claimed. Nothing in this file sets field_0x00.
int UnknownVcrFile::UnknownFunction524dd0(char* name, char* mode) {
    int i;
    if (memory) {
        for (i = 0; i < count; i++) {
            if (!strcmp(name, entries[i].name)) {
                int n = strlen(mode);
                int length = n > 7 ? 7 : n;
                strncpy(entries[i].mode, mode, length);
                entries[i].mode[length] = 0;
                entries[i].position = 0;
                if (entries[i].mode[0] == 'w') entries[i].size = 0;
                return i;
            }
        }
        for (i = 0; i < count; i++) {
            if (!entries[i].field_0x00 && !entries[i].size) {
                int n = strlen(name);
                int length = n > 0x103 ? 0x103 : n;
                strncpy(entries[i].name, name, length);
                entries[i].name[length] = 0;
                n = strlen(mode);
                length = n > 7 ? 7 : n;
                strncpy(entries[i].mode, mode, length);
                entries[i].mode[length] = 0;
                entries[i].position = 0;
                return i;
            }
        }
    } else {
        for (i = 0; i < count; i++) {
            if (!files[i]) {
                files[i] = fopen(name, mode);
                if (!files[i]) {
                    files[i] = 0;
                    return -1;
                }
                return i;
            }
        }
    }
    return -1;
}

// fclose-like.
void UnknownVcrFile::UnknownFunction525000(int handle) {
    if (memory) {
        if (handle >= 0 && entries[handle].field_0x00) {
            entries[handle].field_0x00 = 0;
            entries[handle].position = 0;
        }
    } else if (handle >= 0 && files[handle]) {
        fclose(files[handle]);
        files[handle] = 0;
    }
}

// fread-like: returns the bytes copied (size * count) in memory mode, or 0
// at or past the end. VC6 turns the block-skipping loop into a division by
// 0x19000 (the 0x51eb851f multiply in retail).
int UnknownVcrFile::UnknownFunction525070(void* buffer, int size, int count, int handle) {
    if (memory) {
        for (int i = 0; i < this->count; i++) {
            if (i == handle) {
                int position = entries[i].position;
                if (position >= entries[i].size) return 0;
                UnknownVcrFileBlock* block = blocks[i];
                while (position >= 0x19000) {
                    position -= 0x19000;
                    block = block->next;
                }
                int total = size * count;
                unsigned int remaining = total;
                char* out = (char*)buffer;
                while (1) {
                    if (position >= entries[i].size) return 0;
                    if (remaining + position > 0x19000) {
                        memcpy(out, block->data + position, 0x19000 - position);
                        block = block->next;
                        out += 0x19000 - position;
                        remaining = remaining + position - 0x19000;
                        position = 0;
                    } else {
                        memcpy(out, block->data + position, remaining);
                        entries[i].position += total;
                        return total;
                    }
                }
            }
        }
    } else if (handle >= 0 && files[handle]) {
        return fread(buffer, size, count, files[handle]);
    }
    return 0;
}

// fwrite-like: grows the block chain as needed and extends the size.
int UnknownVcrFile::UnknownFunction5251f0(const void* buffer, int size, int count, int handle) {
    if (memory) {
        for (int i = 0; i < this->count; i++) {
            if (i == handle) {
                UnknownVcrFileBlock* block = blocks[i];
                int position = entries[i].position;
                while (position >= 0x19000) {
                    position -= 0x19000;
                    if (!block->next) {
                        block->next = (UnknownVcrFileBlock*)DebugMalloc(sizeof(UnknownVcrFileBlock), __FILE__, 240);
                        block->next->next = 0;
                    }
                    block = block->next;
                }
                const char* in = (const char*)buffer;
                int total = size * count;
                unsigned int remaining = total;
                while (1) {
                    if (remaining + position > 0x19000) {
                        memcpy(block->data + position, in, 0x19000 - position);
                        in += 0x19000 - position;
                        remaining = remaining + position - 0x19000;
                        position = 0;
                        block->next = (UnknownVcrFileBlock*)DebugMalloc(sizeof(UnknownVcrFileBlock), __FILE__, 264);
                        block->next->next = 0;
                        block = block->next;
                    } else {
                        memcpy(block->data + position, in, remaining);
                        entries[i].position += total;
                        if (entries[i].size < entries[i].position) entries[i].size = entries[i].position;
                        return total;
                    }
                }
            }
        }
        return 0;
    }
    if (handle >= 0 && files[handle]) return fwrite(buffer, size, count, files[handle]);
    return 0;
}

// fseek-like. In memory mode `origin` is ignored and the new position is
// returned (retail leaves `offset` in eax).
int UnknownVcrFile::UnknownFunction5253d0(int handle, int offset, int origin) {
    if (memory) {
        for (int i = 0; i < count; i++) {
            if (i == handle) return entries[i].position = offset;
        }
    } else if (handle >= 0 && files[handle]) {
        return fseek(files[handle], offset, origin);
    }
    return 0;
}

// ftell-like.
int UnknownVcrFile::UnknownFunction525440(int handle) {
    if (memory) {
        for (int i = 0; i < count; i++) {
            if (i == handle) return entries[i].position;
        }
    } else if (handle >= 0 && files[handle]) {
        return ftell(files[handle]);
    }
    return 0;
}

// feof-like; VC6's feof is the `_flag & _IOEOF` macro.
int UnknownVcrFile::UnknownFunction5254a0(int handle) {
    if (memory) {
        for (int i = 0; i < count; i++) {
            if (i == handle) return entries[i].size <= entries[i].position;
        }
    } else if (handle >= 0 && files[handle]) {
        return feof(files[handle]);
    }
    return 0;
}
