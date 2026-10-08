# GAME-0002 independent v4 prelaunch audit

Result PASS_SCOPED_DISPOSABLE_FIXTURE_AND_ORIGINAL_ONLY_OBSERVER_LAUNCH_ONLY. Behavioral equivalence remains BLOCKED.

All thirty freeze entries independently match; DLL SHA-256 E34F66520A6FFB963FD7F5968DDA96DA5F0C39B6CB278264977E6FE729CA7A2E. Freeze-check records are retained separately. Reviewed all modified replacement capture/restore/bridge/fixture interfaces and generated trampoline source; no original executable, implementation, process or UI accessed.

Witness entry544 and exit544 are now saved by actual invocation assembly immediately before/after whole execution at asserted offsets32/576, before any C++ snapshot operation. Seed includes full544, restoreFXRSTOR plus legacyFLDENV. Manual completion copies Witness captures. Generated observer saves FX and legacy environment before instrumentation and restores both before native forward and return. Callback bridge saves legacy fields alongside FX, preserves scalar raw80 and restores both after scalar extraction; typed scripts provide full544 callback returns. No architectural field is masked.

Independently executed replacement-only damage_observer_tests: transparent observer return/ret4/callee registers/live80/fullFP/XMM0..7 PASS. This strict probe compares 540 hardware-written bytes including legacy provenance and patterned XMM against direct dummy execution. It does not establish real original-game behavior.

Approval scope: guarded parked disposable fixed sixty-pair fixture run, twenty-two read-only guard probes, safe actual wrong-caller fallback pair; then original-only short owned test-copy native observations with fixed opt-in observer, all replacement units isolated as authorized. Prior source observations remain historical; corrective clean native captures must be retained for final FP fidelity and replay. Root must retain matching freeze/module/API/thunk/caller provenance, raw records, worker completion and observer restoration/cleanup evidence. Twelve prior semantic core preservation requirements continue.
