// Near-miss dirlist.cpp candidates, kept out of src/reconstructed until they
// match. See docs/DIRLIST.md. The canonical file is included first so the
// TU-local macro and inline helpers are the same.
//
// 0x0044a600 (104 bytes, the found-file filter): everything matches except
// the attribute load: retail loads the whole dword and masks it with 0xff
// before `>> 4 & 1`; with an unsigned char or a cast VC6 here loads only
// the low byte, and an explicit `& 0xff`, `% 256` or `/ 16` on the dword
// (int or unsigned) is folded away. Char and int parameter types, locals
// and `(x & 0x10) >> 4` / `? 1 : 0` forms give one of those two shapes.

#include "../../src/reconstructed/DirectoryList.cpp"

// 0x0044a600
int DirectoryList::UnknownFunction44a600(const char* name, unsigned int attributes) {
    if (!_stricmp("", name) || !_stricmp(".", name) || !_stricmp("..", name))
        return 0;
    if (!includeFiles)
        return ((unsigned char)attributes >> 4) & 1;
    return 1;
}
