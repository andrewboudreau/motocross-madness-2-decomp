// Parser.cpp -- reconstruction of D:\aardvark\VC\krusty2\Parser.cpp
// (0x004b8470..0x004b89f2). See Parser.h and docs/PARSER.md.

#include "Parser.h"

#include <stdlib.h>

#include "DebugAlloc.h"

ParserNode::ParserNode(const char* key, const char* value)
{
    Clear();
    if (key && value) {
        lstrcpy(tagText, key);
        lstrcpy(valueText, value);
        next = 0;
    }
}

ParserNode::~ParserNode()
{
    Clear();
}

BOOL ParserNode::MatchesKey(const char* key)
{
    if (!key)
        return FALSE;
    return lstrcmpi(tagText, key) == 0;
}

void ParserNode::Clear()
{
    tagText[0] = 0;
    valueText[0] = 0;
    next = 0;
}

HRESULT ParserList::Append(const char* key, const char* value)
{
    ParserNode* node;

    if (!head) {
        node = head = new (__FILE__, 73) ParserNode(key, value);
    } else {
        ParserNode* last = head;
        while (last->next)
            last = last->next;
        node = last->next = new (__FILE__, 86) ParserNode(key, value);
    }
    return node ? S_OK : E_OUTOFMEMORY;
}

ParserNode* ParserList::Find(const char* key)
{
    for (ParserNode* node = head; node; node = node->next) {
        if (node->MatchesKey(key))
            return node;
    }
    return 0;
}

HRESULT ParserList::GetNumber(const char* key, unsigned long* out)
{
    if (!key || !out)
        return E_FAIL;
    ParserNode* node = Find(key);
    if (!node)
        return E_ACCESSDENIED;
    *out = strtoul(node->valueText, 0, 10);
    return S_OK;
}

HRESULT ParserList::GetText(const char* key, char* out, unsigned int size)
{
    if (!key || !out || !size)
        return E_FAIL;
    ParserNode* node = Find(key);
    if (!node)
        return E_ACCESSDENIED;
    if ((unsigned int)lstrlen(node->valueText) < size) {
        lstrcpy(out, node->valueText);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

ParserList::~ParserList()
{
    while (head) {
        ParserNode* node = head;
        head = node->next;
        delete node;
    }
    head = 0;
}

char* Parser::SkipBlanks(char* text, char* end)
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

char* Parser::FindChar(char* text, char c, char* end)
{
    if (!end)
        end = text + lstrlen(text);
    while (text != end && *text != c)
        text++;
    if (text == end)
        return 0;
    return text;
}

// Each failed search returns S_OK from inside the loop. With `break` to one exit
// after the loop, VC6 pushes ebx/ebp only after the E_FAIL return; retail saves
// all four registers first (docs/VC6_SHRINK_WRAP.md).
HRESULT Parser::Parse(char* text, int length)
{
    char tag[0x200];
    char endTag[0x200];
    char value[0x200];

    if (!text)
        return E_FAIL;
    pairCount = 0;
    while (1) {
        char* p = SkipBlanks(text, 0);
        if (!p)
            return S_OK;
        char* open = FindChar(p, '<', 0);
        if (!open)
            return S_OK;
        char* close = FindChar(open, '>', 0);
        if (!close)
            return S_OK;
        lstrcpyn(tag, open + 1, close - open);
        p = SkipBlanks(close + 1, 0);
        if (!p)
            return S_OK;
        open = FindChar(p, '<', 0);
        if (!open)
            return S_OK;
        lstrcpyn(value, p, open - p + 1);
        p = SkipBlanks(open, 0);
        if (!p)
            return S_OK;
        open = FindChar(p, '<', 0);
        if (!open)
            return S_OK;
        close = FindChar(open, '>', 0);
        if (!close)
            return S_OK;
        lstrcpyn(endTag, open + 2, close - open - 1);
        text = close + 1;
        if (lstrlen(tag) && lstrlen(endTag) && lstrlen(value) && lstrcmp(tag, endTag) == 0) {
            pairCount++;
            pairs.Append(tag, value);
        }
    }
}

HRESULT Parser::GetText(const char* key, char* out, unsigned int size)
{
    if (!key || !out || !size)
        return E_FAIL;
    return pairs.GetText(key, out, size);
}

HRESULT Parser::GetNumber(const char* key, unsigned long* out)
{
    if (!key || !out)
        return E_FAIL;
    return pairs.GetNumber(key, out);
}
