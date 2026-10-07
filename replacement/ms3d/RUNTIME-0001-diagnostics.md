# RUNTIME-0001 — Ordinary launch and opt-in validation diagnostics

Status: regression/performance candidate; original-process Continue/reentry validation pending root.

The user reported a freeze after Play then Continue. Root could subsequently enter battle, pause and Continue with the previous808D build; a permanent freeze was not reproduced in that run. The exact reported freeze cause remains UNKNOWN. High CPU in an uncapped rendering loop alone does not identify a logic error.

Confirmed replacement-runtime work removed from ordinary launch:

- Automatic character/map capture created large event traces, normalized source snapshots and synchronous JSON output on first/repeat draw positions.
- Lighting stage observers created snapshot/log files on every supported stage call.
- Shared GL trace wrappers invoked a full FXSAVE/FXRSTOR observer preservation pair even when character capture was inactive.

Ordinary execution still installs all eight replacement units, uses the same readable-input/FP compatibility guards and retains original fallback for unsupported inputs. Seven prior semantic core files and character.cpp are unchanged. Only runtime diagnostics/installation glue changed. With captures disabled, map/character emission calls the original live GL dispatch directly; validation-only GL wrappers are not installed.

## Defaults and explicit switches

Default values are empty. Unit names are `ms3d,world,texture,upload,lighting,selection,draw,character`.

- `OPENBOXER_DIAGNOSTICS`: optional comma-separated units, or `1`/`all`. Enables compact runtime timing and selected legacy observer logs.
- `OPENBOXER_CAPTURE`: optional comma-separated units, or `1`/`all`. Enables expensive native source/GL captures. Character capture also enables shared map wrapper ownership, preserving one-owner forwarding.
- `OPENBOXER_ORIGINAL_UNITS`: explicit root diagnostic isolation only. Named units' replacement hooks are omitted so their original routes execute. Default empty leaves all hooks active. Fixture/replay bootstrap rejects this isolation setting (error26) rather than producing an invalid differential result.

Ordinary delivery must clear these three variables and select all replacement modes. Only the normal launcher route is used; fixture/replay commands are explicit validation tools and are not ordinary play entrypoints.

For a lightweight loading/render investigation use `OPENBOXER_DIAGNOSTICS=world,draw,character` with `OPENBOXER_CAPTURE` empty. `runtime-timing.jsonl` records unit, entry/guard/render/leave phase, thread, sequence, monotonic milliseconds and elapsed milliseconds. Render sampling is first three calls and at most one sample per second per unit. FP state is preserved around diagnostic I/O. The ordinary path performs no timing file I/O.

Full native validation must opt in, for example `OPENBOXER_CAPTURE=draw,character`. Existing parked fixture/replay reports remain explicit and are still written by their worker commands independently of these flags.

## Validation and delivery

The nine existing offline behavior/ABI/FP targets passed for F1D6. All seven prior semantic core hashes match their preserved manifest. Root reports that the user confirmed the stalls disappeared during normal play with diagnostics disabled. The exact contribution of each diagnostic operation remains UNKNOWN; no permanent hang was reproduced by root. Earlier differential fixture/replay results describe the historical 808D build and are not claimed as retests of this revision.

Frozen binaries are in frozen-render0006-runtime-fix (DLL F1D6C81914B18556BF1BD19C9062D17FC275E880618C57D49DDDF33365C72591; launcher 584C352BAB0AB4FE0E33EFA780DD3A31AAD46248AE65D59D640CC0662C691223). The original frozen-render0006 directory remains historical. Root updated the guarded user delivery to these binaries, clearing diagnostic/capture/isolation settings and keeping all eight replacement modes active.

