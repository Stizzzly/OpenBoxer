"""Generate dependency-controlled, bounded inputs without original assets."""
import copy
import json
import struct
from pathlib import Path
from importlib.machinery import SourceFileLoader

HERE = Path(__file__).resolve().parent
oracle = SourceFileLoader('anim2_oracle', str(HERE / 'ANIM-0002-independent-oracle.py')).load_module()

def base(name, bound=3, time=0x42480000):
    model = bytearray((i * 71 + 13) & 255 for i in range(112))
    struct.pack_into('<i', model, 40, bound)
    records = {}
    for index, label in enumerate((b'STAND', b'RUN', b'RUNB', b'OTHER')):
        r = bytearray(272)
        r[:len(label)] = label
        struct.pack_into('<I', r, 256, 100 + index * 33)
        records[str(index)] = r.hex()
    return dict(id=name, model_hex=model.hex(), records=records,
                request_hex=b'RUN\0'.hex(), time_bits=time,
                owner_bits=0x12345678, dependencies=[])

def probe(case, index, result, **mutation):
    case['dependencies'].extend([
        dict(kind='lookup', record=str(index), expect=dict(index=index)),
        dict(kind='compare', result=result, **mutation)])

def match(case, index, second=None, **mutation):
    probe(case, index, 0)
    case['dependencies'].append(dict(kind='lookup', record=str(index if second is None else second),
                                     expect=dict(index=index), **mutation))

def generate():
    cases = [base('bound_zero', 0), base('bound_negative', -7), base('bound_min', -2147483648)]
    c = base('no_match')
    for i, r in enumerate((1, 0xffffffff, 0x80000000)):
        probe(c, i, r)
    cases.append(c)
    for index in range(3):
        c = base(f'match_{index}')
        for i in range(index):
            probe(c, i, 7)
        match(c, index)
        cases.append(c)
    c = base('second_lookup_different_record')
    match(c, 0, 3)
    cases.append(c)
    c = base('compare_shrinks_bound')
    probe(c, 0, 5, model_writes=[dict(offset=40, bits=1)])
    cases.append(c)
    c = base('compare_extends_bound', 1)
    probe(c, 0, -33, model_writes=[dict(offset=40, bits=3)])
    probe(c, 1, 19)
    match(c, 2)
    cases.append(c)
    c = base('lookup_changes_request')
    match(c, 0)
    c['dependencies'][0]['request_hex'] = b'STAND\0'.hex()
    cases.append(c)
    c = base('second_lookup_observes_selector_and_changes_start_time')
    probe(c, 0, 1)
    match(c, 1, 3, model_writes=[dict(offset=44,bits=2), dict(offset=76,bits=0x7f800001)],
          record_writes=[dict(record='3',offset=256,bits=0xfedcba98)], time_bits=0x7fc12345)
    cases.append(c)
    c = base('compare_changes_collection_role')
    probe(c,0,0,collection_base_role='changed_records')
    c['dependencies'].append(dict(kind='lookup',record='3',expect=dict(index=0)))
    cases.append(c)
    c = base('empty_names')
    c['request_hex']='00'
    r=bytearray.fromhex(c['records']['0']);r[0]=0;c['records']['0']=r.hex()
    match(c,0)
    cases.append(c)
    for time in (0, 0x80000000, 0x7f800000, 0xff800000, 0x7f800001, 0x7fc12345, 0xffffffff):
        c=base(f'time_raw_{time:08x}',time=time)
        match(c,0)
        cases.append(c)
    c=base('lookup_shrinks_bound_after_entry')
    match(c,0)
    c['dependencies'][0]['model_writes']=[dict(offset=40,bits=0)]
    cases.append(c)
    c=base('compare_mutates_selector_before_own_store')
    probe(c,0,0,model_writes=[dict(offset=44,bits=0xfffffffe)])
    c['dependencies'].append(dict(kind='lookup',record='3',expect=dict(index=0)))
    cases.append(c)
    for c in cases:
        oracle.select(c)
    return cases

if __name__=='__main__':
    cases=generate()
    (HERE/'ANIM-0002-independent-vectors.json').write_text(json.dumps(cases,indent=2)+'\n',encoding='utf-8')
    (HERE/'ANIM-0002-independent-expected.json').write_text(json.dumps([dict(id=c['id'],**oracle.select(c)) for c in cases],indent=2)+'\n',encoding='utf-8')
    print(f'{len(cases)} oracle-checked bounded typed inputs')
