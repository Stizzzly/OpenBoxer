"""Independent approved GAME-0001 own-arithmetic oracle.

Exact rational arithmetic models every specified 53-bit operation/store.
Opaque scalar helpers are inputs, never reimplemented here. This module does
not by itself prove callback FP environment, ABI, natural admission or PASS.
"""
from fractions import Fraction as F
import importlib.util
import json
from pathlib import Path


def pow2(e):
    return F(1 << e) if e >= 0 else F(1, 1 << -e)


def exponent(x):
    x = abs(x)
    e = x.numerator.bit_length() - x.denominator.bit_length()
    return e - (x < pow2(e))


def rn_int(x):
    n, r = divmod(x.numerator, x.denominator)
    return n + (2*r > x.denominator or (2*r == x.denominator and n & 1))


def round_binary(x, precision):
    if not x:
        return F(0)
    quantum = pow2(exponent(x) - precision + 1)
    return (-1 if x < 0 else 1) * rn_int(abs(x)/quantum) * quantum


def f32(bits):
    bits = int(bits, 0) if isinstance(bits, str) else bits
    e, m = (bits >> 23) & 255, bits & 0x7fffff
    if e == 255:
        raise ValueError('nonfinite input is outside admitted domain')
    if e == 0:
        value = m * pow2(-149)
    else:
        value = ((1 << 23) | m) * pow2(e - 150)
    return -value if bits >> 31 else value


def bits32(x):
    if not x:
        return 0
    sign = 0x80000000 if x < 0 else 0
    x = abs(x)
    e = exponent(x)
    if e < -126:
        return sign | rn_int(x / pow2(-149))
    m = rn_int(x / pow2(e-23))
    if m == 1 << 24:
        e, m = e+1, m >> 1
    if e > 127:
        raise ValueError('overflow outside admitted domain')
    return sign | ((e+127) << 23) | (m - (1 << 23))


def bits64(x):
    if not x:
        return 0
    sign = 1 << 63 if x < 0 else 0
    x = abs(x)
    e = exponent(x)
    m = rn_int(x / pow2(e-52))
    if m == 1 << 53:
        e, m = e+1, m >> 1
    return sign | ((e+1023) << 52) | (m - (1 << 52))


def raw80(hexbytes):
    data = bytes.fromhex(hexbytes)
    if len(data) != 10:
        raise ValueError('ST0 must have exactly ten lossless bytes')
    m = int.from_bytes(data[:8], 'little')
    se = int.from_bytes(data[8:], 'little')
    e = se & 0x7fff
    if e == 0x7fff:
        raise ValueError('nonfinite callback outside admitted domain')
    x = m * pow2((e or 1) - 16383 - 63)
    return -x if se & 0x8000 else x


class Arithmetic:
    def __init__(self):
        self.inexact = []

    def own(self, value, label):
        rounded = round_binary(value, 53)
        if rounded != value:
            self.inexact.append(label)
        return rounded

    def store32(self, value, label):
        bits = bits32(value)
        if f32(bits) != value:
            self.inexact.append(label)
        return bits

    def distance_argument(self, a, b):
        dx = self.own(f32(a[0])-f32(b[0]), 'dx')
        dx32 = self.store32(dx, 'dx32')
        xp = self.own(dx*f32(dx32), 'xproduct')
        xp32 = self.store32(xp, 'xproduct32')
        dz = self.own(f32(a[2])-f32(b[2]), 'dz')
        dz32 = self.store32(dz, 'dz32')
        zp = self.own(dz*f32(dz32), 'zproduct')
        total = self.own(zp+f32(xp32), 'distance_sum')
        return bits64(total)

    def timer(self, dt, timer):
        product = self.own(f32(0x3c23d70a)*f32(dt), 'timer_product')
        return self.store32(self.own(product+f32(timer), 'timer_add'), 'timer32')

    def fatigue(self, dt, fatigue):
        if f32(fatigue) <= 0:
            return fatigue
        product = self.own(f32(0x3e19999a)*f32(dt), 'fatigue_product')
        return self.store32(self.own(f32(fatigue)-product, 'fatigue_subtract'), 'fatigue32')

    def accumulator(self, length80, dt, before):
        product = self.own(raw80(length80)*f32(0x3ccccccd), 'length_scale')
        product = self.own(product*f32(dt), 'length_dt')
        return self.store32(self.own(product+f32(before), 'accumulator_add'), 'accumulator32')


def strike(distance32, key, hand):
    upper, types, states = {'Z':4,'X':5,'C':6}, {'Z':1,'X':2,'C':3}, {'Z':(4,3),'X':(6,5),'C':(8,7)}
    d = f32(distance32)
    return dict(hit=F(1)<d<F(upper[key]), type=types[key], latch=1,
                animation=states[key][bool(hand)], hand=0 if hand else 1, attack=1)


def selftest():
    checks = 0
    for b in [0, 0x3f800000, 0xbf800000, 0x3f7fffff, 0x3f800001,
              0x407fffff, 0x40800000, 0x40800001, 0x409fffff,
              0x40a00000, 0x40a00001, 0x40bfffff, 0x40c00000,
              0x40c00001, 0x3c23d70a, 0x3e19999a, 0x3ccccccd]:
        assert bits32(f32(b)) == b
        checks += 1
    for key, upper in [('Z',0x40800000),('X',0x40a00000),('C',0x40c00000)]:
        for b, hit in [(0x3f7fffff,False),(0x3f800000,False),(0x3f800001,True),
                       (upper-1,True),(upper,False),(upper+1,False)]:
            for hand in [0,1]:
                assert strike(b,key,hand)['hit'] == hit
                checks += 1
    assert raw80('00000000000000c00040') == 3
    assert round_binary(F(1)+pow2(-53),53) == 1
    assert round_binary(F(1)+3*pow2(-53),53) == 1+pow2(-51)
    checks += 3
    return {'checks':checks,'failures':0,'scope':'rational arithmetic helper self-checks, not original differential'}


if __name__ == '__main__':
    result = selftest()
    Path(__file__).with_suffix('.json').write_text(json.dumps(result, indent=2))
    print(json.dumps(result))
