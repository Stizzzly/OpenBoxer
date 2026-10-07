"""Independently decode v3 typed sidecars and match their JSON witnesses."""
import pathlib,json,struct,sys
stage=sys.argv[1] if len(sys.argv)>1 else 'stage3'
folder=pathlib.Path(__file__).parent/f'GAME-0001-{stage}-typed'
checks=0;failures=[]
def check(name,a,b):
 global checks
 checks+=1
 if a!=b:failures.append(name)
class Reader:
 def __init__(self,p):self.b=p.read_bytes();self.i=0
 def take(self,n):
  a=self.b[self.i:self.i+n];self.i+=n
  if len(a)!=n:raise ValueError('truncated')
  return a
 def u(self):return int.from_bytes(self.take(4),'little')
 def hx(self,n):return self.take(n).hex()
 def seq(self,size):return [self.take(size) for _ in range(self.u())]
 def snap(self):
  s={'player176':self.hx(176),'player_body2288':self.hx(2288),'opponent_body2288':self.hx(2288)}
  s['globals']=b''.join(self.seq(1)).hex();bodies=self.seq(2288);addresses=[int.from_bytes(x,'little') for x in self.seq(4)]
  for k in ('world','head','tail','count'):s[k]=self.u()
  s['fp512']=self.hx(512);s['active_list']=[{'address':a,'data':b.hex()} for a,b in zip(addresses,bodies)]
  return s
 def event(self):
  e={'id':self.u(),'owner':self.u()};n=self.u();flags=self.u();args=[self.u() for _ in range(4)];e['args']=args[:n];e['caller']=self.u()
  e['arg_before']=self.hx(64);e['arg_after']=self.hx(64);e['pointer_arg_bytes']=[self.u() for _ in range(4)]
  e['eax']=self.u();e['scalar80']=self.hx(10);e['returned_fp512']=self.hx(512)
  returned=self.take(16);e['returned_pointer_size']=self.u();e['owner_size']=self.u()
  e['returned_bytes']=returned[:e['returned_pointer_size']].hex()
  e['owner_before']=self.take(16)[:e['owner_size']].hex();e['owner_after']=self.take(16)[:e['owner_size']].hex()
  e['before']=self.snap();e['after']=self.snap();return e
for path in sorted(folder.glob('strike-fixture-*.skt')):
 r=Reader(path);j=json.loads(path.with_suffix('.json').read_text());name=path.stem
 check(name+'.magic',r.u(),0x31544b53);check(name+'.version',r.u(),3);base=r.u();player=r.u()
 check(name+'.eax',r.u(),j['eax']);check(name+'.candidate',bool(r.take(1)[0]),j['candidate']);supported=bool(r.take(1)[0])
 check(name+'.supported',supported,True)
 check(name+'.before',r.snap(),j['before']);check(name+'.after',r.snap(),j['after'])
 count=r.u();check(name+'.count',count,len(j['events']))
 for i in range(count):
  e=r.event();je=j['events'][i]
  for k,v in e.items():check(name+f'.event{i}.'+k,v,je[k])
 check(name+'.end',r.i,len(r.b))
out={'sidecars':216,'checks':checks,'failures':failures,'scope':'typed v3 sidecar/JSON integrity, no implementation bytes interpreted'}
(folder.parent/f'GAME-0001-independent-{stage}-sidecars.json').write_text(json.dumps(out,indent=2));print(json.dumps({'checks':checks,'failures':len(failures),'first':failures[:5]}))
