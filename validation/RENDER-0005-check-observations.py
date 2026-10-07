"""Independent Agent3 checker: typed fixtures only; no original executable."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("draw_oracle", ROOT / "validation/RENDER-0005-oracle.py")
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def mutation_witnesses(name, events):
    """Literal expected observations for controlled callback mutations.
    These assertions are independent of candidate traversal code."""
    by_api = lambda api: [event for event in events if event["api"] == api]
    normals = by_api("glNormal3fv")
    vertices = by_api("glVertex3fv")
    begins = by_api("glBegin")
    ends = by_api("glEnd")
    if name == "query-model":
        assert len(begins) == len(ends) == 2
        assert all(e["role"].startswith("triangles1+") for e in normals)
        assert all(e["role"].startswith("vertices1+") for e in vertices)
    elif name == "material-relocate":
        assert [e["role"] for e in by_api("glMaterialfv")] == ["materials0+0", "materials1+16", "materials1+32", "materials1+48"]
        assert by_api("glMaterialf")[0]["args"][2] == 0x3F100000 + 16 * 0x100
    elif name == "shader-live":
        vectors = by_api("material_vector")
        assert vectors[0]["args"] == [0xAB000000, 0x3F000000, 0x3F000100, 0x3F000200, 0x3F800000]
        for group in range(1, 4):
            assert vectors[group]["dispatch"] == 1
            assert vectors[group]["args"] == [0xEF000000 + group * 16 + group, *[0x41000000 + group * 0x10000 + group * 4 + component for component in range(3)], 0x3F800000]
        assert by_api("material_scalar")[0]["args"] == [0xEE000004, 0x41040010]
        assert by_api("material_scalar")[0]["dispatch"] == 1
    elif name == "scalar-texture":
        assert by_api("glBindTexture")[0]["args"] == [0x0DE1, 0xFFFFFFFF]
        assert len(begins) == len(ends) == 1 and len(vertices) == 3
    elif name == "bind-threshold":
        assert len(begins) == len(ends) == 1 and len(vertices) == 3
    elif name == "enable-threshold":
        assert not begins and not ends and not vertices
        assert len(by_api("glMaterialfv")) == 4 and len(by_api("material_vector")) == 4
    elif name == "begin-tables":
        assert [e["role"] for e in normals] == [f"triangles0+{index * 76 + corner * 12}" for index in (2, 1) for corner in range(3)]
        assert [e["role"] for e in vertices] == [f"vertices0+{index * 16 + 4}" for index in (6, 7, 8, 3, 4, 5)]
    elif name == "normal-uv-index":
        assert [e["role"] for e in vertices] == [f"vertices0+{index * 16 + 4}" for index in range(6)]
        assert [e["args"] for e in by_api("glTexCoord2f")] == [[0x80000000 + count, 0x3D000000 + count] for count in range(1, 7)]
    elif name == "texcoord-vertices":
        assert all(e["role"].startswith("vertices1+") for e in vertices)
        assert all(e["role"].startswith("triangles0+") for e in normals)
    elif name in ("normal-triangles", "vertex-triangles"):
        assert [e["role"] for e in normals] == ["triangles0+0", "triangles0+12", "triangles0+24", "triangles1+76", "triangles1+88", "triangles1+100"]
    elif name in ("vertex-count", "end-count"):
        assert len(begins) == len(ends) == 1 and len(vertices) == 6
    elif name == "texture-state":
        assert events[0]["result"] == 0x80 and events[-1]["api"] == "glEnable"
    elif name == "all-slots-live":
        assert events[0]["dispatch"] == 0 and all(e["dispatch"] == 1 for e in events[1:])
    return True


def check(directory, fixture):
    rows = [json.loads(line) for line in fixture.read_text(encoding="utf-8").splitlines() if line.strip()]
    checks = []
    errors = []
    for row in rows:
        name = row["case"]
        detail = {"case": name, "guardOnly": row["guard_only"], "mutation": row["mutation"]}
        sides = [side for side in ("original", "candidate") if side in row]
        paths = {}
        for side in sides:
            observation = row[side]
            events = observation["events"]
            files = {stage: directory / f"draw-{name}-{side}-{stage}" for stage in ("pre.bin", "post.bin", "pre.witness", "post.witness")}
            paths[side] = files
            detail[side] = {stage: digest(path) for stage, path in files.items()}
            if row["guard_only"]:
                if events or files["pre.witness"].read_bytes() != files["post.witness"].read_bytes():
                    errors.append(f"{name}/{side}: guard performed callback or state mutation")
                if observation["route_count_after"] != observation["route_count_before"]:
                    errors.append(f"{name}/{side}: guard advanced replacement route")
                continue
            abi = observation["abi"]
            wanted = {"ebx": 0x11223344, "esi": 0x22334455, "edi": 0x33445566, "ebp": 0x44556677}
            if any(abi[k] != v for k, v in wanted.items()) or not abi["stack_balanced"]:
                errors.append(f"{name}/{side}: ABI register/stack mismatch")
            if "stack_before" in abi and abi["stack_before"] != abi["stack_after"]:
                errors.append(f"{name}/{side}: raw stack addresses mismatch")
            if "original" in row:
                wanted_delta = 1 if side == "candidate" else 0
                if observation["route_count_after"] - observation["route_count_before"] != wanted_delta:
                    errors.append(f"{name}/{side}: installed route delta mismatch")
            if abi["cw_before"] != 0x027F or abi["cw_after"] != 0x027F:
                errors.append(f"{name}/{side}: CW mismatch")
            if (abi["sw_before"] & 0x3800) != (abi["sw_after"] & 0x3800):
                errors.append(f"{name}/{side}: x87 stack depth mismatch")
            if events[0]["api"] != "glIsEnabled" or events[0]["result"] != row["query"]:
                errors.append(f"{name}/{side}: query/result mismatch")
            final = "glEnable" if row["query"] & 0xFF else "glDisable"
            expected_eax = 0xE1234567 if final == "glEnable" else 0xD7654321
            if events[-1]["api"] != final or events[-1]["args"] != [0x0DE1] or events[-1]["result"] != abi["eax"] or abi["eax"] != expected_eax:
                errors.append(f"{name}/{side}: savedAL restoration/EAX mismatch")
            if any(e["role"] == "UNREGISTERED" for e in events):
                errors.append(f"{name}/{side}: temporary/unregistered source pointer")
            if row["mutation"] == 0:
                source = oracle.parse_snapshot(files["pre.bin"])
                source["entryQueryResult"] = row["query"]
                verdict = oracle.compare(source, oracle.normalize_events(events))
                detail[side]["contractOracle"] = verdict
                if verdict["result"] != "PASS":
                    errors.append(f"{name}/{side}: immutable exact-call oracle mismatch: {verdict}")
                if files["pre.witness"].read_bytes() != files["post.witness"].read_bytes() or files["pre.bin"].read_bytes() != files["post.bin"].read_bytes():
                    errors.append(f"{name}/{side}: immutable source/global witness changed")
            else:
                try:
                    detail[side]["mutationWitnesses"] = mutation_witnesses(name, events)
                except AssertionError:
                    errors.append(f"{name}/{side}: controlled mutation contract witness mismatch")
        if "original" in row:
            if row["original"]["events"] != row["candidate"]["events"]:
                errors.append(f"{name}: full callback trace mismatch")
            for stage in paths["original"]:
                if paths["original"][stage].read_bytes() != paths["candidate"][stage].read_bytes():
                    errors.append(f"{name}: paired {stage} mismatch")
            a, b = row["original"]["abi"], row["candidate"]["abi"]
            for field in ("eax", "cw_before", "cw_after", "sw_before", "sw_after", "mxcsr_before", "mxcsr_after"):
                if a[field] != b[field]:
                    errors.append(f"{name}: FP/return {field} mismatch")
        checks.append(detail)
    baseline = json.loads((ROOT / "validation/RENDER-0005-prior-core-hashes.json").read_text())
    preserved = [{**item, "currentSha256": digest(ROOT / "replacement/ms3d" / item["name"])} for item in baseline]
    for item in preserved:
        if item["sha256"] != item["currentSha256"]:
            errors.append(f"prior core modified: {item['name']}")
    return {"result": "FAIL" if errors else "PASS", "scope": "original paired observations" if any("original" in r for r in rows) else "offline specification conformance only", "cases": len(rows), "fixtureSha256": digest(fixture), "checks": checks, "priorCores": preserved, "errors": errors}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--directory", type=Path, default=ROOT / "replacement/ms3d/build")
    parser.add_argument("--fixture", default="draw-offline.jsonl")
    parser.add_argument("--output", type=Path, default=ROOT / "validation/RENDER-0005-offline-independent.json")
    args = parser.parse_args()
    result = check(args.directory, args.directory / args.fixture)
    args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({k: result[k] for k in ("result", "scope", "cases", "errors")}, indent=2))
    if result["errors"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
