# GAME-0001 independent validation plan

Status: PENDING APPROVED SPECIFICATION. No behavioral oracle or hit threshold is assumed.

Baseline: eleven semantic-core SHA-256 values independently match the coordinator baseline; recorded in GAME-0001-independent-baseline.json. Prior ANIM-0004 report supplies historical bounded route-validation context, not new hit behavior evidence.

## Admission and boundary

- Use only the approved whole-function interface and admitted path. Test each supported precondition and each rejected state independently; rejection must precede replacement effects.
- Document exact typed inputs, object identity/lifetime, readable/writable extent, callback availability, calling convention, return representation and preservation requirements from the approved contract.
- Record unsupported cases as fallback/admission tests, not implemented original behavior.

## Hit and miss edges

- Once the approved comparison operand, threshold and strictness exist, cover exact equality and the immediate representable neighbors on both sides.
- Cover independent factors with one changed input at a time and interactions where ordered short-circuit evaluation is observable.
- Test signed zero, finite extrema, NaN/infinity or exceptional operands only where admitted and specified. Do not assign compatibility behavior to unspecified values.
- Distinguish pending registration state from subsequent damage response. Compare only specified outputs and downstream dependencies within the admitted whole-function boundary.

## Opaque callbacks

- Compare exact callback identity, argument representation, order and count, including early-return cases.
- Use mutation witnesses where the contract permits callbacks to modify state consumed later. Compare state before/after each callback and final object/global bytes, including required unchanged bytes.
- Exercise callback return values as independent inputs only when the contract identifies them as relevant.
- Preserve full callback provenance and avoid deriving an oracle from replacement outputs.

## Return, ABI and floating point

- Compare specified EAX/return behavior and registers, stack balance, object offsets/alignment and pointer widths under the approved ABI contract.
- Record incoming and final x87 CW/SW/TOP/tag and MXCSR without silently masking differences. Separate callback FP mutations from replacement arithmetic.
- Choose EXACT raw bits for copies and explicitly approved policies for computed values. Threshold decisions need exact approved branch behavior; epsilon must not convert a miss to a hit.
- Include clean incoming FP state and specified sticky/control-state variants, with full environment preservation or changes checked against the contract.

## Evidence and delivery

- Run the same typed fixtures on original and replacement with coordinator-provided original-process provenance, guarded module hashes and capture schema.
- Independently verify fixture integrity, full state/return/callback/FP outcomes and any native-capture/replay correlation.
- Natural supported hit and miss observations require positive replacement activation, registered ownership, verified caller/route and actual state transitions. Counters alone do not establish equivalence.
- Rehash all eleven preserved cores after integration; freeze replacement artifacts used for comparisons.
- PASS is bounded to supported observed/specifed behavior. Missing approved contract or original observations remains BLOCKED.
