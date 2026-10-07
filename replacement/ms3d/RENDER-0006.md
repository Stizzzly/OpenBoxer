# RENDER-0006 implementation and validation interface

Contract: `specs/render/RENDER-0006-character-draw.md`, including its approved loader-backed untextured supplement. This implementation handles frame morph interpolation and triangle emission. Animation advancement, linked-part recursion, transforms, gameplay, material selection, accessors and native GL remain original.

## Source organization

- `character.hpp` / `character.cpp`: readable live source/output interfaces, reversed corner traversal, local pose snapshots and sequential interpolation. No binary offsets, object ownership or runtime patching in the semantic core.
- `character_layout.hpp`: approved raw fields and ABI callback adapters. Unknown fields retain no invented names.
- `character_abi.S`: independently designed stack/register/FP observation adapter for x86 thiscall with one argument and ret4.
- `character_native.hpp` / `character_native.cpp`: loader registration, approved manifest, allocation/pointer/count witnesses, untextured preflight and coherent typed snapshots.
- `character_runtime.cpp`: metadata-only routing, native original/replacement selection and forwarded GL observation.
- `character_trace.hpp`: ordered immediate-bit event storage and FP preservation.
- `character_fixtures.cpp`: 27 bounded synthetic cases and callback mutations; parked-process differential worker.
- `character_replay.hpp` / `character_replay.cpp`: sparse pose rehydration, recorder-only original/replacement CPU replay and source witnesses.

The seven earlier semantic cores are unchanged; verify `validation/RENDER-0006-prior-core-hashes.json`. Integration changes are limited to the launcher, DLL/bootstrap, build configuration and map runtime observer glue.

## Shared GL ownership

Map runtime remains the sole wrapper owner for shared Enable/Bind/Begin/TexCoord/End slots. It exposes one observer callback; the character observer returns immediately outside an explicit character capture tag. It does not change or refresh any dispatch target. Character wraps only its distinct scalar Normal3f and Vertex3f slots and always forwards their saved native targets. There is no character-to-map-to-character wrapper chain. Map semantic behavior and its Normal3fv/Vertex3fv paths are unchanged.

## Native admission and remaining scope

Root first independently verifies original target identity and actor/loader thunk metadata, then confirms all eight lab asset SHA-256/length pairs against the approved typed manifest. The implementation never reads original assets. Root supplies `OPENBOXER_CHARACTER_ASSETS_CONFIRMED=1` only for that disposable verified startup; without it no native registration is admitted.

Every loader entry invalidates its destination model registration. The observer copies a bounded terminated filename before forwarding body 0x7160, then preserves full returned EAX and FP state while copying the 108-byte header. Successful AL=1 must match IDP3, version15, frame749, tags0, surfaces1, manifest V/T, one returned mesh and all four readable allocation pointers. The filename selects a root-confirmed manifest entry; it is not used as hash evidence on its own.

Draw admission requires return RVA0x5FEF, CW027F, one registered model in a witnessed embedded owner model, unchanged collection/mesh identities, V/T and position/normal/UV/triangle pointers, frame indices within registered bounds, valid indices, finite complete A/B frame inputs, UV/factor and final emitted float32 values. Bounds cover full frame allocations; arithmetic scanning covers only current A/B frames. Preflight preserves the complete FP environment. Preserved native GL/accessors are assumed not to mutate these sources. Textured draws use the original body before any replacement callback because owner texture-table capacity is UNKNOWN. Other unsupported states also forward original before replacement effects.

Registry lifetime is explicitly one disposable startup with loader invalidation/revalidation; it is not a general allocation-tracking or destruction hook. No generation from a failed reload is reused. A different owner identity is rejected.

### Root-approved filename observation

Root's typed loader diagnostic in owned lab process PID6364 observed the first two filenames `.\base\fighters\\1_lower.bhm` and `.\base\fighters\\2_lower.bhm`. Confidence: CONFIRMED, root-provided typed observation on2026-10-05. The first diagnostic build rejected these lexical variants and forwarded the original.

Root explicitly approved removing leading dot-separator components and collapsing repeated separators, together with slash/case normalization. `character_path.hpp` implements only that lexical equivalence. It rejects traversal and internal dot components; matching requires the complete normalized approved relative path. It does not accept basename or arbitrary-prefix suffix matches, and rooted/UNC inputs cannot become an approved relative path. Root hash witnesses remain mandatory. Offline cases cover the observed spelling, separator/case equivalents and unsupported traversal/internal-dot/unrelated-prefix paths.

## Root execution

Build directory: `replacement/ms3d/build-validation`. Root executes all original-process/native work; Agent2 runs compiler/offline tests only.

Synthetic differential:

```powershell
& .\ms3d_launcher.exe --character-fixture <absolute-frozen-DLL-path>
```

Character original observation, with previous seven replacements active:

```powershell
$env:OPENBOXER_CHARACTER_ASSETS_CONFIRMED='1'
$env:OPENBOXER_CHARACTER_MODE='pass-through'
& .\ms3d_launcher.exe <absolute-frozen-DLL-path> replace
```

Character replacement changes only `OPENBOXER_CHARACTER_MODE` to `replace`. Launcher-wide `original` skips DLL installation and therefore does not collect character observations.

