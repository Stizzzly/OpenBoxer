# GAME-0002 — Typed layout, locals and ordered FP supplement

Status: APPROVED behavioral supplement to GAME-0002-opponent-strike-consumption.md. Confidence CONFIRMED inspected original forms/addresses/layout and bounded original native activation/fullFP (GAME-0002-original-natural-evidence.md); HIGH conservative bounds. Candidate equivalence remains UNKNOWN pending replay. Evidence same SHA-256 and IDA database, sessions c58b6594/f99e468f, 2026-10-07. Contains behavioral operands/layout only, no original body or machine bytes.

## Exhaustive entry roles and scalar guards

Offsets below are decimal unless written0x. All non-scalar fields/opaque fourth vector words are raw data. Every byte of actor232, both body2288, listed globals and pool7600 remains observable; guards constrain only the listed live reads.

Actor A=module+RVA1849F8, player P=module+184910. A initial arithmetic XYZ fields: +28,+32,+36 (cursor direction), +44,+48,+52 (movement direction), +108,+112,+116 (tail addend). Float32 scalars +140 accumulator; +180,+184,+188,+192,+196 decision timers; +228 block timer. Require normal-or-signed-zero finite abs<=10000. A+12..27 initial position and A+28..43 initial direction are also copied as four raw words to shared globals. A+76..91 and+124..139 are overwritten by movement helpers (preserve opaque W as prescribed), +92..107 and+172 are overwritten by native velocity/scalar callbacks; no float guard is imposed on their unused entry contents. A+212 is overwritten by cooldown calculation; no entry float guard on it. A bytes+171,+176,+177,+178,+204 in0/1, int32+208 in0/1, int32+216 in1..3. +164 is uint32 state0..11 except10. +152 is typed opponent-body pointer. +200/+220/+224 are raw int32 words; +200/+224 may be overwritten by lowcooldown decision. All remaining A bytes are raw preserved data or callback-owned fields.

Mode1 mapping is seven independent integer equalities AIindex1,2,3,4,5,6,7 -> A+220 int32 values1,2,3,4,5,6,7 respectively; no float representation is written. Mode0 writes int32(3), effective difficulty2. Mode1 effective difficulty is int32 global5847C4 in1..5. Readiness int32A+208=1 remains1 when updated timerA+196>=1: that comparison only writes1 when <1, never clears readiness. Block expiry is the path that clears it.

Global entry float32 arithmetic inputs: VA5761CC attack timer;5761D4 playerhealth;576238 opponenthealth;576258 AIfatigue;576260 combotimer;576370 dt. Finite normal-or-zero abs<=10000, plus constraints in main spec. Recoil timer57623C, AIattack timer576230 and table-independent fatigue-add5762BC are raw sentinels in admitted entries because their governing branches are excluded. Cached distance575EEC and position globals5761DC..1E8/576240..24C are overwritten before use; no input-scalar guard on old contents. All globals read only as bytes/integers are raw typed values with specified scope constraints: playerattack5761C9=1; type5761F8=1; pending57622C0/1; block5762340/1; marker57628C0/1; AIattack57622D/recoil57622E/fatiguelock576256/victory576254/combo-active57626C=0; combo-count576270=0; death584794=0; active58479D nonzero; menu577F90=0; mode5847900/1; playerindex5847840..7;AIindex5847881..7. Counter57620C, previoustype576224 and trigger576208, reset/global5761FC/576260/576288, audio source575DF0 and soundflag577F9E are recorded raw; no guessed semantics or bool canonicalization except explicitly admitted flags.

Fighter table comprises8 records at VA5762A0, stride16: +0 damage factor, +4 base duration, +8 health factor, +12 UNKNOWN raw field. Only current player record+0/+4 and current AI record+8 are FP inputs, positive normal finite<=10000; other table words are sentinels. Therefore named arrays5762A0/5762A4/5762A8 are fields of one table, not independent arrays; index offsets are16*index. Original table bytes are data, not implementation.

