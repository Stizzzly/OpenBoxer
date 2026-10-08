# GAME-0002 original-only observer v1

No replacement gameplay behavior is installed by this candidate. Existing twelve
cores remain unchanged. Bootstrap's existing test-copy path, executable SHA256,
module-size guards precede observer installation.

Opt in with `OPENBOXER_CAPTURE=damage`. Do not combine with `strike` capture:
both instrument the same imported audio slot; installation rejects that pairing.
All other twelve replacements remain enabled by default.

Optional `OPENBOXER_DAMAGE_CAPTURE_DIR` selects an existing output directory.
Default is `C:/Users/ADMIN/Boxer-lab/ms3d`.
`OPENBOXER_DAMAGE_CAPTURE_LIMIT` defaults to 256, capped at 256. Sampling keeps
up to eight entries per initial-block/predicted-wait-or-consume category and eight
pending-zero cached-distance>=6 entries. This observation filter does not establish
semantic admission and never changes original execution. It prevents early waiting
frames exhausting the later consumption budget. Sampling requires actual static
actor/caller/mode match, active non-menu scene, player attack/type1. Unsupported
routes and every callback still forward the unchanged original exactly once.

Frozen binaries: `frozen-game0002-observer-v1`. Source/binary hashes:
`validation/GAME-0002-observer-v1-freeze.json`.

Output: `damage-original-NNN.json`, format `GAME-0002-observer-v1`.
Whole snapshots contain actor232, player176, both body2288, concatenated global
spans in approved metadata order, world head/tail/count12, source addresses.
Events contain ordered approved call/return RVAs, callback identity, raw owner and
stack words, raw pointer argument input/output, raw returned bytes/EAX, RNG words,
owner changes, entry/return FP and integer registers. Getter/RNG events carry no
body snapshots. Mutating physics calls capture body2288; cursor captures actor232.
`combat_before16`/`combat_after16` contain four little-endian words: initial/current
block byte widened to32, pending byte widened to32, raw health32, actor state32.
These distinguish block at consumption from final block.

FP544: bytes0..511 FXSAVE; bytes512..539 FNSTENV28 (including full x87 tag word);
bytes540..543 zero padding. Scalar80 is logical ST0 at FXSAVE byte32. Integer36 is
PUSHAD order EDI,ESI,EBP,savedESP,EBX,EDX,ECX,EAX followed by EFLAGS. SavedESP includes
observer pushes: whole exit-entry difference8 corresponds to return address plus
ret4. Instrumentation uses a masked empty private FP environment and restores the
original saved environment, registers and EFLAGS before forwarding/returning.

Installation verifies all approved direct targets, exact thunk/caller, imported
audio instruction and resolved original OpenAL32 alGetSourcei pointer. Every
patched byte/IAT word is saved. Failure restores mutations in reverse order.
Export `damage_observer_restore_worker` restores successful patches when root has
paused the owned process outside an invocation; do not invoke concurrently with
gameplay. Ordinary no-capture launch installs no GAME-0002 hooks/files.

Native-free tests: build target `damage_observer_tests`; CTest selector
`damage_transparent|diagnostics_optin`. Current tests PASS for unchanged forwarding,
exactly-once hook calls, ret4, callee registers, stack balance, raw80 and CW despite
instrumentation FNINIT, plus ordinary default-off flags. Original activation and
equivalence remain unvalidated until root-owned natural capture and Agent3 audit.
