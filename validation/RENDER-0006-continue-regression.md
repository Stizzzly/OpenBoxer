# Play / Continue stall regression

Result: **PASS**, bounded human functional confirmation that the reported stalls disappeared on the updated ordinary launch. This is not a repeated automated menu/restart equivalence result.

Final runtime DLL SHA256: `F1D6C81914B18556BF1BD19C9062D17FC275E880618C57D49DDDF33365C72591`; launcher remains `584C352BAB0AB4FE0E33EFA780DD3A31AAD46248AE65D59D640CC0662C691223`. User directly reported “тормоза пропали” after the F1D6 ordinary probe. Root observed that run entering battle and opening pause. Root did not independently complete repeated Continue/restart cycles on the updated build; further UI testing stopped at user confirmation.

Root separately observed one legacy808D Play -> battle -> Escape pause -> Continue -> battle transition with visible animation/health progress and responsive process. Its archive is `C:/Users/ADMIN/Boxer-lab/ms3d/regression-continue-legacy`, including `observation.json` and `regression-after-continue.png`. Agent3 independently inspected that typed observation and screenshot. This demonstrates no permanent hang reproduced in that run; it does not exclude intermittent stalls.

The user clarified that testing and their own play overlapped. The exact cause of that individual stall remains **UNKNOWN**. No unsupported claim attributes it conclusively to test contention, capture IO, or a particular function. PID12280 disappearing during user interaction is not classified as a crash without crash/exit evidence.

Read-only runtime audit confirms capture/diagnostic flags default false; ordinary launch avoids native trace/snapshot IO, lighting per-stage logging and capture-only shared/scalar GL wrappers. All eight semantic routes and preflight/fallback checks remain active. Explicit diagnostic isolation controls remain opt-in. The updater removes demonstrated heavy diagnostic work; it does not change the approved draw/interpolation behavior. Shared GL ownership has no mutual wrapping cycle in the reviewed code.

All seven prior semantic core hashes independently match the preserved manifest; see `RENDER-0006-regression-preservation-independent.json`. Agent2/root report all nine offline targets passing. Prior37 exact differential cases and bounded native moving-pose proof remain recorded against their original immutable builds; they are not presented as a fresh full differential rerun of F1D6 runtime integration.

Root updates guarded normal delivery to clear inherited test/diagnostic/capture/original-unit overrides before launch. Agent3 has no original executable/IDA/process/UI access and made no replacement behavior changes. This regression result rests on direct user functional confirmation, corroborated source changes and the precisely limited root observations above.