Body guard inputs for both registered bodies: normal-or-zero finite abs<=10000 for+752/+756/+760 positions. Only opponent body additionally requires +408 inverse-factor (meaning beyond multiplication UNKNOWN),+768/+772/+776 velocity, selected quaternion-like words+656/+660/+664/+668 (semantic identity UNKNOWN), fixed matrix+816/+820/+824,+832/+836/+840,+848/+852/+856. Player inverse-factor/velocity/quaternion/matrix are not read on this whole-function route and have no numeric guard; no supported operation is being permitted to consume their subnormal values. All other body data are raw, including vector W. Selector+652=0; graph pointer+216=0 CONFIRMED from original wake473CD0; decimal216 is0xD8, distinct from2160. Selfpointer+0, worldpointer+244, next+2276,prev+2280, activebyte+2284 obey exact lifetime witness. Require readable/writable body2288. Current world W=[5853A4], readable list headW+10968/tailW+10972/countW+10976. Initial bounded native route count2, distinct two bodies, head.prev0, tail.next0, forward/back links mutually coherent, both worldsW/self/active valid. Validate every admitted entry, not cached. Read-only aliases: static actors/globals/table/pool/world list must not overlap either body or each other beyond explicitly declared table/global field aliases; bodies distinct. Originals registered allocations/static ranges provide this naturally.

Health guard predicts actual final unblocked float32 health, rather than separately rounding loss: with saved/restored fullFP CW027F, load float32(2), multiply playerfactor, float32(2.5), AIfactor, subtract product from entry health in that order and round final value float32. Require predicted final health strictly positive finite. This supersedes the main text's conservative verbal 'health greater than maximum loss' as the executable rounding definition. Blocked loss is smaller for admitted positive factors. Check parent arithmetic intermediates finite/nooverflow and generated masked tiny policy before effects; this read-only prediction does not inject flags. With admitted bounds all ordinary parent expressions remain finite; safe output bounds below are independent from input bound10000. Health can increase only through unsupported callback mutation, never predicted.

## Callback mutations and bounds

Native callbacks remain original and do not mutate input decisions/registration/indices/health/timers, except original rand's RNG state and explicit actor/body callback effects. Fixtures may vary getter/vector output XYZ, A+140, live dt or audio output only at their relevant typed callback boundary, retaining mode, indices, body registration, attack/block/menu/health guard invariants. Limit fixture updated dt to[0,10000], A+140 abs<=10000, output XYZ finite abs<=1000000, scalar length/absolute ST0 finite nonnegative<=1000000, and all parent arithmetic predicted finite. Scalar ST0 retains raw80 representation (may contain meaningful extra precision); do not round before the specified store. Require canonical output16 returnedpointer for getter/vector helpers and canonical element pointer for accessors; alternate output/global/body aliases are outside this first fixture contract. Local output16 fourthword remains opaque. Fixture changes do not authorize after-effects fallback; fixture generator rejects incompatible input/return script before execution.

Native position getter outputs remain bounded10000. Impulse yields velocityXYZ abs<=15000, native speed/length finite<=26000. Movement scalar10.5*inputXYZ yields abs<=105000, tail addition at most115000; scalar-first final clamp abs<=30000. These generated/helper values are allowed even though entry scalar bound is10000. Preserved angular/wake effects are body-owned and must be compared raw; their opaque auxiliary words are not reinterpreted as finite game scalars. Unsupported entry structural or callback-mutated state must never be silently accepted. Callback return environments must respect originally audited calling conventions/stack depth; scripted environments may carry different sticky flags/rawEAX but may not create an unsupported CW, invalid scalar, stack fault or nonempty surplus stack.

## Local identities and lifetime

One function-local frame initializes all local data once with raw32CCCCCCCC on entry, before first own arithmetic; local pointer/counter/scalar assignments replace their relevant words. Local data persists until whole return. No reset between events unless explicitly assigned. Buffers listed below are distinct16-byte objects, do not alias actors/globals/body/world/each other. Parent copies use returned output pointer live bytes in ascending order.

