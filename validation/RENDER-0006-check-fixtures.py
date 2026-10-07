"""Independent contract state machine for typed synthetic observations."""
import copy, importlib.util, json, sys
from pathlib import Path
here=Path(__file__).parent
spec=importlib.util.spec_from_file_location('oracle',here/'RENDER-0006-oracle.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
path=Path(sys.argv[1]) if len(sys.argv)>1 else Path('C:/Users/ADMIN/Boxer-lab/ms3d/character-fixtures.jsonl')
rows=[json.loads(s) for s in path.read_text().splitlines()]
results=[]
for row in rows:
    state=copy.deepcopy(row['pre']); model=state['model']; meshes=state['meshes']; cid=row['case']; events=[]
    gate=0 if cid==0 else 0xffffffff if cid==23 else 1
    def emit(name,args=(),role='',result=0,result_role=''):
        events.append(dict(name=name,args=list(args),role=role,result=result,result_role=result_role))
    emit('mesh_gate',role='model.mesh_collection',result=gate)
    count=0; eax=gate
    while gate and count<model['mesh_count']:
        mesh=meshes[count]; emit('mesh_lookup',[count],role='model.mesh_collection',result_role='mesh'+str(count))
        if cid==17:model['mesh_count']=1
        oa=model['frame_a']*mesh['vertices_per_frame']; ob=model['frame_b']*mesh['vertices_per_frame']
        if mesh['textured']:
            emit('glEnable',[0xde1])
            if cid==16:mesh['skin_index']=1
            skin=mesh['skin_index'];emit('skin_lookup',[skin],role='model.skin_collection',result_role='skin'+str(skin))
            emit('glBindTexture',[0xde1,state['owner_texture_ids'][meshes[skin]['skin_texture_index']]])
        emit('glBegin',[4])
        if cid==18:mesh['triangle_count']=1
        triangle=0; alternate_tri=False; alternate_pos=False; alternate_normal=False
        while triangle<mesh['triangle_count']:
            for corner in (2,1,0):
                indices=[1,2,0,7,8,9] if alternate_tri and triangle==0 else mesh['triangles'][triangle]
                index=indices[corner]
                if cid!=7:
                    emit('glTexCoord2f',mesh['uv'][index])
                    if cid==15:alternate_pos=alternate_normal=True
                    if cid==21:model.update(frame_a=1,frame_b=1);mesh['vertices_per_frame']=2
                positions=mesh['alternate_positions'] if alternate_pos else mesh['positions']
                normals=mesh['alternate_normals'] if alternate_normal else mesh['normals']
                pa,pb=positions[oa+index][:],positions[ob+index][:]
                na,nb=normals[oa+index][:],normals[ob+index][:]
                emit('glNormal3f',[oracle.interpolate(a,b,model['factor_bits']) for a,b in zip(na,nb)])
                if cid==14:model['factor_bits']=0x3f400000
                if cid==22:alternate_pos=True;model['factor_bits']=0x3f200000
                emit('glVertex3f',[oracle.interpolate(a,b,model['factor_bits']) for a,b in zip(pa,pb)])
                if cid==20:alternate_tri=True
            triangle+=1
        emit('glEnd')
        if cid==19:model['mesh_count']=0
        count+=1;eax=count
    errors=[]
    for side in ('original','candidate'):
        if row[side+'_events']!=events:errors.append(side+' trace differs from contract oracle')
        if row[side+'_result']!=eax:errors.append(side+' EAX differs')
        a=row[side+'_abi']
        if a[0]!=a[1] or a[2:6]!=[0x11223344,0x22334455,0x33445566,0x44556677]:errors.append(side+' ABI')
        if a[7]!=a[10] or (a[9]&0xffc0)!=(a[12]&0xffc0):errors.append(side+' FP control')
    if row['original_abi'][7:]!=row['candidate_abi'][7:]:errors.append('FP environment mismatch')
    if row['actual_replacement_calls']!=1:errors.append('installed route count')
    if row['post']!=state:errors.append('typed post-state mismatch')
    only_environment=errors==['FP environment mismatch']
    results.append(dict(case=cid,name=row['name'],events=len(events),result='BLOCKED' if only_environment else 'FAIL' if errors else 'PASS',errors=errors))
report=dict(result='FAIL' if len(rows)!=27 or any(r['result']=='FAIL' for r in results) else 'BLOCKED' if any(r['result']=='BLOCKED' for r in results) else 'PASS',cases=results)
(here/'RENDER-0006-fixtures-independent.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
