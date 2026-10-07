"""Generate bounded integer cases; no original assets or implementation input."""
import json
import struct
from pathlib import Path
from importlib.machinery import SourceFileLoader
HERE=Path(__file__).resolve().parent
oracle=SourceFileLoader('anim3_oracle',str(HERE/'ANIM-0003-independent-oracle.py')).load_module()

def base(name, frame=4, start=2, divisor=10, gate=1):
    m=bytearray((i*61+17)&255 for i in range(112))
    for off,val in ((40,gate),(44,1),(48,frame)):
        struct.pack_into('<I',m,off,val&0xffffffff)
    r=bytearray(272)
    struct.pack_into('<II',r,256,start&0xffffffff,divisor&0xffffffff)
    return dict(id=name,model_hex=m.hex(),record_hex=r.hex(),entry_ecx_bits=0x76543210,
                advance_eax=0xfedcba98,time_bits=0x41200000,
                initial_fp=dict(cw=0x27f,sw=0,top=0,tag=0,mxcsr=0x1f80))

def generate():
    out=[base('no_wrap')]
    for f in (6,7,8,9):out.append(base(f'wrap_from_{f}',frame=f))
    for f,s,d,g in ((-1,0,5,1),(-7,-2,5,1),(5,2,-7,1),(-9,-3,-8,-1),
                    (2147483647,0,7,1),(-2147483648,0,7,1),(0,0,1,1),(4,2,1,1)):
        out.append(base(f'signed_{f}_{s}_{d}_{g}',f,s,d,g))
    c=base('gate_zero_unread_poison_record',frame=2147483647,gate=0);c['record_hex']=None;out.append(c)
    c=base('lookup_changes_gate_to_zero');c['lookup_model_writes']=[dict(offset=40,bits=0)];out.append(c)
    c=base('lookup_changes_gate_to_negative',gate=0);c['lookup_model_writes']=[dict(offset=40,bits=0xffffffff)];out.append(c)
    c=base('lookup_mutates_frame_start_divisor');c['lookup_model_writes']=[dict(offset=48,bits=19),dict(offset=44,bits=0xffffffff)]
    c['lookup_record_writes']=[dict(offset=256,bits=3),dict(offset=260,bits=20)];out.append(c)
    for eax in (0,1,0xcccccccc,0xffffffff,0x80000000):
        c=base(f'advance_raw_eax_{eax:08x}');c['advance_eax']=eax;out.append(c)
    c=base('advance_mutates_slots_frame_clock_fp');c['advance_model_writes']=[dict(offset=o,bits=0xa0000000+o) for o in (48,52,56,60,64,68,72,76,80)]
    c['advance_time_bits']=0x7fc12345;c['advance_fp']=dict(cw=0x37f,sw=0x4101,top=0,tag=0,mxcsr=0x3f81);out.append(c)
    for c in out:oracle.prepare(c)
    return out

if __name__=='__main__':
    cases=generate()
    (HERE/'ANIM-0003-independent-vectors.json').write_text(json.dumps(cases,indent=2)+'\n',encoding='utf-8')
    (HERE/'ANIM-0003-independent-expected.json').write_text(json.dumps([dict(id=c['id'],**oracle.prepare(c)) for c in cases],indent=2)+'\n',encoding='utf-8')
    print(f'{len(cases)} oracle-checked bounded typed inputs')
