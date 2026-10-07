# RENDER-0001 — Independent material texture reload validation

Date: 2026-10-04. Agent 3 Validator. Contract: specs/render/RENDER-0001-reload-textures.md. No original implementation/disassembly or legacy prototype reference read. Root owns original test processes; existing user game untouched by validator. Changes limited to validation artifacts.

## Results

- Independent x86 Clang build and CTest: PASS, 4/4 (CONFIRMED).
- Original-process reload traversal differential: PASS, 18/18 cases (CONFIRMED).
- Exact EAX/IDs, callback order, live mutation state and ABI: PASS for tested cases (CONFIRMED).
- Natural render-thread lamp/map/light with original uploader/BMP/GL: PASS, actual per-material ID logs and rendered map1 observed (CONFIRMED).
- Uploader/decoder/GL implementation equivalence: outside this replacement; original dependencies remain preserved. No full-game claim.

## Independent build

PowerShell project root, PATH prefix C:\msys64\ucrt64\bin;C:\msys64\mingw32\bin:

```powershell
cmake -S replacement/ms3d -B replacement/ms3d/build-validation -G Ninja '-DCMAKE_TOOLCHAIN_FILE=C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d/toolchain-i686.cmake' '-DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe' -DCMAKE_BUILD_TYPE=Debug
cmake --build replacement/ms3d/build-validation
ctest --test-dir replacement/ms3d/build-validation --output-on-failure
```

Clang 20.1.8 i686 C++17/ASM configure/build succeeds. Four tests pass: x86 toolchain, MS3D payload/ownership, WORLD orchestration and texture traversal/live state, total 0.57s. Previously recorded nonfatal mingw32 RTTI COMDAT warnings remain; exception interoperability is outside scope.

## Actual original-process evidence

Fresh disposable process PID14152, root reports wrapper exit0/worker0. Semantic texture-fixture-prepatch-observation.json independently read by validator: live module base0x400000, original reload thunk target0x416680, approved full module SHA-25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6; route confirmed at2026-10-04T08:10:25.5122843Z. Root observer/private original bytes are not replacement input.

Original reload body executes unchanged; original strlen/uploader boundaries redirected to typed recorders only in a parked disposable process never resumed normally. Candidate executes through approved reload thunk, with actual replacement-call increment checked per pass. No real decoder, GL, filesystem texture loads or deletion occur in this isolated stage.

## Independent trace/state checks

Validator independently parsed all summary entries and compared complete paired normalized traces: EXACT12,585bytes /76lines, SHA-256`5415b733f26fc217494e95fb7cf83609ec12062fad683778da76e986352a7606`. Summary SHA-256`26e2becedb3c485f2898dccf1c5138512267684e76fd717fbaf8e95e7a063c63`. Machine-readable evidence: validation/RENDER-0001-independent-traces.json.

18 cases cover zero/-1/INT32_MIN count, empty filenames, IDs0/0xFFFFFFFF, failure followed by later success, all-empty, identical shared filename pointer with distinct uploads, second reload, live count shrink to1/0 and grow to3, table relocation during uploader/strlen including empty branch, filename changed after strlen, count changed during strlen. All callback strings, pointer identities, upload return bits, live count/table and material IDs are exact original/candidate matches. The normal second reload assigns IDs30/40, overwriting10/20; shared pointer calls upload twice producing11/22. No introduced deduplication/cache/deletion.

Original zero/negative count EAX0xCCCCCCCC is now runtime confirmed, not just static hypothesis. Stable positive count returnsN; count-shrink exit returns processed index1 even when final count0. Empty filename performs strlen then writesID0 without upload; uploader0 still stores0 and traversal continues. Postcallback table relocation updates tableB while tableA IDs retain seeded values. Filename mutation after nonzero strlen causes upload of the newly empty string, preserving original reread behavior rather than rechecking length.

Harness validates whole fake model bytes outside count/table fields stay0x5B and every material first72bytes stays initial0xA7/0x6C, with unchanged filename ownership except scripted recorder mutations. These exhaustive sentinel checks appear as untouched1/1; traces retain all IDs and normalized filename/table identities. Validator reviewed those loops, independently checked trace states and no masking of ID values occurs. Raw full buffers are not dumped independently; this limit is explicit.

## ABI

No-argument x86 thiscall probe places model inECX, supplies no explicit stack arguments, and verifies ESP restoration. EBP/EBX/ESI/EDI sentinels13579BDF/2468ACE0/55AA55AA/AA55AA55 survive original/candidate. x87CW027F and MXCSR1F80 are stable. uint32 EAX is compared exactly; no epsilon or boolean conversion. Two-pass case checks preservation each pass even though report prints last pass register values.

## Limits

Traversal replacement delegates actual upload to original game uploader, BMP/DIB decoder and GPU API on natural render thread. Isolated tests prove traversal/state parity; they do not exercise preserved uploader GL filter/mipmap/free behavior. No glDeleteTextures hook assertion is needed to establish that independent traversal source introduces no deletion, but actual GPU resource lifecycle remains natural/destructor evidence.

Invalid ownership/non-ASCII or unterminated filenames, exceptions, context misuse and concurrent driver/file behavior are unsupported/fallback scope. Natural repeats, old-ID accumulation and original destructor teardown need separate observation. No arbitrary remote-thread GL invocation or interaction with user's active game was performed.

## Natural render-thread map1 — PASS

Fresh normal test-copy PID23820, independently read semantic texture-natural-prepatch-observation.json confirms original reload route0x416680/modulebase0x400000/exact approved hash before mutation, timestamp2026-10-04T08:10:51.0193241Z. Real uploader, BMP decoder and GPU callbacks are preserved; no recorder substitution in this process.

Validator independently parsed final natural log segment: three replacement reload calls, lamp2 nonempty materials assignedIDs0x5A/0x5B; map27 assignedIDs0x5C..0x76; light1 empty filename overwrittenID0 with no uploader. Total29 delegated uploads, exact filenames match approved lamp/map manifest, every oldID is original untouched0xCDCDCDCD. Return counts2/27/1 match material counts. This records actual replacement and per-material original-dependency output assignments, not just a screenshot.

Validator visually inspected texture-menu-02.png (640x480): rendered combat level with textured floor/walls/posters, both characters and HUD. Root reports normal WORLD selector1 completed return100. Natural map1 smoke PASS; no byte-identical GPU framebuffer or independent uploader implementation equivalence is claimed. Repeated reload is proven under isolated recorders only; natural repeat/destructor and quantitative texture/GPU parity remain unverified.


## Teardown observation limit

Root reports own natural test process PID23820 was already absent before requested exit check; input tool rejected Esc because no target window/foreground existed, and no input was sent. No matching recent application crash event was observed, but absence of an event does not prove graceful shutdown or destructor execution. Teardown remains UNVERIFIED. Core isolated and natural map1 results above stand. Retained validated artifact snapshot: C:\Users\ADMIN\Boxer-lab\ms3d\validated-texture-v1. No further build/test or user-game action required for scoped reload unit.
