# ANIM-0001 independent validation

Role: Agent 3. Read approved behavioral specification, replacement sources,
compiler/tests and root-authorized typed observations only. No original executable, assets,
process, UI, IDA, disassembly or original implementation material accessed.

## Offline result

PASS (HIGH confidence): independent 53-bit arithmetic/explicit float32-store
oracle versus optimized i686 Clang replacement for 88 typed cases. All 112 model
bytes, EAX, clock state, count/lookup count and selector observations compare
EXACT. Raw high-step source overlaps, eleven-step cap, nonbinary intervals,
negative elapsed/nonzero count and live callback mutations are represented.
This is specification conformance; the separate original comparison follows.

PASS (CONFIRMED observed fields): final original-process typed 40-case fixture
archive animation-final-fixture/animation-fixtures.jsonl. Both sides independently
match the oracle for full112 state, EAX, clock and callback order/index. Initial
FP environments match exactly; final full SW, CW, TOP, abridged tag and MXCSR
match exactly in all40 cases. The oracle's pre-clamp condition-code expectation
also matches both sides. Full result: independent-final-differential.json.

The historical prototype failed16 C3 equality cases with0 sticky differences.
The approved condition-status supplement corrected the earlier diagnostic-only
policy. Final candidate naturally performs balanced x87 comparisons in the
specified operand order; no arbitrary status patch is used. Prototype failure
remains archived in independent-prototype-differential.json.

PASS (CONFIRMED): all eight semantic-core SHA256 values match the supplied
prior-core manifest, recorded in ANIM-0001-independent-core-hashes.json.

## Source audit

HIGH confidence: source orders count, time/elapsed snapshot, selector/lookup,
cached rate, frame source/write, live anchor/write and final factor/EAX as
specified. Predictor uses rounded successive residual subtraction, max eleven
steps and binary64 interval precision. Native activation checks caller5E4D,
registered lower owner+644 model, lifetime witnesses, collection bounds/index,
positive rate, constants, CW027F, finite results and at most four steps.
Unsupported calls forward before callbacks/writes; prediction is enclosed by
FXSAVE/FXRSTOR preservation. Stdcall one-argument declaration exists; compiled
runtime ABI witness passed all40 final fixtures (stack/nonvolatile/CW).

## Pending / findings sent to implementer

Resolved: native original4/replacement4 observations and all8 CPU replays now
independently PASS. Native binary/JSON integrity is exact for all8 snapshots.
Final bounded result and explicit limits are in ANIM-0001.md. No whole-game
equivalence claim is warranted.

Resolved: native capture now provides pointer_words_unchanged before pointer
normalization, preventing silent concealment of pointer-field mutation.

Resolved with explicit scope: native CPU replay reports a clean-identical
FNINIT/CW027F/MXCSR1F80 policy, source captured CW/SW separately, and asserts
captured output112/EAX/clock. It does not claim to recreate captured SW. Reports
provide initial/final CW,SW,TOP,tag,MXCSR. The validator requires fullSW EXACT
under the approved supplement, diagnosing condition/sticky differences without
weakening any comparison. Natural source and replay FP witnesses remain
separate claims.

The implementation's offline fixture equality is vacuous when original is
absent. Independent oracle runner supplies actual offline semantic assertions.

Artifacts: independent-oracle.py, independent-vectors.json,
independent-expected.json, independent-runner.cpp, independent-offline.json.
