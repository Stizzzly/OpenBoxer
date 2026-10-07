# RENDER-0003 — Map light-model preparation

Status: READY_FOR_IMPLEMENTATION within scoped valid light-model data; original-process differential and natural validation PENDING. Agent1 analysis, 2026-10-04.

## Identity, purpose and routes

Original program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`, x86 preferred base0x400000, image size0x19F000. Fresh IDA session c8cf9c25 independently verifies identity. Stage A: thunk RVA0x1690 -> body0x27AF0. Stage B: thunk0x1974 -> body0x35D90. WORLD calls A at VA0x42ECAE, then B at0x42ECB9 on the loaded light model after reloadTextures. Only one caller of each thunk was found.

CONFIRMED: A prepares a flat embedded triangle representation; B prepares global spatial/color records plus GL state. HIGH: these global records supply subsequent lighting processing, based on consumer body0x4362A0 reading positions, colors and direction-like triples, sorting records and transferring selected records to later callbacks. Neither selected unit invokes glLight*, draws geometry or implements the subsequent dynamic selection/update. Do not invent a physical light type. Replace these two complete bounded units, preserving WORLD, MS3D, reload/uploader, decoder, GL and downstream lighting consumer.

Approved metadata-only routing: synthesized five-byte relative tail jump at each thunk; original body remains intact callable for fallback/differential. Uninstall synthesizes jump to body. Exact original whole-file hash including overlay, live module identity and privately checked baseline routes are required before mutation. No original instruction bytes/pseudocode supplied to Agent2. Conflicting routes fail guard. Raw evidence stays outside repository.

## Shared ABI/layout

Both x86 Microsoft thiscall: ECX model pointer, no explicit arguments, plain return (no stack argument cleanup). Preserve EBX/ESI/EDI/EBP and stack. EAX/ECX/EDX volatile. No allocation, release, file access, model vtable invocation or texture upload occurs.

Model full size0x8D9C4 from IO-0003. Fields: signed int32 mesh count+0x8D9A4, mesh pointer+0x8D9A8, material pointer+0x8D9B0, signed int32 triangle count+0x8D9B4, triangle pointer+0x8D9B8, vertex pointer+0x8D9C0. Mesh12 bytes: signed material index+0, signed int32 membership count+4, pointer to int32 triangle indices+8. Triangle76: normals0..35, s at36/40/44, t at48/52/56, int32 vertex indices60/64/68. Vertex16: positions at4/8/12. Material80: diffuse RGB at16/20/24; scalar at48 is read by B independently; its semantic name is not required here.

Embedded flat records occupy model+4 with stride116 (0x74). There are5000 available records before mesh-count field. This is an address-derived capacity, not a bounds check: original performs no overflow check. Object+0 vptr is preserved.

## Stage A — flattening and arithmetic center

CONFIRMED data movement and signed loops. Initialize output record index zero. Iterate meshes in increasing index while signed index<live mesh count. Obtain each mesh's signed material index once before iterating that mesh. Iterate membership indices in increasing order while signed index<that mesh's live membership count. Resolve membership's int32 triangle index and triangle76 pointer. Append one flat record for each membership, including duplicates; do not instead iterate the triangle array globally, deduplicate, skip negative materials, or write a new triangle count.

For each output record let R=model+4+116*outputIndex. The exact field mapping is:

| Relative R offset | Type/source |
|---|---|
|0|int32 mesh material index|
|4,8,12|vertex0 position XYZ from triangle vertex-index+60|
|16,20|triangle s0/t0 (+36/+48)|
|24,28,32|triangle normal0 XYZ (+0/+4/+8)|
|36,40,44|vertex1 position XYZ from triangle vertex-index+64|
|48,52|triangle s1/t1 (+40/+52)|
|56,60,64|triangle normal1 XYZ (+12/+16/+20)|
|68,72,76|vertex2 position XYZ from triangle vertex-index+68|
|80,84|triangle s2/t2 (+44/+56)|
|88,92,96|triangle normal2 XYZ (+24/+28/+32)|
|100,104,108|computed center XYZ|
|112|UNCHANGED reserved32 bits|

All position/UV/normal movement is exact32-bit copying, including sign bits; no normal transform, normalization, UV conversion or material lookup occurs. For each center component evaluate position0+position1, then add position2, then divide by exact float32 3.0 (constant bits0x40400000 at original RVA0x162428), finally store float32. XYZ order. The sum/division uses active x87 precision and rounding, with no float32 intermediate store until center output. Output index increments after each record. Other model bytes remain unchanged, including counts/source arrays and unproduced record area. No callbacks/globals are touched by A.

Exact EAX register result: zero or negative mesh count leaves0xCCCCCCCC from original debug initialization; positive mesh count returns number of meshes visited, even if every mesh membership count<=0. It does not return the number of output triangles. Known caller ignores EAX. Signed comparisons and final mesh-index increment establish this (CONFIRMED static); isolated runtime must verify.

## Stage B — global records and GL state

Set global signed count at RVA0x177F88 (VA0x577F88) to zero. Then ordered callbacks, even if triangle count<=0:

1. glColor4f(1,1,1,1), exact float bits0x3F800000.
2. glBlendFunc(0x302,1): SRC_ALPHA, ONE.
3. glBindTexture(0x0DE1, uint32 read from global RVA0x174F08). Read texture ID after preceding callbacks, not at entry.
4. glDisable(0x0B44): CULL_FACE.

Iterate signed index starting0 against live model triangle count+0x8D9B4. This uses the original loader's count, not A's actual flattened membership total. For each i read material index from flat R+0, then selected material80 from current material-array pointer. Before color callback, load float32 material+48, double it with active x87 arithmetic and store float32 into a private unused local. It has no model/global output but can set floating-point status/raise unmasked exceptions; do not silently eliminate its observable FP effects.

Call glColor4f(material diffuse R,G,B, float32 alpha bits0x3F333333). Argument reads occur B,G,R after the unused scalar computation; no callback intervenes among these reads. The alpha is independent of material transparency. Then compute XYZ from the live flat positions AFTER color callback: position0+position1, then +position2, multiply by float32 bits0x3EAAAA3B (original constant RVA0x163B84, approximately0.33333001), store each float32. This differs from A's divide-by3 and does not read A's computed centers. XYZ computed into private locals before global stores.

Write global record G=module+RVA0x178110+56*i:

| G offset | Write |
|---|---|
|0,4,8|computed XYZ float32|
|12,16,20|live material diffuse RGB32bits, reread after color callback/arithmetic|
|24|uint32 zero|
|28|float32 -1 bits0xBF800000|
|32|uint32 zero|
|36|int32 5|
|40|uint32 zero|
|44,48,52|UNCHANGED by this unit|

Do not clear remaining global records or these last12 bytes. Increment global count by reading its current value and adding1 after each complete record; callbacks may have changed it. Record placement uses loop index, not global count. Then increment loop index and compare live triangle count again. Reads of source materials/flat positions after callback must not be replaced with entry-cached snapshots when mutation cases are tested. No material/flat record write by B itself.

After loop call glEnable(0x0B44), then glColor4f(1,1,1,1). Always enable culling; previous state is not queried/restored. Blend function and bound texture remain as set; final color white. No GL error query, texture cleanup, light API or drawing.

B is semantically void. Original final EAX equals register residue left by final glColor4f, passed through normal debug stack checker (checker does not alter EAX on balanced stack). It is NOT triangle count/success. Typed recorder can deliberately expose an EAX marker for exact register comparison; natural GL void API provides no meaningful portable return. ABI equivalence should preserve this residue when strict EAX test is requested. No independent fixed EAX value exists. Known WORLD caller ignores it.

## GL IAT recorder metadata

|Import|Slot RVA|ABI|
|---|---|---|
|glColor4f|0x18CA9C|stdcall(float32 R,G,B,A), pops16; semantically void|
|glBlendFunc|0x18CA98|stdcall(uint32 source,destination), pops8|
|glBindTexture|0x18CA58|stdcall(uint32 target,ID), pops8|
|glDisable|0x18CAB8|stdcall(uint32 cap), pops4|
|glEnable|0x18CA68|stdcall(uint32 cap), pops4|

Patch original-process IAT data slots only for disposable tests; do not patch shared GL DLL code. GL stack mismatch triggers original debug checker and is a harness bug, not game logic.

## Floating-point contract and first scope

Initial comparison EXACT float32 bits for copies/centers/globalXYZ/color args, plus integer/state exact. Original uses x87 and honors current control word; replacement must record and reproduce active precision/rounding rather than assume ordinary Clang SSE float32 arithmetic. No FMA or reassociation. With53-bit precision, each arithmetic operation rounds at53 bits;64-bit precision uses64-bit significand, not binary64. Final store rounds to float32. Special-value payload propagation, exceptions and sticky flags require actual original differential evidence before expanding scope.

First natural scope: independently approved installed light.ms3d files for maps1..7 from IO-0003 manifest, structurally valid nonaliasing source arrays, valid material/triangle/vertex indices, membership total equal loader triangle count, flat total<=5000, three global outputs for these installed fixtures. All seven inspected light files have9vertices/3triangles/3groups/1material and one triangle per group. Failure-empty objects with count0 are supported; do not read absent arrays on skipped loops. Use fallback before writes for unsupported shape/ownership/environment. Actual natural FP control/status still must be measured. NaN/Inf, arbitrary precision modes and unmasked exceptions remain UNKNOWN until specific vectors pass; an implementation must not claim universal exact parity from fixture success.

Global array declared capacity is UNKNOWN. Natural scope bounds to3 records; larger synthetic tests must independently reserve/verify accessible scratch global window in disposable process, not overwrite unrelated game data. A does not validate its own5000 capacity either. Negative material indices in nonempty B are invalid dereferences in original, not a request to substitute default material.

## Differential and natural validation

Use fresh initialized original process, main thread parked, never user-playing process. Stage A needs no GPU/dependency stubs: prepare two equivalent full-size fake models with distinct buffers and sentinel-filled embedded area; invoke intact original A and candidate, compare all model/source bytes normalized pointers, exact EAX, stack/nonvolatile, x87 control/status. Cases: empty/negative mesh count; positive empty meshes; multiple groups, reordered and duplicated triangle memberships, differing materials, normal/UV sign-bit sentinels, cancellation/rounding-sensitive coordinate triples, installed map1 light schema. Do not normalize output away if flattened total differs from loader triangle count.

Stage B: typed GL IAT recorders, snapshot/restore global count and3*56 record window plus texture global between original/candidate runs. Seed untouched record tails and following bytes. Equivalent flat model/material inputs; recorder snapshots global state at each callback to verify ordering. Cases: empty/negative triangle count (still six setup/final callbacks),1/3records, differing diffuse colors, zero/large texture ID, untouched tails, repeated preparation. Final-color recorder EAX marker verifies B residue. Observe no allocations/free/file/other GL callbacks.

Mutation cases in B: initial color/blend changes textureglobal before bind; color changes positions/material-array pointer/diffuse values and compare live subsequent reads; color shrinks count after current record (current still completes); callbacks change global count before increment (no forced count=i+1); final enable/color observes completed global state. Callbacks preserve FP control/status for base comparison; a separate test intentionally changing FP control can establish dynamic-control behavior. Compare state and arguments exactly. Preserve unmasked exception handling scope rather than create guessed recovery.

Record actual original x87 control word and MXCSR on entry and after calls; independently inspect Clang emitted arithmetic/rounding and compare sensitive vectors. Tiny differences remain FAIL_FLOAT under initial EXACT policy until Agent1 explicitly approves a different policy from evidence. There is no time integration here to justify epsilon automatically.

After isolated PASS, authorized test-copy route both units, originals available for fallback. Observe WORLD calls A then B once on light model at natural map1 load; capture flattened records, global count/records before downstream frame consumer mutates/sorts them, GL callback parameters and FP control. Natural expected stable schema yields3 global records. Preserve original renderer/consumer and screenshot textured/lit gameplay. Screenshot alone is insufficient to prove preparation parity. Never alter existing active user game.

## Evidence and remaining questions

CONFIRMED fresh disassembly body0x427AF0..0x427ED9 and0x435D90..0x4360B5; constant data reads; thunk routes; calls0x42ECAE/0x42ECB9; import data addresses; balanced __chkesp0x4953E0; xrefs to global records and downstream consumer0x4362A0. Consumer inspection supports lighting role but does not approve reimplementation of consumer. Raw routing/disassembly stays under C:\Users\ADMIN\Boxer-analysis\fresh-coldet.

UNKNOWN-1: global array declared capacity and malformed overflow semantics. UNKNOWN-2: natural FP control/status and exact special-value exception behavior. UNKNOWN-3: physical meaning of global record offsets36/40 and downstream sorting tails44..52 beyond observed constants/untouched status. UNKNOWN-4: natural exact EAX residue from driver final color; semantic return remains void. These do not require guessing names or extending scope.

## Implementation-readiness supplement

Coordinator reports actual live x87 control word0x027F: all six exception masks set, precision53-bit, round nearest/even. This is a runtime observation supplied by coordinator, distinct from this fresh static review. First natural replacement is gated to that observed control regime until others pass. A semantic arithmetic adapter using independently designed x87 operations is permitted: ordered addition/addition/division for A; ordered addition/addition/multiplication for B; doubling/material float32 store before B color. Do not mechanically translate original instructions. Ordinary SSE float32 operations are not equivalent to this contract.

Require exact32-bit outputs and original x87 sticky exception-status effects of these arithmetic operations, including the unused doubled material scalar. Control word is preserved; no explicit clearing of status by original. Compare exception status bits0..5 and stack balance, plus preserved control. x87 condition-code bits/C1 after arbitrary GL callbacks are not currently independently specified; report observations, do not promise blanket entire-status equivalence without differential evidence. MXCSR should remain unchanged by this unit's own arithmetic in a compatible x87 adapter; original GL callback effects are preserved dependency behavior. Callback recorders use identical controlled FP behavior. Unsupported initial regime falls back before output mutation.

Inline record region is model+4 through model+0x8D9A3 inclusive, exactly5000*116 bytes; object allocation size0x8D9C4=580036 bytes. Each record's final four bytes at relative112..115 remain untouched. Scope guard can preflight valid nonaliasing arrays/indices and sum positive mesh membership counts<=5000 before writing. Do not test overflow by corrupting a natural process. If loader triangle count exceeds flattened membership total, B would read untouched records; first valid scope requires equality and fallback preserves unspecific malformed behavior. A itself does not write total into any object field.

Global56-byte records reside in the executable's static data segment, not a heap allocation or caller-owned container. No allocation/constructor/free/init-other-than-described writes occurs in either unit; do not allocate/free or relocate this original array. Lifetime is process lifetime. B reinitializes count and first44 bytes as listed for produced records, leaves tails and unproduced records unchanged. Downstream original consumer later modifies/sorts these records, and remains preserved. First unit stores at mostthree records corresponding to installed light fixtures; declared capacity beyond this is UNKNOWN.


## Clarification — B post-color diffuse reads

CONFIRMED fresh original session26c0607c with full hash reverified: after the per-record glColor4f callback and XYZ calculation/global stores, each of G+12, G+16 and G+20 independently rereads BOTH the current flat record's signed material index R+0 and model's current material-array pointer+0x8D9B0. Each component resolves its selected material from these fresh values, reads material+16/+20/+24 respectively and stores32 raw bits. Order is R store, then G store, then B store. There is no reused initial selected-material pointer, no index cached across the callback, and no material index/table pointer shared across these three post-color component reads. No callback intervenes among these component stores; nonaliasing stable input naturally yields the same selected material. A per-record color recorder may change R+0 and/or the material-array pointer: subsequent global colors must use the updated index AND updated table, while earlier glColor arguments retain their pre-callback values. The unused material+48 computation precedes the callback and therefore uses the earlier selection. Include combined index+table mutation in differential vectors. Arbitrary data races/aliasing remain outside first scope.
