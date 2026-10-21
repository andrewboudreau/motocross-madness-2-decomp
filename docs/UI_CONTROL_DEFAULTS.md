# UI control default methods

Seven missing definitions now have complete strict VC6 SP3 bodies in
`src/reconstructed/UIControlDefaults.cpp`. Primary RTTI-bound vtables identify
each role independently; folded addresses alone do not establish identity.
The grouping does not establish original translation-unit ownership.

| Class and method | Primary slot | Retail address | Complete bytes |
|---|---:|---|---:|
| UIControl::KillFocus | 33 | 0x00464e80 | 3 |
| UIControl::SyncBoundValue | 59 | 0x00464e80 | 3 |
| UIControl::HandleTimer | 60 | 0x00464e80 | 3 |
| UIControl::IsInteractive | 30 | 0x00467ae0 | 6 |
| UIDropDownList::IsInteractive | 30 | 0x00467ae0 | 6 |
| UIStaticText::IsInteractive | 30 | 0x00478fe0 | 3 |
| UIStaticText::SetState | 29 | 0x00479220 | 20 |

The three UIControl callbacks are `RET 4` and have no state effect. The base
interaction predicate returns one; UIStaticText overrides it to zero and
UIDropDownList overrides it back to one. UIStaticText's state setter first
writes one to `needsRedraw` at +0x1c0, then stores its argument to `currentState`
at +0x60. UIDropDownList inherits that setter. No callbacks, allocation or
resource ownership occur in these bodies.

The primary tables are UIControl `0x00552ba4`, UIStaticText `0x00553420` and
UIDropDownList `0x00553a58`. Retail complete-object locators and full base
relations place each of these objects, UIControl, GameObject and BaseObject
at offset zero, with nonvirtual inheritance. The retained test reads the
locators/type names and compares twelve affected or inherited slots against
the corresponding relocations in the actual compiled GameUi translation unit.

The [retained regression](../tests/test_ui_control_defaults.py) checks all seven
complete providers strictly and performs **12,544 native calls** across whole
guarded storage patterns and seven signed argument values. It verifies return
values, unchanged memory for predicates/callbacks, and exactly the two setter
writes. Actual headers establish the fields and sizes (UIControl/UIStaticText
0x1ec, UIDropDownList 0x21c). Calls are qualified on prepared storage; this is
not a constructed-control lifecycle or rendered-menu test. Vtable target
selection is checked separately in the actual production object.

The isolated real build `work/game-build/build-9rgsrslk/report.json` compiles
223 translation units and removes exactly these seven missing definitions,
**355 → 348**, without new unresolved names. Existing production objects are
unchanged by adding this source file. Startup, menus and racing remain
unverified while the overall link is incomplete.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_ui_control_defaults.py -v
```
