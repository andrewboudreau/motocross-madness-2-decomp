#!/usr/bin/env python3
from __future__ import annotations
import json
from collections import defaultdict
from pathlib import Path
from mcm2tool.pe import PEImage
from mcm2tool.msvc import scalar_deleting_destructor,this_adjustor_thunk


def main():
    pe=PEImage('work/game/mcm2.exe')
    vtables=json.loads(Path('analysis/vtables.json').read_text())
    refs=defaultdict(list)
    for v in vtables:
        for slot,e in enumerate(v.get('entries',[])):
            va=int(e,16)
            refs[va].append({
                'class':v['class'],'object_offset':int(v.get('object_offset',0)),
                'slot':slot,'vtable_va':v['vtable_va']
            })
    dtors=[]; thunks=[]
    for va,uses in sorted(refs.items()):
        try: blob=pe.bytes_at_va(va,40)
        except Exception: continue
        d=scalar_deleting_destructor(va,blob)
        if d:
            d.update({'wrapper_va':f'0x{va:08x}','destructor_va':f"0x{d['destructor_va']:08x}",'operator_delete_va':f"0x{d['operator_delete_va']:08x}",'uses':uses})
            dtors.append(d); continue
        t=this_adjustor_thunk(va,blob)
        if t:
            t.update({'thunk_va':f'0x{va:08x}','target_va':f"0x{t['target_va']:08x}",'uses':uses})
            thunks.append(t)
    Path('analysis/deleting_destructors.json').write_text(json.dumps(dtors,indent=2)+'\n')
    Path('analysis/vtable_thunks.json').write_text(json.dumps(thunks,indent=2)+'\n')
    delete_targets=defaultdict(int)
    for d in dtors: delete_targets[d['operator_delete_va']]+=1
    print(json.dumps({
        'scalar_deleting_destructor_wrappers':len(dtors),
        'vtable_this_adjustor_thunks':len(thunks),
        'operator_delete_targets':dict(sorted(delete_targets.items())),
    },indent=2))

if __name__=='__main__': main()
