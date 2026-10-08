# GAME-0002 independent v6 correction audit

PASS_SCOPED_OWNED_REGRESSION_COLLECTION_ONLY. Final equivalence/delivery remains FAIL for v5 and BLOCKED for v6 until new observations pass.

New frozen DLL B10C2E1D5CF4E422D8A83D2F7470D0E4463200CE4D8BDD09CD7BE4AA1B32052F:63 freeze entries independently hash-verified. Three runtime/header edits explicitly pass BinaryState game base to audioTarget, which resolves current IAT at gameBase+18CC00 when no cached capture target exists. This removes reliance on uninitialized observer module in uncaptured mode. Observer cached target branch remains. New synthetic-IAT regression independently ran PASS: two distinct live targets and cached target with zero base. It tests resolution logic; actual pure runtime installation/native callback execution remains mandatory.

The v5 captured/differential success does not override diagnostics-off PID21008 access violationC0000005. Separate observer/runtime module initialization defect independently established from replacement source. WER unknown EIP alone does not establish faulting callsite; root typed fault provenance remains separate.

Scoped root approval: exact frozen v6 owned diagnostics-off opt-in native retry, plus sixty patterned differential pairs, twenty-two guard checks, actual wrongcaller fallback and twenty-eight admitted clean source replays to verify complete final binary/interface path. Preserve new module/API/thunk/caller/range/hash proof, original-source manifests, raw captures, per-run cleanup and new replacement v6 maps. Use separate v6 outputs; v5 FAIL and all historical observations stay intact. Natural opacity obtains no scripted reset waiver. Fresh v6 reset checkpoint exclusions require applicable exact approved policy, never broad stage5 inheritance.

No installed original modification, no user-owned process manipulation, no final default activation or PASS is authorized by this collection gate. Further test repetition depends on new failures/changes; once these pass, proceed to final native delivery evidence.
