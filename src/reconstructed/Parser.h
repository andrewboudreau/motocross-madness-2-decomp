#pragma once

#include <windows.h>

// Parser.cpp (D:\aardvark\VC\krusty2\Parser.cpp): reads `<Tag>value</Tag>`
// pairs from a text buffer into a linked list and looks them up by tag.
// Callers (0x0049c35d..0x0049c72f) parse a buffer and query keys such as
// "EventTypeIndex". No vtables; names are provisional (tier 3).

// One tag/value pair (0x404 bytes: operator new(0x404) at 0x004b8537).
class UnknownParserNode {
public:
    UnknownParserNode(const char* key, const char* value); // 0x004b8470
    ~UnknownParserNode();                                  // 0x004b84c0
    BOOL UnknownFunction4b84d0(const char* key);           // 0x004b84d0: case-insensitive key match
    void UnknownFunction4b84f0();                          // 0x004b84f0: clears the node

    char field_0x000[0x200];          // tag
    char field_0x200[0x200];          // value
    UnknownParserNode* field_0x400;   // next
};

// The list head; a parser holds it at +4.
class UnknownParserList {
public:
    ~UnknownParserList();                                                 // 0x004b86e0
    HRESULT UnknownFunction4b8510(const char* key, const char* value);    // appends a pair
    UnknownParserNode* UnknownFunction4b85f0(const char* key);            // finds a tag
    HRESULT UnknownFunction4b8620(const char* key, unsigned long* out);   // decimal value
    HRESULT UnknownFunction4b8670(const char* key, char* out, unsigned int size); // text value

    UnknownParserNode* field_0x00;    // first node
};

class UnknownParser {
public:
    char* UnknownFunction4b8720(char* text, char* end);       // skips blanks and ';' comment lines
    char* UnknownFunction4b87b0(char* text, char c, char* end); // finds `c`
    HRESULT UnknownFunction4b87f0(char* text, int length);    // parses the buffer
    HRESULT UnknownFunction4b8990(const char* key, char* out, unsigned int size);
    HRESULT UnknownFunction4b89d0(const char* key, unsigned long* out);

    int field_0x00;                   // pairs parsed
    UnknownParserList field_0x04;
};
