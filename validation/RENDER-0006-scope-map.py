import json,shutil
from pathlib import Path
src=Path('C:/Users/ADMIN/Boxer-lab/ms3d')
dst=Path(__file__).parent/'RENDER-0006-prior-map-current'
dst.mkdir(exist_ok=True)
rows=[json.loads(s) for s in (src/'draw-native.jsonl').read_text().splitlines() if s]
wanted=[r for r in rows if r['route']=='replacement'][-4:]
(dst/'draw-native.jsonl').write_text('\n'.join(json.dumps(r) for r in wanted)+'\n')
for row in wanted:
    for side in ('pre','post'):
        name=f"draw-native-replacement-{row['call']:04d}-{side}.bin"
        shutil.copyfile(src/name,dst/name)
print('Scoped last four replacement rows; excluded',len(rows)-len(wanted),'older rows')
