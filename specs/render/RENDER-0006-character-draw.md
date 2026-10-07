# RENDER-0006 — Actor mesh frame interpolation and emission

Status: READY_FOR_IMPLEMENTATION (bounded valid readable inputs; runtime validation pending).
Confidence: CONFIRMED ABI, layouts used, callback order and arithmetic; UNKNOWN source provenance and whole object ownership.

Original: program.exe SHA256 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, x86 image base 0x400000, image size 0x19F000. Independently reverified in IDA session 8146d6db. Thunk RVA 0x1497, body RVA 0x69A0. The thunk is a complete five-byte direct tail transfer. Metadata-only generic redirection and restoration to the approved body are permitted after root independently checks the target and whole file identity. Implementation must not read or save original instructions.

## Boundary and provenance

This unit emits interpolated mesh triangles. It performs no bone weighting, bone matrix multiplication, normal normalization, animation advancement, material selection or matrix transformation. Frame morphing is CONFIRMED; format/library/source attribution is UNKNOWN. Do not describe this as standard MS3D skinning or classify it as an upstream match without further evidence. Replacement of this authorized unit does not authorize replacing its accessor dependencies.

Body 0x405FC0 calls this thunk at 0x405FEA (return address RVA 0x5FEF), then processes linked parts with original tag interpolation and matrix callbacks and recursively calls itself through thunk 0x401F3C. Its top wrapper 0x405EB0 advances original state through preserved callbacks, then supplies owner+644 as the initial model. All animation, tag, matrix, gameplay and texture-loading behavior stays original. Caller-based native gate: return RVA 0x5FEF, checked readable inputs and the approved current floating environment. Root must record natural calls to establish actor identity; no actor global is asserted here.

## ABI and inputs

x86 thiscall: ECX = owner O; one stack argument = model M; callee removes four bytes. Preserve nonvolatile registers. EAX is a 32-bit result described below. No C++ object construction/destruction is part of this unit.

All offsets decimal unless prefixed 0x. Read-only fields used by this unit:

| Record | Offset | Type / behavior |
|---|---:|---|
| Model M | 0 | signed mesh loop count, live each outer condition |
| Model | 8 | embedded skin collection, passed as ECX |
| Model | 24 | embedded mesh collection, passed as ECX |
| Model | 48 / 52 | signed frame A / B indices |
| Model | 76 | float32 interpolation factor, live per arithmetic component |
| Mesh (288-byte collection element) | 0 | signed vertices per frame |
| Mesh | 4 | signed triangle loop count, live each triangle condition |
| Mesh | 12 | signed skin collection index |
| Mesh | 16 | unsigned byte textured flag |
| Mesh | 272 / 276 | pointers to frame-major position / normal float32 XYZ triples |
| Mesh | 280 | nullable pointer to float32 UV pairs |
| Mesh | 284 | pointer to 24-byte triangle records; first three signed int32 fields are indices |
| Skin (536-byte collection element) | 516 | signed texture-table index |
| Owner | 4 + 4*texture-table index | uint32 texture ID |

Minimum model read span is 80 bytes, mesh 288 bytes, skin 520 bytes. These are access spans, not invented allocation sizes. Actual collection accessors read the begin pointer at collection+4: model+28 mesh base and model+12 skin base. Mesh gate additionally reads collection+8 (model+32 end), returning zero if begin is null, otherwise signed byte difference divided by 288. Preserve callbacks rather than reconstructing STL ownership.

## Preserved callbacks

| Purpose | Thunk RVA / body RVA | ABI |
|---|---|---|
| Mesh collection gate | 0x1C35 / 0x94E0 | thiscall ECX=M+24, no args, EAX int32 |
| Mesh lookup | 0x13FC / 0x9550 | thiscall ECX=M+24, stack signed index, ret4, EAX mesh pointer |
| Skin lookup | 0x173F / 0x9300 | thiscall ECX=M+8, stack signed index, ret4, EAX skin pointer |

