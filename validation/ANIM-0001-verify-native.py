"""Validate each native typed observation against independent spec oracle.
Natural original/candidate calls need not have identical wall-clock inputs.
Clean-identical CPU replay remains the separate original-vs-candidate boundary.
"""
import importlib.util,json,struct,sys
from pathlib import Path
directory=Path(__file__).parent
spec=importlib.util.spec_from_file_location('oracle',directory/'ANIM-0001-independent-oracle.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
rows=[json.loads(line) for line in Path(sys.argv[1]).read_text(encoding='utf-8-sig').splitlines() if line.strip()]
failures=[];witnesses=[]
for row in rows:
    case=dict(model_hex=struct.pack('<28I',*row['before_model_words']).hex(),time_bits=row['clock_bits'],
              count=row['count'],rate=row['rate'],initial_sw=row['sw'])
    expected=oracle.advance(case)
    for field,actual in dict(model_hex=struct.pack('<28I',*row['after_model_words']).hex(),
                             eax=row['eax'],time_bits=row['after_clock_bits'],condition_bits=row['after_sw']&0x4700).items():
        if expected[field]!=actual:
            failures.append(dict(call=row['call'],route=row['route'],field=field,expected=expected[field],actual=actual))
    if not row.get('pointer_words_unchanged',False):
        failures.append(dict(call=row['call'],field='FAIL_STATE_POINTER_WITNESS'))
    if row['cw']!=0x027f or row['top']!=row['after_top'] or row['abridged_tag']!=row['after_abridged_tag'] or row['mxcsr']!=row['after_mxcsr']:
        failures.append(dict(call=row['call'],field='FAIL_FLOAT_ENVIRONMENT'))
    if (row['sw']&0x3f)&~(row['after_sw']&0x3f):
        failures.append(dict(call=row['call'],field='FAIL_FLOAT_STICKY_CLEARED'))
    if row['predicted_steps']!=expected['steps'] or expected['steps']>4:
        failures.append(dict(call=row['call'],field='FAIL_STATE_NATIVE_SCOPE'))
    events=row['events']
    if [e['name'] for e in events]!=[e['kind'] for e in expected['callbacks']]:
        failures.append(dict(call=row['call'],field='FAIL_CALLBACK_ORDER'))
    elif len(events)>1 and events[1]['args']!=[row['index']]:
        failures.append(dict(call=row['call'],field='FAIL_CALLBACK_INDEX'))
    witnesses.append(dict(route=row['route'],call=row['call'],steps=expected['steps'],before_sw=row['sw'],after_sw=row['after_sw'],
                          added_sticky=(row['after_sw']&0x3f)&~(row['sw']&0x3f),initial_mxcsr=row['mxcsr'],final_mxcsr=row['after_mxcsr']))
report=dict(result='FAIL' if failures else 'PASS',confidence='HIGH',cases=len(rows),failures=failures,witnesses=witnesses,
            scope='Each naturally observed typed call vs exact semantic/condition oracle; no claim of identical wall-clock original/candidate input or captured SW reconstruction')
print(json.dumps(report,indent=2));sys.exit(bool(failures))
