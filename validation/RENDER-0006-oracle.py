"""Independent bounded immutable morph-emission oracle from approved spec.

Finite float32 operands have exponents [-149,128]. Sequential binary64
operations round to53 significant bits, matching CW027F x87; the possible
intermediate exponent range is safely within binary64 normal range. Explicit
separate Python operations prevent fusion/reassociation. Only final result is
rounded to float32. No candidate implementation or original binary is used.
"""
import argparse
import json
import math
import struct
from pathlib import Path


def value(bits):
    result = struct.unpack("<f", struct.pack("<I", bits))[0]
    if not math.isfinite(result):
        raise ValueError("nonfinite operand outside bounded oracle")
    return result


def interpolate(a_bits, b_bits, factor_bits):
    a, b, factor = value(a_bits), value(b_bits), value(factor_bits)
    difference = b - a
    scaled = difference * factor
    result = scaled + a
    try:
        bits = struct.unpack("<I", struct.pack("<f", result))[0]
    except OverflowError as exc:
        raise ValueError("nonfinite float32 output outside bounded oracle") from exc
    if bits & 0x7F800000 == 0x7F800000:
        raise ValueError("nonfinite final output outside bounded oracle")
    return bits


def expected(case):
    """Canonical typed input: gate, meshCount, frameA/B, factor, ownerTextures,
    skins[{textureIndex}], meshes[{verticesPerFrame,triangleCount,skin,textured,
    positions:[frame-major XYZ arrays],normals:[same],uv:null|UVarrays,
    triangles:[six signed/raw dwords]}]. No standard format provenance assumed."""
    calls = [{"api": "meshGate", "collection": "model+24", "args": []}]
    gate = case["gate"] & 0xFFFFFFFF
    if gate == 0:
        return {"eax": 0, "events": calls}
    result = gate
    for mesh_index in range(max(0, case["meshCount"])):
        mesh = case["meshes"][mesh_index]
        calls.append({"api": "meshLookup", "collection": "model+24", "args": [mesh_index], "record": f"mesh:{mesh_index}"})
        offset_a = case["frameA"] * mesh["verticesPerFrame"]
        offset_b = case["frameB"] * mesh["verticesPerFrame"]
        if mesh["textured"] & 255:
            calls.append({"api": "glEnable", "args": [0x0DE1]})
            selected = mesh["skin"]
            calls.append({"api": "skinLookup", "collection": "model+8", "args": [selected], "record": f"skin:{selected}"})
            texture = case["ownerTextures"][case["skins"][selected]["textureIndex"]]
            calls.append({"api": "glBindTexture", "args": [0x0DE1, texture]})
        calls.append({"api": "glBegin", "args": [4]})
        for triangle in mesh["triangles"][:max(0, mesh["triangleCount"])]:
            for corner in (2, 1, 0):
                index = triangle[corner]
                if mesh["uv"] is not None:
                    calls.append({"api": "glTexCoord2f", "args": list(mesh["uv"][index])})
                a_position = mesh["positions"][offset_a + index]
                b_position = mesh["positions"][offset_b + index]
                a_normal = mesh["normals"][offset_a + index]
                b_normal = mesh["normals"][offset_b + index]
                normal = [0, 0, 0]
                position = [0, 0, 0]
                for component in (2, 1, 0):
                    normal[component] = interpolate(a_normal[component], b_normal[component], case["factor"])
                calls.append({"api": "glNormal3f", "args": normal})
                for component in (2, 1, 0):
                    position[component] = interpolate(a_position[component], b_position[component], case["factor"])
                calls.append({"api": "glVertex3f", "args": position})
        calls.append({"api": "glEnd", "args": []})
        result = mesh_index + 1
    return {"eax": result, "events": calls}


def compare(case, actual):
    wanted = expected(case)
    if actual["eax"] != wanted["eax"]:
        return {"result": "FAIL_RETURN_VALUE", "expected": wanted["eax"], "actual": actual["eax"]}
    if len(actual["events"]) != len(wanted["events"]):
        return {"result": "FAIL_CALLBACK_ORDER", "expectedCount": len(wanted["events"]), "actualCount": len(actual["events"])}
    for sequence, (want, got) in enumerate(zip(wanted["events"], actual["events"])):
        projection = {key: got.get(key) for key in want}
        if projection != want:
            return {"result": "FAIL_CALLBACK_ORDER" if want["api"] != got.get("api") else "FAIL_FLOAT", "sequence": sequence, "expected": want, "actual": projection}
    return {"result": "PASS", "calls": len(wanted["events"])}


def self_test():
    assert interpolate(0, 0, 0x3F000000) == 0
    assert interpolate(0x80000000, 0x80000000, 0xBF800000) == 0x80000000
    assert interpolate(0x3F800000, 0x40400000, 0x3F000000) == 0x40000000
    assert interpolate(0x3F800000, 0x40400000, 0x40000000) == 0x40A00000
    # Literal witnesses distinguish the specified final-only float32 store
    # from float32 rounding after subtraction/multiplication (SSE-style).
    assert interpolate(0x3E1F0CDA, 0x3EFA3F5D, 0x3E513741) == 0x3E64CFC5
    assert interpolate(0x40CA2AB1, 0x3E01BEE8, 0x3F8DB2AC) == 0xBF092BEE
    assert interpolate(0x40B15EC8, 0x3F3A75C5, 0x3FC5BD66) == 0xBFF28799
    assert expected({"gate": 0})["eax"] == 0
    assert expected({"gate": 0xFFFFFFFF, "meshCount": -1})["eax"] == 0xFFFFFFFF
    mesh = {"verticesPerFrame": 3, "triangleCount": 1, "skin": 0, "textured": 0, "positions": [[0x3F800000 + i * 0x10000, 0, 0] for i in range(6)], "normals": [[0x3F800000, 0, 0]] * 6, "uv": [[i, i + 10] for i in range(3)], "triangles": [[0, 1, 2, 90, 91, 92]]}
    case = {"gate": 1, "meshCount": 1, "frameA": 0, "frameB": 1, "factor": 0, "meshes": [mesh]}
    result = expected(case)
    assert result["eax"] == 1
    assert [e["args"] for e in result["events"] if e["api"] == "glTexCoord2f"] == [[2, 12], [1, 11], [0, 10]]
    assert [e["args"][0] for e in result["events"] if e["api"] == "glVertex3f"] == [0x3F820000, 0x3F810000, 0x3F800000]
    assert not any(e["api"] in ("glEnable", "glDisable", "glBindTexture", "glMaterialfv") for e in result["events"])
    print("RENDER-0006 independent morph/reverse-corner/return witnesses PASS")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--case", type=Path)
    parser.add_argument("--trace", type=Path)
    args = parser.parse_args()
    if args.self_test:
        self_test()
    if args.case:
        case = json.loads(args.case.read_text())
        result = compare(case, json.loads(args.trace.read_text())) if args.trace else expected(case)
        print(json.dumps(result, indent=2))
        if isinstance(result, dict) and result.get("result", "PASS") != "PASS":
            raise SystemExit(1)


if __name__ == "__main__":
    main()
