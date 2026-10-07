"""Read typed fixture JSONL only; independently compare both sides to spec oracle."""
import importlib.util
import json
import struct
import sys
from pathlib import Path

directory=Path(__file__).parent
module_spec=importlib.util.spec_from_file_location('oracle',directory/'ANIM-0001-independent-oracle.py')
oracle=importlib.util.module_from_spec(module_spec)
module_spec.loader.exec_module(oracle)
rows=[json.loads(line) for line in Path(sys.argv[1]).read_text(encoding='utf-8-sig').splitlines() if line.strip()]
failures=[]
status=[]
for row in rows:
    case=dict(model_hex=struct.pack('<28I',*row['initial_model_words']).hex(),
              time_bits=row['initial_clock_bits'],count=row['count'],rate=row['initial_rate'])
    mutation=row.get('mutation',0)
    if mutation==1:
        case.update(count_time_bits=oracle.store_float(211),count_model_writes=[dict(offset=80,bits=oracle.store_float(11)),dict(offset=44,bits=2)])
    if mutation==2:
        case.update(count_time_bits=oracle.store_float(900),count_model_writes=[dict(offset=48,bits=0xaaaa5555)])
    if mutation==3:
        case.update(lookup_time_bits=oracle.store_float(2000),lookup_model_writes=[dict(offset=80,bits=oracle.store_float(300)),dict(offset=60,bits=555)],lookup_rate=30)
    expected=oracle.advance(case)
    for side in ['candidate']+(['original'] if row['differential'] else []):
        fields=dict(model_hex=struct.pack('<28I',*row[side+'_model_words']).hex(),eax=row[side+'_eax'],time_bits=row[side+'_clock_bits'])
        for field,actual in fields.items():
            if expected[field]!=actual:
                failures.append(dict(case=row['case'],side=side,field=field,expected=expected[field],actual=actual))
        events=row[side+'_events']
        if [e['name'] for e in events]!=[e['kind'] for e in expected['callbacks']]:
            failures.append(dict(case=row['case'],side=side,field='callback_order'))
        elif len(events)>1 and events[1]['args']!=[expected['callbacks'][1]['index']&0xffffffff]:
            failures.append(dict(case=row['case'],side=side,field='lookup_index'))
    if not row['abi_ok']:
        failures.append(dict(case=row['case'],field='ABI'))
    if row['differential']:
        if row['original_sw']!=row['candidate_sw']:
            failures.append(dict(case=row['case'],field='FAIL_FLOAT_FULL_SW',expected=row['original_sw'],actual=row['candidate_sw']))
        for side in ['original','candidate']:
            if row[side+'_sw']&0x4700 != expected['condition_bits']:
                failures.append(dict(case=row['case'],side=side,field='FAIL_FLOAT_CONDITION_ORACLE',expected=expected['condition_bits'],actual=row[side+'_sw']&0x4700))
        status.append(dict(case=row['case'],original_sw=row['original_sw'],candidate_sw=row['candidate_sw'],
                           condition_diff=(row['original_sw']^row['candidate_sw'])&0x4700,
                           sticky_diff=(row['original_sw']^row['candidate_sw'])&0x3f,
                           full_diff=row['original_sw']^row['candidate_sw']))
        if 'original_initial_fp' in row:
            if row['original_initial_fp']!=row['candidate_initial_fp']:
                failures.append(dict(case=row['case'],field='FAIL_FLOAT_INITIAL_ENVIRONMENT'))
            old,new=row['original_final_fp'],row['candidate_final_fp']
            status[-1]['final_environment_differences']={k:{'original':old[k],'candidate':new[k]} for k in old if old[k]!=new[k]}
            for field in ['cw','top','abridged_tag','mxcsr']:
                if old[field]!=new[field]:
                    failures.append(dict(case=row['case'],field='FAIL_FLOAT_FINAL_'+field,expected=old[field],actual=new[field]))
    if row.get('has_captured_output') and not row['captured_state_reproduced']:
        failures.append(dict(case=row['case'],field='FAIL_STATE_CAPTURED_OUTPUT'))
report=dict(result='FAIL' if failures else 'PASS',confidence='HIGH',cases=len(rows),
            differential=all(r['differential'] for r in rows),failures=failures,
            fp_status_diagnostic_witnesses=status,
            limitation='Full SW EXACT per 2026-10-05 supplement; mismatches split condition/sticky diagnostics. Native replay uses explicit clean-identical environment, not captured SW recreation')
print(json.dumps(report,indent=2))
sys.exit(bool(failures))
