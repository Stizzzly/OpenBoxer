# GAME-0003 — Inactive combo timer reset admission supplement

Status: APPROVED bounded behavioral correction, exercised by finalv3 independently reported PASS fixture/replay/native evidence (see GAME-0003 main specification evidence update and stage3 reports). Confidence(boundary, exact reset condition/order/writes and FP): CONFIRMED by original-only IDA c161b831, analysisready, program.exe base400000/private database,2026-10-08. Original target SHA25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6. Runtime comparison attribution remains validator reports; this RE supplement alone grants no runtime PASS. Final ordinary composed delivery independently reports PASS86checks/zeroerrors in validation/GAME-0003-independent-composed-delivery.json. No replacement source inspected.

Authority: GAME-0003-opponent-single-attack-initiation.md and inherited GAME-0002 behavioral/typed-FP contracts. Whole thunk13A7/body1C4E0..1E320 unchanged. All other admission conditions and fallback-before-effects rules remain. No midbody oracle or continuation admission.

## Narrow admission change

Raise ONLY GAME-0003 inactive combo timer576260 maximum from float32bits3F19999A(0.6) to float32bits3FCCCCCD(1.6), inclusive. Keep finite normal-or-signed-zero, magnitude<=10000, existing lower bound unchanged, combo-active byte57626C=0 and countint32/raw576270=0. Thus previous negative/small values retain their existing admission; added positive interval is (float32(0.6),float32(1.6)]. Do not admit active combo, nonzero count, nonfinite/subnormal input, or timer>1.6. Timer is independent from AI attack timer576230; do not mix these fields. Entry raw576274/576278/57627C/576288 arbitrary sentinel bytes.

No governing callback or earlier admitted parent arm changes combo-active/count/timer on this first-entry route: pendingAI0 skips consumption's1.6 write, victory0/deathexcluded, and original callbacks keep governing globals stable. Consequently live timer at reset is entry timer. In scripted fixtures only approved typed numeric callback changes remain allowed; a script modifying combo-active/count/timer outside this contract must be rejected before effects, never recovered by post-effect fallback.

## Exact reachable tail reset

After existing fatigue/death/victory tests and before cursor/direction callback1E037:

1. Read combo-active57626C. Because admitted0, skip the timer<1 comparison and increment arm entirely. No FCOMP1DF3B or FSTP1DF5A is reached.
2. Unconditionally load float32 live timer576260 and compare against float32 constantbits3F19999A atVA562448, original FCOMP32/pop1 RVA1DF66, followed status capture. Strict timer>0.6 takes reset; equality, less and negative values preserve all reset-owned data.
3. On reset write byte57626C=0 first (RVA1DF77), then raw32 timer576260=00000000 (RVA1DF7E). This timer reset is an integer/raw zero write, NOT an x87 FSTP; it introduces no FP arithmetic, store rounding or sticky flags.
4. Read live count576270; compare==2, then==3, then signed>=4, in that order. Admitted count0 fails each. No576274 write and no combo sound callback40179E occurs, even when sound577F9E is nonzero. Existing unreachable combo audio sites1DFAB/1DFD6/1E001 are not added to observer/replacement admission.
5. Compare count signed>1. Count0 fails. Preserve byte576278 and raw32/global57627C exactly.
6. Write raw32 count576270=0 (RVA1E020), then raw32 auxiliary576288=15/bits0000000F (RVA1E02A). These writes occur even though input count already0; preserve observable write order. All other globals, actor/body/table/pool/RNG/sentinels unchanged by reset.
7. Continue complete original-specified cursor/physics/audio/speedlimit tail. Initiation state3..8 and committedtype/AIattack/playerpending remain intact. No attack continuation, hit damage, pool creation, new callback, or extra RNG introduced by this reset.

## FP, ABI and provenance

The only reached FP operation in reset decision is inherited original1DF66 FCOMP32: LHS timer loaded from576260, RHS constant562448bits3F19999A; D8/reg3,32-bit RHS, pop exactlyone. With admitted finite values>threshold C0=C2=C3=C1=0; equality setsC3;lesssetsC0. Preserve fullSW including sticky exceptionflags, TOP/tags/CW/MXCSR/XMM/rawlive80. Integer count comparisons/rawzero writes do not reset FP history. Timer value1.6 may retain this comparison's FIP/FDP/FOP through subsequent cursor/getter callbacks; use existing exact original provenance entry1DF66 in GAME-0003-original-fp-provenance.json (unchanged31prefix), no new operation site or blanket stale/reset exemption.

Full wholeEAX/register/stack/callback/FP contract remains. Candidate helper/spill/capture correspondence independently audited; no epsilon, empty-stack-only waiver or callback legacy metadata waiver. No own allocation/IO/ColDet/GL effects.

## Required validation

Add exact whole original/candidate cases timer0, thresholdbits3F19999A, immediate representable value above threshold3F19999B,1.0bits3F800000 and1.6bits3FCCCCCD; sound0/1; initiationZ/X/C and no-start. Assert reset own writes26C/260/270/288 final values and prescribed ordering, unchanged274/278/27C sentinel bytes, absence of combo sound/RNG/pool effects, unchanged attack outputs and complete downstream trace/ABI/FP. Rejection timer nextabove1.6bits3FCCCCCE, nonfinite/subnormal, activecombo1/countnonzero must wholeoriginalonce before candidateeffects. Old fixtures remain valid.

Root-owned natural captures with entryinactive/count0/timer1.6 may now be candidates ONLY when all other exact GAME-0003 guards pass. Confirm actual reset outputs from wholeexit, fullFP and callbacktrace rather than treating a relaxed guard as PASS. Correlate actual opponentj1 selector launch under approved animation metadata; preserve twelve prior cores. Missing comparable observation or provenance stays BLOCKED. This supplement approves behavior/admission, not an unvalidated delivery.
