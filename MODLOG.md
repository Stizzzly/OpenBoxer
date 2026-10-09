# OpenBoxer investigation journal

## 2026-10-04 — Initial project baseline

User instruction: keep analysis → behavioral specification → independent implementation → validation, while checking inherited technical assumptions against this game's evidence.

- Coordinator wrote AGENTS.md and opened REQ-0001 (original collision boundary) and REQ-0002 (incomplete replacement ColDet).
- Agent 1 handles original fingerprint and BOOT-0001, with raw evidence in C:\Users\ADMIN\Boxer-analysis.
- Agent 3 audited replacement only: configure/compile PASS, executable link FAIL_LINK, behavioral equivalence BLOCKED. See validation/BASELINE-0001.md.
- Replacement source is unchanged. No Implementation Agent received original implementation material, and none has yet been assigned behavior lacking a specification.
- The initial internet queries for this title/source and ColDet did not identify an authoritative source repository or loader. No upstream identity inferred from those search results.

Next: recover approved matching ColDet source; establish the original collision loader/query boundary; write a small behavioral contract and collect a reference checkpoint before gameplay implementation.

## 2026-10-04 — User scope correction

User identifies ColDet as the original collision dependency and excludes Bullet. Restart analysis independently from original binaries/data and independently obtained upstream. Existing OpenBoxer work is preserved but excluded from evidence, implementation reference and the new baseline. Historical build findings do not create a requirement to repair that prototype. REQ-0002 is superseded for current scope. Agent 1 assigned fresh COLL-0001 investigation to confirm the actual original ColDet boundary and version without using legacy source.

## 2026-10-04 — Immediate task clarified: ColDet build

User wants ColDet built for the language/toolchain in OpenBoxer before further game investigation. This explicitly authorizes dependency/build repairs in the existing project; independent implementation role assigned with fresh context and without original-game evidence. Validate full library construction and collision queries plus clean Clang executable linkage. The prior prototype freeze remains an evidence restriction for game analysis, not an obstacle to this authorized dependency build.

BUILD-0001 completed: independent upstream ColDet 1.2 restored as a static CMake target, factory/header linkage unified, tritri compiled as C, modern C++ declaration-order portability fix. Implementation and independent Validator both clean-built ColDet, OpenBoxer and smoke with Clang 20.1.8; CTest 1/1 PASS. Provenance: third_party/coldet/README.md. Independent result: validation/BUILD-0001.md. Original ColDet version and original-game behavior remain separate investigation work.

## 2026-10-04 — Other-library inventory

Fresh original PE inventory confirms OpenGL/GLU, OpenAL, libvorbisfile → libvorbis/libogg, and Windows API boundaries. Bundled OpenAL wrapper/EAXAC3 use remains runtime UNKNOWN. Delay imports zero in all installed PE modules. Full report specs/platform/LIBS-0001.md; static IDA evidence recorded separately. User's replacement scope is game-owned code, not reimplementing upstream libraries.

## 2026-10-04 — Loader origin comparison

BMP/DIB family attributed HIGH to statically linked GLaux/TK using main decoder, palette helper, ANSI wrapper and error-popup control correspondence. Pinned independent comparison source and limitations recorded in specs/io/IO-0002-bmp-origin.md. Original pseudocode/source outside project. MS3D comparison delegated separately against independently acquired GameDev NeHe archive.

MS3D result: HIGH modified Brett Porter/PortaLib3D/NeHe Lesson31 family, compared independently against official GameDev source archive; changes and game map-loader call boundary recorded in specs/io/IO-0001-ms3d-origin.md. BMP existence-probe wrapper also matches NeHe LoadBMP helper. Both origins refine the earlier UNKNOWN inventory; exact forks remain unknown, no runtime original replacement performed in this investigation.

## First original-game function replacement — MS3D

