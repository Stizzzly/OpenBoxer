"""Independent Agent3 parsing of permitted behavioral fixtures."""
import hashlib
import importlib.util
import json
import re
import struct
import sys
from pathlib import Path

HERE = Path(__file__).parent
LAB = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('C:/Users/ADMIN/Boxer-lab/ms3d')
spec = importlib.util.spec_from_file_location('oracle', HERE / 'RENDER-0004-oracle.py')
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)
oracle.self_check()


def digest(data):
    return hashlib.sha256(data).hexdigest()


def dump(path):
    data = path.read_bytes()
    header = struct.unpack_from('<8I', data)
    assert header[0:2] == (0x344c4553, 1)
    cursor, windows = 32, {}
    for _ in range(header[7]):
        rva, size = struct.unpack_from('<2I', data, cursor)
        cursor += 8
        assert rva not in windows
        windows[rva] = data[cursor:cursor + size]
        cursor += size
    assert cursor == len(data) == 848
    return data, header, windows


original = [json.loads(line) for line in (LAB / 'selection-original-trace.jsonl').read_text().splitlines()]
candidate = [json.loads(line) for line in (LAB / 'selection-candidate-trace.jsonl').read_text().splitlines()]
assert len(original) == len(candidate) == 36
assert original == candidate
evidence = {'scope': '36 isolated behavioral comparisons; natural validation pending', 'pairs': []}
publication_checks = 0
padding = (0x170184, 0x170194, 0x1701c4, 0x1701d4)
for index, observed in enumerate(original, 1):
    a, ah, aw = dump(LAB / f'selection-{index:02}-original.bin')
    b, bh, bw = dump(LAB / f'selection-{index:02}-candidate.bin')
    _, _, iw = dump(LAB / f'selection-{index:02}-input.bin')
    assert a == b  # Includes full SW, all windows and slot identities; no normalization.
    assert ah[4] == 0x027f and ah[6] == observed['mxcsr']
    assert ah[3] == observed['eax'] and (ah[5] & 63) == observed['exceptions']
    assert aw[0x17810c][:4] == iw[0x17810c][:4]
    assert aw[0x17810c][-4:] == iw[0x17810c][-4:]
    for address in padding:
        base = next(rva for rva, payload in aw.items() if rva <= address < rva + len(payload))
        offset = address - base
        assert aw[base][offset:offset + 4] == iw[base][offset:offset + 4]
    dispatch = [event for event in observed['events'] if event.startswith(('scalar(', 'scalar-alt(', 'vector(', 'vector-alt('))]
    assert len(dispatch) in (7, 14)
    for start in range(0, len(dispatch), 7):
        assert [event.split('(')[0].split('-')[0] for event in dispatch[start:start + 7]] == ['scalar', 'vector', 'vector', 'vector', 'scalar', 'scalar', 'scalar']
        snapshot = dispatch[start].split(' | ')[1].split(',')
        words = [int(value, 16) for value in snapshot[1:]]
        assert len(words) in (72, 79, 136)  # Handles, plus real-wave scalar/table state.
        rows = [words[i:i + 14] for i in range(0, 42, 14)]
        selected = [words[42 + k * 10:52 + k * 10] for k in range(3)]
        expected = oracle.published_checkpoint(rows)
        assert [row[:3] for row in selected] == expected['positions']
        assert [row[3:6] for row in selected] == expected['colors']
        assert [row[6:9] for row in selected] == expected['directions']
        assert [row[9] for row in selected] == expected['parameters']
        publication_checks += 1
    evidence['pairs'].append({'scenario': index, 'name': observed['name'], 'bytes': len(a), 'sha256': digest(a), 'eax': ah[3], 'cw': ah[4], 'sw': ah[5], 'mxcsr': ah[6]})

checkpoints = [json.loads(line) for line in (HERE / 'RENDER-0004-offline-checkpoints.jsonl').read_text().splitlines()]
assert len(checkpoints) == 30
for case in checkpoints:
    assert oracle.ordered_checkpoint(case['sortInputRecords'], case['count']) == case['postSortRecords'], case['label']
    if not case['dispatchMutatesSelected']:
        assert oracle.published_checkpoint(case['postWaveRecords']) == case['selected'], case['label']
    # Dispatch-live publication is independently checked at first scalar event above.

summary = (LAB / 'selection-fixtures-summary.txt').read_text()
assert summary.count('result=PASS') == 36 and 'TOTAL scenarios=36 failures=0' in summary
abi_lines = [line for line in summary.splitlines() if line.startswith(' ESP=')]
assert len(abi_lines) == 36
for line in abi_lines:
    match = re.fullmatch(r' ESP=([0-9a-f]+)/([0-9a-f]+):([0-9a-f]+)/([0-9a-f]+) NV=(.*):(.*)', line)
    assert match and match[1] == match[2] and match[3] == match[4]
    assert match[5] == match[6] == '13579bdf/2468ace0/55aa55aa/aa55aa55'
evidence['trace'] = {'bytes': len((LAB / 'selection-original-trace.jsonl').read_bytes()), 'sha256': digest((LAB / 'selection-original-trace.jsonl').read_bytes())}
evidence['summary'] = {'sha256': digest((LAB / 'selection-fixtures-summary.txt').read_bytes())}
evidence['publicationChecks'] = publication_checks
evidence['offlineOrderingChecks'] = len(checkpoints)
evidence['independentPaddingWitnesses'] = [hex(address) for address in padding]
evidence['paddingOutsideDump'] = ['0x1701a4', '0x1701e4']
(HERE / ('RENDER-0004-final-independent-traces.json' if len(sys.argv) > 1 else 'RENDER-0004-independent-traces.json')).write_text(json.dumps(evidence, indent=2) + '\n')
print(f'PASS: 36 full binary pairs, exact traces, ABI, padding, {publication_checks} original publications and 30 offline ordering checkpoints.')
