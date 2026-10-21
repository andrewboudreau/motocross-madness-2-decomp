# Serial address conversion

`src/reconstructed/SerialAddress.cpp` supplies the missing serial-popup address
builder and its four lookup helpers. The five complete bodies match all 339
retail bytes with relocations resolved under authentic VC6 SP3 `vc6_o2_mt`:

| Operation | Retail address | Complete bytes |
| --- | --- | ---: |
| Baud selection | `0x004ae2f0` | 120 |
| Stop-bit selection | `0x004ae370` | 30 |
| Parity selection | `0x004ae390` | 56 |
| Flow-control selection | `0x004ae3d0` | 64 |
| Address builder | `0x004ae410` | 69 |

The helper extents include their jump tables. The builder ends with the return
at `0x004ae454`, followed by eleven padding bytes. Its four direct calls target
these helpers; the object has no undefined symbols or external runtime closure.
The complete original sample emission is preserved: six nondebug sections and
26 relocations, including all payloads, flags, local destinations and addends.
There is no writable storage or CRT initialization. Calibration now checks the
canonical source, and the duplicate sample body has been removed.

Original TU ownership remains unproven. Placement between Net.cpp and
NetProcs.cpp is navigation evidence only. The source filename and function
names are reconstructed names, not recovered source strings.

## Actual menu boundary

The complete strict 940-byte SerialPopupDlg event handler binds controls to
the five words of `g_SerialSettings` at `0x006886b8`. The complete strict
1,875-byte HandleMenuOk handler clears a local 20-byte output record, pushes
its address at `0x00452af3`, pushes the settings pointer at `0x00452af8`, and
calls the builder at **`0x00452b09`**. Earlier comments incorrectly described
`0x00452af9`, which is inside the immediate operand of that pointer push.
The caller then passes the same local record to the serial CreateAddress path.
Neither caller's source or compiled behavior changes in this promotion.

The builder adds one to the port index and converts the other four selections
to their observed serial values. Parity preserves the dialog's none/even/odd/mark
order, mapping to 0/2/1/3. Out-of-range lookup indices produce zero. Reads and
writes remain interleaved field by field, including when input and output overlap.
Native signed-port overflow retains the observed VC6 two's-complement result;
this is not a portable signed-overflow claim for other compilers.

## Validation

`tests/test_serial_address.py` checks seven complete strict comparisons, the
entire provider object, actual caller relocation/argument instructions, and
equality with the real game compiler command. Its fixture links the complete
source provider and executes the hash-pinned retail builder with its own
unmodified lookup helpers. There are no scripted conversion helpers or patches.

The 28,934 native cases comprise 4,800 ordinary selection combinations, 3,430
signed-edge cases across 49 overlapping/unaligned record offsets, 20,000
deterministic raw-input cases, 700 page-boundary cases, and four null/progressive
fault oracles. All ordinary cases additionally check an independent lookup and
bytewise store-order oracle. Comparisons cover every prepared buffer/guard byte,
readable page bytes, exception code, access kind and fault address. Independent
fault checks require the earlier two stores to survive a later read/write fault.

This finite validation does not execute the menu dispatcher, Windows serial
hardware, DirectPlay initialization, networking, or exception resumption. The
isolated real source-build link removes only the builder's unresolved name,
**268 -> 267**, with no new missing symbols. Full game linking and runtime tests
remain separate gates.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_serial_address -v
```
