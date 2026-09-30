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
    codeview_size:int|None = None

    @property
    def extent_source(self):
        if self.function_size is not None: return 'coff_function_aux'
        if self.codeview_size is not None: return 'codeview_proc'
        return 'symbol_or_section_boundary'

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
        self._read_codeview_sizes()

    def _read_codeview_sizes(self):
        """Read legacy VC6 procedure records, bound by i386 COFF relocations.

        VC6 omits function auxiliary records for generated deleting destructors,
        but /Z7 still emits S_[GL]PROC32_ST with an independent procedure length.
        Layout: microsoft/microsoft-pdb include/cvinfo.h, PROCSYM32. No retail
        bytes, target lengths, debug display names, or padding heuristics enter
        this association. Other CodeView generations are deliberately ignored.
        """
        if self.machine != 0x14c:
            return
        sections = [sec for sec in self.sections if sec.name == '.debug$S']
        signature = struct.pack('<I', 2)  # CV_SIGNATURE_C11
        if not any(self.data[sec.raw_ptr:sec.raw_ptr+4] == signature for sec in sections):
            return
        for sec in sections:
            raw = self.data[sec.raw_ptr:sec.raw_ptr+sec.raw_size]
            if len(raw) != sec.raw_size:
                raise CoffError('truncated CodeView section')
            pos = 4 if raw.startswith(signature) else 0
            rels = [r for r in self.relocations if r.section_number == sec.index]
            while pos < len(raw):
                if pos + 4 > len(raw):
                    raise CoffError('truncated CodeView record header')
                length, kind = struct.unpack_from('<HH', raw, pos)
                end = pos + 2 + length
                if length < 2 or end > len(raw):
                    raise CoffError('invalid CodeView record length')
                if kind in (0x100a, 0x100b):  # S_LPROC32_ST, S_GPROC32_ST
                    if length < 38 or pos + 40 + raw[pos+39] > end:
                        raise CoffError('truncated CodeView procedure record')
                    offsets = [r for r in rels if r.virtual_address == pos+32]
                    segments = [r for r in rels if r.virtual_address == pos+36]
                    if (len(offsets) == len(segments) == 1 and
                            offsets[0].type == 0x000b and segments[0].type == 0x000a and
                            offsets[0].symbol_index == segments[0].symbol_index and
                            raw[pos+32:pos+38] == b'\0' * 6):
                        ref = self.symbol_by_index.get(offsets[0].symbol_index)
                        if ref is None:
                            raise CoffError('CodeView references missing/auxiliary symbol')
                        # VC6 may reference an undefined duplicate of the defined
                        # function symbol. Require a unique exact COFF name.
                        matches = [s for s in self.symbols if s.name == ref.name and
                                   s.section_number > 0 and s.type == 0x20]
                        if len(matches) != 1:
                            raise CoffError('ambiguous or missing CodeView function definition')
                        sym = matches[0]
                        size = struct.unpack_from('<I', raw, pos+16)[0]
                        if ((sym.function_size is not None and sym.function_size != size) or
                                (sym.codeview_size is not None and sym.codeview_size != size)):
                            raise CoffError(f'conflicting function lengths for {sym.name}')
                        sym.codeview_size = size
                pos = end

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
        size = s.function_size if s.function_size is not None else s.codeview_size
        if size is not None:
            declared_end = s.value + size
            if size <= 0 or declared_end > min(end, sec.raw_size):
                raise CoffError(f'invalid function size for {s.name}: {size}')
            end = declared_end
        if end<s.value: raise CoffError('bad symbol extent')
        raw=self.data[sec.raw_ptr+s.value:sec.raw_ptr+end]
        rel=[r for r in self.relocations if r.section_number==sec.index and s.value<=r.virtual_address<end]
        return raw,end-s.value,rel

def alignment_padding(cand:bytes,target_size:int,sym_offset:int,align:int=16,fill:int=0x90)->int:
    """Diagnose a possible alignment tail; never use this as a match boundary.

    Target-dependent NOP trimming is not proof of a function's extent. Matchers
    use symbol_extent and its compiler-emitted auxiliary/CodeView length instead.

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
