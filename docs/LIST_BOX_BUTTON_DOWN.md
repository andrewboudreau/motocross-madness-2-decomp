# List-box mouse button handling

## Identity and complete body

`UIListBox::ButtonDown(int,int)` is primary virtual slot 55 in the retail
UIListBox table at `0x00553100`, object offset zero. The canonical GameUi object
requires this named provider. The second integer carries a POINT pointer. The
method handles visible-row selection and horizontal-drag initiation before
falling back to the existing base control mouse-button behavior. Member names
and original translation-unit ownership remain reconstructed.

The complete retail body spans `0x00477e90..0x00477fe5`: **342 bytes**, including
`ret 8` at `0x00477fa4`, `0x00477fcd` and `0x00477fe3`. Ten NOPs follow before
the next function. All direct branches remain inside the extent. There is no
EH frame, switch table, allocation or global access.

Five direct call sites target the existing GetRowHeight (twice), SelectRow,
and UIControl::ButtonDown (twice) providers. The two indirect calls are USER32
PtInRect at `0x00477f36` through IAT `0x005502d4`, and ReportClick at
`0x00477f8a` through primary slot 66. The retained test checks every call site,
the complete decoded body, and both emitted and retail slot destinations.

## Functional build review

`samples/ui/ListBoxButtonDown.cpp` is the separately compiled complete
candidate. It remains **nonexact: 342 bytes, 306 matching positions** under
VC6 SP3 `/O2 /GR /GX /MT`. The original candidate had the same size and
322 matching positions, but a demonstrated observable behavior difference.

Retail loads the point's Y coordinate at `0x00477f24`, then X at `0x00477f2e`.
The old candidate loaded X first. Native comparison with point address 1
reproduced an access violation reading **address 5 in retail versus address 1
in the old source**, after one identical real GetRowHeight observation.
Explicit ordered volatile reads now load Y and then X into the local POINT.
This narrow correction preserves ordinary coordinate values and matches the
observed fault order. Its lower byte score is recorded rather than hidden by
changing the target extent or replacing the old baseline.

The ignored evidence is `work/list-button35/before-fault.log`,
`behavior-correction.json` and `corrected-proof.json`. The before/after function
hashes, extents, scores and six relocations are retained. Original source
ownership and complete executable equivalence are not claimed.

The aggregate `GameUiNearMisses.cpp` includes the single maintained source.
A same-path VC6 comparison against the saved `b94f621d` aggregate finds exactly
one changed nondebug section: this method. All **349 other functions and 732
other nondebug sections** remain identical, including their complete bytes,
data, initializer/EH information and resolved relocation identities. The total
remains 350 functions, 733 nondebug sections and 3,912 relocations, with no
filename exception. Defined destinations are normalized to section identity
and offset. Reproduction: `work/list-button35/check_extraction.py` and
`same-path-preservation.json`, using `aggregate-before.obj` / `aggregate-after.obj`.

The standalone has only four imports: the three existing source methods
GetRowHeight, SelectRow and UIControl::ButtonDown, plus USER32 PtInRect. It adds
no allocator, global, CRT, initialization or EH dependency. These complete
canonical bodies remain strict in the focused test:

| Existing provider | Retail VA | Complete bytes |
| --- | --- | ---: |
| UIListBox::GetRowHeight | `0x004769a0` | 49 |
| UIListBox::SelectRow | `0x00476a60` | 110 |
| UIControl::ButtonDown | `0x004721b0` | 158 |
| UIListBox::ReportClick | `0x00477d30` | 301 |

The fixture executes an isolated source-derived GetRowHeight under a private
name; that copy also passes the complete strict comparison. It calls the real
Windows PtInRect API behind an observer. **SelectRow, base ButtonDown and
ReportClick are scripted boundaries**, with their real provider identity and
strictness proved separately. Their full UI lifecycle is not exercised here.

## Preserved decisions and reloads

The method dereferences ownerDialog and GUI-user state before testing whether
the point is null. A matching hover control and nonnull point gate the special
paths. For button zero, selectable gates the copied screen rectangle and row
loop. The loop reloads firstVisibleRow and visibleRowCount at its end; it uses
the original row-table stride of 56 and disabled flag bit 0.

Each attempted row calls GetRowHeight for its bottom and PtInRect for the hit.
A miss or disabled row calls GetRowHeight again to advance the top. A selectable
hit calls SelectRow, ignores its return, calls ReportClick with an initially
zero local handled flag, and exits the loop regardless of the flag. A nonzero
handled value returns literal 1; zero falls through to the base method.
Button one with horizontal scrolling enabled stores point X as dragStartX,
then invokes the base method. Fallback paths forward the original button and
point bits and preserve the base return value.

No added bounds checks, exception cleanup, row clamps, callback restrictions
or fallback results change these decisions. Read-order protection is confined
to the two point coordinate loads that differed from decoded retail.

## Native evidence and limits

`tests/test_list_box_button_down_behavior.py` compiles the actual standalone
candidate as a separate object and compares it against hash-pinned retail in
**8,707 native scenarios**. It checks full-body extents, target imports, layouts,
primary slot identity, all call sites, provider strictness and equality of the
standalone/aggregate method output.

The fixture validates the opcode and original destination at each of the five
retail direct call sites before private replacement with observers. Both the
source executable's PtInRect IAT and the mapped retail IAT use an observer
that calls the real API. ReportClick uses a scripted virtual slot. None of
this instrumentation or retail mapping enters the production game.

At each boundary and completion, the fixture compares all allocated control,
dialog-prefix, GUI-user-prefix, row-table and point storage, ordered arguments,
results, handled-output effects, exception kind and fault address. Cases cover
rectangle edges, disabled-row combinations, nonuniform/zero/negative row
heights, bounded visible ranges and row counts, button/hover/selectable/drag
combinations, return-value forwarding, handled zero/positive/negative values,
null and invalid dependencies, point/control-storage aliasing and guarded
coordinate reads. Mutations and thrown callbacks at every reached boundary
exercise changes to point values, screen rectangle, row flags/heights/table,
visible range, owner/user pointers and enable flags.

Independent assertions check selection results, literal handled success,
ignored SelectRow returns, forwarded base results, drag stores, two height
calls per missed row, absent callbacks after faults, and the exact invalid-point
Y-read fault. The pre-correction case is retained as a mandatory passing
regression against address 5. No lock behavior is inferred: this body acquires
none.

These are finite scripted UI boundaries and bounded lists. The test does not
establish real selection propagation, scrollbar updates, event dispatch,
sound/focus behavior, double-click timing, unbounded callback-driven list
growth, concurrent mutation, or working menus. Those existing provider bodies
are strict, but their integrated runtime behavior remains outside this fixture.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_list_box_button_down_behavior.py -v
```

The admitted real link `work/game-build/build-33hph3xk` compiles 231 translation
units and removes exactly `UIListBox::ButtonDown`: **316 to 315 unresolved
symbols**, with no newly missing names. Startup, integrated menus and racing
remain unverified because the source-built executable still does not link.
