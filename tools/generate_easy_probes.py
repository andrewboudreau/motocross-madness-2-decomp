#!/usr/bin/env python3
from __future__ import annotations
import argparse,json
from pathlib import Path

def args_from_pop(pop:int, pointer_first=False):
    n=max(0,int(pop)//4)
    parts=[]
    for i in range(n):
        parts.append(('int* ' if pointer_first and i==0 else 'int ') + f'a{i}')
    return ', '.join(parts)

def method_for(row):
    va=int(row['target_va'],16); name=f'E_{va:08X}'
    k=row['kind']; d=row.get('details',{}); pop=int(d.get('pop_bytes') or 0)
    p='char* p = (char*)this; '
    if k=='return_void':
        sig=f'void {name}()'; body='{}'
    elif k=='return_void_pop':
        sig=f'void {name}({args_from_pop(pop)})'; body='{}'
    elif k=='return_constant':
        sig=f'int {name}({args_from_pop(pop)})'; body='{ return %d; }'%int(d['value'])
    elif k=='return_this':
        sig=f'void* {name}()'; body='{ return this; }'
    elif k=='get_i32':
        sig=f'int {name}()'; body='{ '+p+f'return *(int*)(p + {int(d["offset"])}); }}'
    elif k=='get_i32_ignore_args':
        sig=f'int {name}({args_from_pop(pop)})'; body='{ '+p+f'return *(int*)(p + {int(d["offset"])}); }}'
    elif k=='get_u8':
        sig=f'unsigned int {name}()'; body='{ '+p+f'return *(unsigned char*)(p + {int(d["offset"])}); }}'
    elif k=='address_of_field':
        sig=f'void* {name}()'; body='{ '+p+f'return p + {int(d["offset"])}; }}'
    elif k=='get_float':
        sig=f'float {name}()'; body='{ '+p+f'return *(float*)(p + {int(d["offset"])}); }}'
    elif k=='set_i32_arg':
        sig=f'void {name}(int value)'; body='{ '+p+f'*(int*)(p + {int(d["offset"])} ) = value; }}'
    elif k=='return_float_global':
        g=f'g_{va:08X}'
        sig=f'float {name}({args_from_pop(pop)})'; body=f'{{ return {g}; }}'
        return name,sig,body,f'extern float {g};'
    elif k=='get_nested_float':
        sig=f'float {name}({args_from_pop(pop)})'
        body='{ '+p+f'char* q = *(char**)(p + {int(d["pointer_offset"])}); return *(float*)(q + {int(d["value_offset"])}); }}'
    elif k=='copy_i32_field':
        sig=f'void {name}()'; body='{ '+p+f'*(int*)(p + {int(d["dest_offset"])} ) = *(int*)(p + {int(d["source_offset"])}); }}'
    elif k=='set_i32_const_and_arg':
        sig=f'void {name}(int value)'
        body='{ '+p+f'*(unsigned int*)(p + {int(d["const_offset"])} ) = 0x{int(d["value_u32"]):08X}u; *(int*)(p + {int(d["arg_offset"])} ) = value; }}'
    elif k=='get_indexed_i32_stride32':
        sig=f'int {name}()'
        body='{ '+p+f'int i = *(int*)(p + {int(d["index_offset"])}); char* q = *(char**)(p + {int(d["base_offset"])}); return *(int*)(q + i * {int(d["stride"])} + {int(d["element_offset"])}); }}'
    elif k=='address_of_indexed_stride32':
        sig=f'void* {name}()'
        body='{ '+p+f'int i = *(int*)(p + {int(d["index_offset"])}); char* q = *(char**)(p + {int(d["base_offset"])}); return q + i * {int(d["stride"])} + {int(d["element_offset"])}; }}'
    elif k=='set_i32_arg_if_nonzero':
        sig=f'void {name}(int value)'; body='{ '+p+f'if (value) *(int*)(p + {int(d["offset"])} ) = value; }}'
    elif k=='write_arg_i32_const_return_const':
        sig=f'int {name}(int* out)'; body=f'{{ *out = {int(d["stored_value"])}; return {int(d["return_value"])}; }}'
    elif k=='set_i32_const':
        sig=f'void {name}()'; body='{ '+p+f'*(unsigned int*)(p + {int(d["offset"])} ) = 0x{int(d["value_u32"]):08X}u; }}'
    elif k=='set_i32_constants':
        sig=f'void {name}()'
        writes=' '.join(f'*(unsigned int*)(p + {int(x["offset"])} ) = 0x{int(x["value_u32"]):08X}u;' for x in d['stores'])
        body='{ '+p+writes+' }'
    elif k=='zero_i32_fields':
        sig=f'void {name}()'
        writes=' '.join(f'*(int*)(p + {int(off)} ) = 0;' for off in d['offsets'])
        body='{ '+p+writes+' }'
    else:
        return None
    return name,sig,body,None

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--targets',default='analysis/easy_targets.json')
    ap.add_argument('--out-source',default='generated/easy_probes.cpp')
    ap.add_argument('--out-manifest',default='generated/easy_probes.json')
    ap.add_argument(
        '--stability',
        action='append',
        choices=('high', 'medium'),
        help='compiler-stability tier to emit; repeat to select both (default: both)',
    )
    a=ap.parse_args()
    rows=json.loads(Path(a.targets).read_text())
    selected=[]; decls=[]; defs=[]; externs=[]
    stabilities = set(a.stability or ('high', 'medium'))
    for r in rows:
        if r.get('compiler_stability') not in stabilities: continue
        m=method_for(r)
        if not m: continue
        name,sig,body,ext=m
        decls.append('    '+sig+';')
        defs.append(sig.replace(name,f'EasyProbe::{name}',1)+' '+body)
        if ext: externs.append(ext)
        selected.append({'target_va':r['target_va'],'target_size':r['target_size'],'kind':r['kind'],'details':r.get('details',{}),'method':name,'symbol_contains':name,'uses':r.get('uses',[])})
    src=['// Auto-generated from analysis/easy_targets.json. Do not hand-edit.','// C++98-compatible so the same source can be fed to VC6 SP3.',*sorted(set(externs)),'','class EasyProbe {','public:',*decls,'};','',*defs,'']
    sp=Path(a.out_source); sp.parent.mkdir(parents=True,exist_ok=True); sp.write_text('\n'.join(src))
    mp=Path(a.out_manifest); mp.parent.mkdir(parents=True,exist_ok=True); mp.write_text(json.dumps(selected,indent=2)+'\n')
    print(f'generated {len(selected)} probes -> {sp} / {mp}')
if __name__=='__main__': main()
