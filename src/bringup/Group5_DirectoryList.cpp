// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// 0x004673b0: `mov eax, [ecx + 4]; ret`. The linker folded this accessor with
// FontTexture::GetFontName (same bytes); DirectoryList's caller
// (SelectGamePicProcs.cpp) uses it as the entry count at +0x04.
#include "DirectoryList.h"

int DirectoryList::GetFontName()
{
    return count;
}
