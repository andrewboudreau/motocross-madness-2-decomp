#pragma once

#include <string.h>

#include "DebugAlloc.h"

// Growable array template from ContainerList.h (literal __FILE__ in retail,
// e.g. at 0x005109fd). Member names are provisional; the layout, line numbers
// and bodies are decoded from retail:
//   0x005109c0  default constructor (shared by every instantiation)
//   0x00402040  destructor
//   0x005109e0  Reserve (line 71)
//   Init is inlined into its callers (line 59); Get, Add (with Reserve) and
//   Remove are inlined into JoystickDevice's binding functions.
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

    int m_count;
    T* m_data;
    int m_growBy;
    int m_capacity;
    unsigned char m_allocated : 1;
};
