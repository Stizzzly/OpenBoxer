"""Integrity check of explicitly authorized typed animation data snapshots."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('jsonl');p.add_argument('directory');p.add_argument('output');a=p.parse_args()
    rows=[json.loads(x) for x in Path(a.jsonl).read_text(encoding='utf-8-sig').splitlines() if x.strip()]
    checks=[]
    for row in rows:
        file=Path(a.directory)/f'clip-native-{row["route"]}-{row["call"]:04d}.bin'
        raw=file.read_bytes();h=struct.unpack_from('<9I',raw);magic,version,count,size,clock,after,eax,cw,sw=h
        offset=36;before=raw[offset:offset+112];offset+=112
        records=raw[offset:offset+count*272];offset+=count*272
        request=raw[offset:offset+size];offset+=size
        model=raw[offset:offset+112];offset+=112
        expected_header=(0x32504c43,1,row['count'],len(bytes.fromhex(row['request_hex']))+1,row['clock_bits'],row['after_clock_bits'],row['eax_role_or_bits'],row['initial_fp']['cw'],row['initial_fp']['sw'])
        ok=(h==expected_header and offset==len(raw)
            and before==struct.pack('<28I',*row['before_model_words'])
            and model==struct.pack('<28I',*row['after_model_words'])
            and records.hex()==row['records_hex']
            and request==bytes.fromhex(row['request_hex'])+b'\0')
        checks.append(dict(file=str(file),result='PASS' if ok else 'FAIL_STATE',sha256=hashlib.sha256(raw).hexdigest().upper()))
    report=dict(result='PASS' if checks and all(c['result']=='PASS' for c in checks) else 'FAIL',checks=checks)
    Path(a.output).write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report))
