# ANIM-0001 — Time-driven current-frame and blend-factor advancement

Status: VALIDATED_BOUNDED. See validation/ANIM-0001.md and implementation-freeze.json for exact scope and artifacts.
Confidence: CONFIRMED interface, ordered reads/writes, arithmetic, callbacks and caller boundary. Semantic names beyond the documented rendered frame/factor relation remain UNKNOWN.

Original program.exe SHA256 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, x86 preferred base0x400000, image size0x19F000. Independently verified survey in session43af431c (a session identifier, not an executable hash).

## Scope / hook / ABI

Whole bounded body VA0x406810, thunk VA0x4015AA (RVAs0x6810/0x15AA). The complete thunk is a five-byte direct tail transfer; approved generic metadata-only install/restore uses synthesized jumps and root-preverified target/whole executable identity. No original instruction bytes go to implementer.

ABI: stdcall one stack argument model M, callee ret4. ECX is not a semantic input, despite a debug-local save. Preserve nonvolatile registers. Return full EAX uint32, defined below. This unit writes current frame, elapsed-time anchor and interpolation factor; it does not choose an animation record, generate future frame slots, load assets or draw.

Sole statically verified caller of thunk15AA: VA0x405E48, return RVA0x5E4D, in original VA0x405D40 (thunk1D66). That parent first obtains animation record and prepares model+52/+60/+64/+68/+72 with original signed remainder/wrap behavior, then invokes this unit. Parent remains original at this stage. Original draw wrapper405EB0 calls1D66 first for owner+644, then owner+532, before recursive drawing. Animation selection406E40, animation-file loader405440, original tag/matrix code and all eight earlier replacement cores remain unchanged. The earlier classification of405440 as selection is superseded by ANIM-0002's confirmed loader/selection distinction.

## Layout and dependencies

All offsets decimal; model object112 bytes established in RENDER-0006.

| Input/output | Type / access |
|---|---|
| M+44 | signed animation collection index, read once before lookup |
| M+48 | current rendered frame int32; conditional output |
| M+52 | next rendered frame; not read or written by this unit |
| M+56+4*n | raw uint32 source selected during advancement; n ranges1..11 |
| M+76 | float32 rendered interpolation factor; output; also overlaps source for n=5 |
| M+80 | float32 time anchor; input / conditional output; overlaps source for n=6 |
| M+84 | embedded animation collection passed as ECX |
| Global VA0x57D904 / RVA0x17D904 | float32 current time bits, sampled once after count callback |
| Animation record+268 | signed int32 rate parameter, read once after lookup |

Animation records have272-byte stride. Count callback thunkRVA0x14D8/body0x9730: thiscall ECX=M+84, noargs, EAX signed count. It returns0 for null collection begin; otherwise signed byte difference(end-begin)/272. Begin is M+88, end M+92, capacity end M+96. Lookup thunkRVA0x10B4/body0x97A0: thiscall ECX=M+84, stack signed index, ret4, EAX record pointer. Original implementation returns collection begin+272*index without bounds checking. Both original dependencies are read-only integer accessors and have no GL/file/allocator effects. Preserve them in natural runs; typed recorder stubs permitted in isolated disposable processes.

Constants are float32 values1000.0 (bits0x447A0000, RVA0x162114),1.0(bits0x3F800000,RVA0x162024),+0.0(bits0,RVA0x162040). They are preserved module data, not implementation bytes; original constants must be unchanged in supported native mode.

## Ordered behavior

