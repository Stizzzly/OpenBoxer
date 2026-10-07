"""Spec-oracle validation of authorized unit/composed/natural typed records."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from importlib.machinery import SourceFileLoader
HERE=Path(__file__).resolve().parent
prep=SourceFileLoader('anim3_typed',str(HERE/'ANIM-0003-independent-oracle.py')).load_module()
advance=SourceFileLoader('anim1_composed',str(HERE/'ANIM-0001-independent-oracle.py')).load_module()

def mh(words):
    if len(words)!=28:raise ValueError('model112 required')
    return struct.pack('<28I',*words).hex()

def input_case(row,native):
    c=dict(model_hex=mh(row['before_model_words' if native else 'initial_model_words']),
           record_hex=row['record_hex' if native else 'initial_record_hex'],
           time_bits=row['clock_bits' if native else 'initial_clock_bits'],
           entry_ecx_bits=row['opaque_entry_ecx'],advance_eax=0)
    mutation=0 if native else row['mutation']
    if mutation==1:c['lookup_model_writes']=[dict(offset=40,bits=0)];c['record_hex']=None
    if mutation==2:c['lookup_model_writes']=[dict(offset=40,bits=1)]
    if mutation==3:c['lookup_model_writes']=[dict(offset=48,bits=9)]
    if mutation==4:c['lookup_record_writes']=[dict(offset=256,bits=4),dict(offset=260,bits=7)]
    if mutation==5:c['lookup_model_writes']=[dict(offset=44,bits=91)]
    if mutation==6:
        rec=bytearray.fromhex(c['record_hex']);mod=bytes.fromhex(c['model_hex'])
        rec[256:264]=mod[48:56];c['record_hex']=rec.hex()
    return c

def verify(row,native=False):
    failures=[];check=dict(case=row.get('case',row.get('call')),failures=failures)
    c=input_case(row,native);p=prep.prepare(c);prepared=p['prepared_model_hex']
    composed=native or row['composed'];mutation=0 if native else row['mutation']
    sides=['native'] if native else (['original','candidate'] if row['differential'] else ['candidate'])
    for side in sides:
        events=row['events' if native else side+'_events']
        if mh(row['prepared_model_words' if native else side+'_prepared_model_words'])!=prepared:failures.append('FAIL_STATE '+side+' prepared112')
        fp_initial=row['initial_fp' if native else side+'_initial_fp']
        fp_final=row['final_fp' if native else side+'_final_fp']
        if composed:
            count_events=[e for e in events if e['name']=='count']
            if len(count_events)!=1:failures.append('FAIL_CALLBACK_ORDER count');count=1
            else:count=count_events[0]['result']
            rate=struct.unpack_from('<i',bytes.fromhex(c['record_hex']),268)[0]
            a=advance.advance(dict(model_hex=prepared,time_bits=c['time_bits'],rate=rate,count=count,initial_sw=fp_initial['sw']))
            expected_model=a['model_hex'];expected_eax=a['eax'];expected_time=a['time_bits']
            if fp_final['sw']&0x4700!=a['condition_bits']:failures.append('FAIL_FLOAT '+side+' condition')
            if fp_final['sw']&fp_initial['sw']&0x3f!=fp_initial['sw']&0x3f:failures.append('FAIL_FLOAT '+side+' sticky preservation')
            for f in ('cw','top','abridged_tag','mxcsr'):
                if fp_final[f]!=fp_initial[f]:failures.append('FAIL_FLOAT '+side+' '+f)
        else:
            expected_eax=0 if row['case']==21 else (0xcccccccc if row['case']==22 else 0xfedcba98)
            amended=dict(c,advance_eax=expected_eax)
            if mutation==7:amended.update(advance_model_writes=[dict(offset=64,bits=0xaabbccdd),dict(offset=48,bits=444)],advance_time_bits=0x7fc12345)
            answer=prep.prepare(amended);expected_model=answer['model_hex'];expected_time=answer['time_bits']
            expected_fp=dict(fp_initial)
            if mutation==8:expected_fp.update(cw=0x37f,mxcsr=0x3f80)
            if mutation==9:expected_fp.update(sw=0x3800,top=7,abridged_tag=0x80)
            if fp_final!=expected_fp:failures.append('FAIL_FLOAT '+side+' callback environment')
        if mh(row['after_model_words' if native else side+'_model_words'])!=expected_model:failures.append('FAIL_STATE '+side+' final112')
        if row['eax' if native else side+'_eax']!=expected_eax:failures.append('FAIL_RETURN_VALUE '+side)
        if row['after_clock_bits' if native else side+'_clock_bits']!=expected_time:failures.append('FAIL_STATE '+side+' clock')
        expected_names=['lookup','advancement-enter']
        if native and row['route']=='original':expected_names+=['reference-advance']
        if composed:expected_names+=['count']+(['lookup'] if count else [])
        if not native:expected_names+=['advancement-return']
        if [e['name'] for e in events]!=expected_names:failures.append('FAIL_CALLBACK_ORDER '+side+' names')
        for event_position,e in enumerate(events):
            if e['name']=='lookup':
                # First lookup reads original selector; nested advance reads prepared selector.
                which=0 if event_position==0 else 1
                expected_index=struct.unpack_from('<I',bytes.fromhex(c['model_hex'] if which==0 else prepared),44)[0]
                if e['args']!=[expected_index] or e['role']!='model+84':failures.append('FAIL_CALLBACK_ORDER '+side+' lookup args')
            elif e['name']=='advancement-enter':
                if e['args']!=[row['opaque_entry_ecx']]+list(struct.unpack('<28I',bytes.fromhex(prepared))) or e['role']!='model':failures.append('FAIL_CALLBACK_ORDER '+side+' prepared/ECX args')
            elif e['name']=='advancement-return' and (e['result']!=expected_eax or e['args']):failures.append('FAIL_RETURN_VALUE advancement recorder')
        if not native and row[side+'_advance_ecx']!=row['opaque_entry_ecx']:failures.append('FAIL_CALLBACK_ORDER '+side+' ECX')
    if native:
        if row['caller_rva']!=0x5f0d:failures.append('FAIL_CALLBACK_ORDER native route')
        if row['route']=='replacement':
            if row['animation_replacements_after']-row['animation_replacements_before']!=1 or row['trusted_continuations_delta']!=1 or row['replacement_calls']<1:failures.append('BLOCKED composed activation witness')
    else:
        for k in ('state_equal','callback_equal','return_equal','ecx_equal','fp_equal','abi_ok','captured_state_reproduced'):
            if not row[k]:failures.append('FAIL_'+k.upper())
        if row['differential'] and (row['original_initial_fp']!=row['candidate_initial_fp'] or row['original_final_fp']!=row['candidate_final_fp']):failures.append('FAIL_FLOAT full cross FP')
    check['result']='PASS' if not failures else 'FAIL';return check

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('input');p.add_argument('output');p.add_argument('--native',action='store_true');a=p.parse_args()
    raw=Path(a.input).read_bytes();rows=[json.loads(x) for x in raw.decode('utf-8-sig').splitlines() if x.strip()]
    checks=[verify(row,a.native) for row in rows]
    report=dict(result='PASS' if checks and all(x['result']=='PASS' for x in checks) else 'FAIL',confidence='CONFIRMED',source=a.input,source_sha256=hashlib.sha256(raw).hexdigest().upper(),cases=len(checks),checks=checks)
    Path(a.output).write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))
