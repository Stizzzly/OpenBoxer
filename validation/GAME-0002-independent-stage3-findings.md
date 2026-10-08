# GAME-0002 v3 independent findings

Result: FAIL_FLOAT_CAPTURE; final equivalence BLOCKED. Confidence CONFIRMED for typed-record comparison, HIGH for source-level capture diagnosis.

Sixty root-owned v3 scripted pairs pass 1,068,423 exact state/data/callback/return/live-x87 checks with zero differences. Corrected audio site mapping matches. Exhaustive raw-FP inventory adds 1,208,678 architectural/tag/coverage checks and finds eighteen XMM disagreements, all at manually captured whole entry/exit (nine each). All callback architectural XMM captures match and no dead raw80 disagreement remains. Thus the earlier first-callback XMM mismatch is fixed, while manual whole captures do not yet establish exact XMM state.

Manual begin/end capture reads snapshot data before saving the recorded FP environment; snapshot machinery can alter caller-clobbered XMM. The actual invocation seed is restored immediately before assembly call, so a C++ manual record taken elsewhere is insufficient evidence of actual whole-boundary XMM. Corrective entry/exit assembly captures before C++ work are required, followed by a frozen rerun. No XMM masking is allowed.

Twenty-two reported guard probes pass accepted/expected classification, read-only snapshot, exact saved FP and zero callbacks; safe wrongcaller dispatch reports replacement0/fallback1 and reference-equal state/sites/EAX with ABI. Cleanup reports observer_restored1/failures0. Those checks remain historical v3 witnesses; exhaustive actual fallback raw capture comparison follows corrected capture.

Raw FP inventory retains 41,693 unresolved legacy provenance differences and 1,006 documented reserved/padding differences. New Agent1 metadata approves original operation/operand roles for twenty-two sites. Candidate helper sites are independently located in the frozen replacement DLL, but raw pointer/provenance resolution and capture fidelity must complete before exclusions receive PASS.

Root reports Agent2's new strict save/restore selftest exposes legacy FNSTENV provenance fields not restored by FXRSTOR alone. Until corrected restoration and verification, previous original natural sources remain useful typed historical semantic observations but do not prove legacy FP capture/restoration fidelity. New original-only bounded native observations and clean source replay are required for a final exact native FP claim. Preserve old records/reports unchanged; no retrospective normalization or silent reclassification.
