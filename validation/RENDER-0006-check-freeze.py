import hashlib,json,struct
from pathlib import Path
root=Path(__file__).parent.parent
sha=lambda b:hashlib.sha256(b).hexdigest().upper()
manifest=json.loads((root/'validation/RENDER-0006-implementation-freeze.json').read_text())
before=json.loads((root/'validation/RENDER-0006-format-Before.json').read_text())
def sections(data):
    count=struct.unpack_from('<H',data,2)[0];table=20+struct.unpack_from('<H',data,16)[0]
    symbol,countsym=struct.unpack_from('<II',data,8);strings=symbol+18*countsym;result=[]
    for i in range(count):
        offset=table+40*i;name=data[offset:offset+8].split(b'\0')[0].decode()
        if name.startswith('/'):
            start=strings+int(name[1:]);name=data[start:data.index(b'\0',start)].decode()
        if name.startswith(('.text','.rdata')):
            size,start=struct.unpack_from('<II',data,offset+16)
            result.append(dict(Name=name,Size=size,Sha256=sha(data[start:start+size])))
    return result
objects=[]
for old in before['Objects']:
    data=(root/'replacement/ms3d/build-validation/CMakeFiles/ms3d_replacement.dir'/old['Name']).read_bytes()
    objects.append(dict(name=old['Name'],codeAndConstantsIdentical=sections(data)==old['Sections']))
files=[]
for entry in manifest['Delivery']+manifest['Sources']:
    path=root/entry['Path'];files.append(dict(path=entry['Path'],matches=sha(path.read_bytes())==entry['Sha256']))
report=dict(result='PASS' if all(x['codeAndConstantsIdentical'] for x in objects) and all(x['matches'] for x in files) else 'FAIL',objects=objects,files=files,limitations='Recorded preformat section hashes only; raw source token identity not claimed for split adjacent literals. Frozen delivery uses unchanged validated808D DLL.')
def pe_sections(path):
    data=path.read_bytes();pe=struct.unpack_from('<I',data,60)[0];count=struct.unpack_from('<H',data,pe+6)[0];table=pe+24+struct.unpack_from('<H',data,pe+20)[0];out={}
    for i in range(count):
        offset=table+40*i;name=data[offset:offset+8].split(b'\0')[0].decode();size,start=struct.unpack_from('<II',data,offset+16);out[name]=dict(size=size,sha256=sha(data[start:start+size]))
    return out
old=pe_sections(root/'replacement/ms3d/frozen-render0006/ms3d_replacement.dll');new=pe_sections(root/'replacement/ms3d/build-validation/ms3d_replacement.dll')
report['actualPeSectionComparisons']=[dict(section=name,before=value,after=new.get(name),identical=value==new.get(name)) for name,value in old.items()]
(root/'validation/RENDER-0006-freeze-independent.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
