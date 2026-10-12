// Parser.cpp -- reconstruction of D:\aardvark\VC\krusty2\Parser.cpp
// (0x004b8470..0x004b89f2). See Parser.h and docs/PARSER.md.

#include "Parser.h"

#include <stdlib.h>

#include "DebugAlloc.h"

UnknownParserNode::UnknownParserNode(const char* key, const char* value)
{
    Clear();
    if (key && value) {
        lstrcpy(tagText, key);
        lstrcpy(valueText, value);
        next = 0;
    }
}

UnknownParserNode::~UnknownParserNode()
{
    Clear();
}

BOOL UnknownParserNode::MatchesKey(const char* key)
{
    if (!key)
        return FALSE;
    return lstrcmpi(tagText, key) == 0;
}

void UnknownParserNode::Clear()
{
    tagText[0] = 0;
    valueText[0] = 0;
    next = 0;
}

HRESULT UnknownParserList::Append(const char* key, const char* value)
{
    UnknownParserNode* node;

    if (!head) {
        node = head = new (__FILE__, 73) UnknownParserNode(key, value);
    } else {
        UnknownParserNode* last = head;
        while (last->next)
            last = last->next;
        node = last->next = new (__FILE__, 86) UnknownParserNode(key, value);
    }
    return node ? S_OK : E_OUTOFMEMORY;
}

UnknownParserNode* UnknownParserList::Find(const char* key)
{
    for (UnknownParserNode* node = head; node; node = node->next) {
        if (node->MatchesKey(key))
            return node;
    }
    return 0;
}

HRESULT UnknownParserList::GetNumber(const char* key, unsigned long* out)
{
    if (!key || !out)
        return E_FAIL;
    UnknownParserNode* node = Find(key);
    if (!node)
        return E_ACCESSDENIED;
    *out = strtoul(node->valueText, 0, 10);
    return S_OK;
}

HRESULT UnknownParserList::GetText(const char* key, char* out, unsigned int size)
{
    if (!key || !out || !size)
        return E_FAIL;
    UnknownParserNode* node = Find(key);
    if (!node)
        return E_ACCESSDENIED;
    if ((unsigned int)lstrlen(node->valueText) < size) {
        lstrcpy(out, node->valueText);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

UnknownParserList::~UnknownParserList()
{
    while (head) {
        UnknownParserNode* node = head;
        head = node->next;
        delete node;
    }
    head = 0;
}

char* UnknownParser::SkipBlanks(char* text, char* end)
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
        text = SkipBlanks(text, end);
    }
    return text;
}

char* UnknownParser::FindChar(char* text, char c, char* end)
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

HRESULT UnknownParser::GetText(const char* key, char* out, unsigned int size)
{
    if (!key || !out || !size)
        return E_FAIL;
    return pairs.GetText(key, out, size);
}

HRESULT UnknownParser::GetNumber(const char* key, unsigned long* out)
{
    if (!key || !out)
        return E_FAIL;
    return pairs.GetNumber(key, out);
}
