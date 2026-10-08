import json,hashlib,re,sys,pefile
from pathlib import Path
v=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer\validation');batch=Path(sys.argv[1]) if len(sys.argv)>1 else Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-clean-replay-observations-stage4')
manifest=json.loads((v/'GAME-0002-independent-approved-clean-source-manifest.json').read_text());errors=[];checks=0;rows=[]
stage5='stage5' in str(batch)
stage6='stage6' in str(batch)
dllhash='19838F07440A79D071A91C343AE2BBDA28D4B903A2D9190F2A78124C31BBEE47' if stage5 else 'E34F66520A6FFB963FD7F5968DDA96DA5F0C39B6CB278264977E6FE729CA7A2E'
if stage6:dllhash='B10C2E1D5CF4E422D8A83D2F7470D0E4463200CE4D8BDD09CD7BE4AA1B32052F'

def ck(ok,label):
 global checks
 checks+=1
 if not ok:errors.append(label)
for item in manifest['records']:
 sid=item['sourceId'];folder=batch/sid;path=folder/'damage-replay-prepatch-observation.json';p=json.loads(path.read_text());report=(folder/'damage-replay.txt').read_text();out=(folder/'damage-replay-launcher.out').read_text()
 for key,value in [('module_base','0x400000'),('thunk_target','0x41C4E0'),('original_sha256','77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6'),('live_original_route_confirmed',True),('dependency_call_targets_verified',63),('natural_caller_verified',True),('audio_import_module','OpenAL32.dll'),('audio_import_resolved',True),('replacement_module_sha256',dllhash),('replacement_module_size',7196672 if stage6 else 2293760 if stage5 else 2289664)]:ck(p.get(key)==value,sid+key)
 ck(p['replacement_module_path'].endswith(('damage-fixture-stage6' if stage6 else 'damage-fixture-stage5' if stage5 else 'damage-fixture-stage4')+'\\ms3d_replacement.dll'),sid+'DLLpath')
 ck(int(p['replacement_module_base'],16)>0 and int(p['replacement_module_base'],16)%65536==0,sid+'loaded aligned base')
 ck(int(p['original_audio_api_pointer'],16)!=0,sid+'APIpointer')
 lines=[line for line in report.splitlines() if line.startswith('source=')];ck(len(lines)==2,sid+'reportrows')
 for candidate,line in enumerate(lines):
  for token in [f'source={sid}',f'exportHash={item["sha256"]}',f'sourceHash={item["sourceSha256"]}',f'candidate={candidate}','typed_errors=0','abi=1',f'replacement={candidate}','fallback=0','route=1','source_state=1','source_eax=1']:
   ck(token in line,sid+'report '+token)
 ck('cleanup observer_restored=1 failures=0' in report,sid+'cleanup')
 ck('fixture worker result=0' in out,sid+'worker')
 rows.append({'sourceId':sid,'moduleProofPath':str(path),'moduleProofSha256':hashlib.sha256(path.read_bytes()).hexdigest(),'reportPath':str(folder/'damage-replay.txt'),'reportSha256':hashlib.sha256((folder/'damage-replay.txt').read_bytes()).hexdigest(),'replacementLoadedBase':p['replacement_module_base'],'replacementHash':p['replacement_module_sha256']})
result={'status':'PASS_ROOT_TYPED_MODULE_REPORT_PROVENANCE_ONLY' if not errors else 'FAIL_PROVENANCE','checks':checks,'errors':errors,'records':rows,'confidence':'CONFIRMED typed manifest consistency; process facts are root-owned observations, not Agent3 direct process inspection','equivalence':'Independent semantic comparison is separately authoritative; producer report alone does not establish equivalence'}
(Path(sys.argv[2]) if len(sys.argv)>2 else v/'GAME-0002-independent-clean-replay-provenance.json').write_text(json.dumps(result,indent=2));print(json.dumps({'status':result['status'],'checks':checks,'errors':errors}))
