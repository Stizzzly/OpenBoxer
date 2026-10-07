"""Independent captured-source original/candidate replay validation."""
import argparse,importlib.util,json
from pathlib import Path
here=Path(__file__).parent
s=importlib.util.spec_from_file_location('native',here/'RENDER-0006-check-native.py');native=importlib.util.module_from_spec(s);s.loader.exec_module(native)
def check(directory):
    rows=[json.loads(s) for s in (directory/'character-replays.jsonl').read_text().splitlines()]
    results=[]
    sources=list(directory.glob('character-native-*-pre.bin'))
    for row in rows:
        matched=[]
        for p in sources:
            pre,h,case=native.snapshot(p)
            if list(h[2:])==[row['frame_bounds'],row['vertices_per_frame'],row['triangles'],row['frame_a'],row['frame_b'],row['factor_bits'],row['original_owner_identity'],row['original_model_identity']]:matched.append((p,pre,h,case))
        if not matched:raise ValueError('no corresponding native capture')
        p,pre,h,case=matched[0];expected=native.oracle.expected(case);errors=[]
        route='replacement' if '-replacement-' in p.name else 'original'
        call=int(p.name.split('-')[-2])
        archived=Path('C:/Users/ADMIN/Boxer-lab/ms3d')/('character-native-'+route)
        native_rows=[json.loads(s) for s in (archived/'character-native.jsonl').read_text().splitlines()]
        native_row=[r for r in native_rows if r['route']==route and r['call']==call][-1]
        if (archived/p.name).read_bytes()!=pre:errors.append('replay input differs from native capture')
        events=[]
        for e in expected['events']:
            event=dict(name=e['api'],args=e['args'],role='',result_role='',result=0)
            if e['api']=='meshGate':event.update(name='mesh_gate',role='model.mesh_collection',result=1)
            if e['api']=='meshLookup':event.update(name='mesh_lookup',role='model.mesh_collection',result_role='mesh0')
            events.append(event)
        for side in ('original','candidate'):
            if row[side+'_events']!=events:errors.append(side+' full trace')
            gl=[dict(name=e['name'],args=e['args']) for e in row[side+'_events'] if e['name'].startswith('gl')]
            native_gl=[dict(name=e['name'],args=e['args']) for e in native_row['events']]
            if gl!=native_gl:errors.append(side+' differs from native forwarded emission')
            if row[side+'_result']!=expected['eax']:errors.append(side+' EAX')
            a=row[side+'_abi']
            if a[0]!=a[1] or a[2:6]!=[0x11223344,0x22334455,0x33445566,0x44556677]:errors.append(side+' ABI')
            if a[7]!=a[10] or (a[9]&0xffc0)!=(a[12]&0xffc0):errors.append(side+' FP control')
        if row['original_abi'][7:]!=row['candidate_abi'][7:]:errors.append('initial/final FP mismatch')
        if row['actual_replacement_calls']!=1:errors.append('installed route')
        for suffix in ('pre','original-post','candidate-post'):
            witness=Path(str(p)+'-replay-'+suffix+'.bin')
            if witness.read_bytes()!=pre:errors.append(suffix+' source witness')
        results.append(dict(capture=p.name,events=len(events),pose=list(h[5:8]),result='FAIL' if errors else 'PASS',errors=errors))
    return dict(result='PASS' if results and all(r['result']=='PASS' for r in results) else 'FAIL',cases=results)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('directory',type=Path);parser.add_argument('--output',type=Path);a=parser.parse_args();result=check(a.directory);text=json.dumps(result,indent=2);print(text)
    if a.output:a.output.write_text(text)
