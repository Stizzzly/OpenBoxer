# ANIM-0004 — Activate strike, block and idle animation selection on the actual character model

Status: VALIDATED_BOUNDED as an ANIM-0002 activation extension; see validation/ANIM-0004.md and implementation-freeze.json. No new semantic core. Confidence: CONFIRMED original selector ABI/behavior, active callsites, loader and supplied assets; HIGH natural scene linkage pending actual capture. Preserve eleven prior semantic cores unchanged.

Provenance: original program.exe SHA25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6 rehashed2026-10-06; IDA session736dce9b analysis ready. Full selector406E40, rendering/dispatch437110, player input41EF90, loader404F60 and key-message449BA6/449BB7 independently inspected. Installed asset listing and 1_animation.cfg read as data only. Raw analysis remains outside replacement repository.

## Evidence-based scope correction

An upper-model name selector exists at bodyVA406D40/thunk4015E1. Its offsets map owner+532. However IDA has zero callers to its thunk; an independent raw PE section scan found only thunk4015E1's transfer to406D40, no direct relative call/transfer to4015E1 or406D40 and no literal pointer candidates to either. This is HIGH evidence that it is unused here, not proof against every computed indirect route. No native upper caller or lifetime is established. Loader404F60 formats upper/head filenames but its complete body calls BHM loader144C only for lower filename and owner+644 at4050A7. Installed base/fighters contains eight lower.bhm and eight animation.cfg files, no upper/head files. There is no approved upper manifest to create.

Actual active attack/block/idle dispatch invokes the already specified whole name selector401C26->406E40 on owner+644 using LEGS_* names. The lower file holds the visible character animation. Parent437110 is a large two-actor renderer/dispatcher with position/time writes, GL work and eight actor variants; no separate strike-only dispatcher function exists in the observed branch. Inventing a mid-body hook or replacing the whole renderer is outside this bounded task. Therefore this stage activates the established selector for verified additional strike/block callers, preserving original state decisions, attack timing, damage, input and rendering. Do not implement unused upperselector or add fake upper assets.

## Whole selector contract / ABI

Authority for all exact body behavior remains ANIM-0002-name-selection.md, incorporated in full. Hook thunkRVA1C26/bodyRVA6E40, complete5-byte transfer. thiscall ECX=ownerO; stack requestednameN; ret4; M=O+644. Own ordered match writes M+44 selectedindex, M+48 secondlookup record+256 rawframe, M+76 bits0x3DCCCCCD, M+80 raw globaltime sampled last. Live signed boundM+40 is reread each loop; first matching comparator zero wins; lookup then comparator per candidate, secondlookup after indexwrite on match. EAX ownerO on success, final loopindex on positive no-match,0xCCCCCCCC on initial nonpositive bound. Original lookup10B4/body97A0 ECX=M+84/indexret4 and original locale-dependent cdecl comparator989C0 remain preserved callbacks. Own FP effects none; EXACT callback/environment/state/return comparison. Do not change this semantic core or any ANIM-0001/0003 arithmetic.

Comparator capture seam remains original selection CALLRVA6E94->989C0, return6E99, complete5-byteE8, root-preverified opcode/target and metadata-only synthesized install/restore in isolated fixtures or opt-in capture only; ordinary unpatched. No new body patch or spoofed caller is needed.

## Confirmed active actor0 call roles

In original437110, actor indexj loops0..1 with232-byte state stride. When currentdesired signedstate at5849B4+232*j differs from rememberedstate5849B0+232*j, original stores desired into remembered before selecting. Actorvariant584784+4*j selects one of eight global owners. The variant0 branch explicitly loads ownerVA57ACD0 for every following call. Calls are independent live rememberedstate tests, not an invented cached switch; callback-driven changes could permit later tests too. That parent behavior remains original.

| State | Requested literal | Call RVA | Return RVA | Literal RVA |
|---|---|---|---|---|
|0|LEGS_STAND|0x371B9|0x371BE|0x1623E8|
|1|LEGS_RUN|0x371DA|0x371DF|0x163C70|
|2|LEGS_RUNB|0x371FB|0x37200|0x163C64|
|3|LEGS_LEFTHEAD|0x3721C|0x37221|0x163C54|
|4|LEGS_RIGHTHEAD|0x3723D|0x37242|0x163C40|
|5|LEGS_LEFTTORS|0x3725E|0x37263|0x163C30|
|6|LEGS_RIGHTTORS|0x3727F|0x37284|0x163C1C|
|7|LEGS_LEFTBOK|0x372A0|0x372A5|0x163C0C|
|8|LEGS_RIGHTBOK|0x372C1|0x372C6|0x163BFC|
|11|LEGS_BLOCK|0x37324|0x37329|0x163BD0|

States9 pain/10death/12win remain original in initial ANIM-0004 scope. Other actor variants remain original. Previously approved0/1/2 remain active; add3..8/11 only. This is an explicit route extension, not authorization for arbitrary callers to the selector.

