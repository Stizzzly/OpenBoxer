"""Check authorized typed fixture/native JSONL; never opens original modules."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from importlib.machinery import SourceFileLoader
HERE=Path(__file__).resolve().parent
oracle=SourceFileLoader('anim2_typed_oracle',str(HERE/'ANIM-0002-independent-oracle.py')).load_module()

def model_hex(words):
    if len(words)!=28:raise ValueError('model112 missing')
    return struct.pack('<28I',*words).hex()

def case_for(row,events,native=False):
    raw=bytes.fromhex(row['records_hex' if native else 'initial_records_hex'])
    if len(raw)%272:raise ValueError('record stride')
    records={str(i):raw[272*i:272*(i+1)].hex() for i in range(len(raw)//272)}
    if not native and row['mutation'] in (6,10):
        for i in range(len(raw)//272):
            alt=bytearray(raw[272*i:272*(i+1)]);struct.pack_into('<I',alt,256,700+i)
            records[str(i+3)]=alt.hex()
    c=dict(model_hex=model_hex(row['before_model_words' if native else 'initial_model_words']),
           request_hex=row['request_hex' if native else 'initial_request_hex']+'00',
           time_bits=row['clock_bits' if native else 'initial_clock_bits'],
           owner_bits=0xffffffff,records=records,dependencies=[])
    mutation=0 if native else row['mutation'];lookups=0
    for ev in events:
        a={'kind':ev['name']}
        if ev['name']=='lookup':
            lookups+=1;a.update(record=str(ev['record_role']),expect={'index':ev['index']})
            if mutation==1 and lookups==1:a['request_hex']=b'beta\0'.hex()
            if mutation in (2,3) and lookups==1:a['model_writes']=[dict(offset=40,bits=1 if mutation==2 else 3)]
            if mutation==4 and lookups==2:a.update(model_writes=[dict(offset=44,bits=91)],time_bits=0x80000000)
            if mutation==5 and lookups==2:a.update(record_writes=[dict(record=str(ev['record_role']),offset=256,bits=555)],time_bits=0x7fc12345)
        elif ev['name']=='compare':
            a['result']=ev['result']
            if mutation in (8,9):a['model_writes']=[dict(offset=40,bits=1 if mutation==8 else 3)]
            if mutation==10 and ev['result']==0:a.update(model_writes=[dict(offset=44,bits=77)],time_bits=0xdeadbeef)
            if mutation==11 and ev['result']==0:a.update(record_writes=[dict(record=str(lookups-1),offset=256,bits=666)],request_hex=b'changed\0'.hex(),time_bits=0)
        else:raise ValueError('unknown callback')
        c['dependencies'].append(a)
    return c

def canonical(events):
    out=[]
    for e in events:
        if e['kind']=='lookup':out.append(dict(name='lookup',collection_role='model+84',record_role=int(e['return_record_role']),index=e['index'],result=0))
        else:out.append(dict(name='compare',collection_role='model+84',record_role=int(e['record_role']),index=0,result=e['result'],record_name_hex=e['record_name_hex'],request_hex=e['request_name_hex']))
    return out

def verify(row,native=False):
    result={'case':row.get('case',row.get('call')),'failures':[]}
    fail=result['failures']
    sides=[('native',row['events'])] if native else [('original',row['original_events']),('candidate',row['candidate_events'])]
    predictions=[]
    for side,events in sides:
        try:
            prediction=oracle.select(case_for(row,events,native));predictions.append(prediction)
        except Exception as e:
            fail.append(f'FAIL_CALLBACK_ORDER {side}: {e}');continue
        actual_model=row['after_model_words' if native else side+'_model_words']
        if prediction['model_hex']!=model_hex(actual_model):fail.append(f'FAIL_STATE {side} model112')
        actual_time=row['after_clock_bits' if native else side+'_clock_bits']
        if prediction['time_bits']!=actual_time:fail.append(f'FAIL_STATE {side} clock')
        actual_eax=row['eax_role_or_bits' if native else side+'_eax_role_or_bits']
        if prediction['eax']!=actual_eax:fail.append(f'FAIL_RETURN_VALUE {side}')
        if canonical(prediction['callbacks'])!=events:fail.append(f'FAIL_CALLBACK_ORDER {side} arguments/order')
    if native:
        if row['initial_fp']!=row['final_fp']:fail.append('FAIL_FLOAT native')
        if not row['pointer_words_unchanged']:fail.append('FAIL_STATE pointer witnesses')
        if row['caller_rva'] not in (0x371be,0x371df,0x37200):fail.append('FAIL_CALLBACK_ORDER route')
        if row['owner_role']!='registered-global-owner' or row['model_role']!='owner+644':fail.append('FAIL_STATE native roles')
        if row['route']=='replacement' and not row['replacement_calls']:fail.append('BLOCKED activation witness')
    else:
        for key in ('state_equal','callback_equal','return_equal','fp_equal','abi_ok','captured_state_reproduced'):
            if not row.get(key):fail.append('FAIL_'+key.upper())
        if row['candidate_initial_fp']!=row['candidate_final_fp']:fail.append('FAIL_FLOAT candidate')
        if row['differential']:
            if row['original_initial_fp']!=row['candidate_initial_fp'] or row['original_final_fp']!=row['candidate_final_fp']:fail.append('FAIL_FLOAT cross')
            if len(predictions)==2 and predictions[0]!=predictions[1]:fail.append('FAIL_STATE oracle cross')
    result['result']='PASS' if not fail else 'FAIL'
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('input');p.add_argument('output');p.add_argument('--native',action='store_true');a=p.parse_args()
    raw=Path(a.input).read_bytes();rows=[json.loads(x) for x in raw.decode('utf-8-sig').splitlines() if x.strip()]
    results=[verify(r,a.native) for r in rows]
    report=dict(result='PASS' if results and all(r['result']=='PASS' for r in results) else 'FAIL',
                confidence='CONFIRMED',source=a.input,source_sha256=hashlib.sha256(raw).hexdigest().upper(),
                cases=len(results),checks=results)
    Path(a.output).write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report))