1. Invoke count callback once. Zero returns EAX0 immediately with no writes by this unit and no time sample or lookup. Nonzero (including negative) proceeds; original does not enforce a valid positive collection.
2. Copy current-time global bits once. Subtract the then-live float32 anchor M+80 and store the elapsed result as float32. This elapsed snapshot precedes the lookup callback. Then read signed M+44 and call lookup. Read returned record+268 once and cache the signed rate.
3. Obtain an interval as1000.0 divided by the signed rate converted exactly to x87 numeric form. Divide cached float32 elapsed by this interval and store the initial factor as float32. Interval is regenerated for each subsequent use, not stored as float32.
4. Compare interval to cached elapsed. For finite supported inputs, if interval exceeds elapsed, skip advancement and leave M+48/M+80 unchanged. Store the initial factor to M+76 and return its raw32-bit representation. Negative elapsed in this branch produces a negative factor: no lower clamp occurs here.
5. Otherwise initialize a float32 residual from elapsed and an integer step count0. Subtract a freshly computed interval from the residual and store back as float32; increment count. Continue only while interval<=the newly stored residual and count<=10. Thus at least one subtraction occurs and at most11, including an11th step when the condition after step10 still holds. Do not replace these rounded successive subtractions with floor(elapsed/interval).
6. Copy the raw32 bits at then-current M+56+4*step count to M+48. The source is not validated or interpreted here. For counts1..4 these are M+60/64/68/72 prepared by the parent. Counts5..11 access M+76/80/84/88/92/96/100, crossing fields: this is observed behavior, not a safe extended future-frame array. No invented semantic names or repaired indexing are authorized.
7. Compute freshly regenerated interval * integer step count, add the **current** M+80, and store float32 at M+80. Lookup callback mutations of M+80 therefore affect this update, although elapsed was already copied before that callback. No callback occurs between frame copy and anchor update.
8. Subtract step count multiplied by1.0 from the previously float32-stored initial factor, then store the new factor as float32. If that stored finite value is negative, replace it with literal+0.0 bits. No upper clamp occurs, so a long delay may leave factor>1 after the11-step cap.
9. Copy final factor raw32 bits to M+76 and EAX. The current frame copy occurs before time-anchor write, and factor write is last. This matters for overlapping high-step sources. Global time is not reread after lookup.

The function performs no allocation/free, filesystem operation, rendering or global writes. Its own writes are exactly conditional M+48, conditional M+80, and M+76 when the count gate is nonzero. A recorder dependency can additionally mutate its approved input state; compare those dependency effects separately.

## Floating compatibility

Initial supported environment CW0x027F, round-to-nearest,53-bit x87 arithmetic, finite input global/anchor, strictly positive integer rate, finite intermediates/results. Elapsed is rounded once to float32 before lookup. Initial factor, residual on every subtraction, updated anchor and corrected factor each have explicit float32 stores. Interval arithmetic, comparisons, integer multiplication and addition otherwise retain53-bit intermediate precision. Conversions of int32 rate/count are exact. No FMA/reassociation, no SSE-only replacement assuming interval float32, and no floor simplification. A semantic x87 arithmetic adapter is permitted without copied original instructions. Compare state float32 and EAX bits EXACT; preserve signed zero unless the conditional clamp explicitly supplies+0. Callback order and live input observations are exact as well.

Rate0/negative, nonfinite values, other control words, asynchronous mutation and floating exception delivery are outside the first supported scope and go to original before callbacks/writes. Original has no explicit rate validation. Do not infer crash or clamp behavior for excluded exceptional inputs. x87 exception-status bits are diagnostic witnesses; first validation must report any status mismatch separately from state/callback/EAX results rather than silently loosening output comparisons.

## x87 condition-status supplement — 2026-10-05

Confidence: CONFIRMED comparison operand order and final condition-code producer; HIGH architectural status relationship for the bounded finite scope. Independently inspected original body in IDA session19d211af, same SHA25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6. Evidence addresses below identify operations, not copied implementation text.

This supplement makes condition status an EXACT comparison requirement for supported differential/replay cases. It does not loosen state/EAX comparison. Define condition mask0x4700 (C0=0x0100, C1=0x0200, C2=0x0400, C3=0x4000) and TOP mask0x3800. Require identical CW, initial stack occupancy with at least one usable temporary slot, and normal ABI completion. Original accessors preserve the floating environment; recorder callbacks must either preserve it or have their effects compared explicitly.

| Path / semantic final comparison | Return condition bits (SW & 0x4700) |
|---|---|
| Count=0 | Unchanged from the count callback's return; with original accessor, identical to entry |
| Nonzero count, no advancement: regenerated interval compared **to elapsed**, interval as left operand; interval>elapsed | 0x0000 |
| Advancement: corrected factor after its float32 store compared **to +0**, factor as left operand; positive | 0x0000 |
| Advancement: stored corrected factor is +0 or -0 | 0x4000 |
| Advancement: stored corrected factor is negative, then replaced by integer-written +0 bits | 0x0100 |

