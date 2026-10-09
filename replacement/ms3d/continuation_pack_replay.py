"""Pack only the twelve independently approved typed GAME-0004 exports.

This utility reads the cleaned project exports, never their original source
paths. The immutable approval hash is authority; every packed wire token is
walked independently against its expected typed value through exact EOF.
"""
from pathlib import Path
import hashlib
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
APPROVAL_HASH = "bd945331a1861aefa6a1e99ce55f836053960ac93283691c797ba8331a8370e3"
APPROVED_DIRECTORY = (ROOT / "validation/GAME-0004-approved-native-original").resolve()
SNAPSHOT_BLOBS = (
    "actor232", "player176", "player_body2288", "opponent_body2288",
    "globals", "world_list12",
)
SNAPSHOT_SIZES = (232, 176, 2288, 2288, 8128, 12)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


class Wire:
    def __init__(self):
        self.data = bytearray()
        self.tokens = []

    def word(self, value):
        assert isinstance(value, int) and 0 <= value <= 0xffffffff
        self.data.extend(struct.pack("<I", value))
        self.tokens.append(("word", value))

    def text(self, value):
        data = value.encode("utf-8")
        assert len(data) <= 1024
        self.word(len(data))
        self.data.extend(data)
        self.tokens.append(("bytes", data))

    def blob(self, value, size=None):
        data = bytes.fromhex(value)
        if size is not None:
            assert len(data) == size
        self.word(len(data))
        self.data.extend(data)
        self.tokens.append(("bytes", data))

    def field(self, value):
        kind, name, offset = value
        assert kind in (0, 1, 2)
        self.word(kind)
        self.text(name)
        self.word(offset)

    def snapshot(self, value):
        for key, size in zip(SNAPSHOT_BLOBS, SNAPSHOT_SIZES):
            self.blob(value[key], size)
        for key in ("player_body", "opponent_body", "world"):
            self.word(value[key])
        assert value["global_span_sizes"] == [200, 128, 4, 4, 80, 4, 16, 4, 4, 4, 16, 16, 16, 7600, 4, 28]
        self.word(16)
        for size in value["global_span_sizes"]:
            self.word(size)

    def walk(self):
        """Read the serialized tokens independently; require exact EOF."""
        cursor = 0
        for kind, expected in self.tokens:
            if kind == "word":
                assert cursor + 4 <= len(self.data)
                actual = struct.unpack_from("<I", self.data, cursor)[0]
                assert actual == expected
                cursor += 4
            else:
                end = cursor + len(expected)
                assert end <= len(self.data) and self.data[cursor:end] == expected
                cursor = end
        assert cursor == len(self.data)
        return {"tokens": len(self.tokens), "consumedBytes": cursor, "exactEOF": True}


def expanded_arguments(descriptors):
    words = []
    for descriptor in descriptors:
        # Descriptor count differs from stack-word count for these two types.
        width = 4 if "BY_VALUE" in descriptor else 2 if "float64" in descriptor else 1
        words.extend([descriptor] * width)
    return words


def walk_reader_schema(data, record, expected_roles):
    """Independent count-driven walk of continuation_replay.cpp's Reader."""
    cursor = 0

    def word():
        nonlocal cursor
        assert cursor + 4 <= len(data)
        value = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        return value

    def blob(maximum):
        nonlocal cursor
        size = word()
        assert size <= maximum and cursor + size <= len(data)
        value = bytes(data[cursor:cursor + size])
        cursor += size
        return value

    def text():
        return blob(1024).decode("utf-8")

    def snapshot(expected):
        for key, size in zip(SNAPSHOT_BLOBS, SNAPSHOT_SIZES):
            assert blob(size) == bytes.fromhex(expected[key])
        for key in ("player_body", "opponent_body", "world"):
            assert word() == expected[key]
        assert word() == 16
        assert [word() for _ in range(16)] == expected["global_span_sizes"]

    def field():
        kind, name, value = word(), text(), word()
        assert kind <= 2
        return kind, name, value

    assert word() == 0x33474d44 and word() == 1
    assert text() and len(text()) == 64 and len(text()) == 64
    for key in ("module", "mode", "eax"):
        assert word() == record[key]
    for key in ("entry_fp544", "exit_fp544"):
        assert blob(544) == bytes.fromhex(record[key])
    snapshot(record["before"])
    snapshot(record["after"])
    role_count = word()
    assert role_count == len(expected_roles) <= 64
    roles = {word(): text() for _ in range(role_count)}
    assert roles == expected_roles
    event_count = word()
    assert event_count == len(record["events"]) <= 512
    for event in record["events"]:
        for key in ("call_rva", "owner", "eax", "rng_before", "rng_after"):
            assert word() == event[key]
        count = word()
        assert count == len(event["args"]) <= 4
        assert [word() for _ in range(count)] == event["args"]
        for _ in range(count + 2):
            kind, name, _ = field()
            if kind:
                assert name in roles.values()
        for key, maximum in (("owner_before", 2288), ("owner_after", 2288),
                             ("returned_bytes", 16), ("entry_fp544", 544),
                             ("exit_fp544", 544)):
            assert blob(maximum) == bytes.fromhex(event[key])
        for key in ("arg_before", "arg_after"):
            assert [blob(16) for _ in range(4)] == [bytes.fromhex(value) for value in event[key]]
    assert cursor == len(data)
    return {"events": event_count, "roles": role_count, "consumedBytes": cursor,
            "exactEOF": True, "descriptorWordCountsPreserved": True}


