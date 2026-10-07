"""Independent ANIM-0001 typed behavioral oracle; no original program access.

JSON input: model_hex (112 bytes), time_bits, count, optional mutations:
count_model_writes/lookup_model_writes list {offset,bits},
count_time_bits/lookup_time_bits, rate, optional lookup_rate.
Only finite positive-rate nearest/53-bit arithmetic is supported.
Python binary64 arithmetic supplies the spec's 53-bit intermediates. Every
specified single-precision store is explicit; interval is never stored f32.
Output contains full model state, EAX, time, ordered callback observations.
FP machine status is not predicted: supplied original/candidate status must be
reported separately, never used to weaken exact model/output comparisons.
"""
import argparse
import json
import math
import struct
from pathlib import Path

def u32(model, offset):
    return struct.unpack_from('<I', model, offset)[0]

def i32(model, offset):
    return struct.unpack_from('<i', model, offset)[0]

def read_float(bits):
    return struct.unpack('<f', struct.pack('<I', bits))[0]

def store_float(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]

def write_u32(model, offset, value):
    struct.pack_into('<I', model, offset, value & 0xffffffff)

def writes(model, items):
    for item in items:
        if not 0 <= item['offset'] <= 108:
            raise ValueError('mutation outside typed 112-byte model')
        write_u32(model, item['offset'], item['bits'])

def advance(case):
    model = bytearray.fromhex(case['model_hex'])
    if len(model) != 112:
        raise ValueError('model must contain exactly 112 bytes')
    current_time = case['time_bits']
    callbacks = [{'kind': 'count', 'this_role': 'model+84', 'return': case['count']}]
    writes(model, case.get('count_model_writes', []))
    current_time = case.get('count_time_bits', current_time)
    steps = 0
    condition = case.get('initial_sw', 0) & 0x4700
    if case['count'] == 0:
        eax = 0
    else:
        sampled_time = current_time
        elapsed_bits = store_float(read_float(sampled_time) - read_float(u32(model, 80)))
        elapsed = read_float(elapsed_bits)
        callbacks.append({'kind': 'lookup', 'this_role': 'model+84',
                          'index': i32(model, 44), 'return_role': 'selected_record'})
        writes(model, case.get('lookup_model_writes', []))
        current_time = case.get('lookup_time_bits', current_time)
        rate = case.get('lookup_rate', case['rate'])
        if not 0 < rate <= 0x7fffffff or not math.isfinite(elapsed):
            raise ValueError('outside approved finite positive-rate domain')
        interval = 1000.0 / rate
        condition = 0
        initial_bits = store_float(elapsed / interval)
        factor = read_float(initial_bits)
        if interval <= elapsed:
            residual = elapsed
            while True:
                residual = read_float(store_float(residual - (1000.0 / rate)))
                steps += 1
                if not ((1000.0 / rate) <= residual and steps <= 10):
                    break
            selected_raw = u32(model, 56 + 4 * steps)
            write_u32(model, 48, selected_raw)
            anchor = read_float(u32(model, 80))
            updated_anchor = ((1000.0 / rate) * steps) + anchor
            write_u32(model, 80, store_float(updated_anchor))
            factor = read_float(store_float(factor - (steps * 1.0)))
            condition = 0x0100 if factor < 0.0 else (0x4000 if factor == 0.0 else 0)
            if factor < 0.0:
                factor = 0.0
        eax = store_float(factor)
        write_u32(model, 76, eax)
    return {'model_hex': model.hex(), 'eax': eax, 'time_bits': current_time,
            'callbacks': callbacks, 'steps': steps,
            'condition_bits': condition,
            'fp_status_policy': 'EXACT_FULL_SW_WITH_CONDITION_AND_STICKY_DIAGNOSTICS'}

def self_check():
    model = bytearray(112)
    for n in range(1, 12):
        write_u32(model, 56 + n * 4, 0x41000000 + n)
    write_u32(model, 80, 0)
    def run(time, rate=10, **kw):
        return advance(dict(model_hex=model.hex(), time_bits=store_float(time),
                            rate=rate, count=1, **kw))
    assert run(0)['steps'] == 0
    assert run(-1)['eax'] == store_float(-0.01)
    assert run(100)['steps'] == 1
    assert run(400)['steps'] == 4
    for n in range(5, 12):
        result = run(n * 100)
        assert result['steps'] == n
        assert u32(bytes.fromhex(result['model_hex']), 48) == u32(model, 56+4*n)
    assert run(2000)['steps'] == 11
    assert run(2000)['eax'] == store_float(9)
    # Negative mocked nonzero counts proceed. Count-zero preserves all state.
    zero = advance(dict(model_hex=model.hex(), time_bits=0, rate=10, count=0))
    assert zero['model_hex'] == model.hex() and zero['eax'] == 0
    mutation = run(100, lookup_model_writes=[{'offset':80,'bits':store_float(500)}],
                   lookup_time_bits=store_float(9999))
    assert u32(bytes.fromhex(mutation['model_hex']),80) == store_float(600)
    assert mutation['time_bits'] == store_float(9999)
    return {'result':'PASS', 'confidence':'HIGH', 'scope':'oracle self consistency only'}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('cases', nargs='?')
    args = parser.parse_args()
    if args.cases:
        cases = json.loads(Path(args.cases).read_text(encoding='utf-8-sig'))
        print(json.dumps([advance(case) for case in cases], indent=2))
    else:
        print(json.dumps(self_check(), indent=2))
