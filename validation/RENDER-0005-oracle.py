"""Independent specification oracle for immutable, valid draw fixtures.

Agent3 only: approved behavioral contract and typed test data; no binary access.
Live mutation fixtures require their additional scripted observation audit.
All callback/source values below are raw uint32 bits, never recomputed floats.
"""
import argparse
import json
import struct
from pathlib import Path

TEXTURE = 0x0DE1
ONE = 0x3F800000
THRESHOLD_BITS = 0x3ECCCCCD


def float32(bits):
    return struct.unpack("<f", struct.pack("<I", bits))[0]


def expected_calls(case):
    """Input: meshes [{material,members,count?}], flat material/triangle/vertex
    dword records, handles[5], entryQueryResult, meshCount (optional)."""
    meshes = case.get("meshes", [])
    count = case.get("meshCount", len(meshes))
    materials = case.get("materials", [])
    triangles = case.get("triangles", [])
    vertices = case.get("vertices", [])
    handles = case.get("handles", [0] * 5)
    calls = []

    def append(api, args, pointer=None, payload=None):
        event = {"api": api, "args": list(args)}
        if pointer is not None:
            event["pointer"] = pointer
            event["payload"] = list(payload)
        calls.append(event)

    append("glIsEnabled", [TEXTURE])
    for mesh in meshes[:max(0, count)]:
        index = mesh["material"]
        if index < 0 or index >= len(materials):
            raise ValueError("negative/out-of-range material is fallback scope")
        material = materials[index]
        if len(material) != 20:
            raise ValueError("material must contain20 dwords")
        for offset, name in ((0, 0x1200), (16, 0x1201), (32, 0x1202), (48, 0x1600)):
            append("glMaterialfv", [0x0404, name], f"materials:{index}:{offset}", material[offset // 4:offset // 4 + 4])
        append("glMaterialf", [0x0404, 0x1601, material[16]])
        for group in range(4):
            append("opaqueVector", [handles[group], *material[group * 4:group * 4 + 3], ONE])
        append("opaqueFloat", [handles[4], material[16]])
        if material[18]:
            append("glBindTexture", [TEXTURE, material[18]])
            append("glEnable", [TEXTURE])
        else:
            append("glDisable", [TEXTURE])
        if material[17] & 0x7F800000 == 0x7F800000:
            raise ValueError("special threshold value needs observed FP policy")
        if float32(material[17]) > float32(THRESHOLD_BITS):
            append("glBegin", [4])
            members = mesh.get("members", [])
            membership_count = mesh.get("count", len(members))
            for triangle_index in members[:max(0, membership_count)]:
                triangle = triangles[triangle_index]
                if len(triangle) != 19:
                    raise ValueError("triangle must contain19 dwords")
                for corner in range(3):
                    vertex_index = triangle[15 + corner]
                    if len(vertices[vertex_index]) != 4:
                        raise ValueError("vertex must contain4 dwords")
                    append("glNormal3fv", [], f"triangles:{triangle_index}:{12 * corner}", triangle[3 * corner:3 * corner + 3])
                    append("glTexCoord2f", [triangle[9 + corner], triangle[12 + corner]])
                    append("glVertex3fv", [], f"vertices:{vertex_index}:4", vertices[vertex_index][1:4])
            append("glEnd", [])
    append("glEnable" if case["entryQueryResult"] & 0xFF else "glDisable", [TEXTURE])
    return calls


def compare(case, actual):
    expected = expected_calls(case)
    if len(actual) != len(expected):
        return {"result": "FAIL_CALLBACK_ORDER", "expectedCount": len(expected), "actualCount": len(actual)}
    for index, (want, got) in enumerate(zip(expected, actual)):
        relevant = {key: got.get(key) for key in want}
        if relevant != want:
            return {"result": "FAIL_CALLBACK_ORDER" if got.get("api") != want["api"] else "FAIL_STATE", "sequence": index, "expected": want, "actual": relevant}
    return {"result": "PASS", "calls": len(expected)}


def parse_snapshot(path):
    """Parse approved typed source snapshot v1, including sentinel model bytes.
    Pointer fields are deliberately zero in this schema; no executable bytes."""
    data = Path(path).read_bytes()
    offset = 0

    def take(length):
        nonlocal offset
        if length < 0 or offset + length > len(data):
            raise ValueError("truncated source snapshot")
        value = data[offset:offset + length]
        offset += length
        return value

    def words(n, signed=False):
        return list(struct.unpack("<" + ("i" if signed else "I") * n, take(n * 4)))

    if words(2) != [0x35575244, 1]:
        raise ValueError("unexpected source snapshot magic/version")
    counts = words(4, signed=True)
    handles = words(5)
    model = take(0x8D9C4)
    for field, count in zip((0x8D9A4, 0x8D9AC, 0x8D9B4, 0x8D9BC), counts):
        if struct.unpack_from("<i", model, field)[0] != count:
            raise ValueError("header/model count mismatch")
        if struct.unpack_from("<I", model, field + 4)[0] != 0:
            raise ValueError("model table pointer not normalized")
    case = {"meshCount": counts[0], "handles": handles, "meshes": [], "materials": [], "triangles": [], "vertices": []}
    if counts[0] > 0:
        if any(c < 0 for c in counts) or counts[0] > 65536 or counts[1] > 65536 or max(counts[2:]) > 1000000:
            raise ValueError("source snapshot resource bounds")
        mesh_words = [words(3, signed=True) for _ in range(counts[0])]
        case["materials"] = [words(20) for _ in range(counts[1])]
        case["triangles"] = [words(19) for _ in range(counts[2])]
        case["vertices"] = [words(4) for _ in range(counts[3])]
        for selected, count, pointer in mesh_words:
            n = words(1, signed=True)[0]
            if pointer != 0 or n != count:
                raise ValueError("membership normalization/count mismatch")
            case["meshes"].append({"material": selected, "count": count, "members": words(max(0, count), signed=True)})
    if offset != len(data):
        raise ValueError("source snapshot trailing bytes")
    return case


def normalize_events(events):
    aliases = {"material_vector": "opaqueVector", "material_scalar": "opaqueFloat"}
    strides = {"materials": 80, "triangles": 76, "vertices": 16}
    result = []
    for event in events:
        item = {"api": aliases.get(event["api"], event["api"]), "args": event["args"]}
        if event.get("role"):
            table, plus, number = event["role"].rpartition("+")
            if table in ("materials0", "triangles0", "vertices0"):
                table = table[:-1]
            if not plus or table not in strides:
                item["pointer"] = event["role"]
            else:
                index, within = divmod(int(number), strides[table])
                item["pointer"] = f"{table}:{index}:{within}"
            item["payload"] = event["payload"]
        result.append(item)
    return result


def ordered_digest(events):
    apis = ["glIsEnabled", "glMaterialfv", "glMaterialf", "glBindTexture", "glEnable", "glDisable", "glBegin", "glNormal3fv", "glTexCoord2f", "glVertex3fv", "glEnd", "material_vector", "material_scalar"]
    value = 1469598103934665603  # The Trace format's explicit seed.

    def consume(payload):
        nonlocal value
        for byte in payload:
            value = ((value ^ byte) * 1099511628211) & 0xFFFFFFFFFFFFFFFF

    for event in events:
        consume(struct.pack("<I", apis.index(event["api"])))
        for word in event["args"] + event.get("payload", []):
            consume(struct.pack("<I", word))
        consume(event.get("role", "").encode("utf-8"))
    return f"{value:016x}"


def self_test():
    base = {"entryQueryResult": 0x100, "meshCount": 0}
    assert expected_calls(base) == [{"api": "glIsEnabled", "args": [TEXTURE]}, {"api": "glDisable", "args": [TEXTURE]}]
    base["entryQueryResult"] = 0x80
    assert expected_calls(base)[-1]["api"] == "glEnable"
    material = list(range(20))
    material[17] = THRESHOLD_BITS
    material[18] = 0xFFFFFFFF
    triangle = list(range(19))
    triangle[15:18] = [2, 0, 1]
    case = {"entryQueryResult": 0x80, "meshes": [{"material": 0, "members": [0]}], "materials": [material], "triangles": [triangle], "vertices": [[0, 11, 12, 13], [0, 21, 22, 23], [0, 31, 32, 33]], "handles": [5, 6, 7, 8, 9]}
    calls = expected_calls(case)
    assert len(calls) == 14 and not any(c["api"] == "glBegin" for c in calls)
    assert calls[11] == {"api": "glBindTexture", "args": [TEXTURE, 0xFFFFFFFF]}
    assert calls[6]["args"] == [5, 0, 1, 2, ONE]
    material[17] += 1
    calls = expected_calls(case)
    assert len(calls) == 25
    assert calls[13] == {"api": "glBegin", "args": [4]}
    assert calls[14] == {"api": "glNormal3fv", "args": [], "pointer": "triangles:0:0", "payload": [0, 1, 2]}
    assert calls[15] == {"api": "glTexCoord2f", "args": [9, 12]}
    assert calls[16]["pointer"] == "vertices:2:4" and calls[16]["payload"] == [31, 32, 33]
    case["meshes"][0]["count"] = -2
    calls = expected_calls(case)
    assert len(calls) == 16 and [c["api"] for c in calls[13:15]] == ["glBegin", "glEnd"]
    print("RENDER-0005 immutable specification witnesses PASS")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--case", type=Path)
    parser.add_argument("--trace", type=Path)
    parser.add_argument("--snapshot", type=Path)
    parser.add_argument("--native", type=Path)
    parser.add_argument("--call", type=int, default=1)
    args = parser.parse_args()
    if args.self_test:
        self_test()
    if args.case:
        case = json.loads(args.case.read_text(encoding="utf-8"))
        result = compare(case, json.loads(args.trace.read_text(encoding="utf-8"))) if args.trace else expected_calls(case)
        print(json.dumps(result, indent=2))
        if isinstance(result, dict) and result["result"] != "PASS":
            raise SystemExit(1)
    if args.snapshot:
        case = parse_snapshot(args.snapshot)
        if not args.native:
            print(json.dumps(case, indent=2))
            return
        captures = [json.loads(line) for line in args.native.read_text(encoding="utf-8").splitlines() if line.strip()]
        capture = next(c for c in captures if c["call"] == args.call)
        if "entry_result" in capture:
            case["entryQueryResult"] = capture["entry_result"]
        elif "result" in capture["events"][0]:
            case["entryQueryResult"] = capture["events"][0]["result"]
        else:
            raise ValueError("entry query result missing: cannot infer savedAL from final restoration")
        verdict = compare(case, normalize_events(capture["events"]))
        verdict["digestObserved"] = ordered_digest(capture["events"])
        verdict["digestMatches"] = verdict["digestObserved"] == capture["digest"]
        print(json.dumps(verdict, indent=2))
        if verdict["result"] != "PASS" or not verdict["digestMatches"]:
            raise SystemExit(1)


if __name__ == "__main__":
    main()
