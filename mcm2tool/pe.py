from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import struct

@dataclass(frozen=True)
class Section:
    name: str
    virtual_size: int
    virtual_address: int
    raw_size: int
    raw_offset: int
    characteristics: int

    @property
    def rva_end(self) -> int:
        return self.virtual_address + max(self.virtual_size, self.raw_size)

    @property
    def file_end(self) -> int:
        return self.raw_offset + self.raw_size

class PEFormatError(ValueError):
    pass

class PEImage:
    def __init__(self, path: str | Path):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        if self.data[:2] != b'MZ':
            raise PEFormatError('not an MZ executable')
        self.pe_offset = self.u32(0x3c)
        if self.data[self.pe_offset:self.pe_offset+4] != b'PE\0\0':
            raise PEFormatError('missing PE signature')
        fh = self.pe_offset + 4
        self.machine, self.number_of_sections, self.timestamp, self.symbol_table_ptr, self.number_of_symbols, self.optional_header_size, self.characteristics = struct.unpack_from('<HHIIIHH', self.data, fh)
        oh = fh + 20
        magic = self.u16(oh)
        if magic != 0x10B:
            raise PEFormatError(f'only PE32 supported (magic={magic:#x})')
        self.linker_major = self.data[oh+2]
        self.linker_minor = self.data[oh+3]
        self.entry_rva = self.u32(oh+16)
        self.image_base = self.u32(oh+28)
        self.section_alignment = self.u32(oh+32)
        self.file_alignment = self.u32(oh+36)
        self.size_of_image = self.u32(oh+56)
        self.size_of_headers = self.u32(oh+60)
        self.subsystem = self.u16(oh+68)
        self.number_of_rva_and_sizes = self.u32(oh+92)
        dd = oh + 96
        self.directories: list[tuple[int,int]] = []
        for i in range(min(self.number_of_rva_and_sizes, 16)):
            self.directories.append(struct.unpack_from('<II', self.data, dd + i*8))
        sh = oh + self.optional_header_size
        self.sections: list[Section] = []
        for i in range(self.number_of_sections):
            off = sh + i*40
            name = self.data[off:off+8].split(b'\0',1)[0].decode('ascii','replace')
            vs, va, rs, ro = struct.unpack_from('<IIII', self.data, off+8)
            ch = self.u32(off+36)
            self.sections.append(Section(name, vs, va, rs, ro, ch))

    def u16(self, off: int) -> int:
        return struct.unpack_from('<H', self.data, off)[0]
    def i16(self, off: int) -> int:
        return struct.unpack_from('<h', self.data, off)[0]
    def u32(self, off: int) -> int:
        return struct.unpack_from('<I', self.data, off)[0]
    def i32(self, off: int) -> int:
        return struct.unpack_from('<i', self.data, off)[0]

    def section_for_rva(self, rva: int) -> Section | None:
        for s in self.sections:
            if s.virtual_address <= rva < s.rva_end:
                return s
        if 0 <= rva < self.size_of_headers:
            return None
        return None

    def rva_to_offset(self, rva: int) -> int:
        if 0 <= rva < self.size_of_headers:
            return rva
        s = self.section_for_rva(rva)
        if not s:
            raise PEFormatError(f'RVA {rva:#x} not mapped')
        delta = rva - s.virtual_address
        if delta >= s.raw_size:
            raise PEFormatError(f'RVA {rva:#x} has no file backing')
        return s.raw_offset + delta

    def va_to_offset(self, va: int) -> int:
        return self.rva_to_offset(va - self.image_base)

    def offset_to_rva(self, off: int) -> int:
        if 0 <= off < self.size_of_headers:
            return off
        for s in self.sections:
            if s.raw_offset <= off < s.file_end:
                return s.virtual_address + (off - s.raw_offset)
        raise PEFormatError(f'file offset {off:#x} not mapped')

    def offset_to_va(self, off: int) -> int:
        return self.image_base + self.offset_to_rva(off)

    def is_va_mapped(self, va: int) -> bool:
        try:
            self.va_to_offset(va); return True
        except Exception:
            return False

    def is_code_va(self, va: int) -> bool:
        rva = va - self.image_base
        s = self.section_for_rva(rva)
        # IMAGE_SCN_CNT_CODE (0x20) or IMAGE_SCN_MEM_EXECUTE (0x20000000)
        return bool(s and (s.characteristics & 0x20 or s.characteristics & 0x20000000))

    def read_c_string_at_offset(self, off: int, limit: int=4096) -> str:
        end = self.data.find(b'\0', off, min(len(self.data), off+limit))
        if end < 0: end = min(len(self.data), off+limit)
        return self.data[off:end].decode('latin1','replace')

    def read_c_string_rva(self, rva: int, limit: int=4096) -> str:
        return self.read_c_string_at_offset(self.rva_to_offset(rva), limit)

    def imports(self) -> dict[str, list[str]]:
        if len(self.directories) < 2:
            return {}
        imp_rva, imp_size = self.directories[1]
        if not imp_rva:
            return {}
        out: dict[str,list[str]] = {}
        off = self.rva_to_offset(imp_rva)
        while True:
            oft, ts, fc, name_rva, ft = struct.unpack_from('<IIIII', self.data, off)
            if not any((oft,ts,fc,name_rva,ft)):
                break
            dll = self.read_c_string_rva(name_rva)
            names: list[str] = []
            thunk_rva = oft or ft
            to = self.rva_to_offset(thunk_rva)
            while True:
                val = self.u32(to); to += 4
                if val == 0: break
                if val & 0x80000000:
                    names.append(f'ordinal:{val & 0xffff}')
                else:
                    no = self.rva_to_offset(val)
                    names.append(self.read_c_string_at_offset(no+2))
            out[dll] = names
            off += 20
        return out

    def bytes_at_va(self, va: int, size: int) -> bytes:
        off = self.va_to_offset(va)
        return self.data[off:off+size]
