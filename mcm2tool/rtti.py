from __future__ import annotations
from dataclasses import dataclass
from .pe import PEImage
import struct

@dataclass
class TypeDescriptor:
    va: int
    decorated: str
    name: str

@dataclass
class BaseClass:
    type_va: int
    decorated: str
    name: str
    num_contained_bases: int
    mdisp: int
    pdisp: int
    vdisp: int
    attributes: int

@dataclass
class VTableRef:
    complete_object_locator_va: int
    object_offset: int
    cd_offset: int
    vtables: list[int]

@dataclass
class RTTIClass:
    name: str
    decorated: str
    type_descriptor_va: int
    # Backward-compatible primary COL: prefer the complete-object (offset 0) COL.
    complete_object_locator_va: int
    complete_object_locator_vas: list[int]
    class_hierarchy_va: int
    attributes: int
    bases: list[BaseClass]
    direct_bases: list[str]
    # Flattened list across primary and secondary base-subobject vtables.
    vtables: list[int]
    # Crucial for multiple inheritance: records which vtable belongs to which
    # complete-object-locator/subobject offset.
    vtable_records: list[VTableRef]


def demangle_rtti(name: str) -> str:
    # Handles the overwhelmingly common non-template VC6 RTTI names.
    # Keep complex/template forms losslessly decorated rather than pretending.
    if not name.startswith('.?A'):
        return name
    kindpos = 3
    if len(name) <= kindpos:
        return name
    body = name[kindpos+1:] if name[kindpos] in 'VUTW' else name[kindpos:]
    if body.endswith('@@'):
        body = body[:-2]
    elif body.endswith('@'):
        body = body[:-1]
    if not body or '?$' in body or '?' in body:
        return name
    parts = [p for p in body.split('@') if p]
    if not parts:
        return name
    return '::'.join(reversed(parts))


def find_type_descriptors(pe: PEImage) -> dict[int,TypeDescriptor]:
    out={}
    data=pe.data
    pos=0
    while True:
        pos=data.find(b'.?A',pos)
        if pos<0: break
        end=data.find(b'\0',pos,min(len(data),pos+1024))
        if end<0: break
        raw=data[pos:end]
        try: s=raw.decode('ascii')
        except UnicodeDecodeError:
            pos+=3; continue
        # Type descriptors have two dwords before the decorated type name.
        td_off=pos-8
        if td_off >= 0 and s.startswith('.?A'):
            try: va=pe.offset_to_va(td_off)
            except Exception:
                pos=end+1; continue
            # Type descriptors normally begin with a mapped type_info vftable ptr.
            pvt=pe.u32(td_off)
            if pe.is_va_mapped(pvt):
                out[va]=TypeDescriptor(va,s,demangle_rtti(s))
        pos=end+1
    return out


def _direct_bases(bases: list[BaseClass]) -> list[str]:
    # Base-class arrays are preorder. Element 0 is the complete class itself;
    # numContainedBases tells how large each descendant subtree is.
    direct=[]
    i=1
    while i < len(bases):
        b=bases[i]; direct.append(b.name)
        i += max(1, b.num_contained_bases + 1)
    return direct