## Guard / lifetime / integration

Existing ANIM-0002 loader-backed lower-model lifetime witness remains authoritative: root-preverified RENDER-0006 lower manifest, successful original144C load, exactM identity, counts/pointers/header/source witnesses unchanged, ownerO==base+17ACD0 and M=O+644. Do not broaden old witness to owner+532. Require matching approved returnRVA, requestedpointer==modulebase+the exact listed literalRVA and matching terminated literal bytes, positive bound<=256 consistent with readable272-byte collection, names terminated within256, record start in0..748 and all existing ANIM-0002 readable/FP/time guards. Conservative all-scanned-start checks may remain. Newly admitted caller requests must use the unchanged selector semantics and original dependencies; unsupported states fallback before effects. Actual registry hit is mandatory; static owner relationship alone is insufficient.

Keep eleven previous core hashes immutable. Only activation/capture/fixture glue may change. Native captures record role(state/name/caller), normalized pre/postM112bytes, selected collection record data and requested bytes, exactEAX/callbacktrace/globaltime/FP environment plus unchanged lower manifest/lifetime witnesses. Capture all supported sides of alternation where practical. Composed playback continues through active ANIM-0003 future slots and ANIM-0001 progression; validate activation witnesses for all three during short attack/block pose checkpoints. Ordinary diagnostics off; all supported cores active. No independent launch or installed-game write by Agent1.

Opt-in capture budget: first two accepted calls for each of seven new roles and idle, at most16 data captures per process. Do not spend the new budget on movement1/2. Reference-original and candidate snapshots are identified explicitly; replay batches remain bounded<=16 and may use separate reference/candidate lists if necessary. Ordinary mode performs no new trace/capture filesystem IO or heavy snapshot work. Observation waits belong to a short unparked gameplay scene; no fixture worker remains parked during user gameplay.

## Proven original input scene, not replacement input logic

Original message cases256/257 set/clear bytes at576914+wParam (449BA6/449BB7). Original42D880 invokes player41EF90 at42D89D with ECX584910; its+164 desiredstate is5849B4, read by437110. Supported actorvariant0 is required to hit listedowner.

- Z, VK0x5A/90 => byte57696E: at41F61F guarded press chooses state3 if player+168 nonzero (41F69F), otherwise4 (41F6C7), then toggles+168 and sets attackflag5761C9. Head literal semantics supported by names; hit/damage remains original.
- X, VK0x58/88 => byte57696C: at41F77C chooses state5/6 according to player+169 then toggles it (41F7FC/41F824).
- C, VK0x43/67 => byte576957: at41F8DA chooses state7/8 according to player+170 then toggles it (41F95A/41F982).
- Space, VK0x20/32 => byte576934: at41F531/41F53B sets blockflag5761D0 and state11 when allowed; release resetsstate0 and clearsflag at41F582/41F58C if currentstate11 or0.

Attack gates include !584794 &&58479D, menu577F90==0, opponenthealth576238>=0, per-key latch clear, !5761C9 attack,!5761CA,!5761C8,!5761F2,!5761D0 block. Space requires no menu, no attack5761C9/no hit5761C8. These are capture preconditions, not candidate decision inputs. Root's short actual testcopy scene waits for active gameplay/draw registration, taps Z/releases, waits original attack completion, taps Z/releases again to witness the other hand; X and C likewise for their pairs; holds/releases Space to witness block/idle. Never inject state/damage or assume a tap hit the route. Record actual callbacks; original timers/conditions can prevent selection. Key injection alone is not PASS.

Original1_animation.cfg typed data: leftheadstart0/end10/rate40; lefttors10/23/40; righthead23/33/40; righttors33/46/40; leftbok46/65/40; rightbok65/84/40; stand99/174/30; block224/249/40. These are installed data observations, not universal per-fighter constants; runtime collection is the authority, and otherfighters must retain their actualrecord values.

## Fixtures / validation / unknowns

Existing ANIM-0002 exact fixtures remain mandatory evidence. Add role-table activation tests for all3..8/11, rejectedwrongname/literalpointer/owner/return/unregisteredlifetime, with zero bodyeffects on rejection; original-process captures/replay for real head sides, torso sides, hook sides, block and idle. Compare all112 modelbytes/fullEAX/callbackorder/FP EXACT; retain earlier core integrity evidence. Natural missingroute observations are BLOCKED per role, not blanketPASS. Capture proves selector replacement at attack/block routes; it does not claim recreated attack decision/damage behavior or a new twelfth core.

UNKNOWN: computed indirect reachability of unusedupperselector, unusedupper/headfilename intent, broader gameplay state meanings and other-variant admission. Excluded pain/death/win may be addressed as a later bounded route extension. No missing semantics block this verified activation extension; original observations still required. No original asset/code supplied to implementer beyond these typed contracts.

