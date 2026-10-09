# GAME-0004 implementation candidate v1

The existing whole opponent update now has a separately admitted continuation
domain. It executes the live Z/X/C timer ladder, completes to idle only on a
later entry past the matching threshold, adds the fixed table record 1 +12
field to fatigue, then decays fatigue and applies the specified lock clamp.
The meaning of the fixed field remains UNKNOWN. Reciprocal player damage
remains in the original separate player update.

`damage.cpp` contains the semantic lifecycle. `damage_fp.hpp` preserves the
existing x87 arithmetic adapter; `damage_guard.cpp` performs read-only entry
checks under saved/restored full FP state. `damage_runtime.cpp` admits the new
domain only with `OPENBOXER_AI_CONTINUATION_MODE=replace`. The old consumption
and initiation domains retain their existing opt-in settings and guards.
Unsupported entries invoke the original whole function once before effects.

Native observation requires existing `OPENBOXER_AI_CAPTURE=1` and the new
`OPENBOXER_AI_CONTINUATION_CAPTURE=1`, plus its directory. Candidate observation
also requires `OPENBOXER_AI_MODE=replace`, the continuation mode, Damage enabled
and Clip original. Without the new discriminator the existing GAME-0003 capture
selection and 42 selector roles remain active. GAME-0004 adds seven idle roles;
an idle record requires the latest retained actual completion on the same
thread, preserving existing parent j=1 / caller / owner / literal checks.

New launcher routes:

- `--ai-continuation-fixture`: 357 whole paired typed scenarios, 49 read-only
  guard probes and one safe actual dispatcher fallback pair.
- `--ai-continuation-replay`: approved typed source input specified by
  `OPENBOXER_AI_CONTINUATION_REPLAY`; actual dispatcher route counters and ABI
  witnesses remain mandatory.
- `OPENBOXER_WORKER_OBSERVE_DELAY_MS=1000` uses the existing worker module
  observation delay, without changing its default zero.

Build: Clang 20.1.8, i686 target and MinGW32 sysroot, in
`build-game0004-candidate`. Project unit/ABI CTests passed 22/22, including 29 new
lifecycle/equality/completion/pending/table-alias/fatigue/cross-entry cases.
Build success and project tests do not establish original equivalence. Root
owns game workers/native observations; Agent3 owns differential validation.

Immutable candidate: `frozen-game0004-candidate-v1`. Its manifest records all
source/binary hashes and twelve prior semantic source files unchanged against
the GAME-0003 v3 freeze. Prior frozen DLL and launcher remain untouched.
