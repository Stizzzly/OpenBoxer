# GAME-0002 work-in-progress checkpoint — 2026-10-08

User-authorized source checkpoint, not a validated gameplay release.

Implemented a bounded whole opponent-update replacement for ordinary Z pending-hit timing, damage, blocking, reaction, effects and the required physics/audio tail. Original dependencies remain opaque. Unsupported entries fall back before replacement effects. Damage remains original by default; explicit OPENBOXER_DAMAGE_MODE=replace is an experimental test option.

The twelve earlier semantic cores retain their baseline hashes. The local synthetic CTest suite passed 16/16. The newest local frozen candidate is SHA-256 19838F07440A79D071A91C343AE2BBDA28D4B903A2D9190F2A78124C31BBEE47. Its guarded disposable worker completed 60 scenarios with changing XMM inputs/callback results, 22 admission probes and an actual single original fallback; worker result 0 and observer restoration 1. Independent final comparison of those newest records remains pending. Native-free independent XMM regression passed 19,489 checks.

Earlier comparisons found and corrected swapped audio callsite labels, incomplete legacy FP observer restoration, snapshots taken after instrumentation, and XMM clobbering by memory-copy operations. Historical failed reports are retained. The preceding candidate matched state, callback order, EAX and x87 arithmetic but failed native-input XMM comparison. Do not present those results as full equivalence.

Clean original-only native observations were recaptured with the corrected observer. An independent validator admitted 28 typed records covering wait, unblocked consumption, blocked consumption and far miss. Source-to-original replay passed; the newest candidate still needs the full 28-record replay and independent architectural/provenance comparison. Raw observations, memory records, packed replay inputs, IDA files and frozen binaries are deliberately excluded from this public repository.

Remaining: independently compare the newest 60-scenario/guard/fallback output; rerun and compare 28 source-correlated replays; close documented FP instruction/data provenance mappings; establish natural candidate activation with previous animation/rendering replacements; verify diagnostics-off delivery. The previous validated gameplay-v4 copy is unchanged. No gameplay-v5 release is claimed.

Resume locally from validation/GAME-0002-fixture-v5-freeze.json, damage-fixture-observations-stage5 and the approved clean-source replay manifest. These excluded local paths are provenance references, not shipped test data.
