from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import struct

@dataclass
class CoffSection:
    index:int; name:str; raw_size:int; raw_ptr:int; reloc_ptr:int; reloc_count:int; characteristics:int

@dataclass
class CoffSymbol:
    index:int; name:str; value:int; section_number:int; type:int; storage_class:int; aux_count:int
    function_size:int|None = None

@dataclass
class CoffRelocation:
    section_number:int; virtual_address:int; symbol_index:int; type:int

class CoffError(ValueError): pass

class CoffObject:
    def __init__(self,path:str|Path):
        self.path=Path(path); self.data=self.path.read_bytes()
        if len(self.data)<20: raise CoffError('too small')
        self.machine,self.section_count,self.timestamp,self.sym_ptr,self.sym_count,self.opt_size,self.characteristics=struct.unpack_from('<HHIIIHH',self.data,0)
        if self.opt_size != 0: raise CoffError('expected COFF object, not PE image')
        self._str_off=self.sym_ptr+self.sym_count*18
        self._str_size=struct.unpack_from('<I',self.data,self._str_off)[0] if self._str_off+4<=len(self.data) else 4
        sh=20+self.opt_size
        self.sections=[]
        raw_headers=[]
        for i in range(self.section_count):
            off=sh+i*40
            name8=self.data[off:off+8]
            _,_,raw_size,raw_ptr,reloc_ptr,_,reloc_count,_,ch=struct.unpack_from('<IIIIIIHHI',self.data,off+8)
            raw_headers.append((name8,raw_size,raw_ptr,reloc_ptr,reloc_count,ch))
        for i,(name8,raw_size,raw_ptr,reloc_ptr,reloc_count,ch) in enumerate(raw_headers,1):
            name=self._decode_name8(name8)
            self.sections.append(CoffSection(i,name,raw_size,raw_ptr,reloc_ptr,reloc_count,ch))
        self.symbols=[]
        i=0
        while i<self.sym_count:
            off=self.sym_ptr+i*18
            name8=self.data[off:off+8]
            value,secnum,typ,sc,aux=struct.unpack_from('<IhHBB',self.data,off+8)
            name=self._decode_symbol_name(name8)
            function_size = None
            if i + aux >= self.sym_count:
                raise CoffError('truncated auxiliary symbol records')
            # PE/COFF auxiliary format 1: TotalSize excludes section alignment.
            # VC6 emits this record with /Z7. Do not guess by trimming NOPs or
            # by borrowing the retail function's requested size.
            if secnum > 0 and typ == 0x20 and sc == 2 and aux:
                function_size = struct.unpack_from('<I', self.data, off + 18 + 4)[0]
            self.symbols.append(CoffSymbol(i,name,value,secnum,typ,sc,aux,function_size))
            i += 1+aux
        self.symbol_by_index={s.index:s for s in self.symbols}
        self.relocations=[]
        for sec in self.sections:
            for r in range(sec.reloc_count):
                off=sec.reloc_ptr+r*10
                va,si,typ=struct.unpack_from('<IIH',self.data,off)
                self.relocations.append(CoffRelocation(sec.index,va,si,typ))
        # Compiler-generated functions (e.g. scalar deleting destructors) have
        # no auxiliary TotalSize, but /Z7 still emits a CodeView procedure
        # record with the code length. Use it only where the aux size is absent.
        # The relocation may target an undefined external entry for the same
        # name (VC6 does this for ??_G); resolve it to the single definition.
        defined:dict[str,list[CoffSymbol]]={}
        for s in self.symbols:
            if s.section_number>0 and s.type==0x20: defined.setdefault(s.name,[]).append(s)
        for index,size in self._codeview_proc_sizes().items():
            s=self.symbol_by_index.get(index)
            if s is None: continue
            if s.section_number<=0:
                matches=defined.get(s.name,[])
                if len(matches)!=1: continue
                s=matches[0]
            if s.function_size is None and s.section_number>0 and s.type==0x20:
                s.function_size=size

    # CodeView S_LPROC32/S_GPROC32 (32-bit type index) and their CV4 forms
    # (16-bit type index): record type -> offset of the `off` field after
    # reclen/rectyp. `len` always follows pParent/pEnd/pNext.
    _CV_PROC_OFF={0x100a:28,0x100b:28,0x0204:26,0x0205:26}
    _IMAGE_REL_I386_SECREL=0x000b

    def _codeview_proc_sizes(self)->dict[int,int]:
        """Map function symbol index -> CodeView procedure length.

        The procedure is tied to its function by the SECREL relocation on the
        record's `off` field. Conflicting lengths for one symbol are dropped.
        """
        sizes:dict[int,int]={}; conflicts:set[int]=set()
        for sec in self.sections:
            if not sec.name.startswith('.debug$S'): continue
            b=self.data[sec.raw_ptr:sec.raw_ptr+sec.raw_size]
            secrel={r.virtual_address:r.symbol_index for r in self.relocations
                    if r.section_number==sec.index and r.type==self._IMAGE_REL_I386_SECREL}
            # Per-object symbol sections start with a CV signature; per-COMDAT ones do not.
            pos=4 if len(b)>=4 and struct.unpack_from('<I',b,0)[0] in (1,2,4) else 0
            while pos+4<=len(b):
                reclen,rectyp=struct.unpack_from('<HH',b,pos)
                if reclen<2 or pos+2+reclen>len(b): break
                off_field=self._CV_PROC_OFF.get(rectyp)
                if off_field is not None and reclen>=2+off_field+4:
                    length=struct.unpack_from('<I',b,pos+4+12)[0]
                    index=secrel.get(pos+4+off_field)
                    if index is not None:
                        if index in sizes and sizes[index]!=length: conflicts.add(index)
                        sizes[index]=length
                pos+=2+reclen
        for index in conflicts: del sizes[index]
        return sizes

    def _str(self,offset:int)->str:
        pos=self._str_off+offset
        if pos<self._str_off+4 or pos>=min(len(self.data),self._str_off+self._str_size): return f'<badstr:{offset}>'
        end=self.data.find(b'\0',pos,min(len(self.data),self._str_off+self._str_size))
        if end<0:end=min(len(self.data),self._str_off+self._str_size)
        return self.data[pos:end].decode('utf-8','replace')
    def _decode_name8(self,b:bytes)->str:
        raw=b.split(b'\0',1)[0]
        if raw.startswith(b'/') and raw[1:].isdigit(): return self._str(int(raw[1:]))
        return raw.decode('ascii','replace')
    def _decode_symbol_name(self,b:bytes)->str:
        zeroes,off=struct.unpack('<II',b)
        if zeroes==0 and off: return self._str(off)
        return b.split(b'\0',1)[0].decode('utf-8','replace')
    def section(self,n:int)->CoffSection:
        if n<=0 or n>len(self.sections): raise CoffError(f'invalid section {n}')
        return self.sections[n-1]
    def find_symbol(self,query:str)->CoffSymbol:
        exact=[s for s in self.symbols if s.name==query and s.section_number>0]
        if len(exact)==1:return exact[0]
        partial=[s for s in self.symbols if query in s.name and s.section_number>0]
        if len(partial)==1:return partial[0]
        if not partial and not exact: raise CoffError(f'no symbol matches {query!r}')
        choices=exact or partial
        raise CoffError('ambiguous symbol: '+', '.join(s.name for s in choices[:20]))
    def symbol_extent(self,s:CoffSymbol)->tuple[bytes,int,list[CoffRelocation]]:
        sec=self.section(s.section_number)
        starts=sorted({x.value for x in self.symbols if x.section_number==s.section_number and x.value>s.value and x.storage_class in (2,3,105)})
        end=starts[0] if starts else sec.raw_size
        if s.function_size is not None:
            declared_end = s.value + s.function_size
            if s.function_size <= 0 or declared_end > min(end, sec.raw_size):
                raise CoffError(f'invalid function size for {s.name}: {s.function_size}')
            end = declared_end
        if end<s.value: raise CoffError('bad symbol extent')
        raw=self.data[sec.raw_ptr+s.value:sec.raw_ptr+end]
        rel=[r for r in self.relocations if r.section_number==sec.index and s.value<=r.virtual_address<end]
        return raw,end-s.value,rel