IO-0003 implemented independently from the behavioral/ABI contract in replacement/ms3d, using x86 Clang. The guarded DLL replaces the original loader's virtual slot in C:/Users/ADMIN/Boxer-lab/ms3d only. Installed game files remain unchanged. All sixteen approved original fixtures produced exact normalized payload matches inside the original process, covering 1,996,935 bytes; actual replacement branch, return values, unchanged object spans, stack balance and floating controls were verified. Missing-file behavior delegates to the original. Independent CTest 2/2 PASS; evidence in validation/MS3D-0001.md.

Launcher startup was repaired to wait at the CRT-ready hardware checkpoint and drain pending debug events before detach. Natural startup succeeded and logged three successful replacement calls. The user subsequently confirmed gameplay works perfectly. This completes the first scoped replacement milestone; unknown/nonempty/malformed inputs still use original fallback, and comprehensive reload/destructor or prolonged gameplay equivalence is not claimed.

## WORLD-0001 — Game map loader replaced

Subsequent RENDER-0001 milestone: game-owned reloadTextures independently replaced through approved thunk1A64, with original uploader1799 and BMP/DIB/GL preserved. Independent Clang build/CTest4/4 PASS; actual original-process disposable differential18/18 PASS, including exact EAX0xCCCCCCCC for empty/negative counts, repeat/no-cache, ID0/FFFFFFFF, live-count/table/filename mutations, full unrelated state, thiscall stack/nonvolatile registers and FP controls. Final fixturePID14152 passed root live-route identity check before redirect and was terminated without normal resume.

Fresh naturalPID23820 passed the reload route guard, then executed three real replacements: lamp2 materials, map27 materials and light1 empty material. All29 nonempty filenames delegated to original uploader and received nonzero IDs; empty material stored0. Visible game-client GPU capture showed the textured combat scene; screenshots outside repo at C:/Users/ADMIN/Boxer-lab/ms3d/texture-menu-01.png and texture-menu-02.png. Test window was gone before the exit-check input; input automation refused to target foreground Firefox. Graceful teardown cause/telemetry therefore unverified; no recent matching Application1000 crash event observed, which is not proof of graceful exit. Full report validation/RENDER-0001.md. Validated texture DLL/launcher snapshot retained in lab/validated-texture-v1.

The user authorized the next game-owned replacement. Fresh original analysis established the complete orchestration contract in specs/world/WORLD-0001-map-load.md and disposable harness metadata. Independent implementation in replacement/ms3d/world.cpp and world_runtime.cpp preserves opaque original model/texture/light/scene/physics/RNG/audio callbacks, exact-width global stores, live condition re-evaluation and final callback return forwarding. It redirects the approved routing thunk while retaining the original body for fallback; natural replacement scope is selectors1..7.

Independent x86 Clang build and CTest3/3 PASS. Disposable original-process differential PASS2648/2648: complete normalized callback/state traces match exactly (23,879,472 bytes), return sentinel, actual replacement route, ESP, nonvolatile registers and FP controls all match. Root independently observed the live original thunk target before injection; final guarded fixture PID16316 was terminated after completion without resuming patched dependencies.

Fresh natural process PID23124 passed the same prepatch identity check, invoked WORLD replacement for selector1 and returned100. Both map1/map.ms3d and map1/light.ms3d used the independent MS3D loader. User confirmed “все ок” and supplied a screenshot of the rendered playable combat level, retained outside the repository at C:/Users/ADMIN/Boxer-lab/ms3d/world-gameplay-map1-user.png. The user is actively playing; no further automated input or termination performed. Additional real-map and teardown testing remain unverified rather than inferred. See validation/WORLD-0001.md.

## RENDER-0002 — Game texture uploader replaced

Independent uploader implementation follows specs/render/RENDER-0002-texture-upload.md. The game-owned thunk1799 now routes to upload.cpp through upload_runtime.cpp, retaining original BMP/DIB helper, GL/GLU and original image heap release. Prior MS3D/WORLD/reload replacements are preserved. Independent Clang build/CTest5/5 PASS; actual original-process differential22/22 PASS with exact paired callback/state traces, one-argument cdecl return/stack/nonvolatile/FP checks, null-image/null-pixels ownership cases, generatedID/status edge cases, repeat and live pointer/dimension/ID mutation. Guarded disposable PID23596 was terminated without normal resume.

