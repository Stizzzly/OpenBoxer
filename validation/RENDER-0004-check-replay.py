"""Compare native replay outputs as behavioral state, excluding dispatch slots."""
import hashlib
import json
import struct
import sys
from pathlib import Path

lab = Path('C:/Users/ADMIN/Boxer-lab/ms3d')
final = len(sys.argv) > 1 and sys.argv[1] == '--final'
native = lab / ('selection-final-native' if final else 'candidate-selection-v1/native-evidence')


def read(path):
    raw = path.read_bytes()
    h = struct.unpack_from('<8I', raw)
    o, windows = 32, {}
    for _ in range(h[7]):
        r, n = struct.unpack_from('<2I', raw, o)
        o += 8
        windows[r] = raw[o:o + n]
        o += n
    assert o == len(raw) == 848
    return raw, h, windows


results = []
for call in (1, 2, 3, 120):
    folder = lab / f'selection-{"final-" if final else ""}replay-{call:04}'
    ar, ah, aw = read(folder / 'selection-replay-original.bin')
    br, bh, bw = read(folder / 'selection-replay-candidate.bin')
    nr, nh, nw = read(native / f'selection-native-{call:04}-post.bin')
    _, ih, iw = read(folder / 'selection-replay-input.bin')
    _, ph, pw = read(native / f'selection-native-{call:04}-pre.bin')
    assert ar == br
    for rva in aw:
        if rva not in (0x1855cc, 0x1855dc):
            assert aw[rva] == nw[rva], (call, hex(rva))
            assert iw[rva] == pw[rva], (call, 'input', hex(rva))
    assert ah[3] == nh[3] == 0
    assert ah[4] == nh[4] == 0x027f
    assert (ah[5] & 63) == (nh[5] & 63) == 0x21
    assert ah[6] == 0x1f80 and nh[6] == 0x1fa0  # Harness restores MXCSR before dump.
    a = (folder / 'selection-original-trace.jsonl').read_bytes()
    b = (folder / 'selection-candidate-trace.jsonl').read_bytes()
    assert a == b
    summary = (folder / 'selection-fixtures-summary.txt').read_text()
    assert 'result=PASS' in summary and 'TOTAL replayPairs=1 failures=0' in summary
    assert 'MXCSR=00001fa0/00001fa0' in summary and 'exceptions=21/21' in summary
    results.append({'call': call, 'windowsCompared': len(aw) - 2, 'originalSha256': hashlib.sha256(ar).hexdigest(), 'nativePostSha256': hashlib.sha256(nr).hexdigest(), 'traceSha256': hashlib.sha256(a).hexdigest(), 'replaySW': ah[5], 'nativeSW': nh[5]})
output = {'result': 'PASS', 'scope': 'four final-build native inputs' if final else 'four first-build native inputs', 'comparison': 'final candidate and original math/wave; exact 23 non-dispatch windows', 'observations': results}
(Path(__file__).parent / ('RENDER-0004-final-independent-replay.json' if final else 'RENDER-0004-independent-replay.json')).write_text(json.dumps(output, indent=2) + '\n')
print(json.dumps(output, indent=2))
