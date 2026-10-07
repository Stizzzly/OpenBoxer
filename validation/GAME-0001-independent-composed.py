import pathlib,json,struct,hashlib,importlib.util
root=pathlib.Path(__file__).parent;p=root/'GAME-0001-stage6-composed-natural'
def module(name,path):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
selector=module('selector',root/'ANIM-0002-verify-typed.py');pose=module('pose',root/'ANIM-0003-verify-typed.py')
def rows(n):return [json.loads(x) for x in (p/n).read_text().splitlines() if x]
ss,ps,cs=map(rows,('action-native.jsonl','frames-native.jsonl','action-composed.jsonl'))
sm={r['call']:r for r in ss};pm={r['call']:r for r in ps};fails=[];checks=0;manifest=[]
def ck(label,a,b):
 global checks
 checks+=1
 if a!=b:fails.append(label)
meta=json.loads((root.parent/'specs/animation/ANIM-0004-harness-metadata.json').read_text());roles={r['state']:r for r in meta['new_roles']};roles[0]={'name':'LEGS_STAND','return_rva':'0x371BE','literal_rva':'0x1623E8'}
for s in ss:
 tag=f'selector{s["call"]}';role=roles[s['action_state']]
 ck(tag+'.returnRVA',s['caller_rva'],int(role['return_rva'],16));ck(tag+'.literalRVA',s['literal_rva'],int(role['literal_rva'],16));ck(tag+'.name',bytes.fromhex(s['request_hex']),role['name'].encode())
 ck(tag+'.oracle',[f for f in selector.verify(s,True)['failures'] if f!='FAIL_CALLBACK_ORDER route'],[]);ck(tag+'.lifetime',s['lifetime_witness'],True);ck(tag+'.route',s['route'],'replacement')
for c in cs:
 s=sm[c['selector_call']];r=pm[c['frames_call']];tag=f'composed{c["frames_call"]}'
 ck(tag+'.oracle',pose.verify(r,True)['failures'],[]);ck(tag+'.selectorPost/posePre',s['after_model_words'],r['before_model_words']);ck(tag+'.state',c['role_state'],s['action_state']);ck(tag+'.selectorReplacement',c['selector_replacement'],True);ck(tag+'.framesRoute',r['route'],'replacement')
 for k in ('before_model_words','prepared_model_words','after_model_words','eax'):ck(tag+'.'+k,c[k],r[k])
 ck(tag+'.selectedIndex',c['selected_index'],s['after_model_words'][11]);ck(tag+'.anchor',c['selected_anchor_bits'],s['after_model_words'][20]);ck(tag+'.deltas',tuple(c[k] for k in ('frames_replacements_delta','animation_replacements_delta','trusted_continuations_delta')),(1,1,1))
 record=bytes.fromhex(s['records_hex'])[272*c['selected_index']:272*(c['selected_index']+1)];ck(tag+'.record',record.hex(),r['record_hex'])
 path=p/c['pose_file'];raw=path.read_bytes();manifest.append({'file':path.name,'sha256':hashlib.sha256(raw).hexdigest()})
 ck(tag+'.binLength',len(raw),656);ck(tag+'.binHeader',struct.unpack_from('<12I',raw),(0x334d5246,1,r['clock_bits'],r['after_clock_bits'],r['eax'],r['opaque_entry_ecx'],r['initial_fp']['cw'],r['initial_fp']['sw'],r['before_model_words'][11],r['animation_replacements_before'],r['animation_replacements_after'],r['trusted_continuations_delta']))
 ck(tag+'.binaryBefore',raw[48:160],struct.pack('<28I',*r['before_model_words']));ck(tag+'.binaryRecord',raw[160:432],record);ck(tag+'.binaryPrepared',raw[432:544],struct.pack('<28I',*r['prepared_model_words']));ck(tag+'.binaryAfter',raw[544:],struct.pack('<28I',*r['after_model_words']))
ck('unique linked counts',(len(ss),len(ps),len(cs),len(sm),len(pm)),(16,16,16,16,16));ck('all roles',sorted(set(c['role_state'] for c in cs)),[0,3,4,5,6,7,8,11])
out={'primary':'FAIL' if fails else 'PASS','checks':checks,'failures':fails,'scope':'same frozen GAME0001+ANIM4/2selector,ANIM3frame preparation,ANIM1advancement natural correlation and approved typed numeric oracles;16pose binary integrity','pid':4700,'poses':manifest,'record_hashes':{n:hashlib.sha256((p/n).read_bytes()).hexdigest() for n in ('action-native.jsonl','frames-native.jsonl','action-composed.jsonl')}}
(root/'GAME-0001-independent-stage6-composed.json').write_text(json.dumps(out,indent=2));print(json.dumps({'checks':checks,'failures':fails}))
