# RENDER-0001 — Material texture reload boundary

Status: IMPLEMENTED; ISOLATED_DIFFERENTIAL_PASS (18 cases); NATURAL_MAP_1_SMOKE_PASS. Original-process tests confirmed exact zero/negative-count return, iteration/state and ABI; natural map1 produced three replacement calls with 29 preserved uploader calls and rendered textured gameplay. Implementation: replacement/ms3d/texture.cpp and texture_runtime.cpp. Independent report: validation/RENDER-0001.md. Agent1 specification, 2026-10-04; completion recorded by coordinator. Real-context repeated reload and teardown remain unverified.

## Provenance and first replacement scope

Original program.exe SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`, preferred base0x400000. Fresh IDA session9317c4b6 verified architecture32, image size0x19F000 and hash. Copied original/raw evidence resides under `C:\Users\ADMIN\Boxer-analysis\fresh-coldet`; legacy OpenBoxer code/assets excluded.

Primary replacement: routing thunkRVA0x1A64 -> original reload bodyRVA0x16680. Preserve existing original game texture-uploader callback thunkRVA0x1799 -> bodyRVA0x25420, and preserve upstream BMP/DIB dependency. This first unit independently implements material traversal/state updates while original uploader continues file/decoder/GL behavior. Uploader observations below are a companion boundary, not an instruction to replace upstream decoding.

Approved generic route: synthesized5-byte relative tail jump at reload thunk to replacement; original body untouched and callable directly for fallback/differential side. Uninstall synthesizes jump from thunk to approved body. Metadata-only hook implementation requires exact whole-file hash and live corresponding module identity established before mutation; root/validator privately verifies baseline route. No original bytes/pseudocode are supplied to Agent2. Existing conflicting hook or unsupported module fails guard. Preserve normal render-thread execution; no worker GL calls in natural gameplay.

Known callers: map orchestration callsitesVA0x42EB2F/0x42ECA3 for map/light models, frame startup callsiteVA0x448253 for lamp. Reload is a direct nonvirtual callback through thunk, not an object virtual slot. Only original bodyxref found is thunk0x401A64. Model virtual slot+4 belongs to MS3D loader and is a distinct boundary.

## Reload callable ABI and exact return

CONFIRMED instruction-reviewed ABI: Microsoft x86 thiscall, ECX original model pointer, no explicit stack arguments, plain return with no explicit stack cleanup. EBP/EBX/ESI/EDI preserved; EAX/ECX/EDX volatile. Relevant original structure layout is IO-0003: int32 material count at object+0x8D9AC, pointer32 material array at+0x8D9B0, each record80 bytes, texture ID uint32+72, owned filename pointer32+76.

Actual EAX behavior is not boolean success. For stable positive countN it returnsN; for count0 or negative it performs no iteration and returns0xCCCCCCCC in this original debug build. This comes from original debug-stack initialization's EAX value, which survives the zero-iteration branch and normal epilogue; treat it as target-specific observed register behavior (CONFIRMED), not a meaningful C++ return value. All three known callers ignore the return. If count is changed by a callback mid-loop, nonempty iteration exit returns the processed iteration index (one past last processed), rather than necessarily final material count. First live replacement scope uses stable valid layout/count/string ownership; callback-isolated validation can test live-count behavior separately.

## Observed reload behavior

Confidence HIGH, with signed comparison/record writes CONFIRMED.

1. Iteration index starts0. Before each iteration compare signed index to the current signed material count; stop when index>=count.
2. Get filename pointer from current array record+76 and call original cdecl strlen (RVA0x99880). Null filename is not checked; malformed ownership is outside scope.
3. If length>0, reread filename pointer for that record, call preserved cdecl texture-upload callbackRVA0x1799 with that exact pointer, and store all32 return bits at record+72. Result0 is stored like any other value. Record array base is read again after callback before storing, so do not assume cached table identity if intentionally testing callback mutation.
4. If length0, store uint32zero at record+72; do not invoke uploader.
5. Increment index and repeat. No early stop on upload failure. No other material fields/count/pointer/vtable are written by reload itself.

Paths are passed exactly as the material filename string: no normalization, model-directory prefix, texture-directory search, suffix replacement, extension dispatch, deduplication or caching. Identical filenames in multiple materials produce separate uploader calls. Filename allocation ownership remains unchanged. Empty filename overwrites old ID with0. Missing/unreadable nonempty filename produces uploader0 and overwrites old ID0 under ordinary uploader behavior below.

Reload does not call glDeleteTextures or release previous IDs. Repeated calls create new texture names for nonempty materials and overwrite old names; preserve that behavior rather than introduce automatic deletion/caching. Original model destructor later deletes each current ID, not all previous overwritten IDs (see IO-0003). Do not zero entire material records or alter transparency/padding/filename bytes.

## Preserved game uploader boundary (RVA0x25420)

CONFIRMED ABI: cdecl(const char* filename)->uint32 textureID; caller cleans4. No model pointer. Success/failure path observations HIGH. This callback remains original for first reload replacement; its game-owned behavior may later receive a separate scoped implementation while the decoder remains upstream.

Ordered behavior:

1. Local textureID starts0. Invoke BMP existence/decoder helper thunkRVA0x1AAF -> bodyRVA0x25390 with filename.
2. If returned image pointer is null OR its pixel pointer+8 is null, perform no GL calls, no frees and return0. Notably nonnull record with null pixels is not freed in this path; preserve original leak behavior if that boundary is replaced later.
3. Otherwise invoke glGenTextures(1,&localID), then glBindTexture(GL_TEXTURE_2D,localID).
4. Invoke glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR): numeric0x0DE1,0x2801,0x2703(9987).
5. Invoke glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR): numeric0x0DE1,0x2800,0x2601(9729).
6. Invoke original GLU gluBuild2DMipmaps(GL_TEXTURE_2D,3,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels): numeric0x0DE1,3,image+0 int32,image+4 int32,0x1907,0x1401,image+8 pointer. Return status ignored.
7. Release image pixels first, then image record via original cdecl free-like callbackRVA0x96D10 (calls original debug release with block type1), NOT model's C++ delete facade. Return local generatedID even when mipmap construction fails. glGenTextures output0 likewise proceeds through subsequent GL calls and returns0; no GL error query or rollback exists.

Image record ABI:12 bytes packed scalar record width32 at+0,height32+4,pixel pointer32+8. Pixel format here is RGB unsigned byte as uploaded; decoder details/orientation/row layout remain original dependency contract, not inferred here. Uploader leaves generated texture bound and filter parameters applied, creates mip levels through GLU, and does not restore prior texture binding/state. No direct game-global writes observed in this uploader or reload; dependencies can change CRT/driver globals and OpenGL state.

## Preserved BMP helper / failure boundary

HelperRVA0x25390 is cdecl filename->image pointer. Null filename returnsnull. Otherwise calls original fopen(filename,"r") only as existence probe, closes successful FILE*, then calls existing upstream auxDIBImageLoadA-like entryVA0x492EDE. Probe-open failure returnsnull; decoder result forwarded. Ordinary ASCII/current-directory path behavior is supported; permission/device/encoding/racing-file/error-dialog semantics remain dependency behavior and fallback scope. This helper does not select TGA/PNG/JPEG by suffix; decoder is DIB/BMP path. Root separately established historical GLaux provenance; this unit leaves decoder unchanged.

## Approved recorder harness metadata

Use a fresh disposable initialized original process with main thread parked, not the user's active game. Root must quiesce affected threads; intercept callbacks only while not executing those routines; never resume normal game after stub interception. Invoke untouched original reload body on a fake model and independent candidate on equivalent fake model. No GL/file activity is necessary when uploader callback is a recorder.

Primary isolated harness: redirect5-byte uploader thunkRVA0x1799 to cdecl filename recorder returning chosen uint32 IDs; original strlen can remain (valid local null-terminated buffers only) or be replaced by an equivalent typed recorder in disposable mode to compare exact strlen/uploader call ordering. Fake model must be at least0x8D9B4 bytes to include fields; allocate count×80 materials, assign valid filename pointers, seed IDs and all other bytes with sentinels. No vtable is read by reload, so fake constructor/vtable is unnecessary for this unit. Record strlen filename identity, uploader path content/identity, generated return, record ID changes, complete other bytes unchanged, final EAX and stack/nonvolatile restoration. Normalize fake object/table/string pointer identities.

Cases: count0, signed negativecount, positive mixture of empty/nonempty filenames, repeated identical filenames with different returnedIDs, uploader0, largeIDs0xFFFFFFFF, second reload of same model (no delete callbacks), and no short-circuit after failure. StableN positive expects EAXN; zero/negative expects0xCCCCCCCC. Optional callback mutation case shrinks count during uploader and confirms loop compares live count and returns processed index; table relocation case confirms post-callback table reread. These are explicit original-observation comparisons, not assumptions about idiomatic C++ design.

Supplementary uploader-only recorder test if root chooses to validate companion boundary: replace BMP helper thunkRVA0x1AAF with a recorder returning null/image-with-null-pixels/valid12-byte image record; replace GL import data slots and GLU/free boundaries with typed recorders. Do not patch shared OpenGL DLL code. Confirm exact GL/free order, parameters, generatedID output, ignored mipmap status, no deletion/restore. Still leave upstream decoder unmodified in natural replacement.

CONFIRMED IAT data-slot RVAs:

| Import | SlotRVA | API ABI |
|---|---|---|
| glGenTextures | 0x18CA5C | stdcall(int32 n,uint32* IDs), pops8 |
| glBindTexture | 0x18CA58 | stdcall(uint32 target,uint32 ID), pops8 |
| glTexParameteri | 0x18CA54 | stdcall(uint32 target,uint32 pname,int32 value), pops12 |
| gluBuild2DMipmaps | 0x18C880 | stdcall(uint32 target,int32 components,int32 width,int32 height,uint32 format,uint32 type,const void*), pops28, int32status |

The original GLU call reaches its import through thunkRVA0x92E92; replacing IAT slot suffices. Image release callbackRVA0x96D10 is cdecl(ptr), caller cleans4, return ignored. A disposable process may redirect that entry to a recorder only when all original activity is quiescent; no trampoline/copied original bytes or normal resume afterward. Natural reload uses original uploader on the game's rendering thread with a current appropriate GL context; a fixture worker thread has no such current context automatically. Decoder/UI error dialogs and GL calls must never be triggered by isolated tests.

## Natural validation and limits

After independent recorder parity, deploy only the reload thunk replacement in a separate authorized test game copy retaining original uploader/decoder and previous supported replacements. Observe natural map1 loads: each material filename produces ID assignment matching original dependency outcome; map/light and lamp routes remain original render-thread flow. Log replacement count and delegated upload count separately, including empty filenames and0 returns. No change to installed original or user-playing process is authorized by this analysis task. A screenshot alone cannot prove callback/ID parity; record scoped state/checkpoints.

Invalid model/filename ownership, allocation/driver exceptions, cross-thread/context misuse and decoder-specific malformed images are out of scope; retain original fallback selected before mutation. Repeated reload of valid model is explicitly in scope and intentionally has no old-ID deletion. Numerical comparison EXACT for stored32-bit IDs/count register state and callback arguments; no floating arithmetic belongs to reload itself.

## Evidence

Fresh IDA body reload0x416680, helper0x425420/BMP probe0x425390, thunk/xrefs and independent imports queries. Exact return verified at initial debug register assignment and signed count loop/epilogue; all known caller returns ignored. GL call/filter/GLU/free order instruction-reviewed. Raw routing evidence resides outside repo in `RENDER-0001-hook-evidence.json`, never sent to implementation agent.
