import json, struct, hashlib
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer')
V=R/'validation'
meta=json.loads((R/'specs/gameplay/GAME-0002-observer-metadata.json').read_text(encoding='utf-8-sig'))
manifest=json.loads((V/'GAME-0002-independent-approved-clean-source-manifest.json').read_text())
root=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-clean-replay-observations-stage6')
spans=meta['capture']['globalExtents']
def value(d,when,va,fmt='f'):
    data=bytes.fromhex(d[when]['globals']); pos=0
    for span in spans:
        base=0x400000+int(span['rva'],16)
        if base<=va and va+struct.calcsize('<'+fmt)<=base+span['size']:
            return struct.unpack_from('<'+fmt,data,pos+va-base)[0]
        pos+=span['size']
    raise ValueError(hex(va))
proof=json.loads((V/'GAME-0002-independent-clean-replay-stage6-comparison.json').read_text())
proven={r['sourceId']:r for r in proof['records']}
rows=[]
for item in manifest['records']:
    sid=item['sourceId']; paths=[root/sid/'damage-replay-original-001.json',root/sid/'damage-candidate-002.json']
    o,c=[json.loads(p.read_text()) for p in paths]
    assert hashlib.sha256(paths[0].read_bytes()).hexdigest()==proven[sid]['originalSha256']
    assert hashlib.sha256(paths[1].read_bytes()).hexdigest()==proven[sid]['candidateSha256']
    assert not o['candidate'] and c['candidate']
    pi=value(o,'before',0x584784,'I'); ai=value(o,'before',0x584788,'I')
    ob=value(o,'before',0x576238); oa=value(o,'after',0x576238)
    cb=value(c,'before',0x576238); ca=value(c,'after',0x576238)
    block=value(o,'before',0x576234,'B')
    rows.append(dict(source=sid,playerIndex=pi,opponentIndex=ai,block=block,playerFactor=value(o,'before',0x5762A0+16*pi),opponentFactor=value(o,'before',0x5762A8+16*ai),originalBefore=ob,originalAfter=oa,originalLoss=ob-oa,candidateBefore=cb,candidateAfter=ca,candidateLoss=cb-ca,exactHealth=(struct.pack('<ff',ob,oa)==struct.pack('<ff',cb,ca)),consumed=value(o,'before',0x57622C,'B')==1 and value(o,'after',0x57622C,'B')==0,originalHash=hashlib.sha256(paths[0].read_bytes()).hexdigest(),candidateHash=hashlib.sha256(paths[1].read_bytes()).hexdigest()))
result=dict(status='PASS' if all(r['exactHealth'] for r in rows) else 'FAIL',scope='Health-only audit of 28 previously validated identical-input original/v6 replay pairs from real original observations; not a new independent live playthrough or initial-health investigation',pairs=len(rows),consumed=sum(r['consumed'] for r in rows),rows=rows)
(V/'GAME-0002-focused-health-comparison.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps({'status':result['status'],'pairs':len(rows),'unblockedHits':sum(r['consumed'] and not r['block'] for r in rows),'blockedHits':sum(r['consumed'] and r['block'] for r in rows),'hits':[{'id':r['source'],'block':r['block'],'before':r['originalBefore'],'after':r['originalAfter'],'originalLoss':r['originalLoss'],'candidateLoss':r['candidateLoss']} for r in rows if r['consumed']]},indent=2))