def alignment_padding(cand:bytes,target_size:int,sym_offset:int,align:int=16,fill:int=0x90)->int:
    """Diagnose a possible alignment tail; never use this as a match boundary.

    Target-dependent NOP trimming is not proof of a function's extent. Matchers
    use symbol_extent and its compiler-emitted auxiliary length instead.

    Without COMDAT sections, VC6 pads each function in .text to a 16-byte
    boundary with NOPs, so a symbol's extent (up to the next symbol or section
    end) can exceed the real function. The excess is filler only if it is all
    `fill`, shorter than `align`, and ends on an `align` boundary. Returns the
    filler length, or 0 when the tail is anything else.
    """
    extra=len(cand)-target_size
    if extra<=0 or extra>=align or (sym_offset+len(cand))%align: return 0
    return extra if all(b==fill for b in cand[target_size:]) else 0

RELOC_WIDTH_I386={0x0000:0,0x0001:2,0x0002:2,0x0006:4,0x0007:4,0x0009:2,0x000A:2,0x000B:4,0x000C:4,0x0014:4}

def relocation_mask(obj:CoffObject,s:CoffSymbol,size:int,relocs:list[CoffRelocation])->bytearray:
    mask=bytearray(size)
    for r in relocs:
        w=RELOC_WIDTH_I386.get(r.type,4)
        start=r.virtual_address-s.value
        for i in range(max(0,start),min(size,start+w)):mask[i]=1
    return mask
