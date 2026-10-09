# dirlist.cpp

`src/reconstructed/DirectoryList.h` / `DirectoryList.cpp`. Evidence: the
`__FILE__` literal `D:\aardvark\VC\krusty2\dirlist.cpp` (`0x00569204`;
DebugMalloc line 32, `new` lines 44, 328, 374, 464/465, 480 and 512, delete
line 63), RTTI `DirectoryList` (vtable `0x005516e0`, 0x21c bytes) and
`CombinedDirectoryList : DirectoryList` (vtable `0x005516ec`), and the
Win32 imports the code calls. The file runs from `0x00449e60` to
`0x0044b09f`; dlgprocs.cpp follows. Method names are provisional; field
names follow the Win32 calls that fill them.

- The drive list (no RTTI; 8 bytes, created by the game at `0x005221e7`
  with a folded two-field constructor): `GetLogicalDriveStringsA`,
  `GetDriveTypeA` and, for fixed and CD-ROM drives,
  `GetVolumeInformationA` fill 0x10c-byte entries (root, type, volume
  name). `0x00523bf0` looks for the CD-ROM whose volume is "MCM2".
- `DirectoryList`: a directory (+0x110, `GetCurrentDirectoryA` initially)
  and pattern (+0xc), listed with `FindFirstFileA`/`FindNextFileA` into
  0x10c-byte entries (name, attributes, a tag). Names "", "." and ".." are
  skipped, and only directories are kept unless +0x218 is set. It also
  walks (+0x8), sorts (`qsort`/`_stricmp`) and deletes a tree
  (`DeleteFileA`/`RemoveDirectoryA`).
- `CombinedDirectoryList` lists two directories (+0x21c, +0x320) and
  merges them.

The MAX_PATH - 1 clamped copy is a macro (`COPY_NAME`): retail addresses
the destination at each use and loads the clamp constant first, which an
inline function does not reproduce. The entry setter is an inline method;
CombinedDirectoryList slot 1 `0x0044ac30` copies all three entry lists
through it (written out with COPY_NAME, VC6 keeps a separate pointer and
loses the merged count's register), and initialises the search as
`for (j = 0, found = 0; ...)` (`found = 0` first swaps the two slots).

Exact: 26 calibration cases (everything above except the near miss).

Near miss (`samples/render/DirectoryListNearMisses.cpp`, notes there): the
found-file filter `0x0044a600` (attribute load width).
