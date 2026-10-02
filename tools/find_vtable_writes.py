#!/usr/bin/env python3
from __future__ import annotations
import os, json,struct
from collections import defaultdict
from pathlib import Path
from mcm2tool.pe import PEImage

REG=['eax','ecx','edx','ebx','esp','ebp','esi','edi']

def decode_c7(blob:bytes,i:int):
    if i+6>len(blob) or blob[i]!=0xC7: return None
    modrm=blob[i+1]; mod=(modrm>>6)&3; reg=(modrm>>3)&7; rm=modrm&7
    if reg!=0 or mod==3: return None
    j=i+2; base=None; index=None; scale=None; disp=0
    if rm==4:
        if j>=len(blob): return None
        sib=blob[j]; j+=1
        scale=1<<((sib>>6)&3); idx=(sib>>3)&7; bas=sib&7
        index=None if idx==4 else REG[idx]
        if mod==0 and bas==5:
            base=None
            if j+4>len(blob): return None
            disp=struct.unpack_from('<i',blob,j)[0]; j+=4
        else: base=REG[bas]
    else:
        if mod==0 and rm==5:
            base=None
            if j+4>len(blob): return None
            disp=struct.unpack_from('<i',blob,j)[0]; j+=4
        else: base=REG[rm]
    if mod==1:
        if j>=len(blob): return None
        disp=struct.unpack_from('<b',blob,j)[0]; j+=1
    elif mod==2:
        if j+4>len(blob): return None
        disp=struct.unpack_from('<i',blob,j)[0]; j+=4
    if j+4>len(blob): return None
    imm=struct.unpack_from('<I',blob,j)[0]; end=j+4
    return {'end':end,'imm_offset':j,'immediate':imm,'base_register':base,'index_register':index,'scale':scale,'displacement':disp}

def main():
    pe=PEImage(os.environ.get('MCM2_EXE', 'work/game/mcm2.exe'))
    vtables=json.loads(Path('analysis/vtables.json').read_text())
    byva=defaultdict(list)
    for v in vtables: byva[int(v['vtable_va'],16)].append({'class':v['class'],'object_offset':int(v.get('object_offset',0)),'vtable_va':v['vtable_va']})
    sec=next(s for s in pe.sections if s.name=='.text')
    blob=pe.data[sec.raw_offset:sec.raw_offset+sec.raw_size]
    baseva=pe.image_base+sec.virtual_address
    rows=[]
    i=0
    while i<len(blob)-6:
        d=decode_c7(blob,i)
        if d and d['immediate'] in byva:
            rows.append({
                'code_va':f'0x{baseva+i:08x}',
                'instruction_size':d['end']-i,
                'bytes':blob[i:d['end']].hex(' '),
                'base_register':d['base_register'],'index_register':d['index_register'],'scale':d['scale'],
                'displacement':d['displacement'],
                'written_vtable_va':f"0x{d['immediate']:08x}",
                'vtable_owners':byva[d['immediate']],
            })
            i=d['end']; continue
        i+=1
    Path('analysis/vtable_write_xrefs.json').write_text(json.dumps(rows,indent=2)+'\n')
    byclass=defaultdict(int)
    for r in rows:
        for o in r['vtable_owners']: byclass[o['class']]+=1
    print(json.dumps({'vtable_write_sites':len(rows),'classes_with_vtable_write_evidence':len(byclass),'top_classes':sorted(byclass.items(),key=lambda x:(-x[1],x[0]))[:15]},indent=2))

if __name__=='__main__': main()
