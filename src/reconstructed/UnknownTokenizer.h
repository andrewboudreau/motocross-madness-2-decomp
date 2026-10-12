#pragma once

// 0x00515dc0/0x00515df0: a strtok-style splitter, used on the stack (8 bytes,
// no destructor) by gameui.cpp, Parameterblocks.cpp and others. Its bodies
// sit directly before Track.cpp's 0x00515e70 and are reconstructed in
// Track.cpp, by position only. Names are provisional.
class UnknownTokenizer {
public:
    UnknownTokenizer(char* text);                          // 0x00515dc0
    char* NextToken(const char* delimiters);   // 0x00515df0: next token

    char* token;        // +0x00 current token
    char* rest;         // +0x04 rest of the text, 0 at the end
};
