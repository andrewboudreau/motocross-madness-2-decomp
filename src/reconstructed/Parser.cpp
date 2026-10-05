// Parser.cpp -- reconstruction of D:\aardvark\VC\krusty2\Parser.cpp
// (0x004b8470..0x004b89f2). See Parser.h and docs/PARSER.md.

#include "Parser.h"

#include <stdlib.h>

#include "DebugAlloc.h"

UnknownParserNode::UnknownParserNode(const char* key, const char* value)
{
    UnknownFunction4b84f0();
    if (key && value) {
        lstrcpy(field_0x000, key);
        lstrcpy(field_0x200, value);
        field_0x400 = 0;
    }
}

UnknownParserNode::~UnknownParserNode()
{
    UnknownFunction4b84f0();
}

BOOL UnknownParserNode::UnknownFunction4b84d0(const char* key)
{
    if (!key)
        return FALSE;
    return lstrcmpi(field_0x000, key) == 0;
}

void UnknownParserNode::UnknownFunction4b84f0()
{
    field_0x000[0] = 0;
    field_0x200[0] = 0;
    field_0x400 = 0;
}

HRESULT UnknownParserList::UnknownFunction4b8510(const char* key, const char* value)
{
    UnknownParserNode* node;

    if (!field_0x00) {
        node = field_0x00 = new (__FILE__, 73) UnknownParserNode(key, value);
    } else {
        UnknownParserNode* last = field_0x00;
        while (last->field_0x400)
            last = last->field_0x400;
        node = last->field_0x400 = new (__FILE__, 86) UnknownParserNode(key, value);
    }
    return node ? S_OK : E_OUTOFMEMORY;
}

UnknownParserNode* UnknownParserList::UnknownFunction4b85f0(const char* key)
{
    for (UnknownParserNode* node = field_0x00; node; node = node->field_0x400) {
        if (node->UnknownFunction4b84d0(key))
            return node;
    }
    return 0;
}

HRESULT UnknownParserList::UnknownFunction4b8620(const char* key, unsigned long* out)
{
    if (!key || !out)
        return E_FAIL;
    UnknownParserNode* node = UnknownFunction4b85f0(key);
    if (!node)
        return E_ACCESSDENIED;
    *out = strtoul(node->field_0x200, 0, 10);
    return S_OK;
}

HRESULT UnknownParserList::UnknownFunction4b8670(const char* key, char* out, unsigned int size)
{
    if (!key || !out || !size)
        return E_FAIL;
    UnknownParserNode* node = UnknownFunction4b85f0(key);
    if (!node)
        return E_ACCESSDENIED;
    if ((unsigned int)lstrlen(node->field_0x200) < size) {
        lstrcpy(out, node->field_0x200);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

UnknownParserList::~UnknownParserList()
{
    while (field_0x00) {
        UnknownParserNode* node = field_0x00;
        field_0x00 = node->field_0x400;
        delete node;
    }
    field_0x00 = 0;
}

char* UnknownParser::UnknownFunction4b8720(char* text, char* end)
{
    char* start = text;

    if (!end)
        end = text + lstrlen(text);
    while (text != end && (*text == ' ' || *text == '\n' || *text == '\t' || *text == '\r'))
        text++;
    if (text == end)
        return 0;
    if (*text == ';' && (text == start || text[-1] == '\n')) {
        while (text != end && *text != '\n')
            text++;
        if (text == end)
            return 0;
        text = UnknownFunction4b8720(text, end);
    }
    return text;
}

char* UnknownParser::UnknownFunction4b87b0(char* text, char c, char* end)
{
    if (!end)
        end = text + lstrlen(text);
    while (text != end && *text != c)
        text++;
    if (text == end)
        return 0;
    return text;
}

// 0x004b87f0 (parses the buffer) is a near miss; see
// samples/parser/ParserNearMisses.cpp.

HRESULT UnknownParser::UnknownFunction4b8990(const char* key, char* out, unsigned int size)
{
    if (!key || !out || !size)
        return E_FAIL;
    return field_0x04.UnknownFunction4b8670(key, out, size);
}

HRESULT UnknownParser::UnknownFunction4b89d0(const char* key, unsigned long* out)
{
    if (!key || !out)
        return E_FAIL;
    return field_0x04.UnknownFunction4b8620(key, out);
}
