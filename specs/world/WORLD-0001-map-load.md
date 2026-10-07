# WORLD-0001 — Original map-load orchestration replacement boundary

Status: IMPLEMENTED; CALLBACK_ISOLATED_DIFFERENTIAL_PASS (2648 scenarios); NATURAL_MAP_1_SMOKE_PASS. Implementation: replacement/ms3d/world.cpp and world_runtime.cpp. Independent report: validation/WORLD-0001.md. Natural map1 route completed with original opaque dependencies and user-confirmed gameplay. Another installed map and teardown telemetry remain unverified; supported live scope below is unchanged. Agent 1 specification, 2026-10-04; completion recorded by coordinator.

## Scope, provenance and interception

Original program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`; preferred base 0x00400000. Reverified by fresh IDA survey session 8ce17abd. Original copied binary/database and raw observations stay under `C:\Users\ADMIN\Boxer-analysis\fresh-coldet`. No legacy OpenBoxer source/assets used, no replacement implementation written by Agent 1.

Body RVA 0x2E970 (VA 0x42E970); routing thunk RVA 0x10BE (VA 0x4010BE); observed direct-call site RVA 0x4AD06 (VA 0x44AD06). All addresses resolve relative to verified live module. Boundary is game-specific orchestration; preserved opaque callbacks own texture, model postprocessing, physics/ColDet, entity setup and audio behavior. This specification does not replace their implementations.

Approved runtime route: redirect the verified 5-byte routing thunk to independent replacement with a generic x86 relative tail jump. Its known unmodified target is body RVA 0x2E970; leave body intact and call body directly for fallback. No trampoline copying original body is needed for this route. Preserve hook bytes only in hook engineer/validator evidence outside repo; implementer gets RVA/hash/span/target/interface metadata, never original instructions. Root must hash-guard original module and validate original thunk target before redirecting. Restore generic jump to known body on removal or have hook engineer restore its privately saved bytes. Patch synchronization/protection/flush remain hook-engineering responsibilities. This route intercepts the observed caller; direct body callers would bypass it (none found in bounded static xrefs).

Approved metadata-only implementation alternative: after guarding the whole on-disk executable identity and corresponding loaded module identity, install a synthesized generic E9 relative jump from thunkRVA to replacement; uninstall synthesizes the same generic jump from thunkRVA to approved original-bodyRVA. Agent2 does not need to obtain, save, or copy original machine bytes. Validator independently checks route correctness against private evidence. Existing hook conflicts/live-modified thunk are excluded; original fallback calls bodyRVA directly. This approval concerns this precisely identified routing boundary, not arbitrary function patching.

## ABI and initial implementation scope

Confidence CONFIRMED from body/caller instructions: cdecl, no this pointer, selector int32 at entry stack+4. Observed caller additionally passes two stack slots both equal 1, then caller pops 12. Body reads only selector; +8/+12 are ignored by this unit. A replacement can expose cdecl(selector, unused1, unused2) for that caller, but it must not clean the stack. EBP/EBX/ESI/EDI preserved; EAX returns the final no-argument delegate's value, not boolean success. EAX/ECX/EDX volatile. Actual ordinary final delegate returns 100 (HIGH); forwarding callback result is the proper contract.

Initial live scope: normal initialized game, original dependencies available, selectors 1..7 with installed approved map/light fixtures, successful original allocations/construction/callback completion, original stack/FP environment. Selector8 references missing installed fixtures and out-of-range selectors still have documented normal branch behavior below, suitable for callback-isolated validation; first natural-game deployment should fallback to original for these cases. Exceptions/allocation failure, destructive reentry and unspecified dependency failures retain original fallback selected before mutation. No Clang/original exception interoperation is approved.

## Ordered observed behavior

Confidence HIGH throughout, with exact immediate stores/ABI CONFIRMED where instruction-inspected. Unknown semantic labels remain address identifiers. Read gates/globals at the points stated; preserved callbacks may modify globals, so do not precompute later conditions from initial snapshots.

1. Store uint32 0 at global VA 0x585594.
2. If byte 0x577F9F nonzero, call audio-reset callback at RVA 0x169F with ECX = module+0x175E20 (VA 0x575E20). Actual callback stops its source, seeks its Vorbis stream to PCM offset0, rewinds source. Return ignored.
3. Request 0x8D9C4 bytes through original allocate callback RVA 0x95420. If nonnull, construct model through RVA 0x1956 with ECX allocation. Assign constructor result (or zero for null allocation) to global pointer VA 0x585560. No prior object/global target is deleted by this unit.
4. For selector N=1..8 call this object's dynamic virtual slot+4 with ECX object and filename `.\base\maps\N\map.ms3d`. These are independent equality conditions; other selectors invoke no map filename call. Loader result ignored. Call preserved reloadTextures callback RVA 0x1A64 on global map pointer unconditionally after filename conditions, regardless of loader success.
5. Request and construct another 0x8D9C4 model identically. Assign result to VA 0x585564. For selector1..8 invoke virtual slot+4 with `.\base\maps\N\light.ms3d`. Result ignored. Call reloadTextures on global light pointer unconditionally.
6. On light object ECX, call callbacks RVA 0x1690 then 0x1974, in that order. The second makes GL calls; preserve both as opaque original dependencies.
7. Invoke no-argument callbacks RVA 0x10E1, 0x127B, 0x1131, in order. The third itself calls original rand; do not conflate its RNG use with the later explicit music selection draw.
8. If byte VA 0x577FB8 nonzero, call cdecl callback RVA 0x13D4 with the current int32 global VA 0x584770 (NOT necessarily input selector). Return ignored.
9. If current int32 global VA 0x585594 equals exactly0 or1, invoke no-argument callback RVA 0x1E2E. This is not an unsigned range test that includes other numbers.
10. Call original rand once at RVA 0x9A750, take C signed remainder modulo4, store int32 at VA 0x575D94. Original rand normally returns nonnegative; do not use an independently seeded replacement RNG.
11. Test selection global for0,1,2,3 independently in ascending order. For match K, construct one opaque16-byte filename holder through thiscall callback RVA 0x1924 on local temporary storage, passing `.\base\music\(K+1).ogg` and pointer to a separate scratch4-byte local. Original scratch/storage starts as 0xCC-filled debug-stack data. Constructor copies scratch first byte and initializes holder through original routines. Then call thiscall audio-load callback RVA 0x1FC8 with ECX module+0x17B2B0 (VA0x57B2B0), stack args(holder pointer,1,1). Then destroy holder via thiscall RVA0x1555. No return is checked. Holder ABI remains opaque; allocate16 bytes aligned to4 and initialize holder/scratch payload to0xCC for parity with this original debug build. Constructor returns same holder pointer. A callback-isolated validator may replace these three operations with recorders instead of invoking file/audio APIs. Original exception cleanup destroys live holder if audio call throws; this path remains outside first replacement scope.
12. If byte VA0x577F9F nonzero now, call thiscall audio-play RVA0x1BF9 with ECX VA0x57B2B0. Then store bits0xBFC00000 at VA0x56FE70 (-1.5f if interpreted float32); store byte1 at VA0x5853A2.
13. Call original imported SetCursorPos(320,240), screen coordinates, return ignored. This side effect remains present even in callback-isolated tests through a recorder rather than moving the user's cursor.
14. Perform initial direct-state stores listed below.
15. Conditional state block: if byte VA0x577FB8 nonzero, evaluate independently, in order: (global584788==0 && global584798==1) -> store float bits-16 at584778; (584788==0 &&584798>1) ->0; (584788>0 &&584798==1)->-8; (584788>0 &&584798>1)->0. Comparisons are signed int32. Negative584788 leaves earlier initial0 unchanged. Global584798 was just set1 but preserve gates if instrumentation/global access behavior matters.
16. Perform final direct-state stores below. If current global585594==1, call cdecl RVA0x14EC with one 32-bit stack slot2000 (callee interprets low16). If current global585594==2, call cdecl RVA0x12FD with2000. These conditions are independent and evaluated in order.
17. Invoke no-argument callback RVA0x1122, return its EAX value. Original body never synthesizes a success status.

## Direct state writes

All addresses below are preferred VAs; subtract0x400000 for module RVAs. Width must be respected. Meanings UNKNOWN except identified model pointers/music selection. Store exact bits, not invented semantic names.

Initial phase after SetCursorPos:

| VA | Width | Bits/value |
|---|---|---|
| 0x58477C | 4 | 0x3F800000 |
| 0x584780 | 4 | 0 |
| 0x584794 | 1 | 0 |
| 0x584798 | 4 | 1 |
| 0x58479C | 1 | 0 |
| 0x58479D | 1 | 0 |
| 0x584778 | 4 | 0, then conditional0xC1800000(-16) or0xC1000000(-8) above |

Final phase:

| VA | Width | Bits/value |
|---|---|---|
| 0x56FF18 | 4 | 0x3F800000 |
| 0x585570 | 4 | 0 |
| 0x56FF1C | 4 | 0x40A00000 |
| 0x56FF20 | 4 | 0x3F800000 |
| 0x585574 | 4 | 0xC0000000 |
| 0x56FF24 | 4 | 0x40A00000 |

Other explicit writes are585594=0,585560/585564 model pointers,575D94 music selection,56FE70=0xBFC00000,5853A2=1. Preserved dependencies make additional writes/allocations/callbacks; these must not be suppressed. This table describes this unit's direct writes, not complete transitive footprint.

## Approved dependency callback ABI table

RVA values. thiscall means ECX object, explicit arguments in 4-byte stack slots, callee-cleaned; cdecl explicit args caller-cleaned. No-argument callbacks return values ignored unless specified. Names are interface descriptions, not recovered original identifiers.

| RVA -> body RVA | ABI / args / output |
|---|---|
| 0x95420 | cdecl allocate(uint32 bytes)->pointer; original heap, see IO-0003 |
| 0x1956 ->0x15890 | thiscall constructModel(), EAX same object pointer |
| object vtable+4 | thiscall loadModel(const char*), AL result ignored |
| 0x1A64 ->0x16680 | thiscall reloadTextures(), return ignored |
| 0x1690 ->0x27AF0 | thiscall lightStageA(), no explicit args |
| 0x1974 ->0x35D90 | thiscall lightStageB(), no explicit args, GL side effects |
| 0x10E1 ->0x19E90 | cdecl stageA(), no args |
| 0x127B ->0x2E530 | cdecl stageB(), no args |
| 0x1131 ->0x2E760 | cdecl stageC(), no args, transitive RNG draw |
| 0x13D4 ->0x2A010 | cdecl optionalStage(int32 current584770) |
| 0x1E2E ->0x2CA50 | cdecl stageD(), no args; opaque setup, semantic scope UNKNOWN |
| 0x9A750 | cdecl original rand()->int32 |
| 0x1924 ->0x87D0 | thiscall constructHolder(const char*,scratch4*), callee pops8, EAX holder |
| 0x1FC8 ->0x53120 | thiscall loadAudio(holder*,bool slot,bool slot), callee pops12, result ignored |
| 0x1555 ->0x8840 | thiscall destroyHolder(), no args |
| 0x169F ->0x54270 | thiscall stopSeekRewindAudio(), no args |
| 0x1BF9 ->0x53F50 | thiscall playAudio(), no args |
| import SetCursorPos | stdcall(int32 X,int32 Y), callee pops8 |
| 0x14EC ->0x1A6F0 | cdecl callback2000A(one32 slot, interpreted int16) |
| 0x12FD ->0x1AA10 | cdecl callback2000B(one32 slot, interpreted int16) |
| 0x1122 ->0x22650 | cdecl finalStage()->int32 forwarded; ordinary body writes100 slots in each of two state arrays and returns100 |

The holder constructor/audio ABI differs from IDA's guessed stdcall thunk prototypes; body/callsite instruction review established ECX and explicit cleanup. Clang callback declarations must use this approved table rather than those guesses.

## Failure and cleanup behavior

No ordinary rollback exists in this function. Loader boolean failures are ignored, reload/postprocessing still run. No old map/light object is destroyed before globals are overwritten. Null allocation creates zero global then a selected filename branch would dereference zero; do not invent a clean false return. Original constructor exception cleanup frees the corresponding raw model block, but completed model ownership is global and is not rolled back by this unit. Original music-holder exception cleanup uses its destructor. Dependency exceptions and later crash behavior UNKNOWN in detail: fallback required for first implementation's unsupported paths; there is no approved new RAII rollback semantics that changes observable game state.

For out-of-range selector the function still allocates two empty original models, reloads them and performs every remaining stage, but neither virtual filename callback fires. For selector8 paths are explicit even though installed archive inspection found no map8 fixtures. Callback-stub tests can safely verify these branches; live tests should preserve original fallback.

## Differential checkpoints

First test use an initialized original-process harness with all dependency boundaries replaced by typed recorders. Record ordered callback name/args, selector, path strings, two allocated object identities, global writes and return; normalize pointers and opaque temp addresses. Have finalStage return a sentinel to prove EAX forwarding. Cover selector1..8 and out-of-range0/9/-1, sound gate0/1, mode gate0/1, exact585594 values0/1/2/negative (set by earlier stub), signed584788 casesnegative/0/positive, original rand stub outputs0..3 and a nonnegative value>3. Avoid real GL/audio/cursor effects in this stage. Restore dependencies and validate natural map1 then another installed map with original MS3D/reload/physics/audio callbacks preserved. Track delegated MS3D success/fallback separately. EXACT callback/order/direct bit-state comparison; model payload follows IO-0003. RNG seed/state and dependency transitive RNG calls must match original.

Natural route evidence: only observed static caller is44AD06, guarded by menu-state global577F90==44 and coordinate rectangles before call; it passes selector from current584770 and two1 slots. Frame function448060 creates/loads nested `base/maps/1/lamp/lamp.ms3d` after its startup timer exceeds8 while56FF28 is set; this is a separate model path, not proof map load occurred. Names/signature guesses on misbounded function44ABFD are not trusted. Per-call logging is needed to identify any observed startup successes. No UI input is issued by Agent1.

Raw hook bytes outside repo: `C:\Users\ADMIN\Boxer-analysis\fresh-coldet\WORLD-0001-hook-evidence.json`. Evidence includes complete body analysis42E970, caller instructions44AD06, callback thunks/body ABI, original header/hash survey, instruction-inspected music holder ABI/stack initialization and final stores. No decompiler body or original instructions are embedded here.
