#!/usr/bin/env python3
from __future__ import annotations
import json,re
from collections import defaultdict
from pathlib import Path


def load(path, default):
    p=Path(path)
    return json.loads(p.read_text()) if p.exists() else default


def norm(s:str)->str:
    return re.sub(r'[^a-z0-9]','',s.lower())


def main():
    classes=load('analysis/rtti_classes.json',[])
    vtables=load('analysis/vtables.json',[])
    overrides=load('analysis/vtable_overrides.json',[])
    relation_by_use={}
    for ov in overrides:
        for slot in ov.get('slots',[]):
            relation_by_use[(ov['class'],int(slot['slot']))]=slot.get('relation')
    layouts=load('analysis/class_layout_hints.json',[])
    dtors=load('analysis/deleting_destructors.json',[])
    thunks=load('analysis/vtable_thunks.json',[])
    writes=load('analysis/vtable_write_xrefs.json',[])
    funcs=load('analysis/function_manifest.json',[])
    src_manifest=load('analysis/source_manifest.json',[])
    src_xrefs=load('analysis/source_xrefs.json',[])

    by_vt=defaultdict(list)
    for v in vtables: by_vt[v['class']].append(v)
    by_override={x['class']:x for x in overrides}
    by_layout={x['class']:x for x in layouts}
    by_dtor=defaultdict(list)
    for d in dtors:
        for u in d.get('uses',[]): by_dtor[u['class']].append({**d,'uses':[u]})
    by_thunk=defaultdict(list)
    for t in thunks:
        for u in t.get('uses',[]): by_thunk[u['class']].append({**t,'uses':[u]})
    by_write=defaultdict(list)
    for w in writes:
        for o in w.get('vtable_owners',[]): by_write[o['class']].append(w)
    by_func=defaultdict(dict)
    by_direct_func=defaultdict(dict)
    for f in funcs:
        for u in f.get('vtable_uses',[]):
            c=u['class']; by_func[c][f['target_va']]=f
            # For TU/source attribution, avoid inherited primary-vtable entries:
            # a shared BaseObject/GameObject method is evidence for the base, not
            # for every derived class that happens to inherit the same pointer.
            if int(u.get('object_offset',0))==0 and relation_by_use.get((c,int(u['slot'])))!='inherited':
                by_direct_func[c][f['target_va']]=f
        pc=f.get('provisional_class')
        if pc:
            by_func[pc][f['target_va']]=f
            by_direct_func[pc][f['target_va']]=f

    # Flatten all recovered __FILE__ xrefs for distance scoring.
    all_xrefs=[]
    for s in src_xrefs:
        for x in s.get('text_xrefs',[]):
            va=int(x,16) if isinstance(x,str) else int(x)
            all_xrefs.append((va,s['path']))

    source_files=[s for s in src_manifest if s.get('kind')=='cpp']
    out=[]
    for c in classes:
        name=c['name']
        # Structural anchors (vptr writes / destructor bodies) are much safer for
        # translation-unit proximity than generic shared vtable stubs. Add direct
        # method bodies only when they live near that class's structural cluster.
        structural=set()
        for w in by_write.get(name,[]): structural.add(int(w['code_va'],16))
        for d in by_dtor.get(name,[]):
            structural.add(int(d['wrapper_va'],16)); structural.add(int(d['destructor_va'],16))
        for t in by_thunk.get(name,[]): structural.add(int(t['thunk_va'],16))
        anchors=set(structural)
        direct_vas=[int(f['target_va'],16) for f in by_direct_func.get(name,{}).values()]
        if structural:
            for va in direct_vas:
                if min(abs(va-a) for a in structural) <= 0x4000: anchors.add(va)
        else:
            anchors.update(direct_vas)

        candidates={}
        cname=norm(name)
        for s in source_files:
            stem=Path(s['filename']).stem
            sn=norm(stem)
            reasons=[]; score=0
            if sn==cname:
                score+=100; reasons.append('filename stem exactly matches RTTI class name')
            elif cname and (cname in sn or sn in cname) and min(len(cname),len(sn))>=5:
                score+=45; reasons.append('filename/class normalized names overlap')
            candidates[s['original_path']]={'path':s['original_path'],'filename':s['filename'],'score':score,'reasons':reasons,'nearest_anchor_distance':None,'nearest_anchor_va':None,'nearest_xref_va':None}

        if anchors and all_xrefs:
            # Record the nearest class anchor -> __FILE__ xref for each source path.
            perpath={}
            for xv,path in all_xrefs:
                a=min(anchors,key=lambda av:abs(av-xv))
                dist=abs(a-xv)
                cur=perpath.get(path)
                if cur is None or dist<cur[0]: perpath[path]=(dist,a,xv)
            for path,(dist,a,xv) in perpath.items():
                if path not in candidates: continue
                ent=candidates[path]
                ent['nearest_anchor_distance']=dist
                ent['nearest_anchor_va']=f'0x{a:08x}'
                ent['nearest_xref_va']=f'0x{xv:08x}'
                # Keep proximity evidence deliberately modest vs an exact filename match.
                if dist<=0x100:
                    ent['score']+=35; ent['reasons'].append('__FILE__ xref within 0x100 of class evidence')
                elif dist<=0x400:
                    ent['score']+=25; ent['reasons'].append('__FILE__ xref within 0x400 of class evidence')
                elif dist<=0x1000:
                    ent['score']+=15; ent['reasons'].append('__FILE__ xref within 0x1000 of class evidence')
                elif dist<=0x4000:
                    ent['score']+=5; ent['reasons'].append('__FILE__ xref within 0x4000 of class evidence')
        source_candidates=[x for x in candidates.values() if x['score']>0]
        source_candidates.sort(key=lambda x:(-x['score'],x['nearest_anchor_distance'] if x['nearest_anchor_distance'] is not None else 1<<60,x['filename']))

        rec={
            'class':name,
            'direct_bases':c.get('direct_bases',[]),
            'base_descriptors':c.get('bases',[]),
            'vtables':by_vt.get(name,[]),
            'primary_override_map':by_override.get(name),
            'layout_hints':by_layout.get(name),
            'deleting_destructors':by_dtor.get(name,[]),
            'this_adjustor_thunks':by_thunk.get(name,[]),
            'vtable_write_sites':by_write.get(name,[]),
            'function_targets':list(by_func.get(name,{}).values()),
            'direct_function_targets':list(by_direct_func.get(name,{}).values()),
            'source_candidates':source_candidates[:8],
            'evidence_anchor_count':len(anchors),
        }
        out.append(rec)

    Path('analysis/class_dossiers.json').write_text(json.dumps(out,indent=2)+'\n')

    # Human-readable focused summary for high-value classes, if present.
    focus=['BaseObject','Game','UIControl','UIMultiState','Vehicle','Bike','PhysicsBody','QuadTreeObject']
    bm={x['class']:x for x in out}
    lines=['# Class dossiers — mechanically assembled evidence','',
           'Generated from RTTI, vtables, destructor/thunk analysis, vptr writes, byte-classified functions, and recovered `__FILE__` strings. Source-file rankings are hints, not proof.','']
    for n in focus:
        d=bm.get(n)
        if not d: continue
        lines += [f'## {n}','']
        lines.append('Direct bases: '+(', '.join(d['direct_bases']) if d['direct_bases'] else '(root / none recovered)'))
        lines.append(f"Vtables: {len(d['vtables'])}; vptr write sites: {len(d['vtable_write_sites'])}; direct function targets: {len(d['direct_function_targets'])} ({len(d['function_targets'])} including inherited/shared uses)")
        if d.get('layout_hints'):
            fields=d['layout_hints'].get('fields',[])
            lines.append('Direct layout offsets: '+', '.join(f"+0x{x['offset']:x}" for x in fields))
        if d['source_candidates']:
            lines.append('Top source hints:')
            for s in d['source_candidates'][:4]:
                dist=s.get('nearest_anchor_distance')
                extra=f", nearest evidence/xref distance 0x{dist:x}" if dist is not None else ''
                lines.append(f"- `{s['path']}` — score {s['score']}{extra}; {'; '.join(s['reasons'])}")
        if d['vtables']:
            lines.append('Vtable records:')
            for v in d['vtables']:
                lines.append(f"- `{v['vtable_va']}` at object offset `+0x{int(v.get('object_offset',0)):x}` ({len(v.get('entries',[]))} slots)")
        lines.append('')
    Path('docs/CLASS_DOSSIERS.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps({'class_dossiers':len(out),'classes_with_source_candidates':sum(bool(x['source_candidates']) for x in out),'output':'analysis/class_dossiers.json'},indent=2))

if __name__=='__main__': main()
