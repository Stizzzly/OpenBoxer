# Месть боксера — first investigation plan

Date: 2026-10-04. Scope: behaviorally equivalent reimplementation using the required three-role separation.

User correction: ColDet is used; Bullet is not used. Begin the collision investigation anew from original program.exe/coldet.dll and independently obtained upstream. Do not use existing OpenBoxer source or earlier prototype behavior as evidence. Distinguish user-supplied knowledge from independently confirmed binary observations. Earlier proposed provenance repairs to prototype sources are outside this fresh RE work item.

- Original install: `C:\Program Files (x86)\Alligator Friends\Месть боксера. Московский криминалитет` (CONFIRMED); store/version UNKNOWN.
- Replacement: `C:\Users\ADMIN\CLionProjects\OpenBoxer` (CONFIRMED).
- Binary: `program.exe`, native x86 HIGH; engine identity UNKNOWN. See specs/platform/BOOT-0001.md for immutable file identity and evidence.
- Original ABI hypothesis: VC6-era Microsoft C++ HIGH; exact compiler UNKNOWN. ETS2 compiler and Bullet assumptions discarded for this target.
- Rendering/audio: OpenGL/GLU and OpenAL/Vorbis import boundaries CONFIRMED. Collision: bundled ColDet factory export CONFIRMED; game usage and upstream equivalence UNKNOWN. Bullet UNKNOWN.
- Online/anti-cheat: UNKNOWN; Winsock imports are not proof. No debugger attached, game not launched during this stage.
- Saves/config/log paths: UNKNOWN. Determine by read-only inventory and controlled offline baseline before execution that may change user state.
- Community loader/current route: UNKNOWN; not researched in this bounded stage. No installation or dependency download performed.
- Chosen route: independent implementation from Agent 1 behavioral specifications, followed by Agent 3 differential validation, as explicitly requested.
- Lab: original install remains unchanged. Private binary/IDA evidence in `C:\Users\ADMIN\Boxer-analysis`; spec-only handoff to a fresh Agent 2. Existing prototype preserved.

## Immediate milestones

1. Completed: reproducible program/DLL PE fingerprints; isolated correct IDA target verified by SHA-256; Vorbis static dependency boundary confirmed.
2. Next: resolve requests/open/REQ-0001.md through one actual game collision creation/query boundary. Compare bundled ColDet export and existing source against verified historical upstream provenance; matching names alone are insufficient.
   Priority parallel prerequisite: resolve REQ-0002 for the missing ColDet factory/construction implementation reported by validation/BASELINE-0001.md as FAIL_LINK. Find verified source provenance before approving a dependency replacement or filling missing behavior.
3. Next: identify one map-loader boundary with a known input fixture; specify all observed inputs, outputs, effects and unknowns under IO-0001.
4. Only after an approved behavioral contract: fresh Agent 2 implements the selected small unit; Agent 3 checks the same short original/replacement checkpoint.

No claim of runtime equivalence, confirmed physics library, established engine version or validated gameplay is made by this stage. Missing original observations yield BLOCKED for differential equivalence.

## Authoritative scope update — 2026-10-04

User identifies ColDet and excludes Bullet investigation. Start from original binaries and data, not existing OpenBoxer implementation. Earlier prototype repair priority is superseded; REQ-0002 moved to requests/resolved as superseded, not solved. Keep legacy work untouched. COLL-0001 must establish the original boundary independently and distinguish user-provided identification from binary-confirmed evidence. Upstream comparisons require a separately acquired source with provenance. No new replacement code until a behavioral unit is specified.
