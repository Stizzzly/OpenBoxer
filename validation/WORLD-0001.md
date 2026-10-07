# WORLD-0001 — Independent map orchestration validation

Date: 2026-10-04. Agent3 Validator. Approved contracts: specs/world/WORLD-0001-map-load.md and WORLD-0001-harness-metadata.md. Validator examined behavioral contracts, new replacement and retained observations only; no original code or legacy prototype reference. Root controlled original process launches/injection. Validator changed only validation artifacts.

## Results

- Independent x86 Clang build: PASS (CONFIRMED).
- Offline CTest: PASS 3/3, including 2648-case world orchestration matrix (CONFIRMED).
- Original-process callback-isolated differential: PASS 2648/2648, failures 0 (CONFIRMED from independently parsed full retained traces/summary).
- cdecl return/stack, nonvolatile registers and FP controls: PASS for tested matrix (CONFIRMED).
- Natural map1 with preserved real callbacks: PASS (CONFIRMED route/log and user screenshot; user reports functioning gameplay). Another installed map and orderly unload/destructor: UNVERIFIED.
- Full transitive/game behavioral equivalence: BLOCKED/outside current isolated result.

## Independent build

PowerShell project root, PATH prefix C:\msys64\ucrt64\bin;C:\msys64\mingw32\bin:

```powershell
cmake -S replacement/ms3d -B replacement/ms3d/build-validation -G Ninja '-DCMAKE_TOOLCHAIN_FILE=C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d/toolchain-i686.cmake' '-DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe' -DCMAKE_BUILD_TYPE=Debug
cmake --build replacement/ms3d/build-validation
ctest --test-dir replacement/ms3d/build-validation --output-on-failure
```

Clang 20.1.8 i686-w64-windows-gnu C++17/ASM, successful configure/build. CTest x86_toolchain, independent_payload_and_ownership and world_orchestration_branch_matrix all pass, total 0.55s. Existing nonfatal duplicate RTTI COMDAT size warnings from mingw32 libstdc++ remain; foreign exception interoperability is not approved/tested.

## Original-process provenance

Root final guarded run: owned disposable PID16316, worker result0 and wrapper exit0. Semantic prepatch observation at C:\Users\ADMIN\Boxer-lab\ms3d\world-fixture-prepatch-observation.json records live module base 0x400000, decoded original thunk target0x42E970, exact module SHA-256 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, route confirmed at 2026-10-04T07:53:20.6084927Z. Validator consumed semantic observation, not original instruction bytes/private observer implementation.

Original map body executes unchanged; dependency thunks/direct allocator+rand entries and cursor IAT are redirected to typed recorders only in this never-resumed disposable process. Candidate is reached through redirected approved map thunk. Real GL/audio/cursor effects are suppressed for isolation. This is differential evidence against original orchestration, not parity between two independent implementations or actual transitive dependencies.

## Complete trace verification

Validator independently compared world-original-trace.txt and world-candidate-trace.txt: both EXACT 23,879,472bytes,58,142lines; SHA-256`6e4394086dec2bad3ed1d79eaa846dc93684ed14fe8e94222d142dedb2711ece`. Latest summary SHA-256`01548f3e62c470e176e190b14265a0c090f1f7aca3c6c7b5bc62ae5a1f8c6212`. Machine-readable results: validation/WORLD-0001-independent-traces.json.

Independently enumerated2640 full matrix combinations: selectors1..8/0/9/-1, sound0/1, mode0/1, callback-written decision0/1/2/-1, signed input-1/0/1, rand0/1/2/3/7. Eight focused cases mutate live sound/mode/current optional selector, decision during stageD/optionalStage/2000A, and music selection during audioLoad.

Every scenario has trace/global/actual-route/stack/oracle PASS and original/candidate EAX=-19088743, proving final sentinel forwarding instead of synthesized100/boolean. Full trace includes normalized callback identities/args, every supplied relevant global at each callback boundary and final state. Validator additionally parsed all final states against direct bit/width expectations, model identities, map/light filename strings, two 0x8D9C4 allocation requests, raw holder/scratch 0xCC, audio bool slots 1/1 and SetCursorPos recorder(320,240). LoadModel recorder returnsfalse for all selected filenames, yet reload and remaining stages proceed as specified.

Focused music mutation0->2 produces1.ogg then3.ogg holder/audio/destructor sequences. Focused2000A decision1->2 produces2000B afterA, confirming independent re-evaluation. Signed conditional outputs match 0/-16/-8bitpatterns. Five genuinely neighboring byte witnesses remain0x6D; address 58479B intentionally becomes0 because it is within the specified32-bit 584798=1store, not untouched padding.

## ABI and state

Every original/candidate call restores ESP. Independently authored probe installs and records EBP13579BDF/EBX2468ACE0/ESI55AA55AA/EDIAA55AA55; all four survive both implementations in all 2648 scenarios. Probe passes three32-bitcdeclarguments and performs caller cleanup, while typed delegates supply their approved thiscall/stdcall cleanup. Original/candidate x87 control remains 027F and MXCSR control remains 1F80; reported before/after values match. No numeric epsilon used: state bits, callbacks and normalized trace are EXACT.

Recorder state snapshots cover approved relevant fields plus byte-width witnesses. This validates direct orchestration state, not every byte in entire game memory. Model pointers normalize to model1/model2; holder/temp addresses normalize by identity and constructor/audio/destructor sequence. Test calloc/free ownership belongs to recorder runtime; original-heap ownership and real model payload equivalence remain separate IO-0003 evidence.

## Limits

584798>1 conditional branches are unreachable in ordinary sequential execution because orchestration stores 1 first; initial values do not exercise them and no asynchronous mutation is claimed. Malformed inputs, allocator/foreign exceptions, null-allocation crash behavior, destructive reentry/concurrency and abnormal FP modes are outside first implementation scope. Selector8/out-of-range are tested under recorders, with natural deployment fallback preserved. Isolated rand recorder replaces the stageC transitive draw; real game combined RNG behavior is not established by mocks.

Further scope if requested: another installed map and orderly initialized-GL unload/destructor. These are not established by the map1 observation below.



## Natural map1 — PASS

Fresh normal test-copy PID23124, separate from disposable recorder process. Semantic guard world-natural-prepatch-observation.json confirms module base0x400000, original thunk target0x42E970 and approved executable hash before installation (timestamp2026-10-04T07:53:40.5299628Z).

Validator inspected latest final log segment: approved WORLD route installed; natural entry selector1 with ignored slots1/1 in replace mode; actual replacement returned100. Associated IO-0003 log names `.\base\maps\1\map.ms3d` and `.\base\maps\1\light.ms3d`, each followed by independent replacement success. Real model/reload/physics/audio callbacks remained delegated to original dependencies; no disposable recorder substitutions are present in this fresh natural run.

User reports "все ок". Validator inspected supplied640x480 screenshot C:\Users\ADMIN\Boxer-lab\ms3d\world-gameplay-map1-user.png: rendered3D combat room, standing player, knocked-down opponent, health/stamina HUD and combo/round display. This is concrete natural map1 gameplay evidence combined with actual replacement route, not merely a logo/menu screenshot.

Another map and orderly unload/original destructor remain unverified. Audio waveform, complete long-running physics or full original-game equivalence were not differentially measured. No further UI input, termination or repeated tests were performed while user actively plays.
