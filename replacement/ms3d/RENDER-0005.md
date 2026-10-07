# RENDER-0005 independent replacement

Approved contract: `specs/render/RENDER-0005-map-draw.md` and harness metadata. The implementer used only those contracts and replacement sources; no original files, code, assets or processes were inspected or executed.

`draw.cpp` is the semantic core: configure material, configure texture, emit one triangle, traverse memberships, restore entry texture capability. Source fields/layout are in `draw_model.hpp`; dispatch slots/handles/stack word types in `draw_dispatch.hpp`; x87 threshold comparison and ABI witness in `draw_abi.S`. Unknown record words and opaque shader handle meanings remain unnamed. Floating arguments remain raw32 bits.

Runtime support is conservative: map-global identity, current GL context, CW027F, bounded readable non-overlapping arrays, finite transparency, valid indices and executable callbacks before effects. Identical membership ranges may be shared; partial overlapping ranges are rejected. Limits (65536 meshes/materials,1000000 triangles/vertices/memberships and256MiB readable span) are adapter resource choices, not asserted original capacities. Original body fallback remains intact. The coordinator independently validates the original route before bootstrap; the adapter synthesizes the approved tail jump without reading code bytes.

GL wrappers forward all13 preserved callbacks. Their trace is active only inside tagged map draw; other units, lamps and actors forward without draw tracing. Opaque slots can be unset at bootstrap and are refreshed from current executable targets at each eligible map entry, supporting late shader initialization. Callback slot loads in the core remain live.

Launcher: `ms3d_launcher.exe --draw-fixture <absolute DLL>` invokes a fresh disposable guarded test copy, with root responsible for original execution. It exercises34 cases (16 immutable render cases,15 callback mutation cases,3 preflight rejection cases), preserving all original callback data slots and handles afterward. Full source witnesses include inactive alternate banks, sentinels, table/list roles, handles and normalized slot identities. ABI reports contain actual stack pointers, nonvolatile sentinels, CW/SW/MXCSR and final EAX. Candidate injected cases require installed-route count increment; guard-only cases never invoke an unsafe negative-material original.

Native environment:

- `OPENBOXER_DRAW_MODE=replace`: guarded independent map rendering.
- `OPENBOXER_DRAW_MODE=original`: tagged intact-original map rendering through the same forwarding recorders. This can be combined with the launcher's `replace` mode to retain six previous replacements.
- `OPENBOXER_DRAW_REPLAY=<absolute captured pre.bin>`: adds captured-model CPU replay to `--draw-fixture`; original and installed candidate use isolated callbacks, preserving captured runtime texture IDs/material data.
- `OPENBOXER_DRAW_REPLAY_QUERY=<raw integer, e.g.0x80 or0x100>`: entry query return for replay (default0x80); use captured query event result to reproduce saved AL.

Native full traces and snapshots are captured on draw calls1,2,3,120. `draw-native.jsonl` includes route/call/fallback count, exact ordered event payloads, raw32 argument/results, pointer roles, counts, ordered payload digest and FP state. `draw-routes.jsonl` records bounded unsupported/other-identity fallback routes separately. Snapshots: `draw-native-{original|replacement}-NNNN-{pre|post}.bin`. Capture and logging preserve FP state. There is no native allocation/coordinate transform/material synthesis in the renderer core.

Snapshot binary v1 (little endian): uint32 magic0x35575244,version1; signed32 counts[meshes,materials,triangles,vertices]; uint32 five handles; full580036-byte model with four table-pointer fields zeroed; for positive mesh count, meshes12 with membership-pointer fields zeroed, materials80,triangles76,vertices16; then for each mesh signed32 membership count and that many int32 indices when positive. Replay reconstructs independent pointers. Native roles use `materials+byteoffset`, `triangles+byteoffset`, `vertices+byteoffset`; fixture roles add bank number. Event pointer payloads are captured immediately before callback.

`draw-{offline|fixtures}.jsonl` contains each case, branch inputs, candidate/original event arrays, equality, immutable source state, ABI and route counters. Corresponding `draw-CASE-SIDE-pre/post.bin` are actual typed before/after snapshots; `.witness` captures all banks and normalized pointer/dispatch identities. `draw-replay.jsonl` and pre/post snapshots expose captured-model CPU replay. Build success/offline PASS do not establish original equivalence; original differential and natural proof belong to Agent3/root.

Offline verification: all8 CTest targets passed;34 draw cases and captured-format synthetic replay passed. `RENDER-0005-preserved-units.json` records unchanged prior algorithm/ABI/test file hashes; integrations are limited to CMake, launcher and bootstrap. Freeze manifest records final source/build hashes.