Fresh native PID21228 passed prepatch target425420/module identity checks. Ordinary map1 executed29 uploader replacements,29 decoder calls,29 GLU calls with status0 and58 completed releases. Generated IDs5A..76 were returned and stored by independent reload. Visible foreground game-client capture outside repository at C:/Users/ADMIN/Boxer-lab/ms3d/upload-gameplay.png shows textured combat. User input was active during observation; no automated UI input/focus/termination was performed. The process is left for the user. No GPU-byte parity, native repeat or teardown claim. Report validation/RENDER-0002.md; artifacts retained lab/validated-upload-v1.

## RENDER-0003 — Map lighting preparation replaced

Independent replacement implements both approved lighting preparation stages A/B through guarded thunks1690/1974. Original downstream light selection and rendering remain in the game. Clang build and CTest6/6 PASS; all30 original-process differential pairs match exactly, including state/callback traces, ABI and x87 sticky flags. Fresh native PID3908 passed original-route guards for both thunks; map1 executed both replacements and all three records per stage match independently reconstructed asset-derived expected bytes. See validation/RENDER-0003.md and replacement/ms3d/lighting.cpp.

The automated client screenshot was black and does not establish visual PASS. The user explicitly confirmed that the map displayed normally during this native run. This is user-observed visual confirmation, distinct from the exact native data verification. GPU pixel parity, additional native maps and teardown remain unverified. DLL/launcher snapshot retained outside the repository in lab/validated-lighting-v1.

## RENDER-0004 — Light-record consumer replaced

Agent1 established a bounded noarg cdecl consumer at thunk16E0/body362A0: record scoring and mode-dependent directions through preserved original CPU math helpers, nested distance exchange ordering, unconditional first-three publication and seven live opaque dispatches. This is global record selection, not proven per-object selection. Independent Agent2 implementation selection.cpp/runtime follows specs/render/RENDER-0004-light-selection.md and preserves original math/wave/render dependencies and five prior replacements. Literal displaced-record parameter bits7, callback-live reads, 16-byte vector ABI and explicit x87 float32 stores are preserved.

Final x86 Clang build and CTest7/7 PASS. Root-owned initialized disposable PID14340 passed prepatch module/hash/route guards and all36 differential scenarios, including preserved real math/wave, deterministic mutations, mode branches, short/stale records, repeated sorting, callback return/ABI and floating-point witnesses. Complete final artifacts archived outside repository at lab/selection-final-fixture. Final DLL SHA256285DFCADC65C0D417937E64ADFF5A8427BDA7ACB85956A75AA814C40F63E536C; launcher F69ECBF3B64FDB9E46A17931B1E25DA89877EE2779757EED7A080668001F8A18. Snapshot lab/validated-selection-v1.

Natural root-owned guarded PID7432 (first frozen consumer build) rendered map1/mode1; final frozen PID8432 rendered map2/mode2. Both retained native pre/post calls1/2/3/120, seven real dispatch observations, count3, CW027F and MXCSR1FA0. Screenshots show textured lit combat; all five preceding routes remained active. One initial fallback per launch retained original handling of unsupported entry state. Archives lab/candidate-selection-v1/native-evidence and lab/selection-final-native. Root terminated its owned natural processes after capture; graceful teardown is not claimed.

Eight additional fresh disposable replay pairs used those real frame pre-states with original math and original wave intact, GPU dispatch isolated: mode1 PIDs1488/420/24220/16724 and mode2 PIDs13768/8756/1036/1000. Every pair PASS with actual replacement route, exact state/trace/EAX/ABI and invocation CW/sticky/MXCSR. Replay archives selection-replay-NNNN and selection-final-replay-NNNN. Snapshot header captured after the ABI wrapper has restored harness MXCSR1F80; native/actual invocation MXCSR1FA0 is separately recorded, so these timing boundaries are distinguished. Independent comparison/report in validation/RENDER-0004.md. Scope does not establish GPU pixel parity, arbitrary record counts, unsupported FP environments or full renderer reconstruction. Installed original executable hash remains unchanged77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6.

## RENDER-0005 — Independent map geometry drawing