| ID | Role | Original frame offset relative EBP | Initialization/reuse |
|---|---|---|---|
| L-POS-P | initial player getter | -0x98 | once poison; getter writes4words |
| L-POS-O | initial opponent getter | -0xA8 | once poison; getter writes4words |
| L-NEG | movement negation output | -0xB8 | once poison |
| L-MUL-F | movement choice1 multiply | -0xC8 | once poison |
| L-MUL-R | movement choice2 multiply | -0xD8 | once poison |
| L-HIT | hit impulse XYZ | -0x4C | constructor writes3words, W poison |
| L-HIT-POS | effect position getter | -0xE8 | once poison; getter writes4words |
| L-POOL-V | loop getter output | -0xF8 | reused100iterations, never re-poisoned |
| L-POOL-COPY | unused copied loop output | -0x64 | fourwords copied every iteration, persists |
| L-TAIL-POS | tail position getter | -0x108 | once poison |
| L-ZERO | tail angular argument | -0x84 | constructor writes3words,W poison |
| L-TAIL-V | tail velocity getter | -0x118 | once poison |
| L-ADD | tail vector addition output | -0x128 | once poison |
| L-MUL-S | tail speed-branch multiply output | -0x138 | once poison |
| L-LIMIT | final scalar-first helper output | -0x148 | once poison |

Audio local L-AUDIO is int32 at EBP-0x88, initiallyCCCCCCCC, distinct from all vectors; native alGetSourcei writes it once. Original scalar temporary positions: Xdifference -0x14C, Xproduct -0x150, Zdifference -0x154, effectY -0x158, effectX -0x15C, effectZ -0x160, final-speed Xproduct -0x170; all float32. Hit coefficient -0x50 written+0 then2.5 for Z. Loop index -0x54 int32 initialized0 before loop; three remainder temporaries -0x164/-0x168/-0x16C (third was oncepoison then overwritten) hold signedremainders. Movement choice -0x8 startsCCCCCCCC; effective difficulty -0xC is assigned mode0/1. These addresses define alias/lifetime relationships; a clean implementation need not reproduce frame offsets, but must preserve these distinct data roles and reuse.

Direct callback source/return RVAs and numeric targets are in observer metadata. Dynamic/role mapping: c70B=L-POS-P, c740=L-POS-O; cac6=L-NEG, cad6=L-MUL-F, cb7B=L-MUL-R; d775=L-HIT; d900=L-HIT-POS; d9E8=L-POOL-V; e04C=L-TAIL-POS; e079/e08E=L-ZERO; e0A3=L-TAIL-V; e145=L-ADD,e17E=L-MUL-S;e2F3=L-LIMIT. Sites in this paragraph are low VA suffixes, e.g. c70B means VA41C70B; authoritative metadata stores fullRVA. No callback shares two output roles. Scalar401014 takes four rawwords BY VALUE16, caller cleans16 total despite its temporary stack-slot reuse for the next scalar; absolute takes4bytes, sqrt8bytes. Constructor ret12/accessorret4/bodyret4/otherscdecl cleanup match JSON, never inferred from Hex-Rays prototype.

## Ordered FP comparisons

Each ID is CMP- followed by comparison RVA. At every listed reached site original form is x87 FCOMP against indicated memory operand, popping one scalar; operand already on ST0 is LHS, memory is RHS. LHS float32 load is exact into x87; products remain x87 CW027F precision until comparison. RHS is float32 except final float64. Following status capture uses FNSTSW AX without another FP operation. For finite ordered values: less sets C0=1,C2=0,C3=0; equal C0=0,C2=0,C3=1; greater all0; C1=0. Compare pops leave pre-load TOP/stack; sticky exception flags are retained and can be affected by generated denormal loads. With empty parent stack, postcompare TOP0. The next integer masks/branches do not alter FP; do not substitute FUCOM or C++ comparison. No bytes or disassembly bodies are supplied here.

