"""Independent captured-source parity: exact CPU pairs and native emissions."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("draw_oracle", ROOT / "validation/RENDER-0005-oracle.py")
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def semantic(events):
    return [{key: e[key] for key in ("api", "dispatch", "args", "role", "payload")} for e in events]


def check(directory, native_directory, route, call):
    row = json.loads((directory / "draw-replay.jsonl").read_text().strip())
    captures = [json.loads(line) for line in (native_directory / "draw-native.jsonl").read_text().splitlines() if line.strip()]
    native = next(c for c in captures if c["route"] == route and c["call"] == call)
    source = native_directory / f"draw-native-{route}-{call:04d}-pre.bin"
    errors, reports = [], {}
    case = oracle.parse_snapshot(source)
    case["entryQueryResult"] = native["events"][0]["result"]
    for side in ("original", "candidate"):
        result = row[side]
        events, abi = result["events"], result["abi"]
        files = [directory / f"draw-replay-{side}-{stage}.bin" for stage in ("pre", "post")]
        source_equal = all(path.read_bytes() == source.read_bytes() for path in files)
        raw_expected = {"ebx": 0x11223344, "esi": 0x22334455, "edi": 0x33445566, "ebp": 0x44556677}
        registers = all(abi[key] == value for key, value in raw_expected.items())
        stack = abi["stack_before"] == abi["stack_after"]
        controlled_fp = abi["cw_before"] == abi["cw_after"] == 0x027F and abi["mxcsr_before"] == abi["mxcsr_after"] == 0x1F80 and (abi["sw_before"] & 0x3800) == (abi["sw_after"] & 0x3800)
        query = events[0]["api"] == "glIsEnabled" and events[0]["result"] == case["entryQueryResult"] == row["query"]
        restored = events[-1]["api"] == ("glEnable" if row["query"] & 255 else "glDisable") and events[-1]["result"] == abi["eax"]
        native_emission = semantic(events) == semantic(native["events"])
        verdict = oracle.compare(case, oracle.normalize_events(events))
        delta = result["route_count_after"] - result["route_count_before"]
        route_ok = delta == (1 if side == "candidate" else 0)
        reports[side] = {"preSha256": sha(files[0]), "postSha256": sha(files[1]), "exactCapturedSource": source_equal, "registers": registers, "stack": stack, "controlledFP": controlled_fp, "query": query, "restoreEAX": restored, "routeDelta": delta, "exactNativeSemanticEvents": native_emission, "calls": len(events), "oracle": verdict, "ABI": abi}
        if not all((source_equal, registers, stack, controlled_fp, query, restored, route_ok, native_emission)) or verdict["result"] != "PASS":
            errors.append(f"{side}: replay source/ABI/trace/native-emission mismatch")
    a, b = row["original"], row["candidate"]
    paired_events = a["events"] == b["events"]
    paired_fp = all(a["abi"][key] == b["abi"][key] for key in ("eax", "cw_before", "cw_after", "sw_before", "sw_after", "mxcsr_before", "mxcsr_after"))
    if not paired_events or not paired_fp:
        errors.append("complete original/candidate event or FP/EAX mismatch")
    return {"result": "FAIL" if errors else "PASS", "scope": "captured-source CPU original/candidate parity and exact native semantic emission; driver residues remain separate", "directory": str(directory), "nativeRoute": route, "nativeCall": call, "nativeSourceSha256": sha(source), "replayTraceSha256": sha(directory / "draw-replay.jsonl"), "nativeTraceSha256": sha(native_directory / "draw-native.jsonl"), "exactPairedEvents": paired_events, "exactPairedFPAndEAX": paired_fp, "checks": reports, "errors": errors}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    parser.add_argument("--native", type=Path, required=True)
    parser.add_argument("--route", choices=("original", "replacement"), required=True)
    parser.add_argument("--call", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = check(args.directory, args.native, args.route, args.call)
    args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({"result": result["result"], "callsPerSide": result["checks"]["original"]["calls"], "errors": result["errors"]}, indent=2))
    if result["errors"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
