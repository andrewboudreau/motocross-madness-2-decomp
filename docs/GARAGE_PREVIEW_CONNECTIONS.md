# Garage preview construction and plate painting

KrustyUI's garage builder now constructs the existing D3DIMSoultreeCharacter
class and calls Character::AdvanceMotion. Its plate painter and the single-player,
pro-circuit and multiplayer plate dialogs share the existing painter declaration
and call its model-typed PaintNumber method. The BikeRace setup sample uses that
same declaration and model boundary.

| Existing provider | Retail address | Complete strict bytes |
|---|---:|---:|
| D3DIMSoultreeCharacter constructor | `0x004455b0` | 195 |
| Character::AdvanceMotion | `0x004a6bb0` | 1293 |
| UnknownBikeNumberPainter constructor | `0x00417500` | 97 |
| UnknownBikeNumberPainter destructor | `0x00417570` | 7 |
| UnknownBikeNumberPainter::PaintNumber | `0x00417670` | 1157 |

No provider body changes. Six unresolved call-view names are replaced by these
existing definitions; the void-pointer PaintNumber name was shared by three
dialog translation units.

## Character boundary

The complete retail garage builder (`0x00498cf0`, 3114 bytes) allocates three
0x240-byte characters and calls the constructor at `0x004455b0`. The canonical
class has that same native size and its model pointer at `+0x1a0`. Retail RTTI
identifies Character as a nonvirtual base at offset zero, while GameObject is
a separate virtual base. The allocation's primary result is explicitly cast
back to KrustyUI's existing field/virtual-call view. No virtual slot or field
declaration is changed.

Each of the three AdvanceMotion calls retains `0.03f, 0, 0` and the primary
receiver. The provider's complete 1293-byte body remains strict exact. This
connection does not validate the rest of the garage's model loaders, shadows
or scene ownership; those retain separate incomplete declarations/providers.

## Painter declaration and lifetime

`BikeNumberPainterCore.h` contains the existing complete 0x2c-byte painter class,
with only pointer forward declarations. Its sheet is at zero and its ten digit
widths begin at four. `BikeNumberPainter.h` retains its previous public includes
and includes that core declaration. SelectGamePicProcs uses the same class
instead of its reduced duplicate; KrustyUI's obsolete local painter class is
removed. Constructor and destructor signatures are unchanged.

The painter is nonpolymorphic and has no recovered RTTI or original class name.
Its name and file grouping remain provisional. The constructor's actual resource
load and width stores, complete caller stack layouts, and unchanged cleanup
destinations establish this connection independently of a shared short address.

PaintNumber takes a primary D3DIMSoultreeObject pointer, not a texture pointer.
The old opaque values come from preview characters' `+0x1a0` model field or
race models. Explicit boundary casts retain those addresses. The strict provider
walks model materials and LOD surfaces, paints procedural textures, and updates
plate UVs. The caller comparisons preserve those input pointers and plate numbers.

## Validation

`tests/test_garage_preview_connections.py` passes **317 strict comparisons** and
preserves **six complete partial-function fingerprints**: five BikeRace sample
methods and KrustyUI::Create. The latter remains the unchanged 1102-byte candidate
against 1104 retail bytes; this slice does not promote it. All five providers
are checked in their actual production objects with complete COFF extents and
resolved relocations. Retail constructor/allocation counts, native layouts and
absence of the six retired imports are also checked.

The native fixture links the complete actual BikeNumberPainter production object
and runs **192 lifetime cases**. It checks every constructor field and resource
argument, texture Release receiver, call order, ordinary cleanup, cleanup during
a C++ exception, and absence of cleanup when resource loading throws before the
constructor completes. Resource loading and Release are scripted test boundaries;
an unexercised graphics helper fails immediately if called. This does not execute
DirectDraw painting or validate the full garage scene.

Private before/after comparison in
`work/game-build/garage-round20/object-preservation.json` preserves all **1792
non-debug sections and 11879 relocations** across ten source/sample objects.
External names are normalized only for the six connections; reordered defined
records and local targets are compared by symbol identity and offset. Full
section contents, relocation sites, types and raw addends remain unchanged.

The isolated real build `build-zp7xjvwm` compiles **205/205 translation units**
and removes exactly six unresolved names: **483 to 477**, with none added.
The game still does not link, and startup, rendered menus and racing remain
unverified.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_garage_preview_connections.py -v
```
