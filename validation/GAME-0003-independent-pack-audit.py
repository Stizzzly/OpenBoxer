import json,struct,hashlib
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');V=R/'validation';manifest=json.loads((V/'GAME-0003-approved-native-original-manifest.json').read_text(encoding='utf-8-sig'));packed=json.loads((V/'GAME-0003-packed-replay/manifest.json').read_text());meta=json.loads((R/'specs/gameplay/GAME-0003-observer-metadata.json').read_text());sites={int(s['callRva'],16):s for s in meta['calls']};rows=[];checks=0
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(V/'GAME-0003-approved-native-original-manifest.json')==packed['sourceManifestSha256']=='a281d3c4739ee645a7a45755be5d48b20f10fa2882728f40be844725a4700857'
for record in manifest['records']:
 original=json.loads(Path(record['path']).read_text());pr=next(p for p in packed['records'] if p['id']==record['id']);path=Path(pr['packed']);data=path.read_bytes();assert sha(path)==pr['packedSha256'];pos=0
 def word():
  global pos
  v=struct.unpack_from('<I',data,pos)[0];pos+=4;return v
 def blob():
  global pos
  n=word();v=data[pos:pos+n];pos+=n;assert len(v)==n;return v
 def text():return blob().decode()
 def ck(a,b):
  global checks
  checks+=1;assert a==b,(record['id'],a,b)
 def field():return word(),text(),word()
 ck(word(),0x33474d44);ck(word(),1);ck(text(),record['id']);ck(text(),record['sha256']);ck(text(),record['sourceSha256'])
 for key in ['module','mode','eax']:ck(word(),original[key])
 for key in ['entry_fp544','exit_fp544']:ck(blob(),bytes.fromhex(original[key]))
 for label in ['before','after']:
  s=original[label]
  for key in ['actor232','player176','player_body2288','opponent_body2288','globals','world_list12']:ck(blob(),bytes.fromhex(s[key]))
  for key in ['player_body','opponent_body','world']:ck(word(),s[key])
  n=word();ck(n,len(s['global_span_sizes']));ck([word() for _ in range(n)],s['global_span_sizes'])
 roles={}
 for _ in range(word()):address=word();name=text();assert address not in roles;roles[address]=name
 ck(roles,{int(a):name for a,name in pr['pointerRoles'].items()});ck(word(),len(original['events']))
 for e in original['events']:
  s=sites[e['call_rva']]
  for key in ['call_rva','owner','eax','rng_before','rng_after']:ck(word(),e[key])
  n=word();ck(n,len(e['args']));ck([word() for _ in range(n)],e['args']);owner=field();eax=field();args=[field() for _ in range(n)]
  def resolve(f):
   kind,name,value=f
   if not kind:ck(name,'');return value
   address=next(a for a,role in roles.items() if role==name);return address+(4*value if kind==2 else 0)
  ck(resolve(owner),e['owner']);ck(resolve(eax),e['eax']);ck([resolve(a) for a in args],e['args'])
  definitions=[]
  for name in s['args']:definitions.extend([name]*(4 if 'BY_VALUE' in name else 2 if 'float64' in name else 1))
  ck(len(definitions),n)
  for name,f in zip(definitions,args):ck(bool(f[0]),'pointer' in name)
  for key in ['owner_before','owner_after','returned_bytes','entry_fp544','exit_fp544']:ck(blob(),bytes.fromhex(e[key]))
  for key in ['arg_before','arg_after']:
   for value in e[key]:ck(blob(),bytes.fromhex(value))
 ck(pos,len(data));rows.append({'id':record['id'],'packedSha256':sha(path),'bytes':len(data),'sourceExportSha256':record['sha256'],'events':len(original['events']),'completeEOF':True})
result={'status':'PASS_PACKED_TYPED_SOURCE_WIRE_SCHEMA_ONLY','checks':checks,'records':rows,'sourceManifestSha256':packed['sourceManifestSha256'],'packedManifestSha256':sha(V/'GAME-0003-packed-replay/manifest.json')};(V/'GAME-0003-independent-packed-replay-audit.json').write_text(json.dumps(result,indent=2));print(json.dumps({'status':result['status'],'checks':checks,'records':len(rows),'packedManifestSha256':result['packedManifestSha256']}))
