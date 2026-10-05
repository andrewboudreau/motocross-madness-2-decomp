#pragma once

#include <string.h>

#include "DebugAlloc.h"

// Growable array template from ContainerList.h (literal __FILE__ in retail,
// e.g. at 0x005109fd). Member names are provisional; the layout, line numbers
// and bodies are decoded from retail:
//   0x005109c0  default constructor (shared by every instantiation)
//   0x00402040  destructor
//   0x005109e0  Reserve (line 71)
//   Init is inlined into its callers (line 59) and returns whether it
//   allocated (PCAudio.cpp 0x004bdef0 returns that value); Get, Add (with
//   Reserve) and Remove are inlined into JoystickDevice's binding functions.
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

    // Whether the storage was allocated.
    int Init(int capacity, int growBy) {
        m_data = new(__FILE__, 59) T[capacity];
        m_growBy = growBy;
        m_capacity = capacity;
        m_allocated = 1;
        return m_data != 0;
    }

    int Reserve(int capacity) {
        if (capacity <= m_capacity)
            return 0;
        T* data = new(__FILE__, 71) T[capacity];
        if (!data)
            return 0;
        memcpy(data, m_data, m_capacity * sizeof(T));
        T* old = m_data;
        m_data = data;
        delete old;
        m_capacity = capacity;
        return 1;
    }

    // Zeroes the elements and empties the list.
    void Clear() {
        memset(m_data, 0, m_count * sizeof(T));
        m_count = 0;
    }

    // Whether `item` is in the list.
    int Contains(T item) {
        for (int i = 0; i < m_count; i++) {
            if (m_data[i] == item)
                return 1;
        }
        return 0;
    }

    // Element `index`, or 0 when out of range.
    T Get(int index) {
        if (m_count > 0 && m_data && index < m_count)
            return m_data[index];
        return 0;
    }

    // Appends, growing by m_growBy when full.
    int Add(T item) {
        if (m_count >= m_capacity && !Reserve(m_growBy + m_count))
            return 0;
        m_data[m_count] = item;
        m_count++;
        return 1;
    }

    // Removes the first `item`, moving the last element into its place.
    int Remove(T item) {
        if (m_count > 0) {
            for (int i = 0; i < m_count; i++) {
                if (m_data[i] == item) {
                    m_data[i] = m_data[m_count - 1];
                    m_count--;
                    return 1;
                }
            }
        }
        return 0;
    }

    // Removes the first `item`, keeping the order of the rest.
    void RemoveOrdered(T item) {
        if (m_count > 0) {
            for (int i = 0; i < m_count; i++) {
                if (m_data[i] == item) {
                    memmove(&m_data[i], &m_data[i + 1], (m_count - (i + 1)) * sizeof(T));
                    m_count--;
                    return;
                }
            }
        }
    }

    // Inserts `item` before element `index`, growing when full.
    int Insert(T item, int index) {
        if (m_count >= m_capacity && !Reserve(m_growBy + m_count))
            return 0;
        if (index != m_count)
            memmove(&m_data[index + 1], &m_data[index], (m_count - index) * sizeof(T));
        m_data[index] = item;
        m_count++;
        return 1;
    }

    int m_count;
    T* m_data;
    int m_growBy;
    int m_capacity;
    unsigned char m_allocated : 1;
};
