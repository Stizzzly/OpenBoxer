# WORLD-0001 map-load orchestration

The independent implementation follows the approved behavioral/ABI contracts
`specs/world/WORLD-0001-map-load.md` and
`specs/world/WORLD-0001-harness-metadata.md`. `world.cpp` owns only orchestration
and exact-width direct state writes. `world_runtime.cpp` supplies original
callback addresses and the approved routing boundary. Original model creation,
MS3D, texture reload, postprocessing, physics/ColDet, entity, RNG, holder and audio
dependencies remain opaque callbacks in natural game execution.

The bootstrap verifies the explicit test-copy path, complete executable SHA-256,
loaded module base and approved 0x19F000 image size. The coordinator independently
checks the original routing thunk using private evidence. The implementation
synthesizes an x86 relative tail jump at approved thunk RVA 0x10BE; it never reads,
saves, copies or translates original instructions. Body RVA 0x2E970 is retained
for fallback. This route intercepts the specified thunk caller; hypothetical
direct body callers remain outside interception evidence.

Normal `replace` mode installs both the prior MS3D slot and WORLD thunk. WORLD
replaces selectors 1..7; other selectors delegate before any orchestration state
write. `OPENBOXER_WORLD_MODE` can independently choose WORLD mode. For WORLD,
`pass-through` and `shadow` retain the original orchestration because duplicating
its audio/global/entity side effects in a live shadow call is not approved.
Successful allocation/dependency completion is the live contract scope; failure,
exception, reentry and concurrent state mutation compatibility is not claimed.

WORLD entry/exit and genuine routing records appear in `world-replacement.log`.
MS3D logs include the actual path and replacement/fallback result; reload and
downstream opaque callbacks remain the original operations. WORLD EAX forwards
the final callback's value, including a validator-supplied sentinel.

## Disposable differential execution

```powershell
$p='C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d'
& "$p/build/ms3d_launcher.exe" --world-fixture "$p/build/ms3d_replacement.dll"
```

The launcher creates its own explicit test process and parks the primary thread
at the approved CRT-initialized checkpoint. It runs the WORLD worker and
terminates that process after completion, including on errors. It never resumes
the game's main thread after test dependency redirection. The original allocator
and rand entries plus seventeen dependency thunks are routed to private typed
recorders. The SetCursorPos IAT data slot is replaced by a stdcall recorder; no
GL/audio operation or cursor movement occurs. Model allocations belong only to
the private test runtime, with fake constructors/vtables, and never pass to the
original destructor. These isolated fixtures do not claim natural original-heap
or transitive physics/audio equivalence.

Each scenario invokes the unchanged original map body and the independent
replacement through the installed map thunk. Stable object/holder identities
normalize pointer values. A genuine WORLD replacement count prevents an
original-versus-original comparison from passing. The 2640-case base matrix
covers selectors 1..8/0/9/-1, audio/mode gates 0/1, setup decision 0/1/2/-1,
signed input -1/0/1, and rand 0/1/2/3/7. Eight focused cases mutate later gates,
current selector global, setup decision, music selection and the late2000 gate.
In particular, the audio-load recorder changes music0 to2, requiring music1 then
music3; callback2000A changes decision1 to2, requiring callback2000B as well.

`world-fixtures-summary.txt` contains per-scenario inputs, returns, exact
callback/state comparisons, route, stack and floating-control results. The two
canonical trace files `world-original-trace.txt` and `world-candidate-trace.txt`
contain scenario and sequence IDs, callback names/normalized arguments, relevant
state at each callback boundary and final state. These traces include byte-width
witnesses, holder/scratch CC initialization, holder lifetime identity, opaque
audio ECX identities and final return sentinel. The original and candidate
reports are evidence for an independent validator, not binary similarity tests.

`world_abi.S` is independently authored test glue. It preserves the caller's
registers while placing known sentinels in EBP/EBX/ESI/EDI around each cdecl call,
then records them, ESP and EAX. x87 control and MXCSR are observed before/after;
the worker does not change controls and ignores floating status flags. CTest
also executes all2648 independent branch scenarios through this ABI probe.

The coordinator may set `OPENBOXER_WORLD_GUARD_DELAY_MS` to0..5000 for an external
prepatch observer. The launcher pauses after the initialized checkpoint and
before injection; the default is0. The observer's original-code evidence stays
outside the replacement repository and is not read by the implementation agent.

Original game launches and original-process report review are performed by the
coordinator/validator. The implementation agent has only built and executed its
own independent fixture executable. CTest3/3 PASS is an implementation check;
runtime and natural-game conclusions belong to the independent validation report.
