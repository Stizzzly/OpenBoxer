"""Pack only independently approved typed GAME-0003 boundary exports."""
from pathlib import Path
import json,hashlib,struct,sys
import damage_pack_replay as binary
ROOT=Path(__file__).resolve().parents[2]
def pack(record,destination):
    path=Path(record['path']).resolve();allowed=(ROOT/'validation/GAME-0003-approved-native-original').resolve()
    assert path.parent==allowed
    data=path.read_bytes();assert hashlib.sha256(data).hexdigest()==record['sha256']
    r=json.loads(data);assert r['format']=='GAME-0003-observer-v1' and not r['candidate'] and not r['nested_whole_observed']
    metadata=json.loads((ROOT/'specs/gameplay/GAME-0003-observer-metadata.json').read_text());sites={int(s['callRva'],16):s for s in metadata['calls']}
    roles={int(v):k for k,v in record['pointerRoles'].items()}
    for k,v in record['localPointerRoles'].items():assert int(v) not in roles;roles[int(v)]='L-'+k
    def role(address):
        if address not in roles:
            assert r['module']<=address<r['module']+0x19f000,'Unapproved pointer role'
            roles[address]='G-%08X'%address
        return roles[address]
    def raw(value):return {'raw32':value}
    def pointer(value):return {'role':role(value)}
    normalized=[]
    for e in r['events']:
        s=sites[e['call_rva']];name=s['identity'];abi=s['abi'];cdecl=abi.startswith('cdecl')
        owner=raw(e['owner']) if cdecl else pointer(e['owner'])
        args=[]
        definitions=[]
        for definition in s['args']:
            definitions.extend([definition]* (4 if 'BY_VALUE' in definition else 2 if 'float64' in definition else 1))
        assert len(definitions)==len(e['args'])
        for arg,definition in zip(e['args'],definitions):args.append(pointer(arg) if 'pointer' in definition else raw(arg))
        if s['ret']=='output_pointer_EAX':eax=pointer(e['eax'])
        elif s['ret']=='element_pointer_EAX':
            index=e['args'][0];assert e['eax']==e['owner']+4*index;eax={'elementOf':{'role':role(e['owner'])},'index':index}
        else:eax=raw(e['eax'])
        normalized.append({'ordinal':e['ordinal'],'callRva':e['call_rva'],'owner':owner,'args':args,'EAX':eax})
    out=binary.u32(0x33474d44)+binary.u32(1)+binary.text(record['id'])+binary.text(record['sha256'])+binary.text(record['sourceSha256'])
    out+=binary.u32(r['module'])+binary.u32(r['mode'])+binary.u32(r['eax'])+binary.blob(r['entry_fp544'])+binary.blob(r['exit_fp544'])+binary.snapshot(r['before'])+binary.snapshot(r['after'])
    out+=binary.u32(len(roles))
    for address,name in roles.items():out+=binary.u32(address)+binary.text(name)
    out+=binary.u32(len(r['events']))
    for e,n in zip(r['events'],normalized):
        out+=binary.u32(e['call_rva'])+binary.u32(e['owner'])+binary.u32(e['eax'])+binary.u32(e['rng_before'])+binary.u32(e['rng_after'])
        out+=binary.u32(len(e['args']))+b''.join(binary.u32(x) for x in e['args'])
        out+=binary.field(n['owner'])+binary.field(n['EAX'])+b''.join(binary.field(x) for x in n['args'])
        out+=b''.join(binary.blob(e[k]) for k in ['owner_before','owner_after','returned_bytes','entry_fp544','exit_fp544'])
        out+=b''.join(binary.blob(x) for x in e['arg_before'])+b''.join(binary.blob(x) for x in e['arg_after'])
    destination.write_bytes(out)
    return {'id':record['id'],'sourceExportSha256':record['sha256'],'packed':str(destination.resolve()),'packedSha256':hashlib.sha256(out).hexdigest(),'pointerRoles':roles}
if __name__=='__main__':
    manifest=ROOT/'validation/GAME-0003-approved-native-original-manifest.json';data=manifest.read_bytes();m=json.loads(data)
    assert m['status']=='APPROVED_TYPED_SOURCES_ONLY_NOT_GAMEPLAY_EQUIVALENCE'
    assert hashlib.sha256(data).hexdigest()=='a281d3c4739ee645a7a45755be5d48b20f10fa2882728f40be844725a4700857'
    destination=ROOT/'validation/GAME-0003-packed-replay';destination.mkdir(exist_ok=True)
    records=[pack(r,destination/(r['id']+'.bin')) for r in m['records']]
    (destination/'manifest.json').write_text(json.dumps({'status':'PACKED_APPROVED_SOURCE_ONLY','sourceManifestSha256':hashlib.sha256(data).hexdigest(),'records':records},indent=2))
    print('Packed',len(records),'approved typed sources')
