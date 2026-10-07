"""Validate archived natural boundary invariants, without inventing math replay."""
import importlib.util
import json
import struct
from pathlib import Path
import sys

here = Path(__file__).parent
spec = importlib.util.spec_from_file_location('oracle', here / 'RENDER-0004-oracle.py')
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)
folder = Path(sys.argv[1])


def parse(path):
    raw = path.read_bytes()
    header = struct.unpack_from('<8I', raw)
    cursor, windows = 32, {}
    for _ in range(header[7]):
        rva, size = struct.unpack_from('<2I', raw, cursor)
        cursor += 8
        windows[rva] = raw[cursor:cursor + size]
        cursor += size
    assert cursor == len(raw)
    return header, windows


def value(windows, address):
    base = next(rva for rva, payload in windows.items() if rva <= address < rva + len(payload))
    return struct.unpack_from('<I', windows[base], address - base)[0]


results = []
for call in (1, 2, 3, 120):
    preh, pre = parse(folder / f'selection-native-{call:04}-pre.bin')
    posth, post = parse(folder / f'selection-native-{call:04}-post.bin')
    assert value(pre, 0x177f88) == value(post, 0x177f88) == 3
    assert preh[4] == posth[4] == 0x027f
    rows = [[value(post, 0x178110 + index * 56 + field * 4) for field in range(14)] for index in range(3)]
    expected = oracle.published_checkpoint(rows)
    for key, bases in [('positions', (0x170178, 0x170188, 0x170198)), ('colors', (0x1701b8, 0x1701c8, 0x1701d8)), ('directions', (0x1701f8, 0x170204, 0x170210))]:
        assert [[value(post, base + component * 4) for component in range(3)] for base in bases] == expected[key]
    assert [value(post, base) for base in (0x170228, 0x17022c, 0x170230)] == expected['parameters']
    for address in (0x17810c, 0x1781b8, 0x170184, 0x170194, 0x1701c4, 0x1701d4):
        assert value(pre, address) == value(post, address)
    wave = (0x176370, 0x184af0, 0x177f84, 0x170354, 0x1855f0, 0x1855f4)
    results.append({'call': call, 'mode': value(pre, 0x184770), 'preCW': preh[4], 'preSW': preh[5], 'postSW': posth[5], 'preMXCSR': preh[6], 'postMXCSR': posth[6], 'parameters': expected['parameters'], 'waveBefore': {hex(address): value(pre, address) for address in wave}, 'waveAfter': {hex(address): value(post, address) for address in wave}, 'waveTableChanged': pre[0x17028c] != post[0x17028c]})
print(json.dumps({'result': 'PASS', 'scope': 'natural snapshot invariants only; original replay pending', 'observations': results}, indent=2))
