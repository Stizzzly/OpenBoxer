"""Full typed-source emission oracle for native coherent snapshots v1."""
import argparse, importlib.util, json, struct
from pathlib import Path
here=Path(__file__).parent
spec=importlib.util.spec_from_file_location('oracle',here/'RENDER-0006-oracle.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
def snapshot(path):
    data=path.read_bytes(); h=struct.unpack_from('<10I',data)
    if h[0]!=0x36485243 or h[1] not in (1,2):raise ValueError('unsupported snapshot header')
    _,version,frames,vertices,triangles,a,b,factor,owner,model=h
    if not (a<frames and b<frames):raise ValueError('invalid original frame indices')
    expected=40+80+288+(24*frames*vertices if version==1 else 48*vertices)+8*vertices+24*triangles
    if len(data)!=expected:raise ValueError('snapshot length mismatch')
    m=data[40:120]; mesh=data[120:408]
    u=lambda b,o:struct.unpack_from('<I',b,o)[0]
    if (u(m,48),u(m,52),u(m,76))!=(a,b,factor):raise ValueError('header/model pose mismatch')
    if (u(mesh,0),u(mesh,4),mesh[16])!=(vertices,triangles,0):raise ValueError('unsupported mesh')
    cursor=408;arrays=[]
    payload_frames=frames if version==1 else 2
    for width,count in ((3,payload_frames*vertices),(3,payload_frames*vertices),(2,vertices),(6,triangles)):
        arrays.append(list(struct.iter_unpack('<'+'I'*width,data[cursor:cursor+4*width*count])));cursor+=4*width*count
    positions,normals,uv,indices=arrays
    case=dict(gate=1,meshCount=u(m,0),frameA=a if version==1 else 0,frameB=b if version==1 else 1,factor=factor,ownerTextures=[],skins=[],meshes=[dict(verticesPerFrame=vertices,triangleCount=triangles,skin=0,textured=0,positions=positions,normals=normals,uv=uv,triangles=indices)])
    return data,h,case
def check(directory):
    rows=[json.loads(s) for s in (directory/'character-native.jsonl').read_text().splitlines()]
    loads=[json.loads(s) for s in (directory/'character-loads.jsonl').read_text().splitlines()]
    manifest=json.loads((here.parent/'specs/render/RENDER-0006-approved-bhm-bounds.json').read_text())
    assets={x['sha256']:x for x in manifest}
    results=[]
    for row in rows:
        if row['route'] not in ('original','replacement'):continue
        stem=f"character-native-{row['route']}-{row['call']:04d}"
        pre,h,case=snapshot(directory/(stem+'-pre.bin'));post,_,_=snapshot(directory/(stem+'-post.bin'))
        expected=oracle.expected(case)
        gl=[dict(name=e['api'],args=e['args']) for e in expected['events'] if e['api'].startswith('gl')]
        actual=[dict(name=e['name'],args=e['args']) for e in row['events']]
        errors=[]
        asset=assets.get(row['asset_sha256'])
        registered=[l for l in loads if l['model_identity']==row['model_identity'] and l['registered'] and l['sha256']==row['asset_sha256']]
        if not asset or not registered:errors.append('loader manifest provenance')
        else:
            l=registered[-1];surface=asset['surfaces'][0]
            if not(l['result']&255==1 and l['root_asset_hashes_confirmed'] and l['header_readable'] and l['signature_le']==0x33504449 and l['version']==15 and l['frames']==749 and l['tags']==0 and l['surfaces']==1):errors.append('loader header provenance')
            if l['normalized_filename']!=asset['path'] or h[2:5]!=(asset['frames'],surface['vertices'],surface['triangles']):errors.append('asset allocation bounds')
        if row['route']=='replacement' and row['actual_replacements']!=row['call']:errors.append('native installed route counter')
        if actual!=gl:errors.append('complete ordered emission differs')
        if pre!=post:errors.append('source changed')
        if row['result']!=expected['eax']:errors.append('EAX')
        if row['caller_rva']!=0x5fef or not row['typed_snapshot_complete']:errors.append('route/capture')
        if (h[5],h[6],h[7],h[8],h[9])!=(row['frame_a'],row['frame_b'],row['factor_bits'],row['owner_identity'],row['model_identity']):errors.append('capture identity')
        results.append(dict(route=row['route'],call=row['call'],pose=list(h[5:8]),events=len(gl),result='FAIL' if errors else 'PASS',errors=errors,fp={k:row[k] for k in ('cw','sw','mxcsr','after_cw','after_sw','after_mxcsr')}))
    return dict(result='PASS' if results and all(x['result']=='PASS' for x in results) else 'FAIL',calls=results)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('directory',type=Path);parser.add_argument('--output',type=Path);a=parser.parse_args()
    result=check(a.directory);text=json.dumps(result,indent=2);print(text)
    if a.output:a.output.write_text(text)
