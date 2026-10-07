# WORLD-0001 — Approved disposable differential harness metadata

Agent1, 2026-10-04. Companion to WORLD-0001-map-load.md, semantic/ABI/address data only. Original module SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`, preferred base0x400000, PE loaded image size0x19F000. Full-file identity includes overlay; image section equality alone is not a substitute. Original hash reverified through fresh IDA survey. Root private evidence independently establishes thunkRVA0x10BE -> bodyRVA0x2E970 for that exact module; implementation agent does not obtain original code bytes.

## Test-process-only dependency redirection

Use a dedicated original test process whose main thread is parked at the CRT-initialized checkpoint documented in IO-0003. Ensure no thread can execute affected original routines while patching. Withhold normal game resume after callback-boundary interception: this disposable process is terminated on completion and never reused for a natural-game test. No restoration is required for this disposable mode.

In this mode, direct allocator entryRVA0x95420 and rand entryRVA0x9A750 can be redirected by a synthesized generic5-byte jump to typed recorders. Their original bodies must never execute after interception; no trampoline or original bytes are read/copied by implementation agent. This is a replacement-entry redirect, so overwritten original instruction alignment is irrelevant once routing guarantees the original entry reaches only the recorder. It is not approval to resume execution at an overwritten mid-instruction or call the former body past the redirect. Original map bodyRVA0x2E970 remains unchanged and executes the original orchestration under those typed recorders.

Other5-byte original dependency thunks in WORLD-0001 callback table may likewise redirect to recorders, leaving their bodies untouched. Map originalbody must be invoked directly for the original side. Candidate side calls independent replacement directly with the same callback environment. Do not redirect map thunk until dependency-stub tests conclude if that would alter original invocation selection. If worker needs to allocate its own recorder memory, use its private runtime/system allocator, not the now-stubbed original allocator. DLL/dependency allocations before the recorder phase must complete before redirecting. Remaining original threads must stay quiescent; any unexpected concurrent allocator entry invalidates isolated comparison.

Patch memory protection temporarily for writes and flush instruction cache for synthesized jumps. These are generic platform operations, not translation of original implementation. Root/validator independently validates module identity and routes before mutation; loaded test-copy path, file SHA, mapped main module base/size/path and unchanged-body identity should agree. If live route was modified by another hook, stop disposable test setup; metadata-only synthetic restore is approved only for the root-prevalidated baseline.

## Cursor import data slot

CONFIRMED independent IDA import query: USER32 SetCursorPos IAT slot VA0x58CCA0, module RVA0x18CCA0. Replace this4-byte data-slot function pointer with a private stdcall recorder taking two int32 slots and cleaning8; record320,240 and return a fixed BOOL sentinel, ignored by original map unit. Do not patch shared USER32 code or move the user's actual cursor. Original map body calls through this slot. This import-data redirection requires no original instruction bytes.

## Fake model/vtable contract

Yes: fake constructor/vtable stubs are sufficient for callback-isolated orchestration (HIGH). Original unit allocates raw model bytes, calls constructor callback, stores returned pointer, then reads object word0 as vtable pointer and invokes word at vtable+4 with filename. Later map/light methods are direct dependency thunk calls redirected to recorders. No other model fields are read directly by map body.

Recorder allocator receives byte_count0x8D9C4 exactly twice and returns distinct writable allocations. Fake constructor is thiscall ECX allocation with no explicit args, writes a stable fake vtable pointer at object+0, returns same pointer EAX. Fake vtable has at least two32-bit slots; slot+4 is thiscall filename recorder, cleaning4 and returning AL result (its value is ignored). Slot0 may point to a defensive destructor recorder but is not called on the successful normal path. Initialize model storage in a deterministic way for test only; this test does not replace actual IO-0003 payload equivalence or original ownership validation. Pointer identities are normalized separately as mapObject/lightObject; do not compare raw addresses across runs. Vtable and filename pointer addresses are also normalized; filename content is compared exactly.

## Relevant state snapshot and mutation matrix

Offsets are preferred VAs; subtract0x400000 for RVAs. Snapshot/reset these state bytes for original/candidate between cases:

- Gates/read inputs: byte577F9F, byte577FB8, int32 584770, int32 584788.
- Read/write decision state: int32 585594, int32 575D94, int32 584798.
- Owned pointer outputs: ptr32 585560 and585564, normalize values to mapObject/lightObject.
- Direct4-byte output states: 56FE70,58477C,584780,584778,56FF18,585570,56FF1C,56FF20,585574,56FF24.
- Directbyte output states: 5853A2,584794,58479C,58479D.

Opaque object addresses575E20 and57B2B0 are ECX callback identities, compared as fixed module-relative addresses, not dereferenced by recorder operations. Holder temporary16-byte and scratch4-byte addresses vary; normalize their identities and verify constructor/audio/destructor use the same holder for each matching music branch.

Original first store585594=0 overwrites test initial value; to exercise later branches, a recorder for stageC (RVA0x1131) or another earlier setup callback must intentionally set585594 to0/1/2/-1 before conditional stageD. Set the same deterministic mutation script in original/candidate. A separate script may let optionalStage or stageD change585594 again to exercise late2000 callbacks; these conditions read live state. FinalStage should return a caller-chosen sentinel distinct from100 to verify EAX forwarding. Its optional own writes should be identical for both runs or omitted, and not confused with orchestration's direct-write list.

584798 is explicitly overwritten to1 before conditional584778 stores. Consequently >1 branches are unreachable in ordinary single-thread recorder execution unless the test deliberately emulates asynchronous mutation; do not report them exercised merely by setting initial584798>1. Negative/0/positive584788 and gate577FB8 exercise reachable branches. Similarly, changes to577F9F between first/last audio gates can be scripted by earlier callbacks to prove gates are read separately, while a basic matrix may keep them stable.

For music-selection tests, rand recorder supplies deterministic values; original stageC is itself replaced by a no-argument recorder, eliminating its internal RNG draw from this isolated test. Natural validation must restore the original stageC and original rand to preserve their combined RNG behavior. Do not interpret isolated callback parity as proof of transitive physics/audio/GL equivalence.

Record ordered callback IDs, argument values/content, caller-derived normalized object identities, state snapshots at callback boundaries, final direct-state values, returnEAX, and ABI stack/nonvolatile-register checks. Exact globals/callbacks/path comparisons constitute scoped PASS only when independently executed against the unchanged original map body and independent candidate. No binary similarity requirement applies.
