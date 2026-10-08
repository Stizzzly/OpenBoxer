import sys,json,hashlib
from pathlib import Path
v=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer\validation')
data=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-fixture-observations-stage4')
# Reuse the validation-owned generic typed comparator, without its replay loop.
sys.argv=[__file__,str(data),str(v/'unused.json')]
source=(v/'GAME-0002-independent-compare-clean-replay.py').read_text()
ns={};exec(compile(source.split("for item in MANIFEST['records']:")[0],'<validation comparator>','exec'),ns)
paths=[data/'damage-guard-reference-121.json',data/'damage-guard-candidate-122.json']
if not all(p.exists() for p in paths):
    paths=[next(data.glob('*reference*121.json')),next(data.glob('*candidate*122.json'))]
a,b=[json.loads(p.read_text()) for p in paths]
ns['compare'](a,b,'wrong-caller fallback:')
report=(data/'damage-fixtures.txt').read_text()
for test in range(22):
    line=next((l for l in report.splitlines() if l.startswith(f'guard={test} ')),'')
    ns['eq'](all(w in line for w in ['read_only=1','full_fp_exact=1','no_callbacks=1','pass=1']),True,f'guard{test} report')
for witness in ['guard_dispatch candidate=1 replacement=0 fallback=1 route=1 abi=1 equivalent_state_sites_eax=1','cleanup observer_restored=1 failures=0']:
    ns['eq'](witness in report,True,'report '+witness)
result={'status':'FAIL' if ns['errors'] else 'PASS_GUARDS_FALLBACK_ARCHITECTURAL_STATE_PENDING_PROVENANCE','checks':ns['checks'],'errors':ns['errors'],'paths':[{'path':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in paths],'equivalence':'BLOCKED_RAW_PROVENANCE_AND_REQUIRED_NATURAL_COMPOSED_EVIDENCE'}
(v/'GAME-0002-independent-stage4-fallback.json').write_text(json.dumps(result,indent=2))
print(json.dumps({'status':result['status'],'checks':result['checks'],'errors':result['errors'][:10]}))
