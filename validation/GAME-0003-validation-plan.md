# GAME-0003 independent validation plan

Status: BLOCKED pending approved behavioral specification and original typed observations. Confidence of GAME-0003 behavior: UNKNOWN. This document defines gates, not attack behavior or admission rules.

## Role and preserved baseline

Agent3 reads approved specifications, replacement source and written/typed reports only. No original executable, IDA, disassembly, decompiler, raw implementation dumps or proprietary assets. Root owns native processes and UI. Agent3 does not change replacement behavior or publish.

Preserve GAME-0002 final bounded v6 PASS, thirteen completed semantic cores, ANIM-0004 activation and diagnostics-off delivery. Reference `GAME-0002-independent-final-validation-stage6.md` and its summary, prior-core-v6 hash check, and delivery-v6 runtime report. Recorded delivered DLL SHA-256 is B10C2E1D5CF4E422D8A83D2F7470D0E4463200CE4D8BDD09CD7BE4AA1B32052F. Historical failures remain evidence; their specific rulings do not automatically apply to GAME-0003. A local replacement-source baseline manifest accompanies this plan. Existing GAME-0002 artifacts remain unchanged.

Preservation means twelve earlier cores retain exact hashes and the GAME-0002 admitted domain retains exact differential regressions. GAME-0003 may extend the existing damage whole-function core after approval; the entire extended source is not required to remain byte-identical. Its pre-extension hash is recorded for traceability.

## Required approval and capture metadata

1. Stable approved specification ID/revision/hash, whole-function or legitimate predicate boundary, input/output types, admitted states, unknowns, callback and side-effect ordering, ABI/layout contract and per-value FP policy. Choice labels Z/X/C or animation semantics require evidence before use as expected behavior.
2. Every capture: schema/revision, unique run/pair/scenario/checkpoint IDs, capture hash, process ownership, executable/module SHA-256, actual loaded bases and sizes, route/caller identity, root-written provenance, replacement build/compiler/options/source manifest, observer/fixture versions and hashes. Never reuse another run's ASLR base.
3. Typed entry/exit state and all callback pre/post checkpoints: declared field offset/type/extent, raw bits and decoding, relevant actor/target identity and relationships, global state, array bounds, returned values, writes, callback arguments/results and strict event sequence. Include RNG input/output/call count if specified; do not infer random distribution from a few samples.
4. ABI witnesses: stack/cleanup, calling convention, live arguments/results, preserved registers, relevant flags, x87 control/status/TOP/tags and physical/logical scalar80 registers, MXCSR and XMM0..7; callback chaining must retain native state before/after actual calls. Observe before C++ diagnostic code can mutate FP state.
5. Each FP/provenance difference: exact source and replacement locator, offset/role, owner module/hash/base, operands/result/store lineage where required, confidence and explicitly approved narrow policy. Inventory dead/raw/reserved payloads too. No empty-stack, opaque-callback or compiler-wide waiver.

## Differential and guard gates

After approval, independently derive a coverage table from the approved branches and boundaries; include each supported selection/outcome and rejection case without inventing missing thresholds or selection rules. Freeze typed inputs and callback scripts. Equivalent deterministic inputs must reach original and replacement with the same opaque callback responses; compare state, return, callback arguments/order/results, side effects, ABI and FP under approved policies. Own-source tests/build success alone do not establish equivalence.

Audit admission before any replacement effect. Root supplies guarded disposable probes for wrong executable/module/hash/route/caller, unsupported native state, invalid declared pointers/extent and missing required dependencies where the approved contract permits probing. Require original fallback with zero replacement state writes/callbacks before fallback, preserved ABI/FP and exactly one original invocation. Verify probes actually reached the relevant decision; counters alone do not prove this.

Validate module-relative external resolution and actual live callback target changes where applicable, including nonpreferred loaded bases. Prior GAME-0002 absolute-IAT failure makes this a concrete regression concern, not evidence that GAME-0003 has the same design.

## Native composition and delivery gates

Root captures short owned natural test-copy observations covering every supported newly activated route and at least one observed unsupported fallback, with previous replacements active. Separate natural input-selection evidence from synthetic callback-isolated differential proof. Preserve GAME-0001 hit recognition, GAME-0002 consumption and animation route behavior at the specified boundary; do not assert whole-battle equivalence from isolated pairs.

Root then verifies the frozen delivered binary with diagnostics/capture off: actual DLL hash/base and own-symbol identity, route/enable flags, previous replacement activation, no new runtime capture files, rendered ordinary encounter and user-visible continuity. Native execution proof requires actual route witnesses; no fabricated counter when the sink is optional. Installed original integrity and owned-process cleanup are root-reported facts.

## Reporting

PASS requires all required gates and per-run provenance resolved. FAIL includes return/state/side-effect/callback-order/FP/ABI/fallback category and smallest reproducible witness. Missing approval, observations, branch coverage or difference mapping is BLOCKED. Report scope and limitations; do not promote a bounded pass to whole-game equivalence.
