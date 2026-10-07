# RENDER-0002 — Independent texture uploader validation

Date: 2026-10-04. Agent 3 Validator. Contract: specs/render/RENDER-0002-texture-upload.md. Original code/disassembly and legacy prototype excluded. Root controls original processes; validator changes only validation artifacts. Existing user game untouched.

## Results

- Independent x86 Clang build/CTest: PASS, 5/5 (CONFIRMED).
- Actual original-process uploader differential: PASS, 22/22 (CONFIRMED).
- Exact callback/order/state/EAX and cdecl ABI: PASS for tested cases (CONFIRMED).
- Natural render-thread map1: PASS, actual uploader/dependency logs and rendered combat observed (CONFIRMED).
- Decoder/driver equivalence and complete game equivalence: outside current replacement scope.

## Build

PowerShell project root with C:\msys64\ucrt64\bin;C:\msys64\mingw32\bin prepended to PATH:

```powershell
cmake -S replacement/ms3d -B replacement/ms3d/build-validation -G Ninja '-DCMAKE_TOOLCHAIN_FILE=C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d/toolchain-i686.cmake' '-DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe' -DCMAKE_BUILD_TYPE=Debug
cmake --build replacement/ms3d/build-validation
ctest --test-dir replacement/ms3d/build-validation --output-on-failure
```

Clang 20.1.8 i686 C++17/ASM configure/build succeeds. All five tests pass in 0.65s, including upload_order_ownership_and_live_callback_state. Existing nonfatal mingw32 RTTI COMDAT warnings remain; foreign exceptions are outside scope.

## Runtime identity and independent comparison

Root owned disposable PID23596; worker0/wrapper0. Validator independently read upload-fixture-prepatch-observation.json: module base0x400000, original thunk target0x425420, approved SHA-25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, live original route confirmed at2026-10-04T08:30:31.2994371Z. Only semantic route evidence consumed; original instruction bytes/private observer script excluded.

Intact original uploader body compared against candidate reached through approved uploader thunk. Typed helper/GL/GLU/free recorders exist only in parked disposable process, never resumed normally. No real decoder dialogs or GPU activity on fixture worker.

Validator independently parsed all 22 summary cases and complete normalized traces. Original/candidate are EXACT22,791bytes,185lines, SHA-256`a6eb0390f9cf8c9518beed2afd38edf462899fc74431492a92a37afa31116f34`. Summary SHA-256`5b597ff6c31851578f123702001deaf0a1a1be1e64513a4529527d1515001df4`. Evidence: validation/RENDER-0002-independent-traces.json.

## Verified behavior

- Helper receives identical filename pointer/content once per invocation, including null filename recorder case.
- Null image and initial null pixels return0 with helper-only trace: no GL or release, including no record free for nonnull/null-pixel image.
- Valid trace order: BMP, glGenTextures, bind, MIN parameter, MAG parameter, GLU mipmap, release live pixels, release original record. Repeated case executes sequence twice.
- glGen sees n1 and local output initially0. Untouched output/explicit0 still continue every subsequent call;0xFFFFFFFF returns unchanged.
- Exact numeric GL target3553, parameter10241/9987 then10240/9729, mipmap components3/format6407/type5121. Signed width-4096/height0 are forwarded. Positive/negative GLU statuses are ignored without rollback.
- Bind/min/max mutations affect live record dimensions/pixels at mipmap; candidate explicitly reads pixels then height then width after parameter calls. GLU pixel-pointer changes determine subsequent release, including release(null), then record.
- Retained local-ID changes in bind/parameter/GLU/pixel release/record release survive to final EAX. Record-release mutation returns0xABCDEF01, proving final ID read after both releases.
- All normalized record/pixel/local-output identities, callback args and boundary state snapshots match. No deletion, unbind, error query, extra helper or free callback appears.

Fixture guard bytes and all synthetic pixel bytes are exhaustively checked by reviewed recorder loops and report intact1/1. These checks are independent from normalized pointer comparison, but raw full buffers are not dumped separately. Uploader itself owns no new allocation; recorders retain synthetic storage and record release order without actual free.

## ABI

One-argument cdecl probe passes filename and cleans4 bytes in caller; all EAX bits compared EXACT. ESP before/after restores; EBP13579BDF/EBX2468ACE0/ESI55AA55AA/EDIAA55AA55 survive both original/candidate. x87CW027F/MXCSR1F80 remain stable. No floating arithmetic/tolerance is introduced.

## Limits

Natural helper/BMP decoder, OpenGL/GLU and original free-like callback remain preserved dependencies. Isolated orchestration PASS does not establish decoder format/orientation or GPU framebuffer parity. Unknown malformed ownership, foreign exceptions, asynchronous races, retained stack-pointer misuse and wrong-context GL calls have no new compatibility guarantee. Natural repeat, old-ID lifetime and teardown require observations beyond fixture repeat. No extra active-game/UI actions by validator.

## Natural map1 — PASS

Fresh normal test-copy PID21228. Validator independently read upload-natural-prepatch-observation.json: live route0x425420, modulebase0x400000 and approved executable hash before patch, timestamp2026-10-04T08:30:50.9124373Z. Original BMP helper/decoder, OpenGL/GLU and free-like release remain intact; natural game render thread/context owns these callbacks.

Validator independently parsed all29 final natural upload entries. Each reports one original helper result with nonnull image/pixels, one mipmap status0, and completed pixel/record releases. Counts29 uploader /29 helper /29 GLU /58 original releases. Generated and final IDs are equal and sequential0x5A..0x76; exact filenames match approved lamp then map1 material manifest, and reload logs retain assigned IDs. Actual dimensions include244×114, forwarded through original GLU. WORLD map1 returned100 as root observes.

Validator inspected upload-gameplay.png (640x480): textured combat room, floor/walls/posters, player/opponent and HUD. Natural uploader integration PASS; screenshot and logs do not establish byte-identical GPU output or upstream decoder equivalence. Natural repeat and teardown remain UNVERIFIED. Root reports user actively playing; no UI input/focus/termination was issued in this run, and validator performed no such action. Scoped validation complete; retain user game untouched.

