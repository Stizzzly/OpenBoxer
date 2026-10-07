"""Validate complete typed native draw traces against source and contract."""
import argparse
import hashlib
import importlib.util
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("draw_oracle", ROOT / "validation/RENDER-0005-oracle.py")
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)
APIS = ["glIsEnabled", "glMaterialfv", "glMaterialf", "glBindTexture", "glEnable", "glDisable", "glBegin", "glNormal3fv", "glTexCoord2f", "glVertex3fv", "glEnd", "material_vector", "material_scalar"]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(directory, selected_route=None, peer_directory=None):
    captures = [json.loads(line) for line in (directory / "draw-native.jsonl").read_text().splitlines() if line.strip()]
    if selected_route:
        captures = [capture for capture in captures if capture["route"] == selected_route]
    errors, reports = [], []
    for capture in captures:
        route, call = capture["route"], capture["call"]
        pre = directory / f"draw-native-{route}-{call:04d}-pre.bin"
        post = directory / f"draw-native-{route}-{call:04d}-post.bin"
        case = oracle.parse_snapshot(pre)
        events = capture["events"]
        case["entryQueryResult"] = events[0]["result"]
        verdict = oracle.compare(case, oracle.normalize_events(events))
        digest = oracle.ordered_digest(events)
        counts = Counter(event["api"] for event in events)
        immutable = pre.read_bytes() == post.read_bytes()
        restore = "glEnable" if events[0]["result"] & 255 else "glDisable"
        final = events[-1]
        restored = final["api"] == restore and final["args"] == [0x0DE1] and final["result"] == capture["result"]
        actual_counts = [counts[api] for api in APIS]
        cw = capture["cw"] == capture["after_cw"] == 0x027F
        depth = capture["sw"] & 0x3800 == capture["after_sw"] & 0x3800
        flags = {"stickyBefore": capture["sw"] & 63, "stickyAfter": capture["after_sw"] & 63, "swBefore": capture["sw"], "swAfter": capture["after_sw"], "mxcsrBefore": capture["mxcsr"], "mxcsrAfter": capture["after_mxcsr"]}
        report = {"route": route, "call": call, "modelCounts": [case["meshCount"], len(case["materials"]), len(case["triangles"]), len(case["vertices"])], "queryEAX": case["entryQueryResult"], "queryAL": case["entryQueryResult"] & 255, "finalEAX": capture["result"], "oracle": verdict, "digest": digest, "digestMatches": digest == capture["digest"], "counts": actual_counts, "countsMatch": actual_counts == capture["counts"], "sourceImmutable": immutable, "restoreEAXMatches": restored, "cw027F": cw, "depthPreserved": depth, "FP": flags, "preSha256": sha(pre), "postSha256": sha(post)}
        if verdict["result"] != "PASS" or digest != capture["digest"] or actual_counts != capture["counts"] or not immutable or not restored or not cw or not depth:
            errors.append(f"{route}/{call}: contract/source/trace/ABI-FP invariant mismatch")
        reports.append(report)
    peers = []
    if peer_directory:
        reference = [json.loads(line) for line in (peer_directory / "draw-native.jsonl").read_text().splitlines() if line.strip()]
        for capture in captures:
            peer = next(c for c in reference if c["route"] == "original" and c["call"] == capture["call"])
            semantics = lambda events: [{key: e[key] for key in ("api", "dispatch", "args", "role", "payload")} for e in events]
            match = semantics(capture["events"]) == semantics(peer["events"])
            query = capture["events"][0]["result"] == peer["events"][0]["result"]
            peers.append({"call": capture["call"], "exactAllSemanticEvents": match, "exactQueryEAX": query, "exactAllCallbackResultsObserved": [e["result"] for e in capture["events"]] == [e["result"] for e in peer["events"]]})
            if not match or not query:
                errors.append(f"peer original/{capture['call']}: exact semantic stream/query mismatch")
    return {"result": "FAIL" if errors else "PASS", "scope": "complete native emission/source/restoration invariants; no pixel parity", "directory": str(directory), "traceSha256": sha(directory / "draw-native.jsonl"), "captures": reports, "peerOriginalComparisons": peers, "errors": errors}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--route", choices=("original", "replacement"))
    parser.add_argument("--peer", type=Path)
    args = parser.parse_args()
    result = check(args.directory, args.route, args.peer)
    args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({"result": result["result"], "captures": len(result["captures"]), "errors": result["errors"]}, indent=2))
    if result["errors"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
