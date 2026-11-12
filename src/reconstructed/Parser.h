#pragma once

#include <windows.h>

// Parser.cpp (D:\aardvark\VC\krusty2\Parser.cpp): reads `<Tag>value</Tag>`
// pairs from a text buffer into a linked list and looks them up by tag.
// Callers (0x0049c35d..0x0049c72f) parse a buffer and query keys such as
// "EventTypeIndex". No vtables; the class names follow the unit (tier 2), member names are provisional.

// One tag/value pair (0x404 bytes: operator new(0x404) at 0x004b8537).
class ParserNode {
public:
    ParserNode(const char* key, const char* value); // 0x004b8470
    ~ParserNode();                                  // 0x004b84c0
    BOOL MatchesKey(const char* key);                      // 0x004b84d0: case-insensitive tag match
    void Clear();                                          // 0x004b84f0: clears the node

    char tagText[0x200];              // +0x000
    char valueText[0x200];            // +0x200
    ParserNode* next;          // +0x400
};

// The list head; a parser holds it at +4.
class ParserList {
public:
    ParserList();                                                  // 0x004aae20 (a shared body that stores one zero dword)
    ~ParserList();                                                 // 0x004b86e0
    HRESULT Append(const char* key, const char* value);                   // 0x004b8510: appends a pair
    ParserNode* Find(const char* key);                             // 0x004b85f0: finds a tag
    HRESULT GetNumber(const char* key, unsigned long* out);               // 0x004b8620: decimal value
    HRESULT GetText(const char* key, char* out, unsigned int size);       // 0x004b8670: text value

    ParserNode* head;          // +0x00: first node
};

class Parser {
public:
    char* SkipBlanks(char* text, char* end);                  // 0x004b8720: skips blanks and ';' comment lines
    char* FindChar(char* text, char c, char* end);            // 0x004b87b0: finds `c`
    HRESULT Parse(char* text, int length);                    // 0x004b87f0: parses the buffer
    HRESULT GetText(const char* key, char* out, unsigned int size); // 0x004b8990
    HRESULT GetNumber(const char* key, unsigned long* out);   // 0x004b89d0

    int pairCount;                    // +0x00: pairs parsed
    ParserList pairs;          // +0x04
};