Agent1 approved complete thiscall mesh/material immediate-mode renderer at thunk1B54/body26B00 with exact cached/live-source and raw-bit GL/opaque callback contract. Independent Agent2 implemented readable draw.cpp semantic operations with model layout in draw_model.hpp, dispatch/ABI metadata in draw_dispatch.hpp and x87/ABI adapters in draw_abi.S. Six prior algorithm sources and39 prior source/ABI/test files remain unchanged. Native identity gate supports only current map global185560; other models and unsupported inputs retain intact original fallback before effects.

Frozen x86 Clang DLL SHA256A99D34220583D60B32677C9B7916210C49A7C583A28C2F7E3DE8727F834F9A87. All8offline CTest targets PASS. Guarded disposable original PID18552 worker/launcher0 and31 actual synthetic original/replacement draw pairs PASS, with3 separate zero-effect preflight rejection checks. Exact899 callback events per side, full typed source snapshots/witnesses, pointer roles, handles/13slots, entryAL/finalEAX, actual routecounter and stack/nonvolatile/FP state independently verified. Artifacts lab/draw-final-fixture; report validation/RENDER-0005.md.

Guarded root-owned native original-mode PID18220 and replacement-mode PID7648 both rendered actual map2 with six preceding replacements active. Original/replacement calls1/2/3/120 each emit27295 ordered graphics events:28meshes/2989triangles/8967normal/UV/vertex calls. Every semantic event field matches independently (API/dispatch/argument bits/source pointer role/immediate payload), ordered digest48585047beab54d3. Complete pre/post source snapshots immutable. QueryAL1, finalEnable/EAX1, CW027F, x87sticky21 and MXCSR1FA0 preserved; ignored intermediate driver EAX residue is not assigned semantic meaning. Screenshots independently show textured lit Ment combat. Archives lab/draw-native-original and draw-native-replacement. Owned test processes closed after capture; no graceful teardown claim.

Two fresh guarded disposable CPU replays used original-nativecall1 and replacement-nativecall120 source snapshots (PIDs13960/14140): original/candidate outputs27295 events each match exactly and reproduce native semantic emissions, with ABI/FP/source witnesses PASS. Total33 distinct original/candidate comparisons plus3 guard-only tests; repeated synthetic cases during replay excluded from this count. Replay FP initializes controlled fixtures, so native/replay full FP parity is not inferred. GPU pixel/timing parity, map1 native and whole renderer reconstruction remain unclaimed. Original installed executable SHA remains77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6.

User-requested readability polish formatted only four new core/interface files, verified whitespace-only. Rebuilt DLLADB6BC4CE48E814BC3E46448528B8E551BC4F20CF65773440C9E910B7D7D2187 differs from validated frozen DLL only6 generated PE timestamp/checksum bytes; executable code and other bytes unchanged, offline8checks re-PASS. Validated frozen artifacts remain unchanged and retained lab/validated-draw-v1; updated source-hash manifest and formatting provenance describe this distinction.

## RENDER-0006 — Character frame interpolation and drawing — 2026-10-05

Replaced the game-owned frame morph and triangle emission unit at thunk RVA 0x1497 / body RVA 0x69A0. The readable semantic core is character.cpp; binary layouts, ABI, loader registration and observation are separate. Original animation advancement, linked parts, matrices, texture selection and gameplay remain dependencies. Seven earlier semantic cores retain their exact hashes.

Agent1 confirmed the IDP3/version15 BHM loader and allocation bounds. Root independently verified all eight lab asset hashes and lengths. Native admission uses successful loader registration, exact relative asset association, header and pointer/count witnesses, caller RVA 0x5FEF, finite frame inputs and CW 0x027F. Current supported native scope is untextured registered meshes in a disposable startup. Owner texture table capacity remains UNKNOWN; textured and unsupported native inputs use the original body before replacement effects.