| Comparison RVA / ID | LHS | RHS raw / width | Reached / selected relation |
|---|---|---|---|
|1C50C|A+180 f32|00000000 f32|always; <0 clears177, else decrement |
|1C54E|A+184 f32|00000000 f32|always; <0 clears176, else decrement |
|1C590|A+188 f32|00000000 f32|always; <0 clears178, else decrement |
|1C5D2|A+192 f32|00000000 f32|always; >=0 decrement |
|1C608|A+196 f32|00000000 f32|always; >=0 decrement |
|1C8AA|cached distance f32|40C00000 f32 (6)|when !decision204; > selectsforward, else nextXcheck |
|1C8C5|accessor globalopponentX f32|41180000 f32 (9.5)|prior distance<=6; > selectsforward |
|1C937|distance f32|40C00000 f32|recoil0 and !decision204; < continuesretreat |
|1C94A|distance f32|41000000 f32 (8)|retreat prefix true; < continues |
|1C965|accessor globalopponentX f32|41180000 f32|retreat prefix true,distance<8; < selectsretreat |
|1CB33|distance f32|41000000 f32|movementchoice2,AIattack0,block0; < continues |
|1CB4E|accessor globalopponentX f32|41700000 f32 (15)|previous true; < selectsretreat motion |
|1CBEB|updated A+192 f32|3F800000 f32 (1)|alwaysactive; < setsdecision/randomlimit |
|1CC93|playerhealth f32|00000000 f32|Z menu0; >=0 continues |
|1CD1C|distance f32|3F800000 f32|Z prior integer/byte predicates alltrue; > continues |
|1CD33|distance f32|40800000 f32 (4)|previous true; < initiatesZ (excluded) |
|1CFC9|playerhealth f32|00000000 f32|X menu0; >=0 continues |
|1D051|distance f32|3F800000 f32|X prior integer/byte predicates alltrue; > continues |
|1D068|distance f32|40A00000 f32 (5)|previous true; < initiatesX (excluded) |
|1D312|playerhealth f32|00000000 f32|C menu0; >=0 continues |
|1D39A|distance f32|3F800000 f32|C prior integer/byte predicates alltrue; > continues |
|1D3B1|distance f32|40C00000 f32 (6)|previous true; < initiatesC (excluded) |
|1D6DA|playerduration f32 *3F000000 f32 in x87|liveattacktimer f32|pending1; < consumes |
|1D822|livehealth f32|00000000 f32|consumption; >=0 damage |
|1DAAB|updated A+196 f32|3F800000 f32|alwaysactive; < setsready1 |
|1DAF4|distance f32|40C00000 f32|marker nonzero; < continuesblockstart |
|1DB69|live A+228 f32|3EB33333 f32|blocknonzero; <= increments, else expiry |
|1DD91|fatigue f32|00000000 f32|always; > decrements |
|1DDBC|updatedfatigue f32|42C60000 f32 (99)|always; >= would lock (excluded) |
|1DE11|updatedhealth f32|00000000 f32|always; < would death (excluded) |
|1DF66|livecombotimer f32|3F19999A f32|always; > clearscombo |
|1E123|live A+172 f32 *447A0000 f32 (1000) in x87|44BB8000 f32 (1500)|always; < add, else multiply |
|1E2C0|lastsqrt raw80ST0|4008000000000000 f64 (3)|always; > limit, <= ordinaryreturn |

The table identifies potential reached AI distance tests even though resulting attacks are excluded. Complete short-circuit predicate per type t=1(Z),2(X),3(C), evaluated independently in that order:
menu==0; playerhealth ordered>=0; AIattack22D==0; recoil22E==0; pending22C==0; fatiguelock256==0; block234==0; byteA+204 nonzero; int32A+216==t; distance>1; distance<limit4/5/6. Stop immediately at first failure. Pending1 never reaches subsequent flags/type/distance. Far pending0 may reach the distance tests only for its current type when other flags permit. All three playerhealth comparisons still occur when menu0 even with pending1.

Supported entry recoil0 means its two byte tests before consumption make no FP comparison. Supported AIattack0 means four attackcontinuation byte tests skip all their FP comparisons; lock0 skips <50; victory0 skips its timercomparison; combo-active0 skips initial <1. No such excluded comparison may be inserted 'for consistency'. Default movement comparison is integer choice !=0 && !=1 && !=2. Counter wraps uint32, readiness/difficulty/type comparisons areint32, state/index equalities are exactinteger.

