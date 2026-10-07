"""Validate coordinator-authorized typed ANIM4 archives; never opens original modules."""
import json,struct,hashlib,argparse
from pathlib import Path
from importlib.machinery import SourceFileLoader
HERE=Path(__file__).parent
select=SourceFileLoader('a4sel',str(HERE/'ANIM-0002-verify-typed.py')).load_module()
pose=SourceFileLoader('a4pose',str(HERE/'ANIM-0003-verify-typed.py')).load_module()
META=json.loads((HERE.parent/'specs/animation/ANIM-0004-harness-metadata.json').read_text())
ROLES={r['state']:r for r in META['new_roles']}
ROLES[0]=dict(state=0,name='LEGS_STAND',return_rva='0x371BE',literal_rva='0x1623E8')
def rows(p):return [json.loads(x) for x in p.read_text(encoding='utf-8-sig').splitlines() if x.strip()]
def pack(w):return struct.pack('<28I',*w)
def sha(raw):return hashlib.sha256(raw).hexdigest().upper()
def verify(directory):
 d=Path(directory);ss=rows(d/'action-native.jsonl');ps=rows(d/'frames-native.jsonl');cs=rows(d/'action-composed.jsonl');checks=[]
 seen={s:0 for s in ROLES}; sm={r['call']:r for r in ss};pm={r['call']:r for r in ps}
 for r in ss:
  state=r['action_state'];fail=[];role=ROLES.get(state)
  if not role:fail.append('FAIL_STATE unapproved role')
  else:
   seen[state]+=1
   if r['caller_rva']!=int(role['return_rva'],16) or r['literal_rva']!=int(role['literal_rva'],16) or bytes.fromhex(r['request_hex'])!=role['name'].encode():fail.append('FAIL_CALLBACK_ORDER exact role/literal/name')
  out=select.verify(r,True);fail += [f for f in out['failures'] if f!='FAIL_CALLBACK_ORDER route']
  if not r.get('lifetime_witness'):fail.append('BLOCKED lifetime witness')
  raw=(d/f'action-native-{r["route"]}-{r["call"]:04d}.bin').read_bytes();h=struct.unpack_from('<9I',raw);off=148+r['count']*272;size=len(bytes.fromhex(r['request_hex']))+1
  expected=(0x32504c43,1,r['count'],size,r['clock_bits'],r['after_clock_bits'],r['eax_role_or_bits'],r['initial_fp']['cw'],r['initial_fp']['sw'])
  if h!=expected or raw[36:148]!=pack(r['before_model_words']) or raw[148:off].hex()!=r['records_hex'] or raw[off:off+size]!=bytes.fromhex(r['request_hex'])+b'\0' or raw[off+size:]!=pack(r['after_model_words']):fail.append('FAIL_STATE selector binary integrity')
  checks.append(dict(kind='selector',call=r['call'],state=state,sha256=sha(raw),failures=fail))
 for c in cs:
  fail=[];s=sm.get(c['selector_call']);p=pm.get(c['frames_call'])
  if not s or not p:fail.append('BLOCKED correlation')
  else:
   fail+=pose.verify(p,True)['failures']
   if c['role_state']!=s['action_state'] or c['selector_replacement']!=(s['route']=='replacement') or c['frames_route']!=p['route']:fail.append('FAIL_STATE role/side correlation')
   if s['after_model_words']!=p['before_model_words']:fail.append('FAIL_STATE selectorpost != posebefore112')
   for field in ('before_model_words','prepared_model_words','after_model_words','eax'):
    if c[field]!=p[field]:fail.append('FAIL_STATE composed '+field)
   if c['selected_index']!=s['after_model_words'][11] or c['selected_anchor_bits']!=s['after_model_words'][20]:fail.append('FAIL_STATE selected stamp')
   index=s['after_model_words'][11];record=bytes.fromhex(s['records_hex'])[272*index:272*(index+1)]
   if record.hex()!=p['record_hex']:fail.append('FAIL_STATE selected record correlation')
   if p['route']=='replacement':
    if (c['frames_replacements_delta'],c['animation_replacements_delta'],c['trusted_continuations_delta'])!=(1,1,1):fail.append('BLOCKED activation deltas')
   elif any(c[k] for k in ('frames_replacements_delta','animation_replacements_delta','trusted_continuations_delta')):fail.append('FAIL_STATE original activation')
   raw=(d/c['pose_file']).read_bytes();h=struct.unpack_from('<12I',raw)
   expected=(0x334d5246,1,p['clock_bits'],p['after_clock_bits'],p['eax'],p['opaque_entry_ecx'],p['initial_fp']['cw'],p['initial_fp']['sw'],p['before_model_words'][11],p['animation_replacements_before'],p['animation_replacements_after'],p['trusted_continuations_delta'])
   if len(raw)!=656 or h!=expected or raw[48:160]!=pack(p['before_model_words']) or raw[160:432].hex()!=p['record_hex'] or raw[432:544]!=pack(p['prepared_model_words']) or raw[544:]!=pack(p['after_model_words']):fail.append('FAIL_STATE pose binary integrity')
  checks.append(dict(kind='composition',selector_call=c['selector_call'],frames_call=c['frames_call'],state=c['role_state'],failures=fail))
 failures=[]
 if len(ss)>16 or any(n>2 for n in seen.values()):failures.append('FAIL_SIDE_EFFECT capture budget')
 if len(ss)!=len(cs) or len(cs)!=len(ps):failures.append('BLOCKED unmatched observations')
 if len(sm)!=len(ss) or len(pm)!=len(ps):failures.append('FAIL_STATE duplicate IDs')
 missing=[s for s,n in seen.items() if not n]
 if missing:failures.append('BLOCKED missing roles '+str(missing))
 return dict(result='PASS' if not failures and all(not c['failures'] for c in checks) else 'FAIL',scope='typed native selector/composed observations; numerical replay separately required',role_counts=seen,failures=failures,checks=checks,source_hashes={name:sha((d/name).read_bytes()) for name in ('action-native.jsonl','frames-native.jsonl','action-composed.jsonl')})
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('directory');a.add_argument('output');args=a.parse_args();out=verify(args.directory);Path(args.output).write_text(json.dumps(out,indent=2));print(json.dumps({k:v for k,v in out.items() if k!='checks'}))