Independent validation PASS covers 27 synthetic original/candidate pairs and ten coherent moving-pose CPU replay pairs. All ordered events, exact float32 arguments, EAX, source state, ABI and equal initial/final FP environments match. The ten replays compare 248548 accessor/GL events per side and reproduce actual native emissions. Five native captures per route compare 124264 GL events to an independent source oracle. Original and replacement sessions used different timed poses; equivalent-input evidence comes from replay. Root and validator inspected Boxer versus Ment battle screenshots. Prior map emission checks also pass.

Validated frozen DLL SHA256 808DC5A3F8C01DEA7CA7B26B26C27F04BA26A199FDBA063E32EDF9786C861E24 and launcher SHA256 584C352BAB0AB4FE0E33EFA780DD3A31AAD46248AE65D59D640CC0662C691223 are preserved in replacement/ms3d/frozen-render0006 and lab/validated-character-v1. Nine offline test targets pass. Source formatting has recorded code/constant identity evidence; formatted debug builds are distinguished from the frozen tested binary. Report: validation/RENDER-0006.md; source freeze: validation/RENDER-0006-implementation-freeze.json. Earlier diagnostic fallbacks and an interrupted replay timeout are retained separately and excluded from PASS.

The lab delivery launcher run-character.cmd verifies the executable, frozen binaries, manifest and eight assets before enabling native replacement. Its startup smoke confirmed actual replacement calls and visible animated characters; root stopped only its owned test process. Installed original executable hash remains unchanged. No whole animation-system, arbitrary asset, long-duration, pixel-parity or general lifetime-tracking claim is made.

### Runtime diagnostics regression — 2026-10-05

The user reported stalls around Play/Continue while interacting with test runs. Previous numerical PASS did not cover that menu transition. Root observed one successful legacy Play/pause/Continue route; no permanent hang was reproduced in that observation. The runtime nevertheless contained unnecessary synchronous JSON/snapshot IO and trace-only GL wrappers which saved FP state on every forwarded call even outside an active capture.

Ordinary runtime diagnostics and captures are now opt-in, and trace-only GL wrappers are omitted when no capture is requested. All eight replacements remain enabled and safety guards remain intact. Seven prior semantic core hashes and character.cpp remain unchanged. Nine offline targets pass. Candidate DLL F1D6C81914B18556BF1BD19C9062D17FC275E880618C57D49DDDF33365C72591, launcher 584C352BAB0AB4FE0E33EFA780DD3A31AAD46248AE65D59D640CC0662C691223. Root observed battle/pause in this candidate; the user independently confirmed that stalls disappeared. Exact attribution among diagnostics overhead and simultaneous test workload remains UNKNOWN; no crash or repeated restart claim is made.

The existing lab run-character.cmd delivery now uses this candidate. Its guard clears inherited capture/diagnostic/isolation flags, selects replacement mode for all eight units, validates assets and binaries, and restores the parent environment after launching the child. Historical 808D binaries and their differential evidence remain preserved separately; those 37 numerical comparisons are not relabeled as fresh F1D6 tests. Report: validation/RENDER-0006-continue-regression.md.

### ANIM-0001 — frame/time advancement — 2026-10-05

The original animation advancement thunk 0x4015AA/body 0x406810 is replaced for registered lower character models at the approved caller, finite admitted arithmetic, valid collection/index/rate, empty x87 stack, and predicted step count <=4. Upper models, larger steps and unsupported states conservatively retain the original route. Clip selection and next-frame preparation remain original. The semantic implementation follows approved behavioral specifications; ABI/layout/x87 operations are separate adapters.

Final DLL A6466E1813DBF7429FCAF041F4AFD9E3E77C169BA44492902CDF20849EEF60D9 and launcher 3B479849AFFDAF3988CFA157399D965FDF2A5D2F830B352932217A1C0CD85E84 are frozen in replacement/ms3d/frozen-anim0001 and delivered via lab/validated-animation-v1/run-animation.cmd. All11 offline targets pass; eight prior semantic core hashes remain unchanged. Forty original/candidate synthetic comparisons and eight original/candidate CPU replays exited successfully. Native pass-through PID2984 and replacement PID25232 each supplied four typed captures, including no-advance and advance cases; independent reports record the comparison policies.

