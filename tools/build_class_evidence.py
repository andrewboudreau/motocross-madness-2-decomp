#!/usr/bin/env python3
from __future__ import annotations
import json
from collections import defaultdict
from pathlib import Path


def load(path):
    return json.loads(Path(path).read_text())


def primary_vtable_by_class(vtables):
    out={}
    for v in vtables:
        if int(v.get('object_offset',0))==0:
            out[v['class']]=v
    return out


def primary_direct_base(cls, classes):
    # Prefer the first direct base whose RTTI descriptor is at mdisp 0. This is
    # the base whose primary vtable can safely be compared slot-for-slot.
    direct=set(cls.get('direct_bases',[]))
    for b in cls.get('bases',[])[1:]:
        if b['name'] in direct and int(b.get('mdisp',0))==0:
            return b['name']
    return cls.get('direct_bases',[None])[0] if cls.get('direct_bases') else None


def main():
    classes_list=load('analysis/rtti_classes.json')
    vtables=load('analysis/vtables.json')
    easy=load('analysis/easy_targets.json')
    classes={x['name']:x for x in classes_list}
    pvt=primary_vtable_by_class(vtables)

    # Multiple inheritance / secondary-vtable evidence.
    multi=[]
    for c in classes_list:
        recs=c.get('vtable_records',[])
        if len(c.get('direct_bases',[]))>1 or len(recs)>1:
            multi.append({
                'class':c['name'],
                'direct_bases':c.get('direct_bases',[]),
                'base_descriptors':[
                    {k:b.get(k) for k in ('name','mdisp','pdisp','vdisp','num_contained_bases')}
                    for b in c.get('bases',[])[1:]
                ],
                'vtable_records':recs,
            })
    Path('analysis/multiple_inheritance.json').write_text(json.dumps(multi,indent=2)+'\n')

    # Primary-vtable override map. Secondary subobjects are kept separately in
    # vtables.json; comparing those correctly requires virtual-base adjustment.
    override_rows=[]
    relation_by_use={}
    for name,vt in sorted(pvt.items()):
        cls=classes.get(name,{})
        base=primary_direct_base(cls,classes)
        bvt=pvt.get(base) if base else None
        slots=[]
        for slot,fn in enumerate(vt.get('entries',[])):
            base_fn = bvt['entries'][slot] if bvt and slot < len(bvt.get('entries',[])) else None
            if not bvt:
                rel='root'
            elif base_fn is None:
                rel='introduced'
            elif base_fn.lower()==fn.lower():
                rel='inherited'
            else:
                rel='override'
            row={'slot':slot,'function_va':fn,'relation':rel}
            if base:
                row['base_class']=base
                row['base_function_va']=base_fn
            slots.append(row)
            relation_by_use[(name,0,slot)]=rel
        override_rows.append({
            'class':name,
            'primary_vtable_va':vt['vtable_va'],
            'primary_base':base,
            'slots':slots,
            'summary':{
                'inherited':sum(x['relation']=='inherited' for x in slots),
                'overrides':sum(x['relation']=='override' for x in slots),
                'introduced':sum(x['relation']=='introduced' for x in slots),
                'root':sum(x['relation']=='root' for x in slots),
            }
        })
    Path('analysis/vtable_overrides.json').write_text(json.dumps(override_rows,indent=2)+'\n')

    # Hard layout hints from simple accessors/setters. Only attribute a primary
    # vtable method to a class when it is not merely inherited unchanged.
    hints=defaultdict(lambda:defaultdict(lambda:{'evidence':[]}))
    size_by_kind={
        'get_i32':4,'get_i32_ignore_args':4,'set_i32_arg':4,'set_i32_arg_if_nonzero':4,
        'set_i32_const':4,'set_i32_constants':4,'zero_i32_fields':4,'copy_i32_field':4,'test_i32_nonzero':4,
        'set_i32_const_and_arg':4,
        'get_u8':1,'get_float':4,'address_of_field':None,'sub_i32_fields':4,
        'get_nested_float':4,'get_indexed_i32_stride32':4,'address_of_indexed_stride32':4,
    }
    for e in easy:
        kind=e['kind']; details=e.get('details',{})
        offsets=[]
        if 'offset' in details:
            offsets=[details['offset']]
        elif kind=='sub_i32_fields':
            offsets=[details['lhs_offset'],details['rhs_offset']]
        elif kind=='copy_i32_field':
            offsets=[details['source_offset'],details['dest_offset']]
        elif kind=='get_nested_float':
            offsets=[details['pointer_offset']]
        elif kind=='set_i32_const_and_arg':
            offsets=[details['const_offset'],details['arg_offset']]
        elif kind=='set_i32_constants':
            offsets=[x['offset'] for x in details['stores']]
        elif kind=='zero_i32_fields':
            offsets=list(details['offsets'])
        elif kind in ('get_indexed_i32_stride32','address_of_indexed_stride32'):
            offsets=[details['index_offset'],details['base_offset']]
        if not offsets: continue
        for use in e.get('uses',[]):
            cls=use['class']; objoff=int(use.get('object_offset',0)); slot=int(use['slot'])
            if objoff!=0: continue
            if relation_by_use.get((cls,0,slot))=='inherited': continue
            for off in offsets:
                h=hints[cls][int(off)]
                h['offset']=int(off)
                sz=size_by_kind.get(kind)
                if sz is not None: h['size']=max(int(h.get('size',0)),sz)
                kinds=set(h.get('kinds',[])); kinds.add(kind); h['kinds']=sorted(kinds)
                h['evidence'].append({'target_va':e['target_va'],'slot':slot,'kind':kind,'details':details})
    layout=[]
    for cls,byoff in sorted(hints.items()):
        fields=[byoff[o] for o in sorted(byoff)]
        ends=[f['offset']+f.get('size',0) for f in fields if f.get('size')]
        layout.append({'class':cls,'minimum_size_from_direct_field_evidence':max(ends) if ends else None,'fields':fields})
    Path('analysis/class_layout_hints.json').write_text(json.dumps(layout,indent=2)+'\n')

    print(json.dumps({
        'classes':len(classes_list),
        'vtables':len(vtables),
        'secondary_vtables':sum(int(v.get('object_offset',0))!=0 for v in vtables),
        'multiple_inheritance_or_multi_vtable_classes':len(multi),
        'primary_vtable_maps':len(override_rows),
        'classes_with_direct_layout_hints':len(layout),
        'direct_layout_fields':sum(len(x['fields']) for x in layout),
    },indent=2))

if __name__=='__main__': main()