Gate result is tested for unsigned nonzero, not positivity. The two lookup callbacks are retained originals in natural runs and typed recorder stubs in isolated tests. Their internal callbacks and compiler/debug machinery are outside this replacement.

## Ordered observable behavior

1. Invoke the mesh gate once. Zero returns EAX=0 with no GL calls. Nonzero starts signed mesh index zero and compares it to live model+0.
2. For each mesh call mesh lookup once and retain the returned pointer. Cache frame offsets A and B as low-32-bit products of the corresponding model frame index and mesh vertex count. Each product obtains its own current inputs, before any GL callback for that mesh.
3. Read the textured byte. If nonzero, call glEnable(0x0DE1); afterwards read mesh+12, call skin lookup, read returned skin+516, read the owner texture ID, then call glBindTexture(0x0DE1, ID). Zero flag performs neither enable nor bind and does not disable texturing.
4. Call glBegin(4), including meshes with nonpositive triangle counts. Iterate signed triangles using live mesh+4. Emit corners in order 2, 1, 0. Each corner caches its index from freshly read triangle pointer plus 24*triangle+4*corner. Remaining 12 bytes of the record are unused here.
5. If the current UV pointer is nonnull, read T first, reread the UV pointer and read S, then call glTexCoord2f(S,T). A null UV pointer omits this call. Neither triangle index nor cached frame offsets is recomputed after this callback.
6. After the UV callback, copy four XYZ triples into local float32 snapshots, in this order: A position, B position, A normal, B normal. Each triple uses a fresh appropriate mesh pointer and the cached index+frame offset. Copies preserve component bits. No callback occurs between these snapshots.
7. Compute normal components in order Z,Y,X. Each component uses the corresponding normal snapshots and independently reads current model+76. Call glNormal3f(X,Y,Z). No normalization or clamp occurs.
8. After the normal callback, compute position components in order Z,Y,X from already copied position snapshots, independently rereading model+76 each time. Call glVertex3f(X,Y,Z). Thus a normal callback changing the factor affects the position but does not change the copied position inputs.
9. Repeat corners/triangles; triangle count is live at each condition. Call glEnd once. Increment mesh index and test live mesh count. Cached frame offsets persist through that mesh; subsequent meshes obtain new ones.

The function writes no source/model/owner/global fields and performs no allocation, free or filesystem operation. It enables/binds textures without restoring state. No glMaterial or shader callback belongs to this unit.

## Floating behavior and return

For each component the semantic operation is `(B - A) * factor + A`, with sequential x87 subtraction, multiplication and addition under the active control word; only the final GL argument is stored as float32. First supported environment: CW 0x027F (53-bit intermediate precision), finite source components/factors/intermediates, no reassociation or fused operation. Factor may be outside [0,1]. Compare emitted float32 bits EXACT, including signed zeros, and retain evaluation order. A semantic arithmetic adapter is allowed; copied original instructions are not. Exceptional/nonfinite environments remain original fallback before callbacks.

EAX: zero gate returns zero. Nonzero gate with initial mesh count <=0 returns the unchanged gate result. After at least one mesh, EAX equals the processed outer index after its final increment, even if callbacks shortened the live mesh count. glEnd residue is overwritten by that increment. Callback EAX values otherwise do not constitute the return.

## Approved isolated and natural validation

Recorder slots (RVA; stdcall ABI): glEnable 0x18CA68 (uint32, ret4), glBindTexture 0x18CA58 (uint32,uint32, ret8), glBegin 0x18CA8C (uint32, ret4), glTexCoord2f 0x18CA88 (float32,float32, ret8), glNormal3f 0x18CA84 (three float32, ret12), glVertex3f 0x18CA80 (three float32, ret12), glEnd 0x18CA7C (noargs). Arguments are recorded as immediate raw bits in actual call order. Accessor callbacks record normalized collection roles, indices and returned record roles. Replace these dependencies only in parked-main disposable isolated processes; preserve them in native rendering.

