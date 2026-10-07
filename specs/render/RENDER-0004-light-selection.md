# RENDER-0004 — Light-record scoring, ordering and parameter dispatch

Status: READY_FOR_IMPLEMENTATION for the scoped complete consumer below. Confidence: CONFIRMED static ABI/ordering/state/copy widths; numerical and natural original-process validation PENDING. Agent1, 2026-10-04.

## Original identity, role and bounded replacement

Target is Месть боксера. Московский криминалитет, NOT ETS2. Original program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`, preferred base0x400000, x86, image size0x19F000. Fresh IDA session145b8f86 independently reverified this identity. Replacement route thunk RVA0x16E0 -> body RVA0x362A0 (VA0x4362A0), sole found caller frame VA0x448734. No arguments, cdecl/plain return; ECX is not an input object. Preserve EBX/ESI/EDI/EBP and stack. All32 EAX result bits forward the last opaque dispatch callback result. Caller ignores EAX.

CONFIRMED: consumer scores global records, modifies direction triples for modes1/2, orders records by squared-distance field, writes first three record parameters into other globals, and dispatches seven opaque parameter callbacks. It does not directly draw or call GL IAT. HIGH lighting role: frame invokes it after an opaque program-binding callback and before map rendering; RENDER-0003 populates input records. Do not describe this as selection per individual map object: that semantic scope is not established. Selection is first three global records after the particular ordering below. Preserve original math/vector, wave-update and opaque dispatch dependencies. All five previous replacements stay intact.

Approved hook is a synthesized five-byte tail jump at thunk. Original body remains intact, callable directly for fallback and differential; uninstall synthesizes jump to approved body. Exact full-file identity including overlay, live module identity and privately checked baseline route required before mutation. Existing conflicting route fails guard. Agent2 receives interface metadata only, never original bytes/pseudocode. Raw analysis is outside repository under C:\Users\ADMIN\Boxer-analysis\fresh-coldet.

## Static input/output layout

Addresses below are RVA relative to original module.

| Location | Type/read/write |
|---|---|
|0x177F88|signed int32 live record count, READ only by consumer|
|0x178110 +56*i|14dword static record, READ/WRITE as below|
|0x184C98|three float32 components of a global vector, READ through component helper|
|0x184770|int32 mode, READ independently for mode1 and mode2 checks|
|0x184C90|float32 sin input, read independently at each rotation|
|0x1855CC|live function pointer, stdcall parameter-vector callback|
|0x1855DC|live function pointer, stdcall scalar callback|

Record offsets:0/4/8 position float32;12/16/20 color raw32;24/28/32 direction triple raw32;36 int32-like parameter bits;40 opaque raw32;44 squared-distance float32;48 score float32;52 product float32. Record array has static process lifetime and no allocation/free here. Declared total capacity UNKNOWN; natural approved fixtures yield3records. First three record slots are accessed UNCONDITIONALLY at final publication, even count<=0. Thus short counts publish stale remaining static slots rather than truncate or zero them.

## Preserved dependencies and exact ABI

| Boundary | Thunk/body RVA | ABI and minimum contract |
|---|---|---|
| three-component initializer |0x1839 /0x1EAB0|thiscall ECX scratch; three32-bit arguments XYZ, pops12, EAX=scratch; writes12bytes, leaves fourth dword unchanged|
| component accessor |0x19C9 /0x1B8F0|thiscall ECX vector; signed int32 index, pops4; EAX pointer to selected float32; index0..2 only used here|
| normalize vector |0x1811 /0x20C90|thiscall ECX vector, no stack args; in-place XYZ; EAX ignored; preserve original dependency behavior|
| angular calculation |0x17F8 /0x25010|cdecl two by-value16-byte records, total32 argument bytes caller cleanup; result double via x87 ST0; EAX ignored|
| float magnitude callback |0x11E0 /0x1B730|cdecl(float32), caller cleans4, x87 ST0 float-compatible result; preserve original callback|
| rotate vector |0x101E /0x1BB20|cdecl(output16*, by-value16-byte vector, float32 angle, float32 axisX,Y,Z); total36 bytes caller cleanup; EAX output pointer; caller copies ALL16bytes from returned pointer|
| original sin |body0x95C34|cdecl(double), caller cleans8; double result ST0|
| wave update |0x1393 /0x36180|cdecl noargs; EAX ignored; opaque original update preserved|
| scalar dispatch |slot0x1855DC|stdcall(uint32 handle,uint32 parameterBits), pops8; EAX forwarded only from final call|
| vector dispatch |slot0x1855CC|stdcall(uint32 handle,float32 R,G,B,A), pops20; EAX ignored|

Initializer/normalize/accessor operate on locally owned scratch vectors, not global vector in place. Each by-value vector is16bytes with meaningful XYZ12bytes and a fourth opaque dword. Original local storage is debug-initialized to0xCCCCCCCC; ordinary initializer leaves fourth dword0xCCCCCCCC. Preserve that fourth dword in by-value callbacks and rotate result copying. Structs are four-byte scalar layout, not Clang native SIMD types. Use explicit approved ABI adapters for ST0/by-value/thiscall; do not infer signature from Hex-Rays guessed types. Angular dependency is semantically double despite decompiler presenting integer EAX: instruction-reviewed ST0 return used by caller.

Opaque wave callback transitive state currently known: original body updates RVA0x184AF0,0x177F84,0x1855F0,0x170354,0x1855F4 using inputs including RVA0x176370 and float table0x17028C. This is dependency-owned behavior, not duplicated here. A recorder can emulate chosen state mutation, including global records; consumer reads them after callback.

## Initial scratch construction

Initialize distinct16-byte scratch origin from XYZ=(0,0,10), bits0,0,0x41200000; fourth dword0xCCCCCCCC. Read global vector components through accessor in order index2, then1, then0, each using ECX=module+0x184C98. Read pointed-to32bits immediately after each callback; retain earlier values when later callbacks mutate globals. Initialize a second scratch16 with these XYZ bits and normalize that scratch via original callback. This normalized vector is held for subsequent angular calls; global vector itself is not normalized by this unit.

## Per-record scoring and rounding

Iterate signed i=0 while i<current live count. Recheck count each loop. The reference for squared distance is local origin(0,0,10), NOT the global vector. Every component access below invokes original accessor on local origin; every position read is fresh AFTER that accessor returns. Do not cache record positions across these callbacks.

Let store32 mean float32 rounding under active x87 control, and arithmetic before each store use active x87 operation precision in the stated order. Constant K is float32 bits0x40133333 (approximately2.29999995), original data RVA0x163B74.

Distance sequence:

- Access origin0; subtract live recordX; store32 into a private X difference.
- Access origin0 AGAIN; subtract fresh recordX; multiply by stored first X difference; store32 into private X contribution.
- Access origin1; subtract live recordY; add K; store32 into private Y difference.
- Access origin1 AGAIN; subtract fresh recordY; add K; multiply stored Y difference; add stored X contribution; store32 into private XY contribution.
- Access origin2; subtract live recordZ; store32 private Z difference.
- Access origin2 AGAIN; subtract fresh recordZ; multiply stored Z difference; add stored XY contribution; store32 into record+44.

This is not equivalent for mutated callbacks or all numerical inputs to computing one cached displacement and one consolidated dot product. Operation order and float32 intermediate stores are observable.

Next construct distinct16-byte direction scratch. Access origin2; calculate live recordZ minus returned origin2, store32 Z argument. Access origin1; calculate live recordY minus origin1 then add K, store32 Y argument. Access origin0; calculate live recordX minus origin0, store32 X argument. Call initializer(direction,X,Y,Z), then normalize direction. Pass normalized global-vector scratch first and direction scratch second by value, each16bytes, to angular callback. It returns x87 double. Convert that returned value to float32 once into an otherwise-unused private local WITHOUT consuming the original ST0 value, and independently convert to float32 for magnitude callback argument. These stores preserve original FP-status effects. Invoke magnitude callback with the float32 argument; store its ST0 result to float32 record+48. The result stored at+48 is magnitude CALLBACK output, not raw angular output. Then multiply fresh live float32 record+44 and fresh live record+48, store32 record+52. Magnitude callback may mutate+44 before the product read. Next iteration rechecks live count. No color/direction/parameter fields are directly written by this phase.

## Mode-dependent direction updates

After scoring loop independently read current mode. If mode==1, perform two rotation sequences:

- Initialize a scratch16 to(0,-1,0), fourth dword0xCCCCCCCC. Read current global float32 at0x184C90, widen exactly to double, call original sin. Multiply returned ST0 result by binary64 constant bits0x3FC3333340000000 (approximately+0.1500000059604645). Store float32 into unused local, then independently into angle argument. Invoke rotate with output scratch, by-value initialized vector, angle, axis(1,0,0). Copy16bytes from returned pointer to local source vector. Access resulting indices0,1,2 and immediately copy32bits to record0+24,+28,+32 respectively.
- Repeat independently with a new scratch and freshly read sin input; coefficient bits0xBFC3333340000000 (negative). Publish to record1 direction+24/+28/+32.

Then independently read mode AGAIN. If mode==2, perform the positive coefficient sequence and publish to record0 direction. This is not an else-if or cached mode decision: callbacks in the mode1 branch can change mode to2, causing both branches to execute. Each direction component accessor may mutate globals; component return is read immediately then its destination is written before the next accessor. No record-count gate protects these writes; mode1 writes first2 slots even when count0/negative. Other modes perform no rotation; this is fully observed, not guessed invalid-mode rejection.

Rotation helper remains original; do not reimplement its trig/vector matrix as part of this unit. Preserve live sin input per sequence and both float32 stores. Fourth vector dword is passed/copy-preserved, not used as a mathematical homogeneous coordinate.

## Ordering and original displaced-parameter behavior

Nested increasing indices: outer signed i starts0 and continues while i<(live count minus1); inner j=i+1 increasing while j<live count. First natural scope bounds count0..3, so count-minus1 overflow is excluded; pathological INT_MIN behavior UNKNOWN/fallback.

Compare record_i+44 and record_j+44 as x87 float32 values. Swap only when i value strictly greater than j value. Equal values, signed-zero equality or unordered comparison do not trigger swap. This nested exchange algorithm is not generally stable for equal-key records: indirect swaps can reorder ties. Do not substitute stable_sort, product ordering or a different selection algorithm.

On swap, move ALL14 dwords of record_j to record_i (56bytes) and move original record_i to record_j EXCEPT record_j+36 becomes literal uint32/int32 bits0x00000007. It is NOT float32 seven (0x40E00000). Original record_i+36 is discarded; this changes parameter values as records are displaced and can persist across repeated calls. Exact copies include color/opaque/tail fields and previously computed distances/scores/products. Preserve this apparent game bug. No callback occurs during sorting. Count reads remain live at loop conditions, although ordinary single-thread callbacks cannot intervene in this phase; races outside scope. NaN comparison may set x87 invalid flag; first native scope finite values, NaN semantics require differential vector before claiming support.

## Publication and seven dispatch callbacks

Invoke preserved wave update after sorting. Then UNCONDITIONALLY read first3 static records and publish below, even if live count<3 or negative. Read each source field individually AFTER wave callback; no pre-wave snapshot. Source colors are not used for publication: all selected RGB globals are forced white.

For selected index k=0,1,2:

| k | Position destination RVA (3dwords) | Forced RGB destination (3dwords) | Direction destination (3dwords) | Parameter destination |
|---|---|---|---|---|
|0|0x170178/17C/180|0x1701B8/1BC/1C0|0x1701F8/1FC/200|0x170228|
|1|0x170188/18C/190|0x1701C8/1CC/1D0|0x170204/208/20C|0x17022C|
|2|0x170198/19C/1A0|0x1701D8/1DC/1E0|0x170210/214/218|0x170230|

Within each k: copy position XYZ from record offsets0,4,8 as exact32bits; write forced RGB each0x3F800000; copy direction offsets24,28,32 as exact32bits; copy parameter+36 as raw32bits. Complete k0, then k1, then k2. No dependency callbacks intervene among these writes. No count/other game fields directly written.

Then dispatch exactly seven calls, reloading function slot, handle global and argument globals at EACH call:

1. scalar slot0x1855DC(current uint32 global0x17D950, literal0).
2. vector slot0x1855CC(current global0x17D908, current RGB globals0x1701B8/1BC/1C0, literal float1 alpha).
3. vector slot0x1855CC(current global0x184900, current RGB0x1701C8/1CC/1D0, literal float1).
4. vector slot0x1855CC(current global0x1851EC, current RGB0x1701D8/1DC/1E0, literal float1).
5. scalar slot0x1855DC(current global0x184298, current raw parameter0x170228).
6. scalar slot0x1855DC(current global0x175D90, current parameter0x17022C).
7. scalar slot0x1855DC(current global0x175EE8, current parameter0x170230).

Vector arguments read B,G,R, then handle; scalar reads parameter then handle; slot dereference occurs at invocation. This matters when earlier callbacks mutate later handles/RGB/parameters/slots. No selected positions/directions are passed directly to these seven callbacks; they remain globals for preserved downstream renderer. Forward final scalar callback EAX32 through normal balanced stack checker; do not return selected count or boolean. Semantic identities of handles/dispatch API remain UNKNOWN, though frame context supports shader/light parameter use.

## Supported scope, FP and fallback

Initial natural scope: original initialized render thread/current graphics context, supported original module, count3 from approved map1..7 light-model data, finite positions/global direction/sin input, x87 control word0x027F (53-bit precision, nearest/even, masked exceptions) as coordinator previously observed. Mode1/2/other branches are specified, but natural deployment depends on their isolated parity. Count0/1/2 and signed negative count (excluding INT_MIN subtraction issue) are explicit isolated cases with at least3 valid static record slots: stale publication is intentional. No assumption count<3 means no publication. First native guard may fallback short counts BEFORE invoking ANY callback/writing record state until those cases pass.

EXACT32bits for copies, arithmetic stored outputs and callback arguments; exact EAX/integers/order, normalized scratch pointer identity. Preserve x87 operation precision, explicit float32 intermediate stores, exception sticky bits0..5 and control word; arithmetic adapter independently designed from semantic math allowed. No FMA/reassociation/SSE float32 substitution. Original dependencies preserve their own FP behavior and are called rather than independently recoding sin/normalize/angular/rotate. Record MXCSR and preserve its state subject to original callbacks. Additional x87 condition-code bits require separate evidence; do not blanket claim whole-status equality without tests.

NaN/Inf, unmasked exceptions, nonstandard precision/rounding, dynamic count exceeding3, invalid callback pointers/results, arbitrary races and aliasing are UNKNOWN/out of first native scope. Fallback decision must occur before mutation/callback sequence; do not partially score then call original again. Source global vector may be zero: original normalize dependency has its own zero handling; native expanded support requires original math chain comparison, not guessed default direction. Keep original downstream frame, shader/draw and opaque math/wave dependencies intact. No heap ownership transfer or freeing in this unit.

## Disposable original-process differential fixture

Fresh initialized original test process, main parked/quiescent; never user's existing game. Invoke intact original body and candidate with reset identical module globals and private scratch behavior. Patch scalar/vector DISPATCH DATA SLOTS to typed stdcall recorders, so noGPU. Wave thunk can be typed noarg recorder, controlling deliberate post-sort changes. Math callbacks may remain original for realistic CPU-only chain, or their approved five-byte thunks may be redirected to typed deterministic recorders in a disposable process never resumed afterwards; no original bytes/trampoline or shared DLL code. sin direct body may be recorder-only redirected under same never-resume condition. Original body remains intact. Terminate process afterward.

State snapshots/reset: count0x177F88; full3*56 record window0x178110 (including untouched/stale fields); global vector XYZ0x184C98; mode0x184770; input0x184C90; all selected destinations above; all seven handle globals; dispatch function-slot pointers; if original wave retained, dependency globals0x184AF0/177F84/1855F0/170354/1855F4, delta0x176370 and table0x17028C valid dependency state. Use sentinel padding adjacent to snapshots. No fake model or vtable is needed: no model input is read.

Deterministic math-recorder requirements: initializer writesXYZ and returns this leaving fourth sentinel; accessor logs this-role+index and returns chosen valid float32 pointer; normalize may return chosenXYZ while leaving fourth unchanged; angular receives two16byte values and returns chosen double in ST0; magnitude receives float32 and returns chosen FP value; rotate receives output-pointer+16bytes+fourfloats and returns chosen valid16byte buffer; sin receives double and returns ST0 double; wave receives noargs. Compare normalized scratch roles (origin, global-normalized, per-record direction, per-rotation/output), fourth0xCCCCCCCC and by-value16byte copies. Do not treat recorder EAX as angular FP result.

Required isolated cases: count3 with ordered/reverse/unequal keys; equality keys with indirect swaps; swap+36 literal7 propagation and repeated invocation; count0,1,2,negative with stale first3 publication; modes0,1,2,other; angles0/positive/negative; finite cancellation and rounding-sensitive coordinates; angular output requiring float32 rounding; magnitude recorder alters result; final scalar return0/0xFFFFFFFF/marker. Exact callback counts/order and final states/ABI/FP compared.

Mutation cases: origin accessor changes positions between repeated same-axis reads; angular/magnitude changes distance/count/mode before subsequent reads; magnitude updates+44 before product; scoring callback shrinks count; mode1 callbacks change mode to2 for subsequent independent branch; sin callback changes next angle input; component callbacks change destination globals but returned values still stored immediately; wave mutates sorted records before unconditional first3 publication; early dispatch modifies later handles/RGB/parameters/function slots to confirm live reads. Sort callback-free phase cannot simulate arbitrary between-comparison mutations except concurrency, which is excluded.

Original math preserved differential should separately cover zero global vector and angle degeneracy; dependencies own their resulting NaN/errno behavior. Tests must keep dispatch/wave isolated so no draw or file/UI side effects. NaN sort vectors compare no swap and x87invalid if scope is expanded; don't silently normalize NaN.

## Natural integration proof

After independent PASS, guarded authorized test copy redirects only thunk0x16E0, preserving prior replacements and original dependencies. Before installing route, coordinator may observe original entry metadata/state (count/mode/vector/angle/CW and3records) in a disposable guarded baseline. Native hook logs bounded pre/post snapshot and counters, not original code. Capture first invocation and repeated frame invocation; mode determines which specified branch must have isolated PASS. Log seven callback handles/arguments/results and exact selected globals BEFORE downstream frame modifies them. Compare replayed equivalent original snapshots using original math in isolated process; screenshots supplement, not replace parity evidence. Natural map1 rendered geometry/lighting must remain visible and all five prior routes continue functioning. Existing user game stays untouched.

## Evidence/unknowns

Static review original VA0x4362A0..0x436E27, thunk0x4016E0, caller0x448734; helper bodies0x41EAB0/41B8F0/420C90/425010/41B730/41BB20/436180; constants0x563B74/563BA0/563B90; data dispatch slots0x5855CC/5855DC. CONFIRMED strict comparison, live count/branch reads, unconditional first3 publication, raw56byte swap except displaced int7, and final EAX forwarding. Hex-Rays angle return guess corrected by ST0 evidence. Runtime findings not yet claimed.

UNKNOWN declared array capacity, semantic meaning of handles/field36/field40, full special-value FP policy, natural mode/CW snapshots for this exact boundary, opaque dispatch transitive GPU/global effects. All preserved as dependencies or guarded scope. No claim of entire lighting reconstruction or per-map-object selection.

## Replay readiness supplement — preserved math and wave

CONFIRMED dependency review: wave body0x36180 contains CPU arithmetic/static-table accesses only, no GL/file/heap/callback dispatch. For replay retaining real wave, snapshot/reset float32 delta0x176370, accumulator0x184AF0, phase0x177F84, int32 current index0x170354, int32 previous index0x1855F0, float32 output0x1855F4 and204-byte static data window beginning0x17028C. This covers observed indexed reads0..50 and the current-index location; do not assume it is a clean independent51-element container (the address ranges overlap). Preserve the original dependency to reproduce such behavior. Native observed indices must be within its actual readable range; unsupported/corrupt wave indices fallback before this consumer. Recorder-wave harness needs none of this transitive arithmetic state except chosen mutation outputs.

Preserved normalize/angular/rotate helpers are CPU-only math for valid accessor indices0..2; their CRT acos/sin/cos/sqrt and assertions remain original, no GPU required. Invalid accessor index assertions are not triggered by this consumer. Preserve their exception/errno/FP effects rather than substitute injected math. Real-helper original-vs-candidate replay can use fresh identical record/vector/mode/angle/wave inputs while scalar/vector dispatch slots point to recorders. Natural callback parameters/results can be replayed by typed dispatch recorders, but this only establishes consumer state/callback parity; opaque driver side effects remain tested through natural rendering. Callback mutations observed in natural traces must be reproduced for corresponding checkpoint comparison, not assumed absent.

ABI bridge clarification: angular's first logical argument is16bytes from normalized global-vector local; second argument16bytes from normalized per-record direction. Its actual caller consumes ST0 double, not EAX. Magnitude receives one float32 argument and returns an FP result in ST0. Rotate logical arguments are output16 pointer,16-byte vector value, angle float32, axisX float32, axisY float32, axisZ float32; returned EAX pointer is authoritative for copied16bytes and may differ from output pointer in recorder cases. Initializers and accessors have thiscall callee cleanup12/4; all by-value math/rotation callbacks are cdecl caller cleanup. A metadata-driven bridge may use independently designed ABI glue; no original instructions provided.

## Wave-index clarification — approved guard requirements

Fresh original sessiona97cc40c, whole hash reverified: on-disk initialized current index at RVA0x170354 is signed1; previous index at0x1855F0 is signed0. The supported replay/native-readable range is signed0..50 INCLUSIVE for both. No -1,0xCCCCCCCC or other sentinel is approved. If wave advances, it saves the old current as previous, increments current and wraps to0 only when current>50. Thus current50 is permitted and the indexed slot50 intentionally overlaps the current-index global; preserve the original dependency and raw snapshot. This range is a conservative bounded fixture policy, not evidence that arbitrary outside-range original behavior is safe or impossible.

No additional finiteness guard for original wave delta/accumulator/phase is REQUIRED by this consumer's compatibility contract while wave remains original. Delta0x176370 and accumulator0x184AF0 feed dependency-owned arithmetic; preserve its behavior. Initial phase0x177F84 is overwritten from newly computed accumulator before read, and initial output0x1855F4 is also overwritten; neither initial value is a needed finite-input precondition. Snapshot/reset both for exact complete-state comparison nonetheless. If an implementation chooses a narrower finite-wave fixture for isolated validation it must label that test scope, not silently introduce an unsupported native guard requirement. Nonfinite transitive wave arithmetic is not independently reimplemented here. Original index handling has no bounds check; rejected index scope should invoke intact original fallback BEFORE consumer mutations rather than guess sentinel conversion.

## Coordinator implementation and observation update — 2026-10-04

Independent implementation and scoped validation completed for the documented consumer. Final build CTest7/7, final original-process36 differential scenarios and8 native-prestate original/replacement replay pairs PASS. Native map1/mode1 and map2/mode2 rendered textured lit combat; first3 and120th replacement-call data captured. Implementation guard still conservatively falls back before effects for unsupported states. See validation/RENDER-0004.md for independent checks, exact scope, artifact provenance and FP observation-timing limits. This does not expand unknown capacities, special-value FP or GPU parity claims.

Rotation scratch alias clarification: original each branch's component accessor ECX reuses the SAME initialized source-vector local after16-byte returned rotate result is copied into it. The rotation output scratch is distinct; returned EAX pointer is authoritative. Harness normalizes logical source/output identities, never exact original stack addresses.
