# RENDER-0002 texture upload

`upload.cpp` independently implements the approved game-owned orchestration in
`specs/render/RENDER-0002-texture-upload.md`. Original BMP helper/DIB decoding,
OpenGL, GLU and original free-like ownership release remain the natural callbacks.
Existing MS3D, WORLD and material-reload algorithms are unchanged; installation
adds the uploader seam used by reload's existing dependency address.

The runtime synthesizes a generic relative tail jump at approved uploader thunk
RVA 0x1799, preserving original body RVA 0x25420 for fallback and comparison. It
uses existing explicit test-copy/full executable hash/module guards and the
coordinator's private prepatch route verification. No original instructions are
read, saved, copied or translated by this implementation.

The local texture ID begins at zero. The original helper receives the exact
filename once. Null image or initial null pixels return zero without GL or frees,
including the specified no-free behavior for a nonnull/null-pixel image. Other
records invoke generation, binding, min/mag parameters and mipmap construction
in the specified order. Pixels/height/width are read from the live record after
both parameter callbacks. Pixels are reread after GLU for release, followed by
release of the original record pointer. The final local ID is read after both
releases; zero/FFFFFFFF and ignored nonzero/negative GLU statuses are preserved.
No conversion, dimension validation, deletion, binding restoration or rollback
is added. There is no guessed decoder-storage ownership validator.

`OPENBOXER_UPLOAD_MODE` independently selects uploader mode. `replace` routes to
the candidate; other modes preserve original execution because duplicating a
live upload in shadow mode would add texture/decoder side effects. Natural calls
retain the original rendering thread/current GL context and original allocation
ownership. Exceptions, races, invalid storage and cross-thread/context misuse
remain outside supported compatibility, as described in the behavioral contract.

`upload-replacement.log` reports filename, helper/null-pixel outcome, dimensions
captured at mipmap invocation, generated/final IDs, ignored GLU status, completion
of pixel/record releases and replacement/dependency counts. Observations are
captured before frees; no freed image record is read. Instrumentation adds no
helper, driver or ownership callback invocations.

## Disposable isolated uploader fixture

```powershell
$p='C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d'
& "$p/build/ms3d_launcher.exe" --upload-fixture "$p/build/ms3d_replacement.dll"
```

The launcher parks its owned original test process at the approved initialized
checkpoint, runs the worker, and terminates that process after success or failure.
It never resumes its primary thread after dependency interception. The worker
redirects BMP helper thunk RVA 0x1AAF and release entry RVA 0x96D10 to private typed
recorders, and replaces four GL/GLU IAT data pointers. No shared DLL code is
patched, no original decoding/file/GL operation executes, and synthetic releases
record without freeing the private fixture storage. Original uploader BODY is
compared against the installed candidate thunk on equivalent inputs. Actual
replacement counters prevent original-versus-original false parity.

The 22 cases cover null image/null filename/null pixels; generated, unchanged,
zero and FFFFFFFF IDs; positive/negative ignored mipmap status; repeated calls;
unusual signed dimensions; live image changes during binding and either parameter
callback; pixel relocation/nulling during GLU; and retained local-ID changes during
binding, parameters, GLU, pixel release and final record release. A combined case
checks all live-read boundaries together. Each image has surrounding guards and
three fully checked sentinel pixel buffers. The complete 12-byte record state is
compared with pointers normalized as record/pixels0..2, and the output stack
pointer is normalized as localID. Its value is observed only during callbacks;
the retained pointer is cleared immediately after return.

`upload-fixtures-summary.txt` records each scenario's exact EAX, routing, trace,
state, sentinel, ABI and floating-control results. `upload-original-trace.txt`
and `upload-candidate-trace.txt` contain scenario/sequence IDs, all ordered
callback arguments, initial generation output, status, live record/local-ID
snapshots and final state. The one-pointer cdecl probe in `upload_abi.S` is
independently authored: it captures full EAX/ESP and known nonvolatile-register
sentinels while preserving its own caller's registers. x87/MXCSR are observed
before/after without changing controls. CTest executes all 22 offline cases
through that probe alongside previous units (5/5 PASS).

The existing optional `OPENBOXER_WORLD_GUARD_DELAY_MS` supports the coordinator's
external prepatch observer; default zero, bounded to 5000. Original process
execution, raw route evidence and original-process output review remain the
coordinator/validator's responsibility and are not performed by Agent 2.