Root observed an animated battle and pause/Continue return using the replacement capture run and a separate ordinary default-off guarded delivery run PID24900. No new JSONL/bin output appeared in the latter. Root stopped only these owned test processes. Installed original SHA256 remains 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6. No whole animation-system or long-run/performance equivalence claim is made. Native replay uses an identical clean FP environment for both CPUs; it does not recreate captured live status.

The saved IDA inventory for this exact executable records 3646 functions, 982 library-flagged, 288 named nonlibrary and 2376 unnamed nonlibrary. The nonlibrary remainder is not a confirmed count of game-specific functions.

### ANIM-0002 / ANIM-0003 investigation started — 2026-10-05

User authorizes two sequential replacements: selection by name and future-frame preparation. Agent1 confirmed the selector thunk RVA1C26/body6E40 (thiscall owner and request string), distinct from animation-file loader405440. Preparation belongs to whole parent thunk1D66/body5D40, which subsequently invokes validated ANIM-0001 progression. Root preserved all nine semantic core hashes in validation/ANIM-0002-prior-core-hashes.json; fresh Agent3 independently confirmed them. Fresh Agent2 receives only approved behavior specifications. These work items are not yet replacements or validation PASS.

### ANIM-0002 — name selection bounded PASS — 2026-10-05

Independent Agent3 confirmed24 original-process differential fixtures,23 optimized independent oracle cases plus callback-induced FP preservation, five natural original and six natural replacement observations across all three admitted stand/run/runb callsites, and eleven captured-state CPU replays. All compared model112 bytes, returns, time, ordered callback arguments/results and same-environment FP outputs passed. Natural observations each satisfy their own supplied state; they are not wall-clock synchronized inputs. Original comparator remains a preserved dependency; non-C locale was not newly exercised. Ordinary comparator callsite is unpatched; fixture/capture observer uses synthesized directCALL after root independently verifiesE8 and target.

Final stage1 DLL BC98D10182933A0EF692827AAEEFC14BED98FC78F720F95B8D97A32006C4DD85 and launcher5510BD7E1FE6103B740DA3D8876F4304E26D4A3260CB81EBB3626ABD0C6F253E frozen in replacement/ms3d/frozen-anim0002. Root stopped only owned native PIDs19832/12208 and fixture/replay workers exited normally. Nine prior semantic cores unchanged. Combined guarded delivery awaits ANIM-0003; prior validated-animation-v1 entry stays available.

### ANIM-0003 — preparation and composed advancement bounded PASS

Independent validation confirmed23 optimized oracle cases,34 original-process fixtures (26 unit and8 composed), three natural reference plus three natural replacement captures, and six captured-state composed CPU replays. Native replacement captures prove ANIM-0001 replacement delta1 and consumed trusted continuation delta1 inside the same parent call, with exact prepared/final state and callback/FP policies. Ten prior semantic cores remain unchanged. Final combined DLL23E7EB2BB53034BD1DEE053445E6EE04A7BC8288D44CD123C2C9D8ED317792C3 and launcher04BD1A41D8E6741C89E7C2F7DABDD32DDBC6FC826E17395633828E08AD0D80C2 are frozen under replacement/ms3d/frozen-anim0003. Combined build independently repeated clip24 fixtures/11 replay and preserved ANIM-0001 forty fixtures with PASS.

Delivery: lab/validated-animation-v2/run-animation.cmd. All13 CTests pass. Ordinary diagnostics-off ownedPID16212 showed battle, B/V movement, pause and Continue return; no new or modified JSONL/bin diagnostic files. Installed original unchanged. Unsupported selector routes and upper/invalid/exceptional preparation states retain original fallback; no whole animation-system, synchronized natural inputs, pixel or long-run performance equivalence claim.

### ANIM-0004 — strike/block selector activation bounded PASS — 2026-10-06

Evidence corrected the proposed upper-body stage: this build supplies/loads only eight whole-character lower.bhm assets, and the separate upper selector has no established active route. Original437110 instead invokes existing selector1C26/body6E40 for LEGS_LEFTHEAD/RIGHTHEAD/LEFTTORS/RIGHTTORS/LEFTBOK/RIGHTBOK/BLOCK. Added only these verified actor0 literal/caller routes and bounded idle observations; eleven semantic cores unchanged, no twelfth reconstructed function. Original dispatch, attack timing, input and damage remain original; otheractors/pain/death/win remain fallback.

