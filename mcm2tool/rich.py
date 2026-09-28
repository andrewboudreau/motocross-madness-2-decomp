from __future__ import annotations
import struct
from dataclasses import dataclass
from .pe import PEImage

PROD_NAMES = {
  0:'Unknown',1:'Import0',2:'Linker510',3:'Cvtomf510',4:'Linker600',5:'Cvtomf600',6:'Cvtres500',
  7:'Utc11_Basic',8:'Utc11_C',9:'Utc12_Basic',10:'Utc12_C',11:'Utc12_CPP',12:'AliasObj60',
  13:'VisualBasic60',14:'Masm613',15:'Masm710',16:'Linker511',17:'Cvtomf511',18:'Masm614',
  19:'Linker512',20:'Cvtomf512',21:'Utc12_C_Std',22:'Utc12_CPP_Std',23:'Utc12_C_Book',24:'Utc12_CPP_Book'
}

@dataclass(frozen=True)
class RichEntry:
    product_id: int
    product: str
    build: int
    count: int


def parse_rich(pe: PEImage) -> tuple[int,list[RichEntry]] | None:
    data = pe.data[:pe.pe_offset]
    rich = data.find(b'Rich')
    if rich < 0 or rich+8 > len(data):
        return None
    key = struct.unpack_from('<I',data,rich+4)[0]
    start = None
    for off in range(rich-4, 0x3f, -4):
        if (struct.unpack_from('<I',data,off)[0] ^ key) == 0x536e6144:
            start = off; break
    if start is None or start+16 > rich:
        return None
    entries=[]
    for off in range(start+16, rich, 8):
        compid = struct.unpack_from('<I',data,off)[0] ^ key
        count = struct.unpack_from('<I',data,off+4)[0] ^ key
        pid, build = compid >> 16, compid & 0xffff
        entries.append(RichEntry(pid, PROD_NAMES.get(pid,f'Prod{pid}'), build, count))
    return key, entries
