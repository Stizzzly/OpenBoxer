# Focused ordinary-Z health comparison — 2026-10-08

PASS: all 28 previously validated identical-input original/v6 replay pairs have exactly equal float32 opponent health before and after the update. Record SHA-256 identities were rechecked against the independent stage6 comparison report. These are replays of recorded original gameplay, not a newly launched side-by-side fight.

The same player index0 and opponent index4 occur throughout. Player damage factor is float32 0.9 and opponent factor1.0. There are four unblocked hit consumptions, seven blocked consumptions and seventeen updates without hit consumption. Unblocked loss is4.5; blocked loss is approximately0.9, with identical float32 rounding in both programs.

| Recorded event | Original health | Replacement health | Loss in both |
|---|---|---|---|
| clean-stage4-008, unblocked | 94.959999 → 90.459999 | 94.959999 → 90.459999 | 4.5 |
| clean-stage4-027, blocked | 85.959999 → 85.059998 | 85.959999 → 85.059998 | 0.9000015 |
| clean-stage4-034, unblocked | 61.119995 → 56.619995 | 61.119995 → 56.619995 | 4.5 |

No reduced per-hit damage is observed in these supported same-fighter samples. This audit does not establish whether the user's particular live fight had missed attacks or more blocks, nor initial health differences between opponents. No replacement behavior was edited. The health-only script and local detailed JSON accompany this report; native payloads remain local.
