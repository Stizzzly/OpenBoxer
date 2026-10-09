# GAME-0003 candidate-v2 handoff

Status: implementation and local tests complete; differential and native candidate equivalence UNKNOWN.

Immutable directory: replacement/ms3d/frozen-game0003-candidate-v2. Verify manifest artifacts before each root-owned run.

Use the frozen launcher and frozen DLL. Launcher only creates and controls its own exact marked test-copy process; original installation remains untouched.

## Scripted whole differential

Environment: OPENBOXER_ORIGINAL_UNITS=all; OPENBOXER_DAMAGE_MODE=original. Unset OPENBOXER_AI_CAPTURE and OPENBOXER_AI_MODE.

Invoke `ms3d_launcher.exe --ai-fixture <absolute frozen DLL path>`.

297 paired original/candidate whole calls, 22 guard controls/rejections and wrong-caller whole fallback. Reports: C:/Users/ADMIN/Boxer-lab/ms3d/ai-fixtures.txt; damage-ai-fixture-original/candidate-*.json. Typed callbacks remain scripted in both sides. Independent Agent3 compares complete state, ordered callbacks, ABI and FP; worker success alone is not behavioral PASS.

## Approved natural-source replay

ai_pack_replay.py accepts only independently approved GAME-0003-approved-native-original-manifest.json SHA A281D3C4739EE645A7A45755BE5D48B20F10FA2882728F40BE844725A4700857, manifest paths and exact export hashes. It does not read private source paths.

11 packed inputs and their hashes: validation/GAME-0003-packed-replay/manifest.json.

Use fixture environment above plus OPENBOXER_AI_REPLAY=<absolute approved packed input path>. Invoke `ms3d_launcher.exe --ai-replay <absolute frozen DLL path>` for each input. Reports: ai-replay.txt and damage-ai-replay-original/candidate-*.json.

## Activation and exclusions

OPENBOXER_AI_MODE=replace explicitly enables the added first-entry AI domain when Damage is not forced original. Old GAME-0002 activation stays independently controlled by OPENBOXER_DAMAGE_MODE=replace. Default Damage remains original. Incoming AIattack1, stale timer, player attacks/pending and unrelated blocking/recoil/death/combo remain whole original fallback before candidate effects.

OPENBOXER_AI_CAPTURE=1 remains original-only observer authority; do not use it to claim native candidate replacement. Native candidate capture flags await additive authority.

## Validation

19/19 local CTest tests, including 846 semantic matrix cases. Old GAME-0002 delayed consumption local tests pass. Retest old GAME-0002 whole differential/source replay because own compiled layout and raw FP instruction/data pointers changed.

Own replacement-only FP provenance: validation/GAME-0003-replacement-v2-x87-map.json and GAME-0003-replacement-v2-spill-symbol-map.json. These maps authorize no blanket address/stale-reset waiver and establish no equivalence by themselves.