def parse_rtti(pe: PEImage) -> list[RTTIClass]:
    tds=find_type_descriptors(pe)
    if not tds: return []

    # A single RTTI type can have several CompleteObjectLocators when the class
    # participates in multiple inheritance. The COL offset is the byte offset of
    # the polymorphic base subobject. Retain every subobject record.
    col_by_type: dict[int,list[tuple[int,int,int,int,int]]] = {}
    for sec in pe.sections:
        if sec.raw_size < 20: continue
        # Exclude executable code; COLs live in read-only/read-write data.
        if sec.characteristics & 0x20000000: continue
        start,end=sec.raw_offset,sec.raw_offset+sec.raw_size-20
        for off in range(start + ((4-start)&3), end+1, 4):
            sig, objoff, cdoff, ptype, pchd = struct.unpack_from('<IIIII',pe.data,off)
            if sig!=0 or ptype not in tds or not pe.is_va_mapped(pchd): continue
            try:
                cho=pe.va_to_offset(pchd)
                chsig,attrs,nbase,pba=struct.unpack_from('<IIII',pe.data,cho)
                if chsig!=0 or not (1 <= nbase <= 512) or not pe.is_va_mapped(pba): continue
            except Exception: continue
            try: colva=pe.offset_to_va(off)
            except Exception: continue
            col_by_type.setdefault(ptype,[]).append((colva,pchd,attrs,objoff,cdoff))

    # Pre-index dword references to COLs in non-code data; a vftable begins immediately after one.
    all_col={c[0] for vals in col_by_type.values() for c in vals}
    vtable_refs: dict[int,list[int]]={c:[] for c in all_col}
    if all_col:
        for sec in pe.sections:
            if sec.raw_size < 8 or sec.characteristics & 0x20000000: continue
            st,en=sec.raw_offset,sec.raw_offset+sec.raw_size-8
            for off in range(st + ((4-st)&3),en+1,4):
                val=pe.u32(off)
                if val in all_col:
                    first=pe.u32(off+4)
                    if pe.is_code_va(first):
                        try: vtable_refs[val].append(pe.offset_to_va(off+4))
                        except Exception: pass

    # Aggregate COL variants that share the same type/hierarchy. This preserves
    # secondary vtables without creating duplicate logical class records.
    groups={}
    for tdva, cols in col_by_type.items():
        td=tds[tdva]
        for colva,pchd,attrs_hint,objoff,cdoff in cols:
            try:
                cho=pe.va_to_offset(pchd)
                sig,attrs,nbase,pba=struct.unpack_from('<IIII',pe.data,cho)
                bao=pe.va_to_offset(pba)
                bases=[]
                ok=True
                for i in range(nbase):
                    bdesc_va=pe.u32(bao+i*4)
                    if not pe.is_va_mapped(bdesc_va): ok=False; break
                    bo=pe.va_to_offset(bdesc_va)
                    btdva,num,md,pd,vd,battrs=struct.unpack_from('<IIiiiI',pe.data,bo)
                    btd=tds.get(btdva)
                    if not btd: ok=False; break
                    bases.append(BaseClass(btdva,btd.decorated,btd.name,num,md,pd,vd,battrs))
                if not ok: continue
                key=(tdva,pchd,tuple((b.type_va,b.mdisp,b.pdisp,b.vdisp,b.attributes) for b in bases))
                g=groups.get(key)
                if g is None:
                    g={
                        'td':td,'pchd':pchd,'attrs':attrs,'bases':bases,
                        'records':[]
                    }
                    groups[key]=g
                g['records'].append(VTableRef(
                    complete_object_locator_va=colva,
                    object_offset=objoff,
                    cd_offset=cdoff,
                    vtables=sorted(set(vtable_refs.get(colva,[]))),
                ))
            except Exception:
                continue

    classes=[]
    for g in groups.values():
        td=g['td']; bases=g['bases']; records=g['records']
        # Stable ordering: primary complete-object vtable first, then increasing subobject offset.
        records=sorted(records,key=lambda r:(r.object_offset!=0,r.object_offset,r.complete_object_locator_va))
        primary=next((r for r in records if r.object_offset==0),records[0])
        all_vtables=sorted({vt for r in records for vt in r.vtables})
        classes.append(RTTIClass(
            name=td.name,
            decorated=td.decorated,
            type_descriptor_va=td.va,
            complete_object_locator_va=primary.complete_object_locator_va,
            complete_object_locator_vas=[r.complete_object_locator_va for r in records],
            class_hierarchy_va=g['pchd'],
            attributes=g['attrs'],
            bases=bases,
            direct_bases=_direct_bases(bases),
            vtables=all_vtables,
            vtable_records=records,
        ))
    classes.sort(key=lambda c:(c.name,c.complete_object_locator_va))
    return classes
