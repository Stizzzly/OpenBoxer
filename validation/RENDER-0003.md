# RENDER-0003 РІР‚вЂќ Map lighting validation

Date: 2026-10-04. Agent 3 independent validation. Confidence: CONFIRMED for recorded checks. Replacement sources were read only; no original implementation, IDA, disassembly or legacy loader was consulted. Root owned all original-process activity.

## Results

- Independent x86 Clang build: PASS. Separate replacement/ms3d/build-validation, Debug C++17 i686 toolchain; CTest 6/6 PASS, including lighting_records_x87_and_live_callback_state. Nonfatal pre-existing MinGW libstdc++ duplicate RTTI COMDAT size warnings remain; foreign exception compatibility is outside this unit.
- Original-process differential: PASS, 28 synthetic scenarios plus actual map1/light.ms3d A/B = 30 comparisons, no failures.
- Natural map1 boundary/data integration: PASS; normal rendered map display: PASS by explicit user observation. Full-game equivalence: BLOCKED beyond the approved unit/scope.

## Differential evidence

Root-owned disposable PID8508, worker/wrapper exit0. lighting-fixture-prepatch-observation.json confirms original image SHA256 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, base400000, live original A target427AF0 and B target435D90 before mutation, timestamp2026-10-04T14:02:18.5491899Z. Candidate calls use installed thunks; original bodies remain intact. GL boundaries use typed recorders, without GPU calls on the worker.

Independently compared all30 complete normalized binary dump pairs, byte for byte. Callback traces are identical:100512 bytes,207 lines, SHA2566ef2c381d8977ff3843dfd2d66d262f128bf59eaaca0305f8718a73e22b27406. Summary11197 bytes SHA2560a90ecae2725ff1671880c2f0f982b9d6afc6f86f77dad0b3070aa456914949a. Per-file hashes and sizes are retained in RENDER-0003-independent-traces.json.

Synthetic cases cover signed empty/negative loops, positive empty memberships, reordered/duplicate memberships, copied normals/UV/sign bits, cancellation and rounding, empty/negative/one/three B records, IDs0/FFFFFFFF, repeated calls, live texture changes after color/blend, positions, material table/index/diffuse, triangle-count shrink, global count mutation, and combined live reads. The combined index+table mutation retains pre-callback GL color arguments while independently reading fresh material index/table for each post-callback RGB component.

A returns CCCCCCCC for nonpositive mesh count, otherwise visited mesh count3. B preserves final GL callback EAX marker6A5B4C3D, including empty loops. Both recorded ESP pairs balance; EBP/EBX/ESI/EDI sentinels13579BDF/2468ACE0/55AA55AA/AA55AA55 match. CW027F and MXCSR1F80 are preserved. Exact x87 sticky exception bits0..5 agree:00 for exact/empty cases,20 for precision cases,28 for unused material+48 doubled-and-float32-stored overflow. No numerical tolerance was applied.

Independent parsing of input/result dumps additionally confirms all5000 A four-byte tail witnesses and all3 B twelve-byte tails plus adjacent8 bytes remain equal. Harness checks full normalized model/source and global windows, source witnesses and intentionally callback-mutated state, A untouched four-byte tails, B untouched twelve-byte tails, unproduced records and adjacent eight bytes. Pointer normalization is limited to approved pointer identities. The actual asset pair uses original allocator/constructor/MS3D body API to load two separate objects; both stages compare complete state/trace and same-allocator cleanup excludes GL destructor.

## Independent asset reconstruction and static audit

Allowed asset base/maps/1/light.ms3d SHA25639ebd7ca5735b22963b01e4a403d0b02498c6f0228b8134475dbfbf9a2481a7c has9 vertices,3 triangles,3 groups,1 material. Independently parsed the approved IO schema and reconstructed membership order, positions, UV1-t, normals, material index, ordered53-bit intermediates and final float32 stores. All112 written bytes of each actual A record and44 written bytes of each actual B record exactly match expected-flat/global.bin and expected-native.json. Tail placeholders in those expected files are excluded from arithmetic comparison; tail preservation is verified by the differential witness checks.

lighting.cpp uses explicit x87 adapters for ordered add/add/divide by float32 3, and add/add/multiply by coefficient bits3EAAAA3B. The unused doubled scalar is stored to volatile float32 bits. Both sides maintain exact copies and live reads; B independently reloads material table and flat material index for R/G/B after color. lighting_color.S preserves final stdcall callback EAX through the cdecl bridge. Runtime guards reject unsupported controls/layout/ranges before candidate writes; recorded replacement counters distinguish actual candidate execution from fallback.

## Natural map1 boundary and native fields

Fresh root-owned PID3908 guard lighting-natural-prepatch-observation.json confirms the same image/base and both original thunk targets before mutation, timestamp2026-10-04T14:04:19.5743349Z. Final log segment records actual A then B replacement calls, each count1: A meshes3/triangles3 EAX3; B count3, bindDE1/texture37, blend302/1, culling enabled and final white color. Native files capture records before the preserved original consumer:

- lighting-native-A-0001.bin:384 bytes (36-byte header+348 payload), SHA256b9547acf0c432816a7cf4a5079e9b090405dfdbbac0ca437e157b011c8413527.
- lighting-native-B-0001.bin:204 bytes (36-byte header+168 payload), SHA25600fd0923c079b0fe8aa6e8496e669d98c6af04d1f0bda217b23bd1cd4cc3c94d.

All3x112 written A bytes and3x44 written B bytes independently match the asset-derived expected values exactly. Header records3, CW027F, SW0120 and MXCSR1FA0 are recorded for both stages. Native MXCSR includes an existing precision sticky flag; this is observed state, not evidence of a candidate SSE arithmetic operation or differential native FP parity. A tails remainCDCDCDCD and B tails are zero; absent native pre-snapshots, their preservation is established by mock differential witnesses rather than claimed from a single native image. B native EAX0B988201 is the real final GL residue, not the mock marker.

The current lighting-gameplay.png was independently opened: black640x480 frame with cursor; that capture does not substantiate rendered-level correctness. The user subsequently explicitly reported «Карта отображалась нормально» in response to the native visual question. Normal map display is therefore user-confirmed, distinct from screenshot evidence. No additional test or UI action was taken by this validator. GPU pixel parity and teardown remain unverified.

## Limits

PASS applies to the approved RENDER-0003 A/B boundary and recorded fixtures. No GPU pixel parity, shader parity, arbitrary unsupported model equivalence, other-map native coverage, teardown or long-running rendering equivalence is established. Existing game user input/processes were not touched by this validator. Native snapshots must be taken before the preserved original consumer can sort/mutate records.
