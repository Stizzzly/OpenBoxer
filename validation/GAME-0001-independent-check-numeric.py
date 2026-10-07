"""Invoke replacement x87 helpers; compare them to independent rationals."""
import importlib.util
import json
import random
import subprocess
from pathlib import Path

base = Path(__file__).parent
spec = importlib.util.spec_from_file_location('oracle', base/'GAME-0001-independent-numeric.py')
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)
rng = random.Random(0x600001)
cases = []
for i in range(128):
    dt = rng.randrange(0x3a800000,0x41200000)
    fatigue = rng.randrange(0x3f800000,0x42c60000)
    old = rng.randrange(0x3e800000,0x42000000)
    a = [rng.randrange(0x3e800000,0x42000000) for _ in range(4)]
    b = [rng.randrange(0x3e800000,0x42000000) for _ in range(4)]
    ar = oracle.Arithmetic()
    expected = [ar.timer(dt,0), ar.fatigue(dt,fatigue),
                ar.accumulator('00000000000000c00040',dt,old),
                ar.distance_argument(a,b)]
    cases.append(dict(dt=dt,fatigue=fatigue,old=old,a=a,b=b,expected=expected))

code = '#include "strike_fp.hpp"\n#include <cstdio>\nint main(){unsigned short cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw));\n'
for c in cases:
    a,b,dt,fatigue,old = c['a'],c['b'],c['dt'],c['fatigue'],c['old']
    code += ('{strike::Result r;r.scalar80={0,0,0,0,0,0,0,0xc0,0,0x40};'
             f'auto t=strike::fp::multiplyAdd(0x3c23d70a,{dt}u,0);'
             f'auto f=strike::fp::subtractProduct({fatigue}u,0x3e19999a,{dt}u);'
             'strike::fp::load80(r);'
             f'auto p=strike::fp::scalarProgress(r,0x3ccccccd,{dt}u,{old}u);'
             f'auto dx=strike::fp::difference({a[0]}u,{b[0]}u);'
             f'auto xp=strike::fp::differenceProduct({a[0]}u,{b[0]}u,dx);'
             f'auto dz=strike::fp::difference({a[2]}u,{b[2]}u);'
             f'auto d=strike::fp::differenceSum({a[2]}u,{b[2]}u,dz,xp);'
             'printf("%08x %08x %08x %016llx\\n",t,f,p,(unsigned long long)d);}\n')
code += 'return 0;}\n'
source = base/'GAME-0001-independent-numeric-runner.cpp'
executable = source.with_suffix('.exe')
source.write_text(code)
build = subprocess.run([r'C:\msys64\ucrt64\bin\clang++.exe','-O2','-std=c++17',
                        '-I',str(base.parent/'replacement/ms3d'),str(source),'-o',str(executable)],capture_output=True,text=True)
if build.returncode:
    raise RuntimeError(build.stdout+build.stderr)
run = subprocess.run([str(executable)],capture_output=True,text=True,check=True)
lines = run.stdout.splitlines()
failures = []
assert len(lines) == len(cases)
for i,(line,c) in enumerate(zip(lines,cases)):
    actual = [int(x,16) for x in line.split()]
    if actual != c['expected']:
        failures.append(dict(index=i,input=c,actual=actual))
report = dict(cases=len(cases),checks=4*len(cases),failures=failures,
              scope='replacement x87 helper numerical audit, not original differential/ABI proof',
              compiler='Clang host x64; x87 CW027F; optimization O2')
(base/'GAME-0001-independent-numeric-audit.json').write_text(json.dumps(report,indent=2))
print(json.dumps(dict(cases=len(cases),checks=4*len(cases),failures=len(failures))))
raise SystemExit(bool(failures))
