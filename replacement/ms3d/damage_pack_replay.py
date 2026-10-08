"""Pack explicitly approved typed exports; never read original/process files."""
import hashlib,json,struct,sys
from pathlib import Path
def u32(v):return struct.pack('<I',v&0xffffffff)
def text(v):b=v.encode();return u32(len(b))+b
def blob(v):b=bytes.fromhex(v);return u32(len(b))+b
def field(v):
    if 'raw32' in v:return u32(0)+text('')+u32(v['raw32'])
    if 'role' in v:return u32(1)+text(v['role'])+u32(0)
    if 'elementOf' in v:return u32(2)+text(v['elementOf']['role'])+u32(v['index'])
    raise ValueError('Unsupported typed role descriptor')
def snapshot(s):
    out=b''.join(blob(s[k]) for k in ['actor232','player176','player_body2288','opponent_body2288','globals','world_list12'])
    out+=u32(s['player_body'])+u32(s['opponent_body'])+u32(s['world'])
    out+=u32(len(s['global_span_sizes']))+b''.join(u32(x) for x in s['global_span_sizes'])
    return out
def pack(path,dest):
    source=path.read_bytes();d=json.loads(source)
    assert d['status']=='APPROVED_TYPED_ORIGINAL_SOURCE_REPLAY_INPUT_ONLY'
    assert d['admissionReview']['admission']=='SUPPORTED_ENTRY'
    r=d['sourceRecord'];assert not r['candidate'] and not r['nested_whole_observed']
    out=u32(0x31474d44)+u32(1)+text(d['sourceId'])+text(hashlib.sha256(source).hexdigest())+text(d['sourceSha256'])
    out+=u32(r['module'])+u32(r['mode'])+u32(r['eax'])+blob(r['entry_fp544'])+blob(r['exit_fp544'])+snapshot(r['before'])+snapshot(r['after'])
    out+=u32(len(d['pointerRoles']))
    for role in d['pointerRoles']:out+=u32(role['address'])+text(role['role'])
    assert len(r['events'])==len(d['normalizedEventRoles'])
    out+=u32(len(r['events']))
    for e,n in zip(r['events'],d['normalizedEventRoles']):
        assert e['ordinal']==n['ordinal'] and e['call_rva']==n['callRva']
        out+=u32(e['call_rva'])+u32(e['owner'])+u32(e['eax'])+u32(e['rng_before'])+u32(e['rng_after'])
        out+=u32(len(e['args']))+b''.join(u32(x) for x in e['args'])
        out+=field(n['owner'])+field(n['EAX'])+b''.join(field(x) for x in n['args'])
        out+=b''.join(blob(e[k]) for k in ['owner_before','owner_after','returned_bytes','entry_fp544','exit_fp544'])
        out+=b''.join(blob(x) for x in e['arg_before'])+b''.join(blob(x) for x in e['arg_after'])
    dest.write_bytes(out)
if __name__=='__main__':
    path=Path(sys.argv[1]).resolve();validation=Path(__file__).resolve().parents[2]/'validation'
    allowed={
        (validation/'GAME-0002-approved-source-replay').resolve():validation/'GAME-0002-independent-approved-source-manifest.json',
        (validation/'GAME-0002-approved-clean-source-replay').resolve():validation/'GAME-0002-independent-approved-clean-source-manifest.json',
    }
    assert path.parent in allowed,'Only explicitly approved typed export directories are accepted'
    manifest=json.loads(allowed[path.parent].read_text())
    assert manifest['status']=='PASS_SELECTED_SOURCE_EXPORT_ONLY' and not manifest['errors']
    record=next(v for v in manifest['records'] if Path(v['path']).resolve()==path)
    assert hashlib.sha256(path.read_bytes()).hexdigest()==record['sha256']
    pack(path,Path(sys.argv[2]))
