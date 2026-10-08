import json,sys,hashlib
from pathlib import Path
v=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer\validation')
batch=Path(sys.argv[1]);target=Path(sys.argv[2])
source=(v/'GAME-0002-independent-fp-inventory.py').read_text()
ns={};exec(compile(source.split('for number in range(')[0],'<validation FP engine>','exec'),ns)
manifest=json.loads((v/'GAME-0002-independent-approved-clean-source-manifest.json').read_text())
records=[];missing=[]
for item in manifest['records']:
 sid=item['sourceId'];folder=batch/sid;paths=[folder/'damage-replay-original-001.json',folder/'damage-candidate-002.json']
 if not all(p.exists() for p in paths):missing.append(sid);continue
 export=json.loads(Path(item['path']).read_text());a=export['sourceRecord'];b,c=[json.loads(p.read_text()) for p in paths]
 for left,right,label in [(a,b,'source->original'),(b,c,'original->candidate')]:
  for key in ['entry_fp544','exit_fp544']:ns['inspect'](left[key],right[key],f'{sid}:{label}:{key}')
  ns['check'](len(left['events'])==len(right['events']),sid+label,'eventCount')
  for j,(x,y) in enumerate(zip(left['events'],right['events'])):
   for key in ['entry_fp544','exit_fp544']:ns['inspect'](x[key],y[key],f'{sid}:{label}:event{j}:site{hex(x["call_rva"])}:{key}')
 records.append({'sourceId':sid,'exportHash':item['sha256'],'sourceHash':item['sourceSha256'],'originalPath':str(paths[0]),'candidatePath':str(paths[1]),'originalHash':hashlib.sha256(paths[0].read_bytes()).hexdigest(),'candidateHash':hashlib.sha256(paths[1].read_bytes()).hexdigest()})
counts={}
for d in ns['differences']:counts[d['classification']]=counts.get(d['classification'],0)+1
result={'status':'FAIL' if ns['errors'] else 'BLOCKED_MISSING_RECORD' if missing else 'BLOCKED_PROVENANCE' if any(k.startswith('BLOCKED') for k in counts) else 'PASS_FP_SCOPE','checks':ns['checks'],'records':records,'missing':missing,'errors':ns['errors'],'differenceCounts':counts,'rawDifferenceInventory':ns['differences'],'policyHash':hashlib.sha256((v.parent/'specs/gameplay/GAME-0002-typed-fp-locals.md').read_bytes()).hexdigest()}
target.write_text(json.dumps(result,indent=2));print(json.dumps({'status':result['status'],'records':len(records),'checks':result['checks'],'differenceCounts':counts,'missing':missing,'errors':ns['errors'][:10]}))