Final return status: <=3 path performs final FCOMP at1E2C0, pops scalar, captures SW at1E2C6. No later FP instruction in successful stack checker, so rawEAX=(lastsqrt rawEAX & FFFF0000)|thisSW. For<3 C0=1;=3 C3=1; C2=0,C1=0,TOP0, stickybits unchanged except orderedload/compareeffects. >3 route replaces EAX and callback-returnFP with final45B690 event. External API identity is PE-import OpenAL32 / alGetSourcei at IAT RVA18CC00, callRVA1E1F4 length6 return1E1FA; nativeresolvedpointer is UNKNOWN until root reads initialized IAT, must be saved/verified by loadedmodule/export provenance. A static zero IAT value is not a native API address. Root observer forwards saved original once with unchanged args and records rawEAX/outputInt/fullFP.

## Audit resolution and publication scope

## Authoritative FP capture and equivalence policy — 2026-10-07

Status: APPROVED. Confidence: CONFIRMED for capture preservation and the live-state contract; HIGH for exclusion of architecturally empty register payload from behavioral equivalence. This section supersedes ambiguous uses of complete/exactFP elsewhere in GAME-0002. It does not weaken scalar arithmetic, exception flags or ABI checks.

Capture preservation: retain every raw byte actually captured, including all eight raw80 register payloads, empty slots, FIP/FDP/FOP, selectors and reserved/padding bytes. Never zero stale registers or normalize the saved evidence. Observer save/restore must preserve the original environment; any capture/restoration corruption is FAIL even for an empty slot. Do not discard data from reference/candidate records when preparing a comparator view.

Behavioral equivalence is EXACT for CW, the entire SW (including all sticky flags, C0/C1/C2/C3 and TOP), TOP, full tags and abridged tags, MXCSR, every live raw80 register, every scalar raw80 return and all specified scalar stores. Never use epsilon, compare only SW subsets, or classify a register as dead solely because its numerical value looks unused. Establish each slot's architectural tag and TOP at the particular checkpoint using the capture format's documented logical/physical register ordering. Empty means full tag11 and corresponding abridged bit0; inconsistent tag representations are a failure, not permission to exclude payload.

XMM0..XMM7 are architectural state, not reserved save-area bytes: compare all 128 bits of each EXACT at matched whole entry/exit and callback entry/return checkpoints (FXSAVE offsets0xA0..0x11F, eight16-byte slots). Confidence: CONFIRMED that original whole body RVA1C4E0..1E320 contains no direct XMM-register instruction: original IDA session ee231a18 scanned all1887 instructions without truncation on 2026-10-07. Opaque preserved callbacks can still modify XMM and their observed effects must remain EXACT; absence of direct parent XMM use is not proof of callback preservation. This contract does not claim the caller's subsequent use of XMM is analyzed and does not invoke an ABI volatile-register exclusion. Replacement compiler scratch usage does not justify masking architectural XMM differences. Any XMM mismatch is FAIL_STATE/FAIL_FLOAT pending investigation, or BLOCKED if capture/input comparability is unestablished; never PASS via a reserved-byte mask. Preserve all raw captured XMM bytes.

Explicit equivalence-only exclusions:

- A raw80 payload may be excluded only for a slot marked architecturally EMPTY in both matched captures. Reason: an empty x87 slot has no live operand value; its stale physical payload records historical scratch arithmetic, which a behaviorally equivalent implementation may arrange differently. Supported whole entries are empty, parent arithmetic uses newly loaded operands, and scalar callbacks supply a live ST0 which remains EXACT. A slot live in either capture cannot receive this exclusion. This permission does not authorize clearing captured bytes, observer restore changes, clearing flags or omitting scalar return checks. Record checkpoint, slot, both tags and differing raw payload in the exclusion report.
- FIP/FDP and their selector/address provenance may be excluded from byte equality only after proving they identify corresponding original/replacement instruction or data locations, or stale non-live provenance. Reason: original and replacement code/data locations differ. Retain and report raw values plus provenance/mapping. Unknown or unexpectedly changed provenance is BLOCKED for investigation; never ignore it silently. FOP receives no blanket exclusion: compare decoded operation when it represents live provenance; a stale-only exclusion needs a recorded checkpoint-specific reason.
- Reserved/padding bytes may be excluded only where the documented save-format marks them reserved or outside an architectural field. Record byte offsets and the format reason. Never treat control/status/tag/MXCSR/live80 bytes as padding.

