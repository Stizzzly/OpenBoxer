"""Typed selection oracle derived solely from approved ANIM-0002 specification.

The dependency script defines lookup return record identity and comparator result;
it does not predict original CRT locale semantics. Scripts may mutate model words,
record bytes, request bytes, time and logical collection base. Every callback is
consumed exactly, so missing/extra dependency calls fail rather than being guessed.
"""
import argparse
import json
import struct
from pathlib import Path

def word(buf, off):
    return struct.unpack_from('<I', buf, off)[0]

def signed(value):
    return value - 0x100000000 if value & 0x80000000 else value

def put(buf, off, value):
    if off < 0 or off + 4 > len(buf):
        raise ValueError('typed write out of bounds')
    struct.pack_into('<I', buf, off, value & 0xffffffff)

def cbytes(buf):
    try:
        return bytes(buf[:buf.index(0)]).hex()
    except ValueError:
        raise ValueError('unterminated supplied callback string') from None

def select(case):
    model = bytearray.fromhex(case['model_hex'])
    if len(model) != 112:
        raise ValueError('expected model112')
    records = {str(k): bytearray.fromhex(v) for k, v in case['records'].items()}
    if any(len(v) != 272 for v in records.values()):
        raise ValueError('expected record272')
    request = bytearray.fromhex(case['request_hex'])
    time = case['time_bits'] & 0xffffffff
    base = case.get('collection_base_role', 'initial_records')
    calls = []
    script = iter(case['dependencies'])

    def dependency(kind, **args):
        nonlocal time, request, base
        try:
            action = next(script)
        except StopIteration:
            raise ValueError('missing dependency action') from None
        if action['kind'] != kind:
            raise ValueError('dependency order mismatch')
        for k, v in action.get('expect', {}).items():
            if args.get(k) != v:
                raise ValueError(f'dependency argument mismatch: {k}')
        observed = {'kind': kind, **args, 'model_hex_before': model.hex(),
                    'time_bits_before': time, 'collection_base_role': base}
        for item in action.get('model_writes', []):
            put(model, item['offset'], item['bits'])
        for item in action.get('record_writes', []):
            put(records[str(item['record'])], item['offset'], item['bits'])
        if 'request_hex' in action:
            request = bytearray.fromhex(action['request_hex'])
        time = action.get('time_bits', time) & 0xffffffff
        base = action.get('collection_base_role', base)
        if kind == 'lookup':
            result = str(action['record'])
            if result not in records:
                raise ValueError('missing typed returned record')
            observed['return_record_role'] = result
        else:
            result = signed(action['result'] & 0xffffffff)
            observed['result'] = result
        observed.update(model_hex_after=model.hex(), time_bits_after=time)
        calls.append(observed)
        return result

    index = 0
    eax = 0xcccccccc
    matched = False
    while index < signed(word(model, 40)):
        if len(calls) > 2048:
            raise ValueError('outside bounded oracle scope')
        record = dependency('lookup', this_role='model+84', index=index)
        cmp_result = dependency('compare', record_role=record,
                                record_name_hex=cbytes(records[record]),
                                request_role='original_request',
                                request_name_hex=cbytes(request))
        if cmp_result == 0:
            put(model, 44, index)
            second = dependency('lookup', this_role='model+84',
                                index=signed(word(model, 44)))
            put(model, 48, word(records[second], 256))
            put(model, 76, 0x3dcccccd)
            put(model, 80, time)
            eax = case['owner_bits'] & 0xffffffff
            matched = True
            break
        index = signed((index + 1) & 0xffffffff)
        eax = index & 0xffffffff
    try:
        next(script)
        raise ValueError('unused dependency action')
    except StopIteration:
        pass
    return {'model_hex': model.hex(), 'time_bits': time, 'eax': eax,
            'matched': matched, 'callbacks': calls,
            'request_hex': request.hex(),
            'records': {k: v.hex() for k, v in records.items()},
            'collection_base_role': base}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('input')
    parser.add_argument('output')
    args = parser.parse_args()
    cases = json.loads(Path(args.input).read_text(encoding='utf-8-sig'))
    out = [{'id': c['id'], **select(c)} for c in cases]
    Path(args.output).write_text(json.dumps(out, indent=2) + '\n', encoding='utf-8')
