# RENDER-0005 — Map mesh/material immediate-mode drawing

Status: READY_FOR_IMPLEMENTATION within scoped valid map model. Confidence: CONFIRMED static ABI/calls/branch/data-source contract; isolated and natural replacement validation PENDING. Agent1 original-only review, 2026-10-04.

## Identity, boundary and native scope

Target original Месть боксера. Московский криминалитет program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`, x86 preferred base0x400000, image size0x19F000. Fresh original IDA session674ebfdb independently verified hash; no ETS2 or legacy OpenBoxer source/assets used. Complete body RVA0x26B00 (VA0x426B00..0x426FD8), route thunkRVA0x1B54. Actual role CONFIRMED: traverse original mesh/material/triangle/vertex arrays, configure material and texture/shader parameters, emit GL_TRIANGLES immediate-mode vertices, restore entry texture-enable flag.

Thiscall ECX=model, no explicit stack arguments, plain return. Preserve EBX/ESI/EDI/EBP and stack. Semantically void; final EAX register residue comes from final glEnable/glDisable and passes through balanced debug checker. Known callers ignore EAX. Strict recorder parity forwards all32 bits of final enable/disable callback EAX, NOT triangle count or boolean.

Frame callers VA0x44849B/0x44857A/0x448699/0x4487E3. First two confirmed use global model pointer VA0x585568 (lamp route); main map caller0x4487E3 loads ECX from VA0x585560 after light consumer and other opaque callbacks. First natural replacement gate: ECX must equal nonnull current module global pointer RVA0x185560. Other model identities/callers use intact original body. Global185560 read is approved scope metadata, not a new input used in renderer calculations. Preserve6 prior validated replacements and all opaque frame transforms, shader/render/math and GL dependencies. No new matrix/camera/light transformations in this unit.

Approved hook: synthesized5-byte relative tail jump at thunk to replacement; original body intact for direct fallback/differential. Uninstall synthesizes jump to approved body. Whole-file hash including overlay, live module identity and privately validated baseline route required before mutation; conflicts fail guard. No original bytes/instructions/pseudocode provided to Agent2. Raw analysis under C:\Users\ADMIN\Boxer-analysis\fresh-coldet only.

## Model ABI and ownership

Full model layout IO-0003 size0x8D9C4. Reads signed int32 mesh count+0x8D9A4, mesh pointer+0x8D9A8, material pointer+0x8D9B0, triangle pointer+0x8D9B8, vertex pointer+0x8D9C0. Guard may use known material/triangle/vertex counts+0x8D9AC/+0x8D9B4/+0x8D9BC for valid-data preflight; original drawing does not check those counts.

Mesh12bytes: signed material index+0, signed membership count+4, pointer32 to int32 triangle indices+8. Material80bytes: four float32 vector groups0/16/32/48 (16bytes each), scalar+64, scalar+68, uint32 textureID+72. MS3D schema identifies material+68 as transparency input, HIGH semantic confidence from approved format; compatibility uses raw field and threshold, not inferred alpha compositing rules. Triangle76bytes: three normal XYZ triples at0/12/24, s0..2 at36/40/44, t0..2 at48/52/56, int32 vertex indices60/64/68. Vertex16bytes: XYZ+4/+8/+12. No embedded flattened116-byte records are read here.

No model/source/global writes, allocation, release, file access, texture upload or ownership transfer by this unit. Dependencies can mutate model/GL/global state; live reads below preserve it. Source-pointer arguments must refer to original source arrays, not temporary reconstructed vectors. This makes pointer identity and mutation behavior independently testable. Current GL matrix/context, camera, normal transforms and shader program are owned by caller/dependencies and preserved.

## Ordered entry, mesh loop and material callbacks

Call glIsEnabled(0x0DE1) once. Save ONLY returned AL byte as entry texture flag, not full EAX or canonical C++ bool conversion. Any nonzero low byte means restore enabled; upper bits ignored. Then loop signed mesh index0 while index<live model mesh count, rechecking count each iteration. Count0/negative still performs entry query and final restore, no material/geometry reads. Obtain current mesh signed material index ONCE before material callbacks, using current mesh table; retain this index through entire mesh even if callbacks mutate mesh material index. Material-array base, however, is read fresh before EACH argument component/pointer construction below.

For nonnegative selected index M invoke in exact order:

1. glMaterialfv(0x0404 FRONT,0x1200 AMBIENT,current material base+80*M+0).
2. glMaterialfv(FRONT,0x1201 DIFFUSE,current base+80*M+16).
3. glMaterialfv(FRONT,0x1202 SPECULAR,current base+80*M+32).
4. glMaterialfv(FRONT,0x1600 EMISSION,current base+80*M+48).
5. glMaterialf(FRONT,0x1601 SHININESS, raw float32 material+64 from fresh base).
6. Opaque vector slotRVA0x1855CC(current handle global0x1850E0, material rawRGB+0/+4/+8, literal float1 alpha).
7. Same vector slot(current handle0x17564C, RGB+16/+20/+24, literal1).
8. Same vector slot(current handle0x17D900, RGB+32/+36/+40, literal1).
9. Same vector slot(current handle0x1842B4, RGB+48/+52/+56, literal1).
10. Opaque float scalar slotRVA0x1855D8(current handle global0x176A14, raw float32 material+64).

Each glMaterialfv pointer is directly into current original material array; recorder captures its16bytes immediately at call. Do not snapshot all four vectors at entry or reuse the first pointer after callbacks relocate model's material table. Shader vector args read B then G then R, EACH with fresh material-array pointer but same cached M; alpha literal0x3F800000; handle read last, then live slot dereferenced. No callback intervenes among component reads, except arbitrary races excluded. Shader scalar reads current material+64 then handle, then live slot. All callbacks are performed even when this mesh later fails geometry threshold or has membership count<=0. Source vector alpha fields are passed to glMaterialfv but opaque vector callback alpha is forced1, not copied material alpha.

Negative M skips the ten material callbacks and directly glDisable(TEXTURE_2D). IMPORTANT original subsequently still reads material+80*M+68 for threshold; it does not safely select a default material. First native guard rejects negative/out-of-range material index BEFORE glIsEnabled/callbacks and calls intact original fallback. Do not claim negative-material safe behavior or substitute default transparency.

## Texture branch and geometry gate

After ten callbacks for nonnegative M, read uint32 textureID+72 from fresh material base. If nonzero, independently reread current material base+72 for glBindTexture(0x0DE1,ID), then glEnable(0x0DE1). All32 ID bits accepted, including0xFFFFFFFF; predicate is unsigned nonzero, not signed positive. No callback between test and ID reread. If initial ID zero, call glDisable(0x0DE1), no bind. Negative M uses the disable route described above.

After bind/enable/disable callbacks, reread float32 field+68 from current material-array base with cached M. Compare with exact float32 constant bits0x3ECCCCCD (approximately0.40000000596; original dataRVA0x16244C). Draw only if field is strictly greater. Equal/lower or unordered values skip drawing. No blending configuration, alpha rescaling, glColor call, sorting or partial-opacity draw path is added. Original x87 comparison can set FP status (including invalid for NaN); first natural scope finite values/masked regime. This unit does no floating arithmetic or coordinate transforms; all floating callback args otherwise bit-preserving loads/pointer arguments.

## Triangle membership and emission

If threshold passes, call glBegin(4=GL_TRIANGLES) ONCE for this mesh, even if membership count0/negative. Recheck signed membership index against current selected mesh membership count using freshly read mesh table each iteration. Thus glBegin or vertex callbacks may change count/table before next check. For each member, read current mesh table membership-list pointer and selected int32 triangle index, then current model triangle-array pointer to obtain triangle76 pointer. Cache THAT triangle pointer for all3 vertices of this membership; do not reread triangle array pointer inside its vertex loop. A later membership resolves fresh tables/pointers, including duplicate memberships.

For vertex k=0,1,2 in order:

1. Read int32 vertex index from cached triangle+60+4*k BEFORE glNormal callback; retain it for this vertex.
2. Call glNormal3fv(cached triangle+12*k), pointer to XYZ normals. Caller does not normalize or copy vector.
3. After normal callback, read raw t float32 at triangle+48+4*k, then raw s at+36+4*k. Call glTexCoord2f(s,t). No t flip here; loader already supplied runtime values.
4. After texcoord callback, read current model vertex-array pointer and call glVertex3fv(base+16*cachedVertexIndex+4). The current vertex index is not reread after normal/texcoord callbacks. glVertex pointer refers directly to current source positions; no bone transform, scaling, offset or coordinate conversion.

A glNormal callback changing this vertex's triangle index does NOT change its pending vertex lookup, but may change subsequent vertex indices. A glTexCoord callback relocating model vertices affects the pending glVertex pointer. Triangle-array relocation during a vertex does not change remaining vertices' cached triangle pointer, but affects next membership. Normal callback UV mutation affects pending UV args. Do not batch/collapse callbacks across vertices or meshes.

After all membership iterations call glEnd() once. No early stop on driver error and no GL error queries. Then increment mesh index, recheck live mesh count. If threshold fails there is no Begin/End or emission but material/texture callbacks remain. Original source data and counts remain untouched unless callbacks change them.

## Entry-enable restoration and returned residue

After mesh loop, if saved low byte from entry glIsEnabled nonzero, call glEnable(TEXTURE_2D); else glDisable(TEXTURE_2D). Always perform one restore callback even if current state already agrees, or count<=0. Do not query again. Forward final callback EAX32 through balanced ABI return for strict recorder equivalence; GL APIs are semantically void, so natural value is driver residue with no portable success meaning. Restore only TEXTURE_2D enable flag: bound texture, material, normal, UV, opaque shader parameters and other GL state remain last-set. No push/pop attributes/matrices. Driver callbacks/context requirements stay original.

## Callback/IAT approved metadata

All GL APIs x86 stdcall; RVAs are import DATA slots, not shared DLL code.

| API | SlotRVA | Arguments / callee cleanup |
|---|---|---|
|glIsEnabled|0x18CAEC|uint32 cap; pops4; GLboolean AL byte|
|glMaterialfv|0x18CAE8|uint32 face,pname,const float32*4; pops12|
|glMaterialf|0x18CAE4|uint32 face,pname,float32 value; pops12|
|glBindTexture|0x18CA58|uint32 target,ID; pops8|
|glEnable|0x18CA68|uint32 cap; pops4|
|glDisable|0x18CAB8|uint32 cap; pops4|
|glBegin|0x18CA8C|uint32 mode; pops4|
|glNormal3fv|0x18CADC|const float32*3; pops4|
|glTexCoord2f|0x18CA88|float32 s,t; pops8|
|glVertex3fv|0x18CAD8|const float32*3; pops4|
|glEnd|0x18CA7C|noargs; pops0|
|opaque vector|DATA0x1855CC|stdcall(uint32 handle,float32 R,G,B,A); pops20; EAXignored|
|opaque float|DATA0x1855D8|stdcall(uint32 handle,float32 scalar); pops8; EAXignored|

Slots and handles are read live at each call; do not replace these original dependencies in natural execution. glEnable/Disable recorder may expose chosen EAX residue, preserving full32bits in strict harness. Native pointers/payloads are observed immediately inside callback instrumentation before source relocation/free or later mutation. Source pointer identities normalized by model/table/record/index/component role, never absolute original addresses.

## Scope and compatibility guard

First natural map1 route: current nonnull map global185560 identity, initialized rendering thread/current appropriate GL context, valid original IO-0003 supported model layout, finite material+68 values, valid nonnegative material/triangle/vertex indices, bounded/count-consistent readable source allocations and original dispatch pointers. Installed map1..7 fixtures provide approved valid format inputs; first natural smoke map1. Guard evaluates before entry glIsEnabled or any callback; reject unsupported identity/input/environment to intact original body before effects. No fallback after partial GL/material emission. Other model callers retain original independently of valid map1 gate.

Preflight may validate source arrays/counts/indices of entire model conservatively, even geometry that threshold would skip; this is scoped support, not a new claim original validates indices. Mesh count<=0 requires no source arrays and is supported isolated. Membership count<=0 emits Begin/End if valid material field passes; negative material is outside native replacement and retains original fallback. Null model, invalid ownership, overflowed indices/strides, malformed aliases, arbitrary concurrent mutations, unmasked FP exceptions and special-value comparison remain UNKNOWN. Controlled recorder mutations using valid replacement buffers are supported differential cases below, not permission to validate after callbacks.

EXACT raw32bits for all float arguments/source vectors/indices/IDs, exact callback order/branch/return and unchanged state. No epsilon justified: no arithmetic here. Preserve x87 comparison behavior/status exceptions and CW; independently designed semantic x87 comparison adapter permitted, no original instructions. For finite normal values comparison is exact regardless53/64-bit precision; coordinate/material values are copied, not recalculated. Additional NaN/denormal/status-policy scope expands only with original observations. Initial CW0x027F as previous native observation may be guarded until this boundary is recorded.

## Disposable differential fixtures and native proof

Fresh initialized/quiescent original process with main thread parked; never existing user game. Invoke intact original body with fake full-size model and corresponding candidate inputs. No constructor/vtable needed: renderer reads no vptr. Populate approved model fields, POD materials80/meshes12/triangles76/vertices16 and local null-free membership lists; seed other object/source bytes with sentinels. Patch ONLY GL IAT DATA slots and opaque DATA slots to typed recorders. No GPU/file/heap dependency required. Restore snapshots between sides; terminate disposable process after interception. No original-byte copying or shared GL code patch.

Record all calls and pointer roles, immediate source4float/3float snapshots rawbits, scalar argument bits, glIsEnabled AL, final EAX, source/model unchanged bytes, stack/nonvolatile registers and FPcontrol/status. Shader handles and slots also snapshot for mutation parity. Compare no unexpected GL/material/opaque callbacks. Pointer identity is part of contract: glMaterialfv/Normal/Vertex must designate approved source locations, not temporary buffers.

Required cases: meshcount0/negative with both saved enable outcomes; noncanonical saved AL0x80 enabled and EAX0x100 with AL0 disabled; valid empty mesh above threshold Begin/End; membership countnegative; material fieldbelow/equal/nextfloatabove threshold; textureID0/nonzero/0xFFFFFFFF; multiple meshes/materials, shared materials, duplicate/reordered triangle memberships; all3 vertex normal/UV/position sentinelbits; threshold fail still fullmaterial/texture callbacks; final enable/disable EAX markers. Negative material supported only as preflight-fallback case before query, not unsafe direct dereference. Native fallback route tests count callbacks from original separately.

Mutation cases: entryquery changes model fields before loop; material callback relocates table and edits cached mesh materialindex (index must remain original for currentmesh); each opaque callback changes later handle/slot/materialdata; shader scalar changes ID/threshold beforetexturegate; bind or enable changes threshold; Begin changes membershipcount/table; normal changes pending UV and currentvertexindex (UV follows change,indexcached); texcoord relocates vertexarray; normal/vertex relocates triangletable (cachedcurrenttriangle,nextmembershipfresh); vertex or End shrinksmeshcount; callbacks change textureenable state but finalrestoration uses savedAL. Valid lifetimes retained in allmutationfixtures. Compare exactstate/callorder. Store bounds/sentinelchecks are legitimate fixtures, not guessed originalvalidation.

After isolated PASS, install guarded test-copy thunk route only for mapglobal identity and keep six prior replacements. Bounded natural per-frame trace for map draw: entrymodel/counts/CW/savedAL, permesh cachedmaterialindex/field68/ID, material/shaderargs, Begin/End counts, totalnormal/UV/vertexcounts and digest of ordered rawfloatpayloads plus a bounded sample. Context-tag wrappers to distinguish map unit from original lamp/othermodel emissions sharing IAT. Capture one complete original baseline mapdraw snapshot/trace where possible, replay modelarrays against original/candidate with recorders; count equality alone insufficient. Current transform/shader/context remain original; rendered gameplay screenshot supplements emission/state parity. Finalcaprestored,binding/materialnotrestored expected. Native originalothers/fallback calls logged separately; prove map-globalrouteused and allsix priorroutescontinue.

No installed original changes or active user game mutation. Native integration occurs in guarded authorized lab copy and user-authorized UI workflow handled by coordinator/validator.

## Evidence and unknowns

Fresh original body0x426B00..0x426FD8, thunk0x401B54, framecallers/xrefs, material thresholdconstant0x56244C, IAT/callbackrefs independently reviewed. CONFIRMED selectedmaterialindexcachedoncemesh, eachmaterialtable/readfresh, currenttrianglepointercachedoncemembership, vertexindexcachedbeforeNormal, vertexbasefreshafterTexcoord, entryALsaved/finalrestoreEAX. No raworiginalimplementationincluded. UNKNOWN shaderhandle meanings, transitive drivereffects, malformed negative-index behavior, specialFPstatus/fullcontextconstraints. Preserved frame/dependencies and originalfallback maintain those boundaries; no claim renderer/world fully reconstructed.

## Native-input replay supplement

Approved native observation may capture the typed renderer input state: eight known model count/pointer fields with normalized identities, complete80-byte material records (including live generated texture IDs),12-byte mesh records, each int32 membership list,76-byte triangle records and16-byte vertices bounded by validated counts. These are asset/runtime input data, not original implementation or code bytes. Renderer does not require vptr/embedded flat records/filename dereference, so no original code-bearing memory is needed. Asset rereading alone is insufficient to reproduce live IDs/material changes; replay complete used source payload plus recorded entry glIsEnabled AL and opaque handles. Pointer identities restored with independently allocated fixture buffers for both sides; synthetic full model580036bytes, no original constructor/destructor necessary. Fixture ownership belongs harness allocator, release through same owner after both calls; original draw never frees storage. Original heap callbacks are not needed for renderer fixture itself.

Natural logging can observe first and later bounded frame (for example120th supported map call), forwarding every real GL/opaque callback with exact ABI/return. Snapshot pointer-vector bytes immediately while valid, no second renderer invocation and no later dereference after callbacks relocate storage. Preserve glIsEnabled AL and final restore EAX residue via explicit adapter. Full ordered callback trace can be represented by streaming rawbit digest plus sample and totals; independent isolated replay should compare complete emission sequence, not merely digest/counts. Context tags identify map-only supported invocation so original lamp/fallback callback traffic is not mixed into its trace. Current transform/shader program stays original.

## Fixture bounds and material-vector semantics

No4000/5000 triangle drawing cap exists in this analyzed unit. The5000 embedded-record capacity belongs RENDER-0003 and is irrelevant to this array renderer. Initial native map1 fixture tuple from independent IO-0003 inspection is meshes27/materials27/triangles2788/vertices1806. Approved manifest contains map1..7 tuples; metadata includes exact paths/hashes/counts/membership lengths. Maximum inspected installed-map triangle count3214 is a fixture fact, NOT engine capacity. A bounded trace/fixture allocator may impose a documented resource limit, but exceeding it selects intact fallback BEFORE callbacks; never truncate/skip original traversal. Safe source bounds mean validated allocation lifetimes and complete count*stride spans, plus each mesh index-list length and index ranges, not merely an invented mesh maximum. Callback mutation fixtures retain valid buffers explicitly.

CONFIRMED GL parameter labels ambient/diffuse/specular/emission/shininess follow concrete pnames0x1200/1201/1202/1600/1601. Four-component material-pointer callbacks receive source alpha component at vector+12; opaque vector dispatch receives exactly three source components plus forced alpha1.0. Field+68 is not supplied to these GL material vectors or opaque scalar here: its sole direct use is geometry threshold. Its MS3D schema name transparency is HIGH provenance, not a claim that this renderer implements conventional alpha blending. Preserve concrete parameters and rawbits without redesigning opacity semantics.

## Coordinator implementation and observation update — 2026-10-04

Scoped replacement implemented independently and validated:8offline targets,31 synthetic original/candidate draw pairs,3 separate preflight rejection checks and2 captured-native-source replay pairs PASS. Actual native smoke was map2 (approved installed fixture), original/replacement calls1/2/3/120 with27295 ordered graphics events each matched semantic fields and rendered textured lit combat. Map1 native coverage is not claimed. Source layouts/ABI remain separate from formatted draw.cpp operations. See validation/RENDER-0005.md for independent proof, exact limits and artifact provenance. Original frame/GL/shader dependencies, other-model fallback and six preceding replacements are preserved. No expansion of unknown malformed input, GPU pixel/timing or full-renderer equivalence policy.
