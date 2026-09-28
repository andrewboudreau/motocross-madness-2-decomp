from __future__ import annotations
import struct


def rel32_target(va:int, blob:bytes, opcode_offset:int) -> int:
    rel=struct.unpack_from('<i',blob,opcode_offset+1)[0]
    return va + opcode_offset + 5 + rel


def scalar_deleting_destructor(va:int, blob:bytes):
    # Canonical VC6 scalar deleting destructor wrapper observed throughout MCM2:
    # push esi; mov esi,ecx; call dtor; test byte ptr [esp+8],1; je ...;
    # push esi; call operator delete; add esp,4; mov eax,esi; pop esi; ret 4
    if len(blob)<30: return None
    checks=[
        (0,b'\x56\x8b\xf1\xe8'),
        (8,b'\xf6\x44\x24\x08\x01\x74\x09\x56\xe8'),
        (21,b'\x83\xc4\x04\x8b\xc6\x5e\xc2\x04\x00'),
    ]
    if not all(blob[o:o+len(p)]==p for o,p in checks): return None
    return {
        'kind':'scalar_deleting_destructor',
        'size':30,
        'destructor_va':rel32_target(va,blob,3),
        'operator_delete_va':rel32_target(va,blob,16),
    }


def this_adjustor_thunk(va:int, blob:bytes):
    # Fixed non-virtual-base adjustment: sub ecx, imm32; jmp target
    if len(blob)>=11 and blob[:2]==b'\x81\xe9' and blob[6]==0xE9:
        adj=struct.unpack_from('<i',blob,2)[0]
        return {'kind':'this_adjustor_thunk','size':11,'adjustment':-adj,'target_va':rel32_target(va,blob,6),'virtual_base_adjust':False}
    # Compact imm8 form.
    if len(blob)>=8 and blob[:2]==b'\x83\xe9' and blob[3]==0xE9:
        imm=struct.unpack_from('<b',blob,2)[0]
        return {'kind':'this_adjustor_thunk','size':8,'adjustment':-imm,'target_va':rel32_target(va,blob,3),'virtual_base_adjust':False}
    # VC6 virtual-base adjustor: sub ecx,[ecx-4]; jmp target
    if len(blob)>=8 and blob[:3]==b'\x2b\x49\xfc' and blob[3]==0xE9:
        return {'kind':'this_adjustor_thunk','size':8,'adjustment':'-[this-4]','target_va':rel32_target(va,blob,3),'virtual_base_adjust':True}
    # Virtual-base adjustment followed by an additional fixed subtraction.
    if len(blob)>=14 and blob[:3]==b'\x2b\x49\xfc' and blob[3:5]==b'\x81\xe9' and blob[9]==0xE9:
        adj=struct.unpack_from('<i',blob,5)[0]
        return {'kind':'this_adjustor_thunk','size':14,'adjustment':f'-[this-4]-0x{adj:x}','fixed_subtract':adj,'target_va':rel32_target(va,blob,9),'virtual_base_adjust':True}
    return None
