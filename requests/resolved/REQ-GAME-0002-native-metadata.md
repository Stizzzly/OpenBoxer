# REQ-GAME-0002 — Complete approved execution metadata

Related specification: GAME-0002-opponent-strike-consumption.md

Status: RESOLVED — approved execution metadata supplied, 2026-10-08.
Blocking: NO for the three metadata questions in this request. Differential/native validation remains a separate requirement.

The original request was opened while only preliminary typed scaffolding was authorized. Subsequent approved behavioral supplements and READY metadata supersede that historical restriction. This resolution records metadata completeness, not validation PASS or deployment authorization.

Required answers:

1. Which exact raw float32 values does each mode1 AIindex equality match write
   to actor+220? The current contract mentions corresponding values but does
   not enumerate them.
2. Supply approved callback-site/return identifiers, callback ABI metadata and
   patchable observer boundaries, including repeated loop sites and imported
   audio invocation. No addresses or boundaries will be inferred.
3. Supply the exact reached x87 comparison load/compare-pop relationship and
   conditional direction needed to reproduce raw status flags, with approved
   scalar callback FP/EAX capture and exit-state requirements.

Evidence needed: approved behavioral/ABI supplements and typed metadata;
no original implementation or disassembly may be supplied to Agent2.

Resolution and current status:

1. specs/gameplay/GAME-0002-typed-fp-locals.md, Typed inputs: mode1 AIindex1..7 independently writes int32 A+220 values1..7 respectively; mode0 writes3 and uses difficulty2. The earlier raw-float wording was mistaken; no float representation is written there.
2. specs/gameplay/GAME-0002-observer-metadata.json supplies callback call/return RVAs, exact ABI/target/observer metadata, repeated loop sites and OpenAL32 imported alGetSourcei IAT provenance. The semantic specification's Delayed consumption section and metadata contain the corrected block/no-block sound sites.
3. specs/gameplay/GAME-0002-typed-fp-locals.md, Ordered FP comparisons and Authoritative FP capture and equivalence policy, supplies reached load/compare-pop directions, full status/live raw80/XMM policies, exact returnEAX and scalar/callback capture requirements. GAME-0002-original-fp-provenance.json supplies typed original site/operand mappings. Stage4 and Stage5 scripted-reset rulings are restricted to their explicit checkpoint inventories.

Root-owned original/candidate whole-function typed executions and independent comparison are now underway; validation/GAME-0002-independent-stage5-legacy-reset-checkpoints.json and GAME-0002-independent-stage5-unresolved-provenance.json record the current evidence/provenance follow-up. The old statement that only an unintegrated interface exists with no effects/guards/hooks is obsolete. This RE resolution does not independently inspect replacement source or assert final differential/natural PASS. Preserve the existing validated baseline until all required current validation is complete.