An independent validator must publish both exact architectural-state results and a separate inventory of every raw capture difference covered by these explicit exclusions. Four or any other number of dead-slot differences is not independently evidence of PASS; each needs the tag/checkpoint justification above. The earlier natural-evidence admonition against clearing stale bytes concerns capture/restoration fidelity, while this section defines equivalence without demanding historical dead-register scratch payload equality. No original/candidate comparison result is claimed by this specification correction.

### Stage4 explicit scripted-reset legacy provenance ruling — 2026-10-08

Status: APPROVED, scope ONLY the15 checkpoint records in validation/GAME-0002-independent-stage4-legacy-reset-checkpoints.json. Confidence(original parent non-consumption): CONFIRMED by original IDA session4de47d92: all1887 whole-body instructions scanned, no FNSTENV/FSTENV/FLDENV/FXSAVE/FXRSTOR/FSAVE/FRSTOR instructions;41 FNSTSW sites consume status, which remains EXACT. The parent does not read/dereference legacy FIP/FDP/FOP/FCS/FDS. Confidence(scripted fixture provenance non-liveness under the conditions below): HIGH. This ruling does not establish native opaque callbacks' treatment of such metadata or permit an arbitrary empty-tag waiver.

The named checkpoints are pair7:event1:site1C740:exit; pair7:event2:site1C76F:entry/exit; pair7:event3:site1C77D:entry/exit; pair35:event280:site1DA04:exit; pair39:event307:site1DA04:exit; pair41:event61:site1DA3B:exit; pair63:event275:site1DA04:entry/exit; pair71:event165:site1DA6C:exit; pair79:event403:site1DA04:exit; pair83:event135:site1DA04:exit; pair107:event81:site1DA6C:exit; pair111:event389:site1DA6C:exit. Each uses the fp544 checkpoint suffix in the cited raw inventory.

At these checkpoints ONLY, equivalence may exclude the five legacy provenance fields FIP/FDP/FOP/FCS/FDS where one capture has allfive reset0 while the other retains an identified earlier parent arithmetic provenance. This is an explicit stale/reset comparison, not decoded equal-operation comparison against an opcode0. Keep all raw bytes and report every excluded field/checkpoint/rawpair, the reset direction and retained parent's operation/operand role. There are15 checkpoint rulings and75 possible legacy field differences; actual differences/counts remain the validator's report. No other field or checkpoint obtains this exclusion automatically.

Required gates: CW027F and all exception masks remain set; fullSW matches EXACT including SF/ES/B and SF is0; TOP0/fulltagFFFF/abridgedtag0 in both; XMM/MXCSR match EXACT; no live or scalar raw80 difference is excluded. Callback results/state/order must match the declared scripted typed outputs. Scripted callback contracts produce results from explicit typed arguments/fixture outputs, not legacy instruction/data provenance or selector/opcode values. The original parent neither consumes these legacy fields nor enables an unmasked exception route; subsequent arithmetic loads new explicit operands and updates arithmetic provenance. Thus reset vs identified stale history does not influence the admitted parent's values, status-driven branches or scripted callback results. It may not be used to pardon capture/restoration corruption, altered architectural state, unmasked exception handling, stack faults, metadata-reading callbacks or natural original dependencies whose metadata consumption is UNKNOWN; those remain FAIL or BLOCKED.

