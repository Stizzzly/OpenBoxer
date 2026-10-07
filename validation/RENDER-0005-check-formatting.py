"""Validate cosmetic rebuild using replacement binaries only, never original."""
import hashlib
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT / "replacement/ms3d"


def u16(data, offset):
    return struct.unpack_from("<H", data, offset)[0]


def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def metadata(data):
    assert data[:2] == b"MZ"
    pe = u32(data, 0x3C)
    assert data[pe:pe + 4] == b"PE\0\0"
    optional = pe + 24
    assert u16(data, optional) == 0x10B
    sections = optional + u16(data, pe + 20)
    export_rva = u32(data, optional + 96)
    export_offset = None
    for i in range(u16(data, pe + 6)):
        section = sections + 40 * i
        rva = u32(data, section + 12)
        size = max(u32(data, section + 8), u32(data, section + 16))
        if rva <= export_rva < rva + size:
            export_offset = u32(data, section + 20) + export_rva - rva
            break
    assert export_offset is not None
    return {"COFF TimeDateStamp": pe + 8, "PE CheckSum": optional + 64, "Export TimeDateStamp": export_offset + 4}


def sha(data):
    return hashlib.sha256(data).hexdigest().upper()


def main():
    frozen = (PROJECT / "frozen-render0005/ms3d_replacement.dll").read_bytes()
    rebuilt = (PROJECT / "build/ms3d_replacement.dll").read_bytes()
    assert len(frozen) == len(rebuilt) == 3237371
    ranges = metadata(frozen)
    assert ranges == metadata(rebuilt)
    changed = [i for i, (a, b) in enumerate(zip(frozen, rebuilt)) if a != b]
    allowed = {offset + delta for offset in ranges.values() for delta in range(4)}
    assert len(changed) == 6 and all(i in allowed for i in changed)
    mask = lambda data: bytes(0 if i in allowed else value for i, value in enumerate(data))
    assert mask(frozen) == mask(rebuilt)
    manifest = json.loads((PROJECT / "RENDER-0005-freeze.json").read_text())
    assert all(sha(Path(entry["path"]).read_bytes()) == entry["sha256"] for entry in manifest)
    baseline = json.loads((ROOT / "validation/RENDER-0005-prior-core-hashes.json").read_text())
    assert all(sha((PROJECT / entry["name"]).read_bytes()) == entry["sha256"] for entry in baseline)
    preserved = json.loads((PROJECT / "RENDER-0005-preserved-units.json").read_text())
    assert all(sha((PROJECT / entry["file"]).read_bytes()) == entry["sha256"] for entry in preserved)
    report = {"result": "PASS", "scope": "replacement-only cosmetic rebuild and source preservation", "frozenSha256": sha(frozen), "rebuiltSha256": sha(rebuilt), "fileLength": len(frozen), "differingBytes": len(changed), "differences": [{"field": field, "offset": hex(offset), "differingBytes": sum(offset <= i < offset + 4 for i in changed)} for field, offset in ranges.items()], "allNonMetadataBytesIdentical": True, "freezeEntriesVerified": len(manifest), "rootPriorCoreEntriesVerified": len(baseline), "preservedSourceABIEntriesVerified": len(preserved), "formattingNonWhitespaceEquality": "Agent2 reported; compiled non-metadata byte identity independently verified"}
    (ROOT / "validation/RENDER-0005-formatting-independent.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