def pack(approved, metadata, destination):
    path = Path(approved["path"]).resolve()
    assert path.parent == APPROVED_DIRECTORY
    data = path.read_bytes()
    assert sha256(data) == approved["sha256"]
    assert approved["status"] == "APPROVED_TYPED_ORIGINAL_SOURCE_FOR_REPLAY_ONLY"
    record = json.loads(data)
    assert record["format"] == "GAME-0004-observer-v1"
    assert not record["candidate"] and not record["nested_whole_observed"]
    assert record["module"] == 0x400000 and record["mode"] in (0, 1)
    assert record["admitted"] and record["effects_before_fallback"] == 0
    assert record["original_invocations"] == 1 and record["candidate_invocations"] == 0
    sites = {int(site["callRva"], 16): site for site in metadata["calls"]}

    roles = {int(address): name for name, address in approved["pointerRoles"].items()}
    for name, address in approved["localPointerRoles"].items():
        assert address not in roles
        roles[int(address)] = "L-" + name

    def pointer_role(address):
        if address not in roles:
            assert record["module"] <= address < record["module"] + 0x19f000
            roles[address] = "G-%08X" % address
        return 1, roles[address], 0

    def raw(value):
        return 0, "", value

    normalized = []
    for ordinal, event in enumerate(record["events"]):
        assert event["ordinal"] == ordinal
        site = sites[event["call_rva"]]
        assert event["return_rva"] == int(site["returnRva"], 16)
        assert event["identity"] == site["identity"]
        descriptors = expanded_arguments(site["args"])
        assert len(descriptors) == len(event["args"]) <= 4
        assert len(event["arg_before"]) == len(event["arg_after"]) == 4
        owner = raw(event["owner"]) if site["abi"].startswith("cdecl") else pointer_role(event["owner"])
        args = [pointer_role(arg) if "pointer" in descriptor else raw(arg)
                for arg, descriptor in zip(event["args"], descriptors)]
        if site["ret"] == "output_pointer_EAX":
            eax = pointer_role(event["eax"])
        elif site["ret"] == "element_pointer_EAX":
            index = event["args"][0]
            assert event["eax"] == event["owner"] + 4 * index
            eax = 2, pointer_role(event["owner"])[1], index
        else:
            eax = raw(event["eax"])
        normalized.append((owner, eax, args))

    assert len(roles) <= 64 and len(record["events"]) <= 512
    wire = Wire()
    wire.word(0x33474d44)
    wire.word(1)
    for value in (approved["id"], approved["sha256"], approved["sourceSha256"]):
        wire.text(value)
    for key in ("module", "mode", "eax"):
        wire.word(record[key])
    wire.blob(record["entry_fp544"], 544)
    wire.blob(record["exit_fp544"], 544)
    wire.snapshot(record["before"])
    wire.snapshot(record["after"])
    wire.word(len(roles))
    for address, name in roles.items():
        wire.word(address)
        wire.text(name)
    wire.word(len(record["events"]))
    for event, (owner, eax, args) in zip(record["events"], normalized):
        for key in ("call_rva", "owner", "eax", "rng_before", "rng_after"):
            wire.word(event[key])
        wire.word(len(event["args"]))
        for arg in event["args"]:
            wire.word(arg)
        wire.field(owner)
        wire.field(eax)
        for arg in args:
            wire.field(arg)
        for key in ("owner_before", "owner_after", "returned_bytes"):
            wire.blob(event[key])
        for key in ("entry_fp544", "exit_fp544"):
            wire.blob(event[key], 544)
        for key in ("arg_before", "arg_after"):
            for value in event[key]:
                wire.blob(value)
    audit = wire.walk()
    reader_audit = walk_reader_schema(wire.data, record, roles)
    destination.write_bytes(wire.data)
    assert destination.read_bytes() == wire.data
    return {
        "id": approved["id"], "approvedTypedExportPath": str(path),
        "sourceExportSha256": approved["sha256"], "sourceSha256": approved["sourceSha256"],
        "packed": str(destination.resolve()), "packedSha256": sha256(wire.data),
        "packedBytes": len(wire.data), "events": len(record["events"]),
        "pointerRoles": roles, "wireWalk": audit, "readerSchemaWalk": reader_audit,
    }


def main():
    approval_path = ROOT / "validation/GAME-0004-approved-native-original-manifest.json"
    approval_bytes = approval_path.read_bytes()
    assert sha256(approval_bytes) == APPROVAL_HASH
    approval = json.loads(approval_bytes)
    assert approval["status"] == "APPROVED_TYPED_SOURCES_ONLY_NOT_GAMEPLAY_EQUIVALENCE"
    assert len(approval["records"]) == 12
    metadata_path = ROOT / "specs/gameplay/GAME-0004-observer-metadata.json"
    metadata_bytes = metadata_path.read_bytes()
    assert sha256(metadata_bytes) == approval["observerMetadataSha256"]
    metadata = json.loads(metadata_bytes)
    destination = ROOT / "validation/GAME-0004-packed-replay"
    destination.mkdir(exist_ok=True)
    records = [pack(item, metadata, destination / (item["id"] + ".bin"))
               for item in approval["records"]]
    manifest = {
        "status": "PACKED_APPROVED_SOURCE_ONLY_NOT_EQUIVALENCE",
        "sourceManifestSha256": APPROVAL_HASH,
        "packerPath": str(Path(__file__).resolve()),
        "packerSha256": sha256(Path(__file__).read_bytes()),
        "observerMetadataSha256": sha256(metadata_bytes),
        "wireSchema": "GAME0004 continuation_replay.cpp Reader DMG3 version1",
        "records": records,
    }
    (destination / "manifest.json").write_text(json.dumps(manifest, indent=2))
    print("Packed twelve approved typed sources; every token verified through exact EOF.")


if __name__ == "__main__":
    main()
