# Vehicle default and pure virtual entries

The primary tables identify two missing Vehicle definitions independently:

| Entry | Vehicle `0x00558a84` | Bike `0x00550900` | KrustyBike `0x00554548` |
|---|---|---|---|
| Slot 87 | `0x00464e90`, `RET` | Inherits Vehicle | Inherits Vehicle |
| Slot 94 | `0x00464e90`, `RET` | Inherits Vehicle | Override `0x004963f0` |
| Slot 101 | Not present | CRT `_purecall`, `0x00534cfe` | Override `0x00496d10` |

`VehicleDefaults.cpp` supplies the two complete one-byte methods. Their
provisional slot names remain distinct: a shared folded address does not make
them the same method. Neither default accesses the receiver or performs extra
cleanup. This reconstructed source grouping does not establish original TU
ownership.

Bike's slot 101 declaration is now pure virtual. Its retail entry is the
CRT error path, not an empty game method. Ordinary C++ `= 0` makes VC6 emit
the correct `__purecall` relocation. KrustyBike retains its concrete override.
The existing CRT library supplies the symbol; no replacement body, alias,
dispatch wrapper or forced unresolved link is introduced.

The retained test reads each retail table's complete-object locator and type
descriptor before checking the entries. It independently compiles the actual
Vehicle, Bike and KrustyBike translation units, checks the emitted target names,
and requires the same three object offsets in both retail and compiler output:

| Class | Primary | Secondary Character | Virtual GameObject |
|---|---:|---:|---:|
| Vehicle | 0 | 540 | 1472 |
| Bike | 0 | 540 | 1848 |
| KrustyBike | 0 | 540 | 5644 |

Both complete default bodies match retail strictly. A native VC6 fixture links
the actual provider object and checks 512 qualified calls across 256 whole
storage patterns, preserving every byte and surrounding guards. Native size
assertions retain the complete class sizes. The fixture does not execute the
aborting purecall entry or claim a working vehicle lifecycle; its correct
selection is checked through the emitted production vtable.

The isolated real build `work/game-build/build-0ghzixa0/report.json` compiles
221 translation units and removes exactly three missing symbols:
**362 → 359**. All 220 earlier production objects retain 10,478 complete
nondebug sections and 45,333 relocations under the four independently reviewed
text-provider substitutions plus Bike slot 101's correction to `__purecall`.
No other slot, section byte, addend or relocation destination changes.

With native `VC6_ROOT` and the private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_vehicle_default_slots.py -v
```
