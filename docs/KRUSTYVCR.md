# KrustyVCR

Canonical reconstruction: `src/reconstructed/KrustyVCR.cpp` and
`KrustyVCR.bindings.json`. The class is declared in `BikeRace.h`.

## Evidence

- Confirmed: RTTI `KrustyVCR : VCRInterface`, vtable `0x00550e9c`.
- Confirmed: the literals `"MCMVCR"` (`0x0056da6c`) and `"2.0\x1a"`
  (`0x0056da64`).
- The six methods `0x0049bf10..0x0049c2e9` sit after krustyui.cpp's last
  `__FILE__` xref (`0x0049bb93`). They have no `__FILE__` literal of their
  own, so the file name is ours (tier 3). bikerace.cpp calls all six on its
  replay and ghost recorders.
- Member names are provisional.

## Layout

These offsets follow from the decoded stores (BikeRace.h):

| Offset | Contents |
|---|---|
| `+0x0d8` | Playback time (float) |
| `+0x0dc` | The 0xeb0-byte header block that `VCRInterface::Start` (`0x004e7a90`) and `0x004e86d0` take: the signature and version (8 bytes each) |
| `+0x0ec` | A 0x20-byte description |
| `+0x10c` | Length (float) |
| `+0x110` | A 0x1ec-byte copy of TrackGame+0x2d70 |
| `+0x300` | Eleven 0x124-byte racer records |

Each racer record holds:
- an id and an AI byte;
- five clamped names (0x10 and 4 × 0x40 bytes);
- three ints.

## Matches (6 of 6 exact)

| VA | Size (bytes) |
|---|---|
| `0x0049bf10` | 215 |
| `0x0049bff0` | 13 |
| `0x0049c000` | 7 |
| `0x0049c010` | 96 |
| `0x0049c070` | 363 |
| `0x0049c1e0` | 265 |

Source forms that mattered:
- Retail copies each clamped string as `count = length > max ? max :
  length; strncpy; dest[count] = 0`, and addresses the array afresh for the
  terminator.
  - An inline helper with a destination pointer shares that pointer in a
    register, which retail does not do.
  - The file therefore uses the `COPY_TRUNCATED` macro.
- The racer records are indexed as `field_0x300[slot].member` in every
  statement. Taking a record pointer folds `+0x300` into the base, which
  retail does not do.