Synthetic fixtures: zero gate; nonzero gate with zero/negative mesh count; untextured/textured mesh; zero/negative triangle count; nullable UV; reversed corner order; unequal frame indices; factor zero/one/intermediate/extrapolation; signed-zero components; multiple meshes; callback changes factor at normal; UV callback changes geometry pointers; enable changes skin index; accessor changes live count; begin/end changes counts; triangle-pointer changes after vertex; frame indices changed during callbacks must not alter cached frame offsets. Controlled mutations must keep every subsequent access readable and finite. Compare exact ordered trace, source-state witnesses and EAX.

Natural proof: observe approved caller and owner/model identities, capture typed model fields, mesh records, owner texture entries, skin records and required frame-major position/normal arrays, UV pairs and full triangle records. Store frame indices and factor before each captured draw; capture at least two distinct moving poses and ordered forwarded GL traces. Replay this captured input against original body and replacement with accessor/GL recorders and no GPU. Preserve pointer roles rather than numerical addresses. Capture must be coherent before entry; do not snapshot asynchronously mid-draw. Current matrix/state behavior is outside the CPU replay and remains original in native mode. Root/validator provide screenshot and forwarded-call proof.

Native guard must establish readable complete accessed ranges, nonnegative frame/index/table indices, nonoverflowing address products, index<vertices-per-frame, complete known frame bounds, finite source values/factor and CW. Do not invent a fixed engine capacity; use the captured allocation/asset bounds with a configured capture-memory budget. Unknown callback mutations, invalid pointers, nonfinite arithmetic, unmatched caller, or unavailable bounds use original body before any callback or GL effect. Unsupported behavior is not silently skipped.

## Remaining unknowns / evidence

### Native bounds supplement — loader-backed untextured scope

Header representation supplement (CONFIRMED, reverified 2026-10-05, IDA eb2fe0ef exact executable hash): the header is a raw 108-byte file-data copy, not a decoded/reordered structure. Original read at 0x407205 copies file bytes [0,107] to loader L+[4,111]. Signature is L+4, four ASCII bytes `IDP3` (hex 49 44 50 33), equivalently uint32 little-endian 0x33504449 = 860898377. Version is int32 little-endian at L+8, expected 15 = 0x0000000F. In a copied 108-byte header buffer H starting from L+4, signature offset is H+0, version H+4, frame count H+76, tags H+80 and surfaces H+84. These buffer-relative offsets must not be confused with loader-relative 80/84/88. File header bytes are not original implementation bytes and may be copied as typed input witnesses. Original parser 0x407280 writes temporary allocation pointers at L+112 and above, not header bytes; postparse cleanup thunk 0x401523 -> 0x408700 only closes L+0 stream and does not clear header. Thus immediately after successful original return the header numeric witness remains unchanged for valid manifest files. Original loader does not check the read result or validate signature/version; AL1 alone must not authorize malformed files. Registry signature/version checks are bounded activation guards combined with preverified asset hashes, not claims of original error checking.

CONFIRMED original loader route: thunk RVA 0x144C -> body RVA 0x7160. ABI thiscall ECX=loader L; stack arguments (model M, filename pointer); ret8; success boolean AL=1, failure AL=0. Forward original full EAX unchanged in an observer. Loader entry filename/model and successful return are an approved typed observer boundary. After success, model mesh records and frame-array pointers are ready; record them before caller continues. Loader header occupies L+4 through L+111 and remains available at return. Header frame count L+80, tag count L+84, surface count L+88. File signature IDP3 and version15 are independently observed in installed BHM files; this establishes format family, not source provenance.