Capture calls are first three approved natural-call positions and bounded repeat positions120/240. Root should archive/clear prior logs/captures before a new comparison run. Files are in `C:/Users/ADMIN/Boxer-lab/ms3d/`:

- `character-loads.jsonl`: loader/header/hash/count registration provenance.
- `character-native.jsonl`: natural caller and owner/model/mesh identities, frames/factor, route, replacement counter, returned EAX, FP environment and complete forwarded GL events.
- `character-native-{original|replacement}-{0001|0002|0003|0120|0240}-{pre|post}.bin`: coherent typed source captures.

Recorder-only differential CPU replay after native captures:

```powershell
& .\ms3d_launcher.exe --character-replay <absolute-frozen-DLL-path>
```

The worker enumerates the ten standard `*-pre.bin` names. Zero captures returns102 (BLOCKED). Output is `character-replays.jsonl` plus `<capture>-replay-{pre|original-post|candidate-post}.bin`. Each row includes exact full source byte-vector equality (including runtime pointer words), routed replacement delta, full original/candidate events, EAX and ABI/FP observations. Supplemental full-array hashes were removed because exact byte comparison and typed witnesses already provide stronger state evidence. Replay uses a bounded300second remote worker timeout; other modes retain60seconds. A timeout terminates the launcher's own test copy and its partial report is excluded from PASS.

Offline candidate replay of an approved typed capture:

```powershell
& .\character_loader_tests.exe <absolute-capture-path> <absolute-report-path>
```

## Typed snapshot encoding, version2

All words are little-endian. This is source/pose data, never code or raw implementation dumps.

1. Ten uint32 words: magic0x36485243, version2, complete frame bound, vertices per frame, triangle count, original frameA, original frameB, factor bits, numeric owner identity, numeric model identity.
2. Model accessed span80bytes. Collection pointer words at12/16/20/28/32/36 are zeroed.
3. Mesh record288bytes. Position/normal/UV/triangle pointer words at272/276/280/284 are zeroed.
4. Complete A position frame, B position frame, A normal frame, B normal frame, each `12*V` bytes of float32 XYZ bits.
5. `8*V` UV bytes.
6. `24*T` complete triangle-record bytes, including unused remaining12bytes.

Replay constructs bounded full frame-major arrays, places captured A/B payloads at their actual indices, and zeroes unobserved frames. It does not substitute frame0/1. For A==B both captured payloads must represent the same original frame. Native scope is untextured, so no skin or owner texture entry is read or claimed. Numeric identities are metadata and not replay pointers. Synthetic fixtures separately contain explicitly sized owner textures/skins and test textured behavior.

## Floating-point and validation status

Each component computes sequential x87 `(B-A)*factor+A` under active 53-bit precision with a final float32 store; component order is Z/Y/X for normals, then Z/Y/X for positions. No clamp/normalization/fusion or intermediate float32 staging. Fixtures include two rounding-sensitive literals and negative-factor signed-zero behavior.

The differential harness restores the same captured full initial FXSAVE state immediately before each side. It compares original/candidate before and after CW/SW/MXCSR exactly, along with nonvolatile registers and stack restoration. Caller FP state is restored when the worker exits.

Root and Agent3 completed final original-process differential validation:27synthetic cases and10captured moving-pose CPU replays PASS, including equal initial/final FP observations, ordered callbacks, EAX, source-state witnesses and installed route checks. Five coherent original native captures and five replacement captures exercised moving poses; root observed normal battle rendering. Scope is the approved loader-backed untextured unit. Owner texture capacity, nonmanifest assets and general lifetime tracking remain outside this admission scope.

Final root evidence: `validation/RENDER-0006-root-final-provenance.json`, fixture PID17132 exit0, replay PID11108 exit0. The earlier timed-out replay PID26312 is excluded. Root independently reconfirmed the installed executable SHA256 unchanged.

## Frozen delivery and formatted sources

`frozen-render0006/ms3d_replacement.dll` preserves the validated808DC5A3F8C01DEA7CA7B26B26C27F04BA26A199FDBA063E32EDF9786C861E24 binary. `frozen-render0006/ms3d_launcher.exe` preserves launcher584C352BAB0AB4FE0E33EFA780DD3A31AAD46248AE65D59D640CC0662C691223. Root supplies the lab launch guard that verifies target/assets before enabling native registration; default asset assertion remains false.

Only new character source files were formatted for readability. All six character DLL objects have identical compiled `.text*` and `.rdata*` contents before/after formatting, recorded in `validation/RENDER-0006-format-{Before|After|identity}.json`. Clang-format split long adjacent narrow string literals, so raw token hashes differ in five files; code and constant bytes remain exact. This does not claim full PE/object identity: rebuilt debug metadata differs. Formatted rebuild DLL SHA256157F0DA64E102ED313233845FBD7D32A58BD92092880B71FFCDA9AFDB2234659 is retained in the build directory; validated binaries remain the frozen delivery.

All nine offline targets pass after formatting, including independent interpolation literals and semantic mutation/cached-offset/corner-order checks. The replay-only performance change preserves byte-identical native/core/fixture/ABI compiled objects relative to the native-tested A0A3 build. Freeze metadata links the validated binaries, source hashes, preservation manifest, offline log and formatting evidence. Original assets and native source snapshots are not part of frozen delivery.
