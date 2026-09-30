#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, hashlib, re, datetime
from pathlib import Path
from dataclasses import asdict
from mcm2tool.pe import PEImage
from mcm2tool.rich import parse_rich
from mcm2tool.rtti import parse_rtti, find_type_descriptors
from mcm2tool.easy import discover_easy_vtable_targets

SOURCE_RE = re.compile(rb'([A-Za-z]:\\[^\x00\r\n]{1,500}?\.(?:cpp|c|h|hpp))\x00', re.I)

def sha256(path: Path) -> str:
    h=hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda:f.read(1<<20),b''): h.update(chunk)
    return h.hexdigest()

def collect_vtable_entries(pe: PEImage, vtable_va: int, limit=128) -> list[int]:
    out=[]; off=pe.va_to_offset(vtable_va)
    for i in range(limit):
        p=pe.u32(off+i*4)
        if not pe.is_code_va(p): break
        out.append(p)
    return out

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('exe')
    ap.add_argument('--out',default='analysis')
    ap.add_argument('--skeleton-root',default='src/krusty2')
    args=ap.parse_args()
    exe=Path(args.exe); out=Path(args.out); out.mkdir(parents=True,exist_ok=True)
    pe=PEImage(exe)
    source_paths=sorted({m.group(1).decode('latin1') for m in SOURCE_RE.finditer(pe.data)},key=str.lower)
    type_descriptors=find_type_descriptors(pe)
    classes=parse_rtti(pe)
    imports=pe.imports()
    rich=parse_rich(pe)
    fp={
      'file': exe.name,
      'sha256': sha256(exe),
      'size': exe.stat().st_size,
      'machine': f'0x{pe.machine:04x}',
      'timestamp_unix': pe.timestamp,
      'timestamp_utc': datetime.datetime.fromtimestamp(pe.timestamp,datetime.timezone.utc).isoformat(),
      'image_base': f'0x{pe.image_base:08x}',
      'entry_point_va': f'0x{pe.image_base+pe.entry_rva:08x}',
      'linker_version': f'{pe.linker_major}.{pe.linker_minor:02d}',
      'sections': [asdict(s) for s in pe.sections],
      'source_path_count': len(source_paths),
      'rtti_type_descriptors': len(type_descriptors),
      'rtti_class_records': len(classes),
      'rich': None if rich is None else {
         'xor_key': f'0x{rich[0]:08x}',
         'entries': [asdict(e) for e in rich[1]],
      }
    }
    (out/'fingerprint.json').write_text(json.dumps(fp,indent=2)+'\n')
    (out/'imports.json').write_text(json.dumps(imports,indent=2,sort_keys=True)+'\n')
    (out/'source_paths.txt').write_text('\n'.join(source_paths)+'\n')
    source_manifest=[]
    root=Path(args.skeleton_root); root.mkdir(parents=True,exist_ok=True)
    # promoted translation units may live in area subfolders (e.g. broadphase/Quadtree.cpp);
    # don't recreate a flat skeleton next to them
    existing={f.name.lower() for f in root.rglob('*') if f.is_file()}
    for p in source_paths:
        name=p.rsplit('\\',1)[-1]
        ext=Path(name).suffix.lower()
        source_manifest.append({'original_path':p,'filename':name,'kind':ext.lstrip('.')})
        dst=root/name
        if not dst.exists() and name.lower() not in existing:
            comment='//'
            dst.write_text(f'{comment} Recovered translation-unit name from Motocross Madness 2 retail binary.\n{comment} Original path: {p}\n{comment} Status: skeleton; implementation not yet reconstructed.\n')
    (out/'source_manifest.json').write_text(json.dumps(source_manifest,indent=2)+'\n')

    # Direct immediate xrefs from .text to each recovered __FILE__ string. These are
    # strong evidence for translation-unit attribution when an assert/log references __FILE__.
    text_sec=next((sec for sec in pe.sections if sec.name=='.text'),None)
    source_xrefs=[]
    if text_sec:
        text_blob=pe.data[text_sec.raw_offset:text_sec.raw_offset+text_sec.raw_size]
        for m in SOURCE_RE.finditer(pe.data):
            path=m.group(1).decode('latin1')
            try: string_va=pe.offset_to_va(m.start(1))
            except Exception: continue
            pat=string_va.to_bytes(4,'little'); pos=0; refs=[]
            while True:
                hit=text_blob.find(pat,pos)
                if hit<0: break
                refs.append(pe.image_base+text_sec.virtual_address+hit); pos=hit+1
            source_xrefs.append({'path':path,'string_va':f'0x{string_va:08x}','text_xrefs':[f'0x{x:08x}' for x in refs]})
    (out/'source_xrefs.json').write_text(json.dumps(source_xrefs,indent=2)+'\n')

    class_rows=[]; edge_set=set(); vtables=[]
    for c in classes:
        d=asdict(c)
        d['type_descriptor_va']=f'0x{c.type_descriptor_va:08x}'
        d['complete_object_locator_va']=f'0x{c.complete_object_locator_va:08x}'
        d['complete_object_locator_vas']=[f'0x{x:08x}' for x in c.complete_object_locator_vas]
        d['class_hierarchy_va']=f'0x{c.class_hierarchy_va:08x}'
        d['vtables']=[f'0x{x:08x}' for x in c.vtables]
        for vr in d.get('vtable_records',[]):
            vr['complete_object_locator_va']=f"0x{vr['complete_object_locator_va']:08x}"
            vr['vtables']=[f"0x{x:08x}" for x in vr['vtables']]
        for b in d['bases']:
            b['type_va']=f"0x{b['type_va']:08x}"
        class_rows.append(d)
        for b in c.direct_bases: edge_set.add((c.name,b))
        for vr in c.vtable_records:
            for vt in vr.vtables:
                vtables.append({
                    'class':c.name,
                    'vtable_va':f'0x{vt:08x}',
                    'object_offset':vr.object_offset,
                    'complete_object_locator_va':f'0x{vr.complete_object_locator_va:08x}',
                    'entries':[f'0x{x:08x}' for x in collect_vtable_entries(pe,vt)],
                })
    td_rows=[{'va':f'0x{td.va:08x}','decorated':td.decorated,'name':td.name} for td in sorted(type_descriptors.values(),key=lambda x:(x.name,x.va))]
    (out/'rtti_type_descriptors.json').write_text(json.dumps(td_rows,indent=2)+'\n')
    (out/'rtti_classes.json').write_text(json.dumps(class_rows,indent=2)+'\n')
    edges=[{'derived':d,'base':b} for d,b in sorted(edge_set)]
    (out/'inheritance_edges.json').write_text(json.dumps(edges,indent=2)+'\n')
    dot=['digraph mcm2_inheritance {','  rankdir=LR;']
    for e in edges:
        d=e['derived'].replace(chr(34),'\\'+chr(34)); b=e['base'].replace(chr(34),'\\'+chr(34))
        dot.append(f'  \"{d}\" -> \"{b}\";')
    dot.append('}')
    (out/'inheritance.dot').write_text('\n'.join(dot)+'\n')
    (out/'vtables.json').write_text(json.dumps(vtables,indent=2)+'\n')
    easy_targets=discover_easy_vtable_targets(pe,vtables)
    (out/'easy_targets.json').write_text(json.dumps(easy_targets,indent=2)+'\n')
    (out/'rtti_types.txt').write_text('\n'.join(sorted({td.name for td in type_descriptors.values()},key=str.lower))+'\n')

    print(json.dumps({'source_paths':len(source_paths),'rtti_type_descriptors':len(type_descriptors),'rtti_records':len(classes),'classes_with_vtables':sum(bool(c.vtables) for c in classes),'imports':sum(len(v) for v in imports.values()),'easy_targets':len(easy_targets)},indent=2))

if __name__=='__main__': main()
