# TrackRecord

The literal `__FILE__` `D:\aardvark\VC\krusty2\TrackRecord.cpp`
(`0x0057536c`) is referenced by `0x0051ef30`, `0x0051ff40` and
`0x0051ffe0`. Canonical source is `src/reconstructed/TrackRecord.h`,
`TrackRecordDlg.h` and `TrackRecord.cpp`; every name is provisional.

The TU covers at least `0x0051ee90..0x00520692`. ChatOverlay's code ends
before it. `0x005206a0` writes another class's vtable (`0x005588c4`) and
`0x005206d0` belongs to TransparencyMod, so neither is placed here.

## Ownership evidence

| Evidence | Functions |
|---|---|
| `__FILE__` reference (confirmed) | `0x0051ef30`, `0x0051ff40`, `0x0051ffe0` |
| TrackRecordDlg vtable `0x0055177c` slot 29 (confirmed) | `0x0051f600` |
| Called only from the above, or by position (strong inference) | `0x0051ee90`, `0x0051eed0`, `0x0051ef00` (qsort callbacks of `0x0051f3c0`), `0x0051efc0`..`0x0051f3c0`, `0x0051fc40`..`0x005204e0` |

## Data

- **High-score table** (`UnknownTrackGameObject3400`, 0xe8 bytes, at
  TrackGame+0x3400). It holds an extension index (+0x00) and the extensions
  `.hs1`, `.hs2` and `.hs3` (+0x04, 5 bytes each). Then come ten records
  (+0x14), a DirectoryList (+0xdc), the record count (+0xe0) and a stored
  value (+0xe4). The file is the value, the count and the ten records,
  written as one block.
- **Record**, 0x14 bytes: a 16-byte name and a float time or score.
- **TrackRecordDlg** : UIDialog (RTTI), 0x7f68 bytes (the allocation size
  at `0x0044c27b` and `0x0049a2e6`). Its inline constructor passes
  `UIDialog(1, ...)`.
- `0x0051f3c0` reads the racer entries in EventManager+0x50, at +0x08,
  +0x0c (an int), +0x14, +0x18, +0x1c, +0x2c and the name at +0x40.
- `0x0051f0b0` and `0x0051f260` index six 0x100-byte directory names at
  TrackGameMode+0xa0. `0x0051f2c0` passes TrackGameMode+0x6a0 as a string
  pointer, although `EventManager.cpp` uses it as an int.
- TrackRecordDlg fields: the current series (+0x7f58), the selected tab
  (+0x7f5c), and a realloc'd array of 8-byte track rows (+0x7f60, count
  +0x7f64). Each row is a strdup'd file name and an int. The event argument
  of `0x0051fe80`, `0x00520390`, `0x00520480` and `0x005204e0` carries the
  dialog pointer at +0x0c.
- `UIDialog::UnknownFunction46ebf0` looks a control up by name. GameUi.h
  already declares the same address as `UnknownGameUiPage::FindControl`.
  The new declaration was added because TrackRecordDlg derives from
  UIDialog, so the two names are one function, not proven types.
- `0x00520390` calls control slot 66 (`+0x108`) on the track list. Vtable
  `0x00553100` (67 slots, probably UIListBox) is the nearest fit, so the
  cast type `UnknownTrackRecordListBox` stays provisional.

## Status

The following 18 functions are exact under `vc6_o2_mt`, with every
relocation bound:

| VA | Bytes | Body |
|---|---:|---|
| `0x0051ee90` | 62 | record constructor |
| `0x0051eed0` | 47 | qsort order, lower value first |
| `0x0051ef00` | 47 | qsort order, higher value first |
| `0x0051ef30` | 142 | table constructor |
| `0x0051efc0` | 31 | table destructor |
| `0x0051efe0` | 193 | clears the table |
| `0x0051f0b0` | 82 | reads from a TrackGameMode directory |
| `0x0051f110` | 156 | reads the table |
| `0x0051f1b0` | 163 | writes the table |
| `0x0051f260` | 90 | writes to a TrackGameMode directory |
| `0x0051f2c0` | 253 | reads the table if the directory scan finds it |
| `0x0051f3c0` | 564 | enters a racer and re-sorts |
| `0x0051fc40` | 567 | tab captions for the series |
| `0x0051fe80` | 188 | selects the series tab |
| `0x0051ff40` | 149 | frees the track rows |
| `0x00520390` | 240 | shows a series |
| `0x00520480` | 93 | adds a score row |
| `0x005204e0` | 437 | shows one track's scores |

`0x0051ffe0` (932 bytes, lists the tracks that have a high-score file) is a
near miss in `samples/track/TrackNearMisses.cpp`. The candidate is 964
bytes. Retail cross-jumps its two copies of the digit-suffix branch, and
VC6 here allocates a different register in the second copy, which blocks
the merge. `0x0051f600` (slot 29, 1596 bytes) has not been attempted.

## Source shapes that mattered

- The name length clamps are `int n = strlen(s); int length = n > 15 ? 15 : n;`.
  `if (length > 15) length = 15;` swaps the branch layout.
- `0x0051f3c0`:
  - `float value = 0.0f;` before the switch, and a separate copy of the
    "higher first" check in cases 4 and 0. Without the initializer, VC6
    merges the three `return 0` epilogues into one. A `goto` into a shared
    check swaps case 0's registers.
  - The racer entry must be indexed in every expression. Taking a pointer
    to it changes the address arithmetic.
- Call order: in `a->Find(...)->Method(args)`, VC6 pushes `args` before it
  calls `Find`. Storing `Find`'s result in a local first makes it call
  `Find` first, as `0x00520480` does.
- `if (!times) f(0x938, ...); else f(0xbc5, ...);` matches `0x005204e0`;
  the hoisted push of the common arguments is VC6's doing. A ternary
  argument gives `neg/sbb/and`.
- `0x0051ffe0`: the frame is 0x350 only with a 260-byte buffer. Declaration
  order does not change VC6's slot order.

## Reproduce

```bash
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```

The `# TrackRecord.cpp` block at the end of `CASES` in
`tools/run_calibration.py` lists the TrackRecord cases.
