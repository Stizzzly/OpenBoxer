# RENDER-0004 independent replacement

Implemented exclusively from approved `specs/render/RENDER-0004-light-selection.md`; no original executable, original code, IDA, game assets or original-process execution was accessed by Agent2.

`selection.cpp` owns the consumer's scoring, direction publication, exchange ordering, raw displaced parameter value 7, unconditional three-slot publication and seven live dispatches. `selection_fp.S` independently implements the documented ST0 conversion sequences without intermediate double narrowing. Original initializer/accessor/normalize/angular/magnitude/rotate/sin/wave and live scalar/vector dispatch dependencies remain dependencies. Structures are explicit four-dword records and callback calling conventions are separately declared.

Native deployment is bounded: count exactly three, finite positions/global vector/sin input, nonzero global vector, control word 0x027F, executable dispatch pointers and signed original wave indices 0..50. Unsupported input falls back to intact original body before callbacks or mutations. Native captures first three replacement calls and call 120; no per-frame file output after that. Snapshot/log output preserves full FX state. Existing five replacement implementation files remain unchanged.

All numerical copies, float32 stores, callback argument bits, callback order, final EAX and ABI preservation use EXACT comparisons. Arithmetic uses x87 active precision and separate specified float32 stores; there is no consolidated dot product, SSE arithmetic replacement or reassociation. Sticky x87 exception bits 0..5 and CW are compared, with MXCSR observed rather than assumed unchanged by preserved dependencies. Broader special-value/rounding domains are not claimed.

## Offline checks

The existing isolated Clang i686 CMake project adds `selection_loader_tests` and CTest `selection_live_records_dispatch_and_x87`. It exercises 30 synthetic scenarios and emits `validation/RENDER-0004-offline-checkpoints.jsonl`: one JSON object per case, with live count, three 3x14 raw-dword record checkpoints, final selected globals and an explicit dispatch-mutation flag. The checkpoint observer performs integer copies only and is inactive in normal deployment.

## Original-process harness (coordinator executes)

`ms3d_launcher --selection-fixture <absolute DLL path>` runs only the authorized guarded disposable test copy and terminates it after the worker. Existing startup hash/module/loader-slot checks and coordinator's private prepatch route observation remain prerequisites. The worker first executes six pairs with preserved original CPU math: modes 0/1/2/9, zero global vector, and preserved original wave. It then redirects math/wave helpers to deterministic ABI recorders for 30 pairs. The direct sin body is redirected only after all real-math cases; no original code is read/saved/restored, and this process must never resume gameplay.

Fixtures include live repeated-axis positions, live count/mode/distance/product mutations, independent mode checks, live sin input, post-sort wave mutations, dispatch-slot/handle/parameter/RGB changes, stale short-count publication, indirect equal-key exchange and displaced raw integer 7. Rotate deliberately returns a buffer distinct from its output argument. Compiler-produced by-value callback ABI is checked against original calls through these recorders, with an independent existing register/stack probe.

Output under the explicit lab directory: `selection-fixtures-summary.txt`, `selection-original-trace.jsonl`, `selection-candidate-trace.jsonl`, and numbered `selection-NN-input/original/candidate.bin` snapshots. Build success alone is not an equivalence result. Coordinator reported 36/36 differential PASS for the first frozen DLL SHA256 `0B5E61E36E4FBE8CD54307BF53EFFFA5D2AC66A618FF353E1A993271C4B846DF`; later logging/replay changes require their own validation.

## Native binary schema and replay

Each native snapshot starts with eight uint32 words: magic 0x344C4553, version 1, replacement-call counter, EAX, CW, SW, MXCSR, window count 25. It is followed by 25 entries, each `{uint32 rva,uint32 byteLength,byte payload[byteLength]}`. Windows include padded three-record state; count/reference/mode/sin; padded selected positions/RGB/directions/parameters; six wave scalars and the overlapping 204-byte wave-table window; seven handles; two dispatch slots. No original code is captured.

Native files are `selection-native-NNNN-pre.bin` and `selection-native-NNNN-post.bin`; seven actual dispatch calls and results are appended in `selection-replacement.log`. Full x87 condition-code equivalence is not claimed by sticky-bit comparisons.

Set `OPENBOXER_SELECTION_REPLAY_FILE` to one approved native pre snapshot, `OPENBOXER_SELECTION_REPLAY_EAX` to the hexadecimal native final EAX and `OPENBOXER_SELECTION_REPLAY_ONLY=1` to run a fresh replay-only fixture. It validates the exact snapshot schema, restores captured data while replacing only scalar/vector dispatch slots with CPU recorders, and retains original math and original wave. It initializes sticky bits 0..5 and MXCSR from the snapshot and compares all non-slot windows, trace, ABI, return and numerical state. Outputs `selection-replay-input/original/candidate.bin` plus scenario 0 in the usual traces and summary. GPU dependency side effects are outside recorder replay and still require coordinator/Agent3 comparison against native post state and actual rendering.

Natural rendering and independent Agent3 validation remain required before declaring the complete work item PASS.