CONFIRMED allocation behavior in original 0x407830, called from 0x407280: for each surface with F frames, V vertices and T triangles, separate original operator-new calls allocate positions 12*F*V bytes and normals 12*F*V bytes (allocation sites within 0x407830), UV 8*V bytes and triangle records 24*T bytes (allocation sites within 0x407830). Allocation outputs become mesh+272/+276/+280/+284. Mesh+0=V, +4=T, +8=V: **mesh+8 is not frame count**. F is not retained in this mesh. Original disposal 0x404C90 deletes these four pointers; the renderer owns none of them. Registration must invalidate when pointers/counts change or destruction/reload occurs; observer may scope to one disposable startup with no reload and compare pointer/count witnesses before each draw.

The approved typed manifest RENDER-0006-approved-bhm-bounds.json independently records all eight installed BHM paths, SHA256, header/surface counts and exact end offsets. Each has one surface, 749 frames, zero tags; surface F equals file F. V/T pairs respectively: (1999,2832), (1944,2794), (2036,2714), (2003,2827), (2462,2878), (2071,2962), (2098,2836), (2178,2786). Confidence counts/hash/end: CONFIRMED from files. Hash-bound loader registration plus matching returned mesh count/V/T and unchanged pointers establishes frame-array lengths from original allocation behavior, not VirtualQuery. Root supplies approved typed/hash metadata; implementer must not read original assets.

Initial native activation is explicitly limited to successful registered manifest loads, frame indices in [0,748], matching one-mesh pointer/count witnesses, valid bounded triangle indices, readable allocated ranges, finite inputs/CW, and **textured byte zero**. Original surface creation zero-initializes that byte. The character loader 0x404F60 calls BHM loading and animation loading; it does not call the separate skin/shader loaders. Natural typed observation must still confirm zero at entry. This scope exercises complete moving geometry and normal emission while avoiding any owner texture-table access. Textured native draws fallback before effects until table capacity is established. Synthetic textured tests remain approved with explicitly sized fixture tables.

Native loader-backed untextured scope status: READY_FOR_IMPLEMENTATION_AND_RUNTIME_VALIDATION. Filename lifetime: copy a bounded terminated filename at entry; the caller may use stack storage which is not retained after the original call. M is explicitly passed as the first stack argument and is the destination used by the original parse call, so capture its numeric identity at entry. Capture L identity at entry and copy its header immediately after original return, before wrapper returns; L may also be a caller-local object. Do not retain L or filename pointers as future-readable objects. Register a successful load only after AL=1 and matching manifest header/mesh witnesses. At entry to every subsequent load for M, invalidate any old registration; at success replace it, at failure leave it invalid. Native draw revalidates registered M identity, collection begin/end, mesh record identity, V/T and all four pointers. Frame indices/factor are live draw inputs, not registration constants. No generation is reused after destruction; a registered model must remain in the witnessed owner or disposable startup lifetime. Root confirms asset hashes independently before launch, passes typed manifest identity, and observer filename selects that root-confirmed entry. Filename alone is insufficient evidence if hashes have changed.

Owner constructor 0x4046C0 constructs and clears nine embedded 112-byte model regions at offsets 420,532,644,756,868,980,1092,1204,1316; destructor 0x404A70 cleans these regions. Confidence CONFIRMED. Top draw supplies owner+644; recursive linked models remain original. Owner+404 has a separate collection cleanup; therefore no claim that every byte before the first model belongs to a texture array is permitted. Exact owner texture-table capacity remains UNKNOWN. No guessed 100/104-slot native limit is authorized.

UNKNOWN: full owner allocation extent and texture-table capacity, upstream source origin, behavior for nonmanifest assets. Manifest hashes and 749-frame allocation bounds are confirmed above. No native actor call has yet been observed for this new unit. READY applies to the specified synthetic/readable contract, not a claim of completed runtime validation. Evidence: original body 0x4069A0 through return, sole caller 0x405FEA in 0x405FC0, recursive call 0x406631, top wrapper 0x405EB0, accessors 0x4094E0/0x409550/0x409300 and begin-pointer helpers 0x40CF10/0x40CD40. Confidence of these static facts CONFIRMED.

