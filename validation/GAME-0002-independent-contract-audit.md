# GAME-0002 contract audit before READY

Status: BLOCKED for execution/equivalence. Approved bounded spec read 2026-10-07; no original implementation accessed. Confidence: HIGH in explicit behavioral ordering; UNKNOWN in incomplete fixture metadata and executing natural FP. Twelve preserved core hashes recorded independently. This is a contract audit, not a compatibility PASS.

## Sufficient independent assertions

- Half-duration equality waits; only strict greater attack timer consumes. Pending=0 is admitted only for far miss distance>=6.
- Damage uses entry/live block before the later block activation/expiry sequence. Expired entry block mitigates this call. Entry-unblocked consumption can end blocked/state11. Impulse always occurs on consumption.
- Low cooldown is supported and invokes exactly one RNG callback before consumption, selected modulo by effective difficulty. Unblocked consumption adds 300 pool RNG calls and 100 velocity getters; blocked consumption adds neither pool population nor pool RNG events.
- Consumption sets combo timer1.6 then mandatory tail reset clears it in the same call, writes combo count0 and auxiliary288=15. Waiting/miss timer<=0.6 avoids that reset.
- Whole physics/audio tail executes for every admitted wait/hit/miss; speed and XZ limit branches are independently observable. No predicate elimination is allowed merely because attacks are excluded.

## Questions required for executable typed vectors

AUDIT-1 (blocking schema): provide exhaustive scalar input/global/table lists, raw offsets/extents and alias/pointer roles, including parent/cursor vectors and all A scalar fields read. 'Initialized finite own scalars' is not an enumerated guard contract. Specify legal callback-mutated regions and validation envelope after each mutation; fixture flexibility must not silently violate native admission.

AUDIT-2 (blocking FP): each conditional must have a stable comparison event/site ID, operand width/order, compare form, reached branch and resulting FP state. 'Load/compare-pop' does not completely define whether status uses FCOM/FCOMP/FUCOM family or exact intermediate precision; finite values still require exact SW/TOP and incidental EAX. Specify ordered AI Z/X/C predicate operands/conditions, including type216, currently listed without the complete logical relationship.

AUDIT-3 (blocking tail locals): define distinct local identities, lifetime/reset points, initial poison application and whether temporary words retain prior callback writes. Need the tail alGetSourcei localInt identity plus raw FP/cdecl scalar conventions and EAX metadata at every site. Otherwise independent observer could accept incorrect alias/local reuse.

AUDIT-4 (blocking readiness/AI selection): give the seven mode1 A+220 values and input tables' exact named addresses/record field layout. State whether readiness1 with A+196>=1 is retained and whether A+216 types are bounded; these must be explicit typed fixture constraints.

AUDIT-5 (blocking return): provide exact final sqrt compare FP status and EAX provenance for <=3, including callback EAX upper-half and status capture timing before/after stack checker; callback observer metadata must include true native imported alGetSourcei identity. Avoid treating import-name bytes as resolved pointer.

AUDIT-6 (blocking guards): define health prediction rounding representation (extended intermediate versus final float32 loss) and entry body/world witness exact links/extents. Upper-bound inputs near10000 can produce larger intermediate outputs; clarify whether guarding checks predicted outputs or only input bounds. Verify no unbounded post-callback outputs are admitted under fixture mutation.

Natural block before and after must be independently retained in source records; final block alone cannot prove mitigation scale. Natural low-cooldown source record is needed to prove original decision RNG event preceding consumption/pool RNG. Full READY remains coordinator authority.

## Prepared independent vector matrix

All raw encodings and expected numeric results will be generated only after metadata settles; the following expected relations are already supported by the approved spec.

| Family | Variants | Required distinction |
|---|---|---|
| Timing | below/equal/above halfduration | pending retained versus cleared, impulse only on strict above |
| Block sequencing | entryblock0/1 x marker0/1 x readiness0/1 | damage scale from pre-tail live block, final state/block separate |
| Expiry | timer immediately below/equal/above raw0x3EB33333 | equal increments, above clears after consumption |
| Decision RNG | updated cooldown below/equal/above1 x modes/difficulty | strict below only; effective mode0 difficulty2; count/order/final RNG |
| Pool | unblocked/blocked/wait/miss with sentinel7600 bytes | only unblocked consume writes +0/+40/+44/+48;100/300 callback counts |
| Counter | same/different previous type x counter0/FFFFFFFF | modulo wrap; gated no-block only |
| Audio | sound0/1 x AIindex5/other x walk flag/state | consumption audio and unconditional query/tail play-stop sequence |
| Movement | decision combinations and no flags | prioritized choice plus independent timer writes/default poison route |
| Tail | speed1.5 and XZ3 each below/equal/above | correct add/multiply and scalar-first limiting callbacks |
| Fatigue/FP | zero/positive and approved generated subnormal cases | exact store widths and unmasked sticky flags |
| Negative admission | every unsupported input independently | unchanged state/FP read-only admission; separate dispatch fallback exactly once |

Compare source-original to original replay and candidate replay independently; exact state, ordered callbacks, full EAX/FP/ABI plus sentinels. No test runner or behavioral oracle will execute before full READY. No replacement edits or delivery/publication changes.