The comparison uses the pre-clamp stored corrected factor, not the returned post-clamp factor. Consequently final factor bits0 alone do not imply C3: a negative value clamped to +0 retains C0 instead. Equality treats either signed zero as equal; a -0 result is not lower-clamped. Finite comparisons clear C1 and C2. The interval comparison before the advancement decision is at VA0x40688F; loop continuation compares regenerated interval (left) to freshly stored residual (right) at VA0x4068CB. Each yields0x0000 for greater,0x4000 for equal,0x0100 for less, and permits continuation for less/equal. The loop count test does not alter x87 conditions. Advancement later overwrites those interval-comparison conditions with the final factor comparison at VA0x40691B.

After that final comparison, the conditional clamp and output writes use integer operations. No later floating operation overwrites the conditions. The normal stack-check return at VA0x4953E2 is also free of floating operations; abnormal ABI failure/reporting is outside this scope. On noadvance, the interval comparison remains the last floating operation; count0 executes none in the body. ThunkVA0x4015AA has no floating effects.

CW is unchanged (0x027F in supported scope). Every temporary floating load has a balancing pop, so return TOP equals entry TOP; with the replay's empty stack this is0. Existing sticky exception bits are not cleared. Arithmetic/conversion/store operations can add sticky flags (e.g. precision/inexact and, where applicable, denormal/underflow); evaluate these by the already specified ordered arithmetic, not from final factor alone. Finite comparisons add no invalid-comparison flag. Compare full observed SW EXACT for matching initial environments, while diagnosing sticky flags separately from condition bits and TOP. This is a correction of the earlier diagnostic-only status policy for these bounded cases, not a new promise for exceptional inputs or arbitrary initial floating stacks.

## Native activation and replay

Native gate: caller returnRVA0x5E4D and successful live RENDER-0006 manifest model registration (initially owner+644 only); unchanged model lifetime/source witnesses; animation begin/end form a valid272-byte collection; index inrange; record+268 positive; module constants unchanged; finite global/anchor; CW0x027F; exact-semantic predicted step count<=4; selected future-slot frame in[0,748]. Inspect read-only collection data to make this guard **before** callbacks and writes. Original accessors have been confirmed pure, so this inspection is not an invented callback substitute: supported body must still invoke the approved callbacks in order. Guard calculations must not alter the caller floating environment/status; save/restore diagnostic FP environment around prediction or use an approved semantics helper which restores it. Unsupported inputs call original body before observable effects. Negative elapsed with zero predictedsteps is allowed if outputs remain finite. Upper/empty unregistered models initially stay original.

Runtime observer entry captures coherent model112-byte typed state, global time rawbits, collection pointer roles/end/count, index, selected272-byte data record and rate, CW/status; postreturn captures model112 bytes, full EAX, time witness and ordered callback arguments/returns. Collection/model pointers are normalized roles, not source numerical addresses. Root may capture selectedrecord as data only; no implementation bytes. Capture an advancing pose, subinterval/noadvance pose and a natural repeat checkpoint; replay against original6810 and candidate with typed accessor recorders on copied state, no GPU. Natural forwarded original dependencies remain intact and diagnostics defaultoff. Root uses the unchanged F1D6 baseline and lab copy.

Isolated cases: count0 including callback mutation of time/model; positive and negative mocked nonzero count; elapsed0/negative/subinterval/exactthreshold/above threshold; rates such as10/30/60 with nonbinary interval; one throughfoursteps, ten/eleven/cap-exceeding delays; callback mutates selector before lookup, rate record, anchor/globaltime/future slots; zero result/signedzero; overlapping source counts5..11 using full112-byte fixture state. Only finite positive-rate CW027F inputs are approved for candidate arithmetic. High-step synthetic cases must reproduce field overlap, even though natural high-step calls fallback. Compare all112 model bytes, callback order/rawbits, EAX and unchanged globals.

## Evidence / unknowns

CONFIRMED: body406810 full instruction span through ret4; count409730 and lookup4097A0; sole caller405E48 in parent405D40; parent callers405F08/405F1A; constant rawbits; original hash survey. UNKNOWN: time-source generation/wrap semantics, game meaning of overlapping fields when step>4, exceptional/nonfinite/rate-invalid behavior and source provenance. Those are outside this first unit. No replacement implementation was written by Agent1.

