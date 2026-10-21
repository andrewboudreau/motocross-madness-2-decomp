# FilePrintf declaration connection

The existing `FileStream.cpp` provider at **0x00461d40**, complete extent
**24 bytes**, returns an `int`. Its readable body starts a `va_list` and returns
`vfprintf`'s result. Two consumer declarations incorrectly specified `void`,
leaving one unresolved decorated name even though the implementation was
already present.

`FilePrintf.h` now provides the shared declaration:

```cpp
int FilePrintf(FILE* file, const char* format, ...);
```

The provider includes it, and ManagedTextureGroup.cpp and EcoSystem.h include
it at their previous declaration boundaries. The only external-name join is
`?FilePrintf@@YAXPAU_iobuf@@PBDZZ` to
`?FilePrintf@@YAHPAU_iobuf@@PBDZZ`. No implementation, wrapper, linker alias,
storage, class layout or caller expression is added or changed. FileStream's
original translation-unit ownership remains provisional.

## Complete call boundaries

The decoded provider reads the FILE pointer, format and first variable argument
at stack offsets 4, 8 and 12. It passes those to the existing CRT target binding
at `0x00536b23`, removes its three pushed arguments, and returns without changing
EAX. This is an ordinary free cdecl function, with no object receiver or hidden
return object. `0x00536b23` is code, not a PE import thunk; the native regression
uses the linked VC6 CRT and does not assert complete retail CRT byte identity.

ManagedTextureGroup's complete strict **224-byte** `WriteDebugReport` at
`0x0050c7e0` contains five calls, at `0x0050c808`, `0x0050c81f`, `0x0050c85a`,
`0x0050c88f` and `0x0050c89d`. EcoSystem's unadmitted **461-byte** `WriteListing`
at `0x00458da0` contains two more, at `0x00458e3d` and `0x00458f35`.
Both FILE pointers originate in the callers' `fopen` results. The calls use the
same stack arguments and caller cleanup, including promoted doubles; they
ignore the integer result. The listing calls immediately replace EAX and clean
12 and 52 argument bytes, respectively.

The shared declaration therefore describes the existing provider without
changing either call's behavior. WriteListing remains nonexact, **457 matching
positions out of 461**, and is not admitted by this connection.

## Preservation and retained regression

The same-path before/after trial preserves all **300 nondebug sections and
1,464 relocations** across FileStream, ManagedTextureGroup, EcoSystem and
EcoSystemNearMisses, under exactly the one external-name substitution. It
retains complete payloads, section flags, relocation types/addends, public
offsets and defined section/offset destinations. No filename exception is
needed for that private comparison. The retained whole-object pins make only
the enumerated complete source filenames portable across checkout roots.

These are all direct source/header consumers of the changed declarations.
The separate full game-object audit also preserves all **253 existing objects,
10,669 nondebug sections and 45,717 relocations**, under that same single name.
No strict function or partial score is relaxed.

The older clipper-global regression's two EcoSystem pins were first checked
against the original objects. Canonical EcoSystem's pin is unchanged; only the
near-miss aggregate's two FilePrintf relocation names change its pin. Its
complete before/after contents agree under the same one-name join. This is
recorded in the fixture's baseline note rather than silently replacing a hash.

`tests/test_file_printf_connection.py` checks **74 strict comparisons**, all
four complete object pins, the seven actual source/retail call sites, both
fopen result transfers, and the unchanged WriteListing partial extent/score.
The complete 24-byte provider is extracted unchanged for private native
linkage; its bytes and relocation record must equal its actual full source-TU
emission and match retail strictly. Its sole dependency is `_vfprintf`; the
extracted object adds no data, initializer or other exported function.

The fixture runs **227 native cases** with real VC6 CRT streams:

- The seven observed caller format families, each with 32 signed and finite
  binary-fraction argument sets, compared to `fprintf` for complete output,
  integer return, position and stream error state.
- Empty output and a combined string, embedded NUL, `%n`, dynamic width and
  precision case, including the independently expected `%n` count.
- A real read-only FILE failure, requiring a negative return, error flag and
  unchanged file contents.

No FILE layout, formatter, allocator, retail instruction or OS boundary is
mocked. This is a finite source-wrapper/CRT integration test, not execution of
the two complete callers, every format or error, a texture-debug session, or
EcoSystem asset export. It neither promotes WriteListing nor establishes
integrated game behavior.

The isolated real source build compiles **253/253** translation units and
reduces unresolved names **245 -> 244**, removing only the obsolete void-return
spelling and adding none. Linking and running the game remain unfinished.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_file_printf_connection test_clipper_global_connections -v
```