Retained provenance lineage: pair7 starts from original1C62F A+196 decrement store (corresponding candidate71044A97 FSTP32); its reset propagates through two raw accessor callbacks until the next own distance arithmetic. Pool1DA04 randX checkpoints inherit preceding1DA97 poolZ store, including pair63's propagation from the preceding velocity getter;1DA3B randY inherits1DA35 poolX store;1DA6C randZ inherits1DA66 poolY store. Resolve original pool FDP as57F740/57F738/57F73C+76*i using the captured loop/event index. The independently audited candidate71044928/71044936 stack-spill provenance must map to that same preceding arithmetic result under GAME-0002-original-fp-provenance.json; an unidentified spill/selector/address is BLOCKED, never silently excused. The cited15-record export preserves both same-event entries and prior original exits to substantiate reset/propagation direction. This is not a runtime PASS claim.

### FNSTENV FOP memory-addressing canonicalization

Approved machine-readable original FIP/typed operand-role mapping: GAME-0002-original-fp-provenance.json. It covers the22 original FIP sites reported by the validator, including pool stores and their next callback checkpoints. Original-only IDA site reinspection confirms opcode class/width and memory operand locations; formulas and lifetime roles derive from this approved behavioral contract. Each captured FDP still requires resolution using actual actor/frame/stack/index evidence. Expected typed FDP mapping is not an unverified claim that an arbitrary observed raw address has been resolved.

Status: APPROVED, Confidence: HIGH for decoding the reported raw11-bit FOP values; validation of every checkpoint's operation/data provenance remains the validator's responsibility. Preserve raw FOP and raw FIP/FDP in evidence. For a memory-form x87 operation, FOP encodes both the operation and the ModRM address form. A replacement can use a different base/displacement addressing form for the same operand role. Compare the decoded escape opcode, operation, operand width, operand order and stack effect EXACT. Permit ignoring ONLY memory-form ModRM mod/rm address-encoding bits when both forms are memory (mod!=3), their /reg operation fields match, and recorded checkpoint/site/operand-role provenance proves correspondence. /reg, escape opcode, register-form operands, width, order and stack effect are never masked. This is decoded-operation comparison, not blanket FOP exclusion. Matching C bits alone cannot establish operation identity.

Reported FNSTENV little-endian FOP pairs decode as follows:

|Original raw bytes|Candidate raw bytes|Required identical decoded operation|
|---|---|---|
|9a01,9d01,1d01,1c01,9901|5d01|D9 /3: FSTP m32real, sourceST0, round/store32 then pop|
|9a01|5c01|D9 /3: FSTP m32real, sourceST0, round/store32 then pop|
|1d00|5d00|D8 /3: FCOMP m32real, compareST0 against memory32 then pop|
|1d04|5c04|DC /3: FCOMP m64real, compareST0 against memory64 then pop|

For example original final FIP0x41E2C0 denotes the approved final FCOMP m64real against constant3 (table above). Candidate FIP0x7103EE79 is only an observed address supplied by the validator, not original-code evidence: authorize its mapping only when the validator records the corresponding final comparison helper/site, operand float64 bits4008000000000000, stack effect and exact resulting architectural state. Other FIP/FDP differences require equivalent checkpoint-specific operation/data-role mappings to the approved comparison/store contract, not a broad address-range exemption. The report must inventory each differing raw FOP/FIP/FDP pair, count/checkpoints, decoded operation, matched operand role and justification. FXSAVE fields and FNSTENV fields retain their documented format meaning independently; duplicate capture fields that differ due to save-format semantics are not silently substituted for one another. Unresolved correspondence is BLOCKED; an actual operation/width/order/operand-role mismatch is FAIL_FLOAT.

Resolves Agent3 AUDIT-1..6 and Agent2 typed questions about input/alias list, compare form/reached predicates, local poison/lifetime, readiness/selection/table, final rawEAX/API and health prediction. Observer metadata contains only numeric addresses, instruction kinds/length/targets and typed behaviors; original machine bytes remain private. No runtime equivalence claim is added. Native original captures must record lowcooldown RNG before consumption, entryblock, consumptionblock and finalblock separately; pool sentinelbytes/RNG state and fullFP required. Validation remains BLOCKED until actual reference/candidate observations pass.

