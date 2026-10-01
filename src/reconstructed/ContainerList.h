#pragma once

#include <string.h>

#include "DebugAlloc.h"

// Growable array template from ContainerList.h (literal __FILE__ in retail,
// e.g. at 0x005109fd). Member names are provisional; the layout, line numbers
// and bodies are decoded from retail:
//   0x005109c0  default constructor (shared by every instantiation)
//   0x00402040  destructor
//   0x005109e0  Reserve (line 71)
//   Init is inlined into its callers (line 59).
template <class T>
class ContainerList {
public:
    ContainerList() {
        m_data = 0;
        m_count = 0;
        m_growBy = 0;
        m_capacity = 0;
        m_allocated = 0;
    }

    ~ContainerList() {
        delete m_data;
    }

    void Init(int capacity, int growBy) {
        m_data = new(__FILE__, 59) T[capacity];
        m_growBy = growBy;
        m_capacity = capacity;
        m_allocated = 1;
    }

    int Reserve(int capacity) {
        if (capacity <= m_capacity)
            return 0;
        T* data = new(__FILE__, 71) T[capacity];
        if (!data)
            return 0;
        memcpy(data, m_data, m_capacity * sizeof(T));
        delete m_data;
        m_data = data;
        m_capacity = capacity;
        return 1;
    }

    int m_count;
    T* m_data;
    int m_growBy;
    int m_capacity;
    unsigned char m_allocated : 1;
};