Independent Agent3 PASS:32 exact selector fixtures,72 independent role-admission checks, original and candidate each16 selector+16 correlated pose observations (two per eight roles),64 same-input selector/pose CPU replays and34 preserved frame fixtures. Native candidate pose rows prove frames/A1/trusted deltas1 in correlated subsequent lower-model calls. Original reference counters0. Exact state/callback/EAX/FP/ABI comparisons and binary/input/output correlation pass. All14 CTests pass; all11core hashes unchanged.

Final DLL462EFA353BA8DB08E99205EBFA3E7D4A0038EFEC7CECF089C562ED118F40B96B and launcherCC04D6C6800D4F66CE273E01E04A23AC55371F7CCD46DA9C16679201D9384C71 frozen in replacement/ms3d/frozen-anim0004, delivered via lab/validated-animation-v3/run-animation.cmd. Diagnostics-off ownedPID21480 showed battle and pause/Continue return, with no new/modified JSONL/bin output. Installed original SHA unchanged; root stopped only owned tests. Reports retain per-input natural versus identical-input replay distinction and exclude full gameplay/long-run parity.


## 2026-10-07 — GAME-0001 fresh strike registration

Guarded whole-player-update replacement for admitted fresh Z/X/C strikes validated against original state, callback order, full FP and ABI. Natural hits/misses, recorded source replay, all previous animation routes and diagnostics-off Pause/Continue smoke pass. Eleven prior cores and installed original unchanged. Delivery: `C:/Users/ADMIN/Boxer-lab/ms3d/validated-gameplay-v4/run-strike.cmd`. Scope/report: `validation/GAME-0001.md`; independent verdict: `validation/GAME-0001-independent-final.md`. Delayed damage remains original; ColDet migration deferred.

## 2026-10-08 — GAME-0003 opponent single-strike initiation

Final frozen candidate v3 DLL295C9122B7A24AD7746ADCBE528604082F0B7F37593F31B1C0917552F29B5C13 adds bounded ordinary opponent Z/X/C initiation and the reached same-call timer tail at the whole opponent-update boundary. Incoming continuation, unsupported AI states and opponent selector retain original routes. Twenty offline targets,337 AI differential pairs,11 original-source replays, preserved60 damage pairs and native actual-dispatch/causal six-state observations pass independent checks; twelve previous semantic cores unchanged. Explicit combo reset supplement covers inactive timer up to1.6. No entire AI or ColDet migration claim.

Root packaged diagnostics-off delivery, photographed pause/Continue return and read actual DLL redirects/flags. User also participated in live fights; controlled equivalence is established separately by paired tests/replays. Guarded local entry: `C:/Users/ADMIN/Boxer-lab/ms3d/validated-gameplay-v6/run-ai.cmd`; earlier v5 damage delivery preserved. Scope, provenance, historical failures and independent reports: `validation/GAME-0003.md`. This local work item has not yet been published to GitHub.

## 2026-10-10 — GAME-0004 opponent attack continuation/completion

Bounded whole opponent-update replacement now advances and completes active Z/X/C attacks, including original threshold ordering, fatigue update/new lock, idle transition and inactive-combo reset. Unsupported states fall back before effects; reciprocal player hit consumption remains original. Immutable C8EA DLL passed22 own CTests,357 paired cases plus fallback,12 approved-source replays, preserved337AI/60damage regression pairs and exact native Z/X/C/idle witnesses. Twelve previous semantic cores unchanged. Root completed actual packaged diagnostics-off pause/Continue smoke; installed original unchanged. Local launch: C:/Users/ADMIN/Boxer-lab/ms3d/validated-gameplay-v7/run-continuation.cmd. Report: validation/GAME-0004.md. Service-limited independent final packaging review is not invented; final smoke is attributed to root. Not pushed to GitHub.
