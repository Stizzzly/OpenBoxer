# REQ-0003 — Animation inputs outside initial supported scope

Related: ANIM-0002, ANIM-0003, ANIM-0001.
Blocking: NO for finite guarded lower-model units; YES before expanding scope.
Status: OPEN.

Question: What observable behavior is required for stored animation bound/vector count mismatch, malformed/nonterminated requested or record names, callback-driven unbounded count changes, future-preparation zero divisor/INT32_MIN divide by-1, upper-model/other-owner natural lifetime admission, or exceptional advancement floating inputs?

Evidence needed: separate bounded original-only observations with appropriate data/fault witnesses, exact target SHA25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, validated callroute/lifetime and environment. Preserve installed original and use disposable testcopy only. Divide trap is statically CONFIRMED as an architectural consequence; delivered crash/handler behavior remains UNKNOWN. Original comparison preserves locale-dependent CRT behavior; host case-folding is not approved.

Current decision: fallback before effects for unsupported native inputs; never invent repair/clamp/range semantics. Initial supported behavior is fully specified, including nontrapping signed arithmetic synthetic cases, preserved original dependencies, narrow lower-owner natural guards and composed ANIM-0001 admission. Original-process differential and natural observations are still required validation work, not missing semantics to guess.
