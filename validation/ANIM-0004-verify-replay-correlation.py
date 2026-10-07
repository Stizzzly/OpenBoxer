import json,collections
from pathlib import Path
p=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d');out=[]
def rows(path):return [json.loads(x) for x in path.read_text().splitlines() if x.strip()]
for side in ('original','replacement'):
 for kind in ('selector','pose'):
  native=rows(p/f'action-native-{side}'/('action-native.jsonl' if kind=='selector' else 'frames-native.jsonl'))
  replay=rows(p/(f'action-stage4-replay-{side}' if kind=='selector' else f'action-stage4-pose-replay-{side}')/('action-replays.jsonl' if kind=='selector' else 'frames-replays.jsonl'))
  def nativekey(n):
   if kind=='selector':return (tuple(n['before_model_words']),n['records_hex'],n['request_hex'],n['clock_bits'])
   return (tuple(n['before_model_words']),n['record_hex'],n['opaque_entry_ecx'],n['clock_bits'])
  def replaykey(r):
   if kind=='selector':return (tuple(r['initial_model_words']),r['initial_records_hex'],r['initial_request_hex'],r['initial_clock_bits'])
   return (tuple(r['initial_model_words']),r['initial_record_hex'],r['opaque_entry_ecx'],r['initial_clock_bits'])
  fail=[];bykey=collections.defaultdict(list)
  for n in native:bykey[nativekey(n)].append(n)
  for r in replay:
   matching=bykey[replaykey(r)]
   if not matching:fail.append('FAIL_STATE missing captured input');continue
   n=matching.pop()
   for s in ('original','candidate'):
    if r[s+'_model_words']!=n['after_model_words'] or r[s+'_clock_bits']!=n['after_clock_bits']:fail.append('FAIL_STATE captured output')
    if kind=='selector':
     if r[s+'_eax_role_or_bits']!=n['eax_role_or_bits'] or r[s+'_events']!=n['events']:fail.append('FAIL_CALLBACK_ORDER captured selector')
    else:
     if r[s+'_eax']!=n['eax'] or r[s+'_prepared_model_words']!=n['prepared_model_words']:fail.append('FAIL_STATE captured pose boundary/EAX')
  if any(bykey.values()):fail.append('BLOCKED unreplayed captures')
  out.append(dict(side=side,kind=kind,cases=len(replay),failures=fail))
report=dict(result='PASS' if all(not x['failures'] for x in out) else 'FAIL',checks=out)
Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer\validation\ANIM-0004-independent-replay-correlation.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))
