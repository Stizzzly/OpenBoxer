"""Integer future-slot oracle derived only from approved ANIM-0003 contract."""
import argparse
import json
import struct
from pathlib import Path

def read(buf, offset):
    return struct.unpack_from('<I', buf, offset)[0]

def signed(value):
    value &= 0xffffffff
    return value - 0x100000000 if value & 0x80000000 else value

def write(buf, offset, bits):
    if not 0 <= offset <= len(buf)-4:
        raise ValueError('typed write outside buffer')
    struct.pack_into('<I', buf, offset, bits & 0xffffffff)

def mutations(buf, items):
    for item in items:
        write(buf, item['offset'], item['bits'])

def remainder(value, divisor):
    if divisor == 0 or (value == -2147483648 and divisor == -1):
        raise ValueError('original exceptional division excluded')
    q = abs(value) // abs(divisor)
    if (value < 0) != (divisor < 0):
        q = -q
    return value - q * divisor

def prepare(case):
    model = bytearray.fromhex(case['model_hex'])
    if len(model) != 112:
        raise ValueError('model112 required')
    record = None if case.get('record_hex') is None else bytearray.fromhex(case['record_hex'])
    if record is not None and len(record) != 272:
        raise ValueError('record272 required')
    index = signed(read(model, 44))
    callbacks = [dict(kind='lookup', this_role='model+84', index=index,
                      model_hex_before=model.hex(), return_role=case.get('record_role','selected_record'))]
    mutations(model, case.get('lookup_model_writes', []))
    if record is not None:
        mutations(record, case.get('lookup_record_writes', []))
    callbacks[0]['model_hex_after'] = model.hex()
    start, divisor = 0, 1
    if read(model,40) != 0:
        if record is None:
            raise ValueError('nonzero gate requires typed readable record')
        start, divisor = signed(read(record,256)), signed(read(record,260))
    own_writes = []
    def slot(offset, value):
        r = remainder(signed(value + 1), divisor)
        write(model,offset,r)
        own_writes.append(dict(offset=offset,bits=r & 0xffffffff))
        if r == 0:
            write(model,offset,start)
            own_writes.append(dict(offset=offset,bits=start & 0xffffffff))
    slot(52, signed(read(model,48)))
    write(model,60,read(model,52));own_writes.append(dict(offset=60,bits=read(model,60)))
    slot(64,signed(read(model,60)))
    slot(68,signed(read(model,64)))
    slot(72,signed(read(model,68)))
    prepared = model.hex()
    callbacks.append(dict(kind='advance',model_role='same_model',ecx_bits=case['entry_ecx_bits'] & 0xffffffff,
                          model_hex_before=prepared,eax=case['advance_eax'] & 0xffffffff))
    mutations(model,case.get('advance_model_writes',[]))
    callbacks[-1]['model_hex_after']=model.hex()
    return dict(model_hex=model.hex(),prepared_model_hex=prepared,eax=case['advance_eax'] & 0xffffffff,
                callbacks=callbacks,own_writes=own_writes,
                time_bits=case.get('advance_time_bits',case.get('lookup_time_bits',case.get('time_bits',0))) & 0xffffffff,
                fp_after=case.get('advance_fp',case.get('lookup_fp',case.get('initial_fp'))))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('input');p.add_argument('output');a=p.parse_args()
    cases=json.loads(Path(a.input).read_text(encoding='utf-8-sig'))
    Path(a.output).write_text(json.dumps([dict(id=c['id'],**prepare(c)) for c in cases],indent=2)+'\n',encoding='utf-8')
