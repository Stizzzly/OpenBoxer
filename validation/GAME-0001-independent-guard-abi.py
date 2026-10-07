import pathlib,re,json,hashlib,sys
stage=sys.argv[1] if len(sys.argv)>1 else 'stage5'
p=pathlib.Path(__file__).parent/f'GAME-0001-{stage}-typed'
lines=(p/'strike-fixtures.txt').read_text().splitlines();fail=[];checks=0
def ck(label,condition):
 global checks
 checks+=1
 if not condition:fail.append(label)
cases=[];routes=[];guards=[]
for line in lines:
 if line.startswith('key='):
  m=re.fullmatch(r'key=(\d+) case=(\d+) candidate=(\d+) eax=([0-9a-f]+) abi=(\d+) esp=([0-9a-f]+)/([0-9a-f]+) regs=([0-9a-f]+)/([0-9a-f]+)/([0-9a-f]+)/([0-9a-f]+)',line)
  ck('ABI row schema',bool(m))
  if not m:continue
  key,case,candidate,eax,abi,before,after,ebx,esi,edi,ebp=m.groups();key,case,candidate=map(int,(key,case,candidate));cases.append((key,case,candidate))
  ck('reported ABI',(abi=='1'));ck('ESP exact',before==after)
  ck('nonvolatile sentinels',(ebx,esi,edi,ebp)==('11223344','22334455','33445566','44556677'))
  number=key*72+case*2+candidate+1;j=json.loads((p/f'strike-fixture-{number:03}.json').read_text());ck('EAX capture/text',int(eax,16)==j['eax'])
 elif line.startswith('route '):
  m=re.fullmatch(r'route key=(\d+) case=(\d+) candidate=(\d+) replacement=(\d+) fallback=(\d+) valid=(\d+)',line);ck('route schema',bool(m))
  if m:
   k,n,c,r,f,v=map(int,m.groups());routes.append((k,n,c));ck('route supported exactly once',(r,f,v)==((1 if c else 0),0,1))
 elif line.startswith('guard='):
  m=re.fullmatch(r'guard=(\d+) rejected=(\d+) unchanged=(\d+) reason=(.*)',line);ck('guard schema',bool(m))
  if m:
   n,r,u,reason=m.groups();guards.append(int(n));ck('negative rejected',(r,u)==('1','1'));ck('negative reason',reason!='admitted')
ck('216 unique ABI vectors',len(cases)==216 and len(set(cases))==216)
ck('216 unique route vectors',len(routes)==216 and len(set(routes))==216)
ck('24 unique guard vectors',sorted(guards)==list(range(24)))
ck('actual entry wrapper ABI','entrywrapper abi=1 eax=fedc1234' in lines)
ck('original fallback exactly once','fallback replacement=0 original=1 exactlyonce=1' in lines)
guard=json.loads((p/'strike-guard-217.json').read_text());ck('guard fallback capture',not guard['candidate'])
out={'checks':checks,'failures':fail,'primary':'FAIL' if fail else 'PASS','scope':'216 scripted whole-function ABI/route vectors;24 pre-effect read-only admission negatives;actual entry wrapper unsupported fallback once. Negative cases invoke admission directly, not full dispatch separately24times.','provenance':{'pid':14328 if stage=='stage6' else 8480,'dll':'0604B11ADC5BB31E0C012E4B78C1061DF9803236F37719DA207276B4200B3015' if stage=='stage6' else '7D85CBB8B189D5FE9B5869DB49F72A67E90FFED2E677BA58E3A5ADBE2006A7D8'},'text_sha256':hashlib.sha256((p/'strike-fixtures.txt').read_bytes()).hexdigest()}
(p.parent/f'GAME-0001-independent-{stage}-guard-abi.json').write_text(json.dumps(out,indent=2));print(json.dumps({'checks':checks,'failures':fail}))
