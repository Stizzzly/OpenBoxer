# RENDER-0001 material texture reload

`texture.cpp` independently implements the approved reload contract in
`specs/render/RENDER-0001-reload-textures.md`. The original uploader, BMP/DIB
decoder and GL/GLU routines remain opaque dependencies in natural execution.
This unit changes only traversal and ID assignment; it neither implements the
uploader companion boundary nor replaces image decoding.

The runtime uses the existing path/full executable hash/module-image guards and
the coordinator's private baseline route verification. It synthesizes a generic
relative tail jump at approved reload thunk RVA0x1A64, leaving original body
RVA0x16680 unchanged for fallback and independent comparison. No original
instructions are read, copied, saved or translated by this implementation.
Existing MS3D and WORLD behavior is retained; WORLD's opaque reload callback now
reaches the texture hook through its original thunk address.

The loop reads signed count before each iteration, invokes original strlen, then
rereads the filename and material table as required around callbacks. Nonempty
strings invoke original uploader RVA0x1799; empty strings write ID0 without an
upload. All returned32 bits, including0/FFFFFFFF, are stored. It does not cache
or deduplicate names, stop on failure, delete old IDs or change filename ownership.
Stable positive countN returnsN; zero/negative isolated cases return target-specific
0xCCCCCCCC; callback-mutated loops return the processed index. No floating
arithmetic occurs in this unit.

Natural replacement requires a readable valid model/material layout with
nonnegative count up to65535 and accessible ordinary ASCII null-terminated names
within128 bytes, matching the approved MS3D-derived ownership boundary. Unsupported
models delegate before writes. This preflight is a scope selector, not a complete
proof of allocator ownership or protection against concurrent mutation. Driver,
allocation/foreign exceptions, malformed ownership and cross-thread GL-context
misuse remain outside validated compatibility. Negative counts are tested under
isolated replacement mode; natural deployment delegates them to original.

`OPENBOXER_TEXTURE_MODE` independently chooses texture mode. In `replace`, normal
reloads use the candidate; other modes preserve original behavior because a live
shadow reload would duplicate texture creation. `texture-replacement.log` records
each material's exact path, old/current ID, whether upload was delegated, and
total upload/call counts, followed by the exact return. Empty names and zero IDs
are explicitly logged. Natural calls remain on the game's original render thread
with its existing context; the implementation does not create a worker GL context.

## Disposable differential fixture

```powershell
$p='C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d'
& "$p/build/ms3d_launcher.exe" --texture-fixture "$p/build/ms3d_replacement.dll"
```

The launcher parks its own original test copy at the approved initialized
checkpoint, runs the fixture worker, then terminates that owned process on
success or failure. It never resumes the primary thread after dependency
interception. The worker redirects only original strlen entryRVA0x99880 and
uploader thunkRVA0x1799 to typed private recorders. No original uploader, decoder,
filesystem or GL operation runs during isolated comparisons. Original reload
BODY is invoked directly on one fake model; candidate is invoked through the
installed reload thunk on an equivalent model, with a genuine replacement counter
preventing original-versus-original parity.

Eighteen independent cases cover count0/-1/INT_MIN, empty/all-empty names,
ordinary and0/FFFFFFFF IDs, duplicate identical filename pointers with distinct
IDs, failure followed by later upload, a second reload of the same model,
count shrink1/0 and growth1-to3, table relocation during strlen/upload,
empty-name table relocation, filename substitution after strlen and count
shrink during strlen. Models/materials carry deterministic sentinel bytes.
The harness checks all unrelated model bytes and every material byte before
the ID/filename fields, plus normalized pointer/filename identity and complete
callback ordering/state snapshots. No destructor or delete callback is invoked.

`texture-fixtures-summary.txt` provides each case's EAX, routing, callback/state,
untouched-byte, ABI and FP results. `texture-original-trace.txt` and
`texture-candidate-trace.txt` contain scenario/sequence IDs, strlen/upload input
identity/content/result, table/count and ID snapshots, and final normalized
state. Pointer addresses are normalized as tableA/B and string indices; identical
content with different identities remains distinguishable.

`texture_abi.S` is independently authored no-argument thiscall observation glue:
ECX carries the object; no explicit argument stack cleanup occurs. It installs
and verifies EBP/EBX/ESI/EDI sentinels, captures ESP and full EAX, and preserves
its own caller's state. x87/MXCSR controls are observed before/after without
changing them; status flags are excluded. CTest executes all18 offline cases
through this probe, alongside the prior MS3D and2648 WORLD checks (4/4 PASS).

The coordinator's optional `OPENBOXER_WORLD_GUARD_DELAY_MS` also applies to this
fixture for private prepatch route observation; default0, maximum5000. Original
process launches, raw routing evidence and original-process output review belong
to the coordinator/validator, and are not performed by the implementation agent.
