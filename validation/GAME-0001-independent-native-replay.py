import pathlib,json,re,hashlib,sys
stage=sys.argv[1] if len(sys.argv)>1 else 'stage5'
root=pathlib.Path(__file__).parent;checks=0;fail=[]
spans=[(int(a,16),int(n)) for a,n in re.findall(r'\{(0x[0-9a-f]+),(\d+)\}',(root.parent/'replacement/ms3d/strike_capture.hpp').read_text().split('struct Snapshot')[0])]
offsets={};o=0
for r,n in spans:offsets[r]=o;o+=n
def word(b,o):return int.from_bytes(b[o:o+4],'little')
def ck(label,a,b):
 global checks
 checks+=1
 if a!=b:fail.append({'label':label,'source':a,'replay':b})
def fp(h):
 b=bytes.fromhex(h);sw=word(b,2)&65535;top=(sw>>11)&7;tag=b[4]
 return (b[:4].hex(),tag,b[24:28].hex(),[b[32+i*16:42+i*16].hex() for i in range(8) if tag&(1<<((top+i)&7))])
def roles(j):
 g=bytes.fromhex(j['before']['globals']);p=next(e['owner'] for e in j['events'] if e['id']==4)
 return [(p,176,'P'),(word(g,offsets[0x1849a8]),2288,'playerBody'),(word(g,offsets[0x184a90]),2288,'opponentBody'),(j['before']['world'],11000,'world')]+[(0x400000+r,n,f'global{r:x}') for r,n in spans]
def role(x,rs):
 for start,n,name in rs:
  if start<=x<start+n:return (name,x-start)
 return ('opaque',x)
def normalize_bytes(h,positions,rs):
 b=bytearray.fromhex(h);mapped=[]
 for pos in positions:
  mapped.append(role(word(b,pos),rs));b[pos:pos+4]=b'\0'*4
 return (b.hex(),mapped)
def snapshot(s,rs):
 v={k:x for k,x in s.items() if k not in ('fp512','player176','player_body2288','opponent_body2288','globals','active_list','world','head','tail')}
 v['fp']=fp(s['fp512']);v['player']=normalize_bytes(s['player176'],[152],rs)
 for k in ('player_body2288','opponent_body2288'):v[k]=normalize_bytes(s[k],[0,244,2276,2280],rs)
 v['globals']=normalize_bytes(s['globals'],[offsets[x] for x in (0x1849a8,0x184a90,0x1853a4)],rs)
 for k in ('world','head','tail'):v[k]=role(s[k],rs)
 v['active_list']=[(role(n['address'],rs),normalize_bytes(n['data'],[0,244,2276,2280],rs)) for n in s['active_list']]
 return v
def compare(label,a,b):
 ar,br=roles(a),roles(b)
 ck(label+'.entry',snapshot(a['before'],ar),snapshot(b['before'],br));ck(label+'.exit',snapshot(a['after'],ar),snapshot(b['after'],br));ck(label+'.EAX',a['eax'],b['eax']);ck(label+'.events',len(a['events']),len(b['events']))
 for i,(x,y) in enumerate(zip(a['events'],b['events'])):
  tag=label+f'.event{i}';ck(tag+'.id',x['id'],y['id']);ck(tag+'.argmask',x['pointer_arg_bytes'],y['pointer_arg_bytes'])
  pointers=[(x['owner'],y['owner'])] if x['owner'] else []
  for j,n in enumerate(x['pointer_arg_bytes']):
   if n:pointers.append((x['args'][j],y['args'][j]))
  for j,(u,v) in enumerate(pointers):
   if role(u,ar)[0]!='opaque':ck(tag+f'.stablepointer{j}',role(u,ar),role(v,br))
   for k,(uu,vv) in enumerate(pointers[:j]):ck(tag+f'.alias{j}.{k}',u==uu,v==vv)
  for j,(u,v) in enumerate(zip(x['args'],y['args'])):
   if not x['pointer_arg_bytes'][j]:ck(tag+f'.primitive{j}',u,v)
  if x['returned_pointer_size']:
   xr,yr=role(x['eax'],ar),role(y['eax'],br)
   if xr[0]!='opaque':ck(tag+'.returnfixed',xr,yr)
   else:ck(tag+'.returnlocal',[(j,x['eax']-u) for j,(u,v) in enumerate(pointers) if 0<=x['eax']-u<16],[(j,y['eax']-v) for j,(u,v) in enumerate(pointers) if 0<=y['eax']-v<16])
  else:ck(tag+'.EAX',x['eax'],y['eax'])
  for k in ('arg_before','arg_after','owner_before','owner_after','owner_size','returned_bytes','returned_pointer_size','scalar80'):ck(tag+'.'+k,x[k],y[k])
  ck(tag+'.returnFP',fp(x['returned_fp512']),fp(y['returned_fp512']))
  ck(tag+'.entry',snapshot(x['before'],ar),snapshot(y['before'],br));ck(tag+'.exit',snapshot(x['after'],ar),snapshot(y['after'],br))
for n in ((14,) if stage=='stage6' else (10,11) if stage=='stage6resolved' else (6,10,11)):
 source_folder='GAME-0001-stage6-original-natural' if stage=='stage6' else 'GAME-0001-stage3-original-natural'
 source=json.loads((root/source_folder/f'strike-native-{n:03}.json').read_text())
 p=root/(f'GAME-0001-stage6-replay-miss-{n:03}' if stage=='stage6' else f'GAME-0001-stage6-resolved-replay-{n:03}' if stage=='stage6resolved' else f'GAME-0001-stage6-replay-{n:03}' if stage=='stage6hits' else f'GAME-0001-stage5-replay-{n:03}')
 a,b=[json.loads((p/f'strike-replay-{i:03}.json').read_text()) for i in (1,2)]
 compare(f'{n}.original-vs-source',source,a);compare(f'{n}.candidate-vs-source',source,b);compare(f'{n}.paired',a,b)
out={'checks':checks,'failures':fail,'primary':'FAIL' if fail else 'PASS','scope':'source-original natural transcripts vs whole-original and candidate replay; fixed pointer roles only, local per-event aliases; FP excludes instruction pointers/reserved/deadregister bytes'}
(root/f'GAME-0001-independent-{stage}-native-replay.json').write_text(json.dumps(out,indent=2));print(json.dumps({'checks':checks,'failures':len(fail),'first':[{'label':x['label']} for x in fail[:15]]}))
