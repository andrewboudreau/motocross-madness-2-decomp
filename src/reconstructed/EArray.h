#pragma once

#include "DebugAlloc.h"

// Array template from EArray.h (literal __FILE__ "D:\aardvark\VC\krusty2\
// EArray.h" at 0x0056b294; xrefs 0x004671ce in FontTexture.cpp and
// 0x0050ac82 in Texmap.cpp's font code). Both sites inline the same
// constructor: a 4-byte-element block from line 53, a base index of 0 and
// the element count. Elements are addressed relative to the base index.
// The class name follows the file name; member names are provisional.
template <class T>
class EArray {
public:
    EArray(unsigned int count) {
        m_data = new(__FILE__, 53) T[count];
        m_base = 0;
        m_count = count;
    }

    ~EArray() {
        delete m_data;
        m_data = 0;
    }

    T& operator[](unsigned int index) {
        return m_data[index - m_base];
    }

    unsigned int Count() {
        return m_count;
    }

    T* m_data;
    unsigned int m_base;
    unsigned int m_count;
};
