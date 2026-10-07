# IO-0003 — First MS3D loader replacement contract

Status: IMPLEMENTED; SCOPED_DIFFERENTIAL_PASS. Independent implementation: replacement/ms3d. Validation: validation/MS3D-0001.md. All sixteen approved fixtures match exactly in the original process; ordinary missing-file behavior is retained through fallback. Natural startup invoked the replacement successfully three times; the user subsequently reports that gameplay works perfectly. This does not expand the supported boundary below.
Agent 1, 2026-10-04. The contract is behavioral and ABI data. No original assembly/decompiler body is provided to Agent 2.

## Supported boundary and provenance

Original program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`; preferred image base 0x00400000. Loader body VA 0x004159B0 / RVA 0x159B0, vtable-route thunk VA 0x00401C21 / RVA 0x1C21. Addresses are resolved against the live original module base. Independently verified fresh IDA session a6e1ce44; evidence outside replacement repository. No legacy OpenBoxer code/assets used.

Initial implementation scope: an original-constructor-initialized, empty live model object; ordinary accessible ASCII filename resolving to any unchanged original fixture in IO-0003-approved-fixtures.json; successful allocations; normal round-to-nearest floating environment. Missing-file behavior is separately specified for an ordinary nonexistent filename. Fixtures outside that approved manifest, existing nonempty objects, null filenames/objects, malformed/truncated files, concurrent changes, exceptional floating environments, allocation failure and exception paths must take the preserved original fallback. Fallback decision must occur before original object fields are mutated; late fallback cannot safely follow partial replacement allocation/writes. Those cases are UNKNOWN rather than newly defined behavior.

Fixture: installed `base/maps/1/map.ms3d`, SHA-256 `36B60F829267A4A68FC35CE97B145490E6AA3CABB07AD8531CA6338454C58775`, 238581 bytes. Independently read original fixture; magic MS3D000000, version 4. Vertex/triangle/group/material counts: 1806/2788/27/27. Parsing ends at byte 238567; remaining 14 bytes are ignored by this loader. All material texture strings terminate within their 128-byte fields. All t coordinates finite, range -28.642353057861328..39.40823745727539; smallest nonzero absolute t 0.0004942417144775391. Fixture inspection CONFIRMED; actual in-process invocation still requires validator observation.

Scope extension, 2026-10-04 (CONFIRMED file inspection): recursively enumerated the installed original base tree and checked all 16 MS3D paths, including both `base/maps/1/lamp.ms3d` and `base/maps/1/lamp/lamp.ms3d`. Every file has supported header, bounded vertex/triangle/group/material sections, valid vertex/triangle/material references, nonzero section counts, ASCII texture strings terminated within the 128-byte disk field, finite t values with nonzero magnitude between 2^-24 and 2^24, and 14 trailing bytes ignored by the original loader. Full relative-path/hash/count/end/trailing/numerical/filename records are approved behavioral fixture data in `IO-0003-approved-fixtures.json`. The valid-file contract above applies to each manifest row marked approved; this is an evidence-based scope extension, not a runtime PASS. Maps 1..7 are present; map8 paths in binary remain unsupported because no installed fixture was found. Manifest content is data extracted from original files, never source implementation.

## Callable ABI

Confidence CONFIRMED (instruction review): Microsoft x86 thiscall. At function entry ECX = original object pointer, stack +4 = const char* filename, stack +0 = return address. Exactly one explicit 32-bit argument is callee-cleaned (4 bytes). Boolean result AL=0 on observed stream-open failure, AL=1 on successful completion. Upper EAX bits are unspecified. EBP/EBX/ESI/EDI and caller stack restored; EAX/ECX/EDX volatile. No SSE-vector or C++ object return ABI. Independent hook glue must satisfy this contract; a cdecl two-argument function is not directly interchangeable.

Do not expose C++ exceptions across Clang/original boundaries. No original STL/ifstream object may be laid out by guessing Clang library structures. The independent implementation can read immutable supported fixture bytes through its own file layer; file semantics compared here are bytes/path resolution, successful close, result/object state, and no texture load. Exact original CRT stream-buffer bookkeeping/internal allocation trace is excluded from this first compatibility boundary and remains UNKNOWN.

## Original object fields

All fields below are 32-bit little-endian. Pointer width 4. Counts are zero-extended disk u16 values represented as nonnegative int32. No object bytes outside these eight fields are modified by successful loader parsing. Vtable stays intact. Confidence HIGH, corroborated by constructor, loader, destructor.

| Object offset | State |
|---|---|
| +0x8D9A4 | group/mesh count |
| +0x8D9A8 | pointer to count × 12 runtime group records |
| +0x8D9AC | material count |
| +0x8D9B0 | pointer to count × 80 runtime material records |
| +0x8D9B4 | triangle count |
| +0x8D9B8 | pointer to count × 76 runtime triangle records |
| +0x8D9BC | vertex count |
| +0x8D9C0 | pointer to count × 16 runtime vertex records |

Original constructor initializes these eight fields to zero; observed original object allocation size is 0x8D9C4. Allocate neither a substitute tiny Model object nor Clang STL containers into these fields.

## Disk parsing and exact runtime state

Confidence HIGH for copy behavior; signed material extension and untouched spans CONFIRMED by instruction inspection. All packed disk scalars little-endian. Read section counts in this order after 14-byte header: vertices, triangles, groups, materials. The original performs a magic comparison but discards its result; original version rejection is absent. Replacement gating uses fixture identity as scope selection, and must not claim new malformed-file behavior.

Vertex disk stride 15, runtime stride 16:

| Disk source | Runtime destination | Behavior |
|---|---|---|
| +13, one byte | +0 | copy bone identifier bit pattern unchanged; interpret as int8 when needed, no widening |
| +1, 12 bytes | +4 | copy three coordinate float32 bit patterns |
| flags +0, reference count +14 | none | ignored |
| none | +1..+3 | untouched allocator payload |

Triangle disk stride 70, runtime stride 76:

| Disk source | Runtime destination | Behavior |
|---|---|---|
| +8, 36 bytes | +0 | copy nine normal float32 bit patterns |
| +44, 12 bytes | +36 | copy s float32 bit patterns |
| +56/+60/+64 | +48/+52/+56 | separately compute float32 result of 1.0 minus each disk t |
| u16 +2/+4/+6 | int32 +60/+64/+68 | zero-extend each vertex index |
| flags +0, smoothing/group bytes +68/+69 | none | ignored |
| none | +72..+75 | untouched allocator payload |

Group disk records are variable: skip flags byte and 32-byte name; read u16 triangle count, then that many u16 indices, then one signed material-index byte. Runtime stride 12:

- +0: signed int8 material-index promoted to int32 (0xFF -> -1, 0x80 -> -128).
- +4: zero-extended u16 triangle count as int32.
- +8: original-heap pointer to count × 4 bytes; each index zero-extended u16 -> int32.

Material disk stride 361, runtime stride 80:

| Disk source | Runtime destination | Exact state |
|---|---|---|
| +32, 16 bytes | +0 | ambient float32[4], bit copy |
| +48, 16 bytes | +16 | diffuse float32[4], bit copy |
| +64, 16 bytes | +32 | specular float32[4], bit copy |
| +80, 16 bytes | +48 | emissive float32[4], bit copy |
| +96, 4 bytes | +64 | shininess float32, bit copy |
| +100, 4 bytes | +68 | transparency float32, bit copy |
| null-terminated string at +105 | pointer at +76 | allocate strlen+1 bytes, copy through terminator, no normalization/prefixing |
| none | +72..+75 | texture ID untouched; loader does not initialize it |
| name +0, mode +104, alpha-map +233 | none | ignored |

Vertex/group counts and indices are ordinary unsigned on disk even though count loops use signed int32. The stored bone byte is raw; material-index promotion is explicitly signed. Do not conflate these.

## Arithmetic policy

CONFIRMED: original loads float32 1.0, subtracts each t through x87, and stores each result to float32 before copying. No multiply/fusion or coordinate transform. Use exact float32 equality for supported fixture results; normal/position/s/material copies must be bit-exact including signed zero.

Supported numerical environment is round-to-nearest, ordinary x87 precision of at least 24 significand bits, and equivalent SSE rounding; root/hook validator must record actual x87 control word and MXCSR. All supported fixture t values permit an exact binary64 intermediate subtraction of 1.0 and the float32 operand, so one final nearest conversion to float32 is an independent implementation strategy. The original result is also float32; no accumulating extended-precision value is retained. Other rounding environments/NaNs/infinities/denormals remain fallback scope. Floating status flags are not part of the first payload contract; control modes must not be changed by replacement.

## Allocation callbacks and ownership

Confidence HIGH for entry ABI and pairing; successful calls are safe to express as explicit 32-bit cdecl callbacks from injected code after original CRT initialization.

`allocate(uint32_t byte_count) -> void*`: original module RVA 0x95420 (preferred VA 0x495420). `release(void*) -> void`: RVA 0x958B0 (VA 0x4958B0). Both cdecl, caller pops the single 4-byte argument. The loader and original destructor call this same pair for these POD buffers, including zero-length requests; no array cookie is added at the observed call sites. Callback identity matters more than the compiler's new/new[] spelling. Do not allocate owned arrays with injected DLL malloc/new and pass them to original deletion; its debug heap expects original metadata. Do not subtract/add a guessed debug header or free through another runtime.

Original allocator routes through its debug heap and fills successful payload with a configurable byte at module RVA 0x171AFE (VA 0x571AFE); original image initial byte is 0xCD. Preserve untouched allocation bytes; no zero-initialization of entire structures. Live fill value/heap flags can change; use callbacks and let original allocator fill. Debug allocator counters/hooks and new-handler effects are not emulated. Return-null/handler exceptions are out-of-scope fallback decisions before ownership transfer, not a license to return false after partial state writes.

Observed model allocation sequence, excluding stream-runtime internal allocations:

1. Whole-file temporary byte buffer, length from end-seek/tell.
2. Set vertex count; allocate 16 × count; assign vertex pointer; fill records.
3. Set triangle count; allocate 76 × count; assign triangle pointer; fill records.
4. Set group count; allocate 12 × count; assign group pointer; allocate/fill each group's int32 index list in file order.
5. Set material count; allocate 80 × count; assign material pointer; allocate/fill each texture filename in file order.
6. Release whole-file buffer through original callback; close/destruct file stream; return AL=1.

For first validation, deterministic payload/ownership matters; stream-runtime allocation identity and exact global heap-counter increments are explicitly not claimed equivalent. If validator requires those global side effects, retain original fallback until a file-reader callback contract supplies equivalent original I/O. Successful replacement arrays must survive the original destructor's group-list, filename, main-array deletion order.

Important lifecycle boundary: original destructor additionally deletes texture IDs through glDeleteTextures, and loader leaves IDs untouched until reloadTextures fills them. Validate destructor ownership only after original reload routine has assigned every material ID (or use a validation cleanup adapter that releases buffers without calling GL). Never treat raw just-loaded 0xCDCDCDCD IDs as valid GL names.

## Missing-file result

Original calls old ifstream with mode 0xA1 and protection argument 420 (0x1A4), then tests stream state mask 6. If opening fails, it destroys local stream and returns AL=0 before any eight object-field writes or model-array allocation. Object state remains unchanged, existing ownership remains unchanged, no texture request. Confidence HIGH, static evidence; a deterministic nonexistent ASCII filename must be run against original by validator. Permission/locking/device/path-encoding behavior beyond ordinary nonexistent file is UNKNOWN and falls back.

## Reload and validation boundary

Fixture harness lifecycle callbacks (HIGH): construct at RVA 0x15890 / VA 0x415890 is thiscall with ECX pointing to an original-heap block of at least 0x8D9C4 bytes, no explicit arguments; EAX returns the same pointer. It delegates base initialization, sets primary vptr to module RVA 0x16220C / VA 0x56220C, zeros the eight fields above, and does not initialize other object bytes. No observed allocation, filesystem, GL, or original global write occurs in this constructor. Preserve all nonvolatile registers as usual. Base destructor RVA 0x16420 / VA 0x416420 is thiscall with ECX object, no explicit arguments, no meaningful return; it releases member allocations but does not release the enclosing object. Release object block separately through original release callback. It changes vptr to base table and zeroes the eight fields; it also calls GL texture deletion as described above. A just-loaded pre-reload harness must use buffer-release cleanup rather than call that GL destructor with uninitialized texture IDs. Repeated destructor invocation is out of scope.

CRT-ready debugger checkpoint (HIGH): executable CRT start at RVA 0x9BE60 initializes heap, low-level I/O, argv/environment and C initializers before calling WinMain at callsite RVA 0x9BF73. WinMain thunk RVA 0x16C7 leads to body RVA 0x4CD90. Pausing at body entry 0x44CD90 therefore occurs after original CRT initialization, before executing this game's WinMain body. WinMain itself is stdcall with four 32-bit arguments; it is a checkpoint, not the model callback. This supports a root-managed test-copy process harness that parks the main thread and invokes constructor/loader on a worker; live safety, imported DLL initialization and synchronization remain validator responsibilities. No game GUI navigation is required to call this static model loader because it does not make GL calls. Waiting at the raw PE entrypoint would be too early for original allocator callbacks.

No texture creation, loading, GL call, joints/keyframes, transform, or animation is performed by this replacement unit. Original game uses separate reload routine RVA 0x16680 after map loading. Preserve that call in the test game.

Validator should invoke on two independently original-constructed empty objects in a fully initialized original test process, once original fallback and once replacement with the same immutable fixture and cwd. Compare return AL, eight counts/pointer validity, arrays with pointer relocation normalized, index buffers, owned filename bytes, every untouched span's expected allocator fill, and all unrelated object bytes unchanged. EXACT comparison of stored scalar bytes; pointer numeric addresses excluded, ownership/dereference shape compared. Compare missing-file return/object snapshot. Check stack/nonvolatile-register restoration and original callback releases. Build success alone is not PASS. Runtime trace, allocator failure testing and crash behavior remain PENDING.

Raw detour expected bytes and instruction spans are outside repo in `C:\Users\ADMIN\Boxer-analysis\fresh-coldet\IO-0003-hook-evidence.json`, Agent1/hook engineer/validator only. Implementation Agent receives addresses, hash and this ABI/state contract, never raw patch instructions or original code. Root handles suspended-process/harness feasibility; no early-process allocator call is approved before runtime initialization.

## Evidence

Fresh IDA body disassembly/decompile and independent fixture inspection. Signed material-index extension at 0x415DEA; x87 result stores 0x415BFE/0x415C10/0x415C22; epilogue cleanup 0x416051; material copies 0x415ED5/0x415EFD/0x415F25/0x415F4D, scalar writes 0x415F6D/0x415F89; allocator 0x495420 -> 0x496320 -> 0x496340 -> 0x4963B0; deletion 0x4958B0. Approved comparison references independently retrieved upstream described in IO-0001, not a specification substitute.
