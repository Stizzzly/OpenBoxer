# RENDER-0002 — Game texture upload

Status: IMPLEMENTED; ISOLATED_DIFFERENTIAL_PASS (22 cases); NATURAL_MAP_1_SMOKE_PASS. Confidence: CONFIRMED for tested ABI, ordered calls, field reads and return. Independent implementation: replacement/ms3d/upload.cpp and upload_runtime.cpp. Validation: validation/RENDER-0002.md. Original-process tests and live route checks passed; natural map1 recorded 29 replacement uploads, 29 successful GLU calls and 58 releases, with rendered textured combat. Real-context repeated upload and teardown remain unverified.

## Identity and scope

Original program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`; x86 preferred base 0x400000, image size 0x19F000. Fresh IDA session 73763694 independently reverified identity on 2026-10-04. Body RVA 0x25420, route thunk RVA 0x1799. Preserve original BMP helper thunk RVA 0x1AAF/body 0x25390 and its upstream DIB decoder. Preserve original OpenGL/GLU and original heap release in natural execution. Legacy replacement sources were not used as evidence.

Approved routing is a synthesized five-byte relative tail jump at the thunk. Original body remains intact for independent comparison and fallback. Uninstall synthesizes a jump to approved body. Coordinator privately verifies original route, whole-file identity including overlay and live module identity before mutation; implementation receives addresses/interfaces only. Conflicting route fails guard. No original instruction bytes or relocation are needed by implementer.

## ABI and image layout

cdecl, one pointer argument filename at entry stack+4, caller cleans four bytes; all 32 EAX bits are the returned texture ID. No this pointer. Preserve EBX, ESI, EDI, EBP and stack; EAX, ECX, EDX volatile. Dependencies may affect floating-point environment; this unit performs no floating arithmetic.

Helper returns a pointer to a 12-byte image record: signed int32 width at +0, signed int32 height at +4, pointer32 pixels at +8. These are scalar fields with four-byte offsets. Filename pointer is passed unchanged to the helper, once. There is no independent string length, suffix handling, path normalization or model access here.

## Observable behavior

1. Local uint32 texture ID starts zero before helper invocation. Invoke preserved helper once with exact filename.
2. If helper returns null, return zero with no GL or release callbacks. Otherwise read record+8 once for the null-pixel test. If null, return zero without freeing even the nonnull record.
3. Call glGenTextures(1, address of local ID). Its output slot is initially zero. Continue regardless of the resulting value, including zero or unchanged output.
4. Call glBindTexture(0x0DE1, current local ID).
5. Call glTexParameteri(0x0DE1, 0x2801, 0x2703), then glTexParameteri(0x0DE1, 0x2800, 0x2601). These configure minimum LINEAR_MIPMAP_LINEAR then maximum LINEAR filters.
6. After both parameter callbacks, obtain mipmap arguments from the live image record. Read pixels first, height second, width third; do not cache these from the initial null check or before GL callbacks. Call gluBuild2DMipmaps(0x0DE1, 3, width, height, 0x1907, 0x1401, pixels). The three means three color components; format RGB, unsigned-byte type. Ignore its int32 status.
7. After the mipmap callback, read live record+8 again and pass that pointer to original release. Thus a mipmap callback changing the pixel pointer changes the released pointer. Then release the original image record pointer. No additional null check occurs after the initial pixel check.
8. Return the current local ID after both releases. Do not infer success from GLU status or validate the generated ID. The glGen output pointer designates this local through the remainder of the call; typed mutation tests may retain it and change it later, and the final return is a live local read.

No old texture deletion, GL error query, binding restoration, rollback, deduplication, dimension validation or pixel conversion occurs. No direct game globals are written. Driver/decoder/CRT side effects remain dependency behavior. Width/height values, including unusual signed values, are forwarded when a nonnull pixel record reaches this boundary; decoder support for such records is not inferred.

## Preserved dependency interfaces

| Dependency | Approved RVA | ABI |
|---|---|---|
| BMP helper route | 0x1AAF | cdecl(filename)->image pointer; caller cleans4 |
| glGenTextures IAT slot | 0x18CA5C | stdcall(int32 n,uint32* IDs), pops8 |
| glBindTexture IAT slot | 0x18CA58 | stdcall(uint32 target,uint32 ID), pops8 |
| glTexParameteri IAT slot | 0x18CA54 | stdcall(uint32 target,uint32 pname,int32 value), pops12 |
| gluBuild2DMipmaps IAT slot | 0x18C880 | stdcall(target,components,width,height,format,type,pixels), pops28; int32 status |
| image release body | 0x96D10 | cdecl(pointer), caller cleans4; return ignored |

Original GLU import route is RVA0x92E92. Release is original free-like CRT boundary, which delegates debug release with block type1; do not substitute injected CRT free, original C++ delete facade, or another heap. Release order is current pixels then record. Preserve the original allocation ownership returned by BMP helper. Helper null filename returns null; otherwise it probes fopen(filename,"r"), closes a successful probe, and delegates original DIB decoder. Decoder-specific errors/dialogs and file races stay original.

## Isolated differential plan

Use a fresh disposable initialized original process with main thread parked and affected routines quiescent. Never use the active user's game. Invoke intact original body and candidate on equivalent controlled inputs. Redirect BMP helper thunk to a cdecl recorder, GL/GLU IAT data slots to typed stdcall recorders, and original release body to a cdecl recorder. Direct-entry interception is permitted only in this disposable, never-resumed process; no copied original bytes/trampoline and no shared GL DLL patches. Terminate afterwards.

Record callback sequence and all arguments, normalized record/pixel/local-output identities, output-slot initial value, final EAX, complete fixture state, stack balance and nonvolatile registers. Release stubs record without actually freeing synthetic storage. GLU does not access pixels in this harness.

Required cases: null image; nonnull image with null pixels; valid image with generated ID; glGen leaves output unchanged; generated zero; generated 0xFFFFFFFF; positive/nonzero and negative GLU status; repeated call; signed unusual dimensions with nonnull pixels. Mutation cases: change record dimensions/pixels from bind or parameter callback and verify live mipmap inputs; change pixel pointer inside GLU and verify release uses changed pointer; retain output pointer and change ID during a later callback and verify final returned value. A GLU callback changing pixels to null still leads to release(null), then release(record). No release occurs for initial null-pixel failure.

Comparison policy EXACT for IDs, scalar arguments, call order, return and state, normalized pointers for identities. Success of these cases establishes uploader orchestration, not decoder/driver equivalence.

## Natural validation

After independent differential PASS, install only uploader route in the authorized test copy alongside approved replacements. Keep BMP helper, decoder, original GL/GLU and release intact. Execute on the game's rendering thread with its current GL context; parked-process worker callbacks are solely isolated recorder tests. RENDER-0001 reload delegates one uploader call per nonempty material and stores the returned ID; known natural map/light/lamp routes provide the integration boundary.

Log filename, width/height captured at mipmap invocation, generated ID, mipmap result, and completion of pixels/record releases. Instrumentation must observe the existing dependency calls without adding decoder invocations, changing return values or reading freed records. Include failed helper calls distinctly. Observe natural map1 textured rendering and report replacement/dependency call counts; compare equivalent baseline checkpoints where available. Screenshot alone does not prove ABI or callback parity. Do not touch an existing user-playing process.

## Unknowns and limits

Exception propagation, malformed decoder-owned storage, asynchronous mutation/data races, callbacks writing invalid retained stack pointers, cross-thread GL context use and decoder-specific malformed input are UNKNOWN. Original fallback must be selected before mutation where scope guards reject input/environment. Normal helper-null failure and nonnull/null-pixel failure are supported, including the original no-free behavior. Original behavior does not provide an opportunity to inspect decoded image validity before invoking its helper; do not add a guessed ownership validator.

## Evidence

Fresh identity survey and instruction review of body VA0x425420 through 0x425526 confirms cdecl, initialized ID, branch gates, ordered GL arguments, live record reads at mipmap and release boundaries, and final local-ID return. Approved helper/free/import identities independently established in RENDER-0001; this review rechecked their uploader call sites. Raw evidence remains outside repository under C:\Users\ADMIN\Boxer-analysis\fresh-coldet. No original implementation is included in this handoff.
