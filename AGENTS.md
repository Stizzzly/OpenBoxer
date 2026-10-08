# OpenBoxer — reverse engineering contract

Target: Месть боксера. Московский криминалитет.
Original install: C:\Program Files (x86)\Alligator Friends\Месть боксера. Московский криминалитет
Replacement project: C:\Users\ADMIN\CLionProjects\OpenBoxer

## Required role separation

1. Reverse Engineering Agent: original binaries, IDA, upstream comparisons and runtime observations; writes behavioral specifications, never replacement source.
2. Implementation Agent: specifications, replacement source, compiler, tests and approved upstream dependencies only. Never expose original binaries, IDA databases, disassembly, pseudocode, original implementation dumps or screenshots to this agent. Spawn with fresh context and explicitly supply approved specifications. Do not inherit the RE conversation.
3. Validation Agent: compares observable outputs against specifications and original observations; never demands source similarity. May audit the replacement independently. Report PASS, FAIL with category, or BLOCKED.

An agent that has viewed original implementation must not subsequently become Implementation Agent. The coordinator may route specifications and record evidence, but must not implement replacement behavior after examining original implementation. Raw analysis and decompiler output belong outside this project, under C:\Users\ADMIN\Boxer-analysis. Behavioral specs may record addresses, call names, field offsets and confidence, but must not embed copied assembly or decompiler bodies.

## Evidence and uncertainty

Use CONFIRMED, HIGH, MEDIUM, LOW, UNKNOWN for important conclusions. Separate observed fact from hypothesis. Every claim needs provenance: binary SHA-256, module, address/RVA when relevant, observation or reproducible tool output. Timestamps, linker versions, imports and strings alone do not prove compiler or engine identity.

MSVC 2010, ETS2's VC9 evidence and Bullet 2009–2011 are inherited template assumptions, not evidence for this game. Original compiler, ABI, collision library version and engine start UNKNOWN until investigated. Fingerprint actual dependencies first; compare available upstream sources before reverse engineering library internals. Classify compared units as UPSTREAM_MATCH, MODIFIED_UPSTREAM, GAME_SPECIFIC or UNKNOWN. Matching names do not establish matching behavior.

User direction (2026-10-04): restart investigation from the original game. ColDet is the dependency identified by the user; do not pursue Bullet. Independently verify how ColDet is used and its version/modifications. Existing OpenBoxer source and assets are preserved legacy work, excluded from behavioral evidence and from the new implementation baseline. Do not read or reuse them as an analysis reference. Obtain upstream ColDet independently with recorded provenance and license.

## Specifications and requests

Use stable IDs: BOOT-xxxx, IO-xxxx, COLL-xxxx, GAME-xxxx, RENDER-xxxx.
Each spec includes status, confidence, observed behavior, inputs, outputs, side effects, unknowns and evidence. Record known external calls and parameters. Unknown field meaning stays UNKNOWN.
Implementation implements approved behavioral contracts, never guesses missing compatibility behavior. Create requests/open/REQ-xxxx.md with related spec, question, evidence needed and Blocking YES/NO. Resolve through investigation and update the spec before implementation.

## Validation

Build success does not imply behavioral equivalence. Prefer small deterministic scenarios, asset-loading checkpoints, collision query inputs/results, then short gameplay traces. Report original and replacement input/output provenance. Missing original observations means BLOCKED, not PASS.
Each numerical comparison must choose EXACT, ABSOLUTE_EPSILON, RELATIVE_EPSILON, ULP or BEHAVIORAL with justified thresholds. Compiler, x87/SSE, rounding and evaluation differences remain hypotheses until examined. Direct communication with original structures requires an explicit separate ABI/layout contract; do not assume Clang compatibility.

## First milestones

1. Fingerprint program.exe and dependency DLLs; confirm architecture, PE layout, imports and packing indicators. Establish correct IDA target before analysis.
2. Preserve the existing prototype; its completed audit is historical only and does not define the new work. Investigate the original afresh.
3. Identify the original map loader and collision creation/query boundary without referencing legacy OpenBoxer code. Compare original ColDet exports and layouts with independently obtained historical upstream sources.
4. Approve one behavioral spec with a reproducible original observation, then give only that spec to a fresh Implementation Agent.
5. Validate the same short scenario and retain failures as investigation requests.

Work in small units. Never claim the whole game is reconstructed from a working map viewer. Do not modify the installed original while collecting baseline evidence.


## Dependency build authorization — 2026-10-04

The user clarified the immediate task: build ColDet under the project's C++/Clang toolchain. Existing project source may be read and updated for dependency/build integration. The earlier legacy freeze applies to game reverse-engineering evidence, not this explicitly requested build repair. A fresh Implementation Agent handles source changes using project interfaces and independently obtained upstream code; no original-game or IDA material is supplied. Scope: complete ColDet library, CMake integration, clean build and collision smoke checks. Game-behavior implementation and equivalence are outside this build task.

## Current authorized game replacements

The user now authorizes RENDER-0003, the map lighting preparation boundary currently represented by WORLD callbacks 0x1690 and 0x1974. Agent1 must establish each callback's actual behavior and floating-point policy before implementation. Preserve previous four replacements and opaque dependencies; require original-process state/callback/numerical comparison and natural game validation. Do not infer semantic field names from call order alone.

The user subsequently authorizes RENDER-0002, the game-owned texture uploader formerly preserved by RENDER-0001. Use specs/render/RENDER-0002-texture-upload.md after Agent1 approves it. Preserve the original BMP/DIB helper/decoder and real OpenGL/GLU dependencies, with independent implementation, disposable callback-isolated differential tests and natural game validation. Existing MS3D, WORLD and reload replacements remain intact.

The user additionally authorizes RENDER-0001, the original game-owned reloadTextures function. Use its forthcoming approved behavioral/ABI specification under specs/render. Preserve the original BMP/DIB decoder dependency and the validated MS3D/WORLD baseline. Require callback-isolated original-process differential and natural game-route validation, maintaining the same role separation and test-copy guards.

IO-0003 MS3D is implemented and has scoped differential PASS plus user-reported gameplay smoke. The user now authorizes replacement of the game map-loading orchestration through WORLD-0001. Use specs/world/WORLD-0001-map-load.md as the independent implementation contract. Preserve original opaque graphics/audio/physics callbacks and original fallback for unsupported live inputs. Validate the original and replacement callback order/state in a disposable process with dependencies isolated, then validate the natural game route. Test-copy-only hash/module guards and required role separation continue to apply. Preserve the validated MS3D baseline.

## RENDER-0004 authorization — 2026-10-04

The user authorizes investigating and replacing the next game-owned lighting consumer after RENDER-0003, proposed as light selection for map objects. Agent1 must establish the actual role and a bounded behavioral contract before implementation; the proposed name is not evidence. Preserve all five validated replacements and opaque original rendering dependencies. Use fresh-context Agent2 exclusively with approved specifications, then Agent3 differential and natural validation. Installed original stays untouched; guarded test copy only.

## RENDER-0005 authorization — 2026-10-04

The user authorizes the next bounded replacement in map geometry rendering. Agent1 must establish the exact game-owned unit and observed materials/texture/vertex emission contract before implementation; proposed role is not evidence. Preserve six validated prior units and opaque dependencies. Fresh-context Agent2 implements approved behavior only; Agent3 compares original/replacement observations. Test-copy hash/module/route guards and original fallback before unsupported effects apply. Native test-copy menu/game validation is already authorized by the user in this conversation; do not request it again.

## RENDER-0006 authorization — 2026-10-04

The user authorizes character rendering, beginning with a bounded pose/vertex calculation or rendering unit established by Agent1 evidence. Do not assume standard MS3D skinning or infer field meanings from desired names. Preserve seven validated prior units and opaque animation/gameplay/frame/GL dependencies. Fresh-context Agent2 implements approved behavior only, separating readable semantic operations from binary layout/ABI; Agent3 validates equivalent state, emitted vertices/normals/callbacks, FP policy and natural actor routes. Guarded authorized test copy only; installed original untouched. Previous authorization to drive the test copy persists.

## ANIM-0001 authorization — 2026-10-05

The user authorizes proceeding to character animations, beginning with one bounded frame selection or progression unit identified from the original. Agent1 establishes its exact behavior, ordering, ABI and floating policy before implementation. A fresh-context Agent2 implements only the approved specification; Agent3 independently validates original/replacement state, callbacks, returns and short natural animation observations. Preserve all eight validated semantic cores and the ordinary diagnostics-off runtime baseline F1D6. Native captures remain opt-in; ordinary launch keeps every supported replacement active without test isolation or heavy tracing. Unsupported states fall back before replacement effects. Prior test-copy UI authorization persists; keep observation sessions short and do not mix a parked differential test with user gameplay.

## ANIM-0002 / ANIM-0003 authorization — 2026-10-05

User explicitly authorizes two successive stages: animation selection, then preparation of subsequent frames. Agent1 establishes actual original boundaries, behavior and separate stable specifications before a fresh Agent2 implements either stage. Do not assume each proposed stage is a distinct function without evidence. Preserve nine validated replacement semantic cores including ANIM-0001 and the diagnostics-off guarded delivery. Agent3 independently validates equivalent inputs/state/returns/callbacks/ABI/FP and root executes bounded original-process and short natural test-copy observations. Unsupported native states retain original before effects. Previously authorized test-copy UI control persists.

## ANIM-0004 authorization — 2026-10-06

User accepts the proposed next stage: upper-body animation selection for attacks, blocking and idle. Agent1 must establish the actual bounded selector and its relation to lower-model selection; the proposed description does not establish behavior or permit guessing attack/damage rules. A fresh-context Agent2 implements approved behavior only; independent Agent3 validates exact state/returns/callbacks/ABI/FP and root performs original differential and bounded natural observations. Preserve eleven semantic cores and validated-animation-v2 diagnostics-off delivery. Original installed game remains untouched, test-copy hash/route guards and conservative fallback persist. Previously authorized short test-copy UI control persists; do not manipulate or stop a user-owned running process.

### ANIM-0004 evidence-based scope correction

Independent Agent1 analysis and installed asset inventory show this build uses eight whole-character lower.bhm files; no upper assets are installed or loaded. The separate upper selector appears unused. The active bounded next work item is extending the already independently implemented ANIM-0002 name selector to verified attack/block callsites for the registered actor, preserving its semantic core. Full437110 render/state orchestration, input rules and damage remain original. ANIM-0004 records an activation-scope extension, not a new reconstructed game function or twelfth semantic core. Require newly admitted callsite/callback/state provenance and original/replacement natural replay validation before delivery.

## GAME-0001 authorization — 2026-10-06

User explicitly chooses registration of successful strikes as the next task, rather than migration to the rebuilt ColDet. Agent1 must find the actual active bounded whole function or predicate involved in recognizing a hit and establish observed conditions/order/outputs/side effects/ABI/FP before a fresh Agent2 implements it. The suggested distance/direction/frame/block checks are hypotheses only. Do not invent a partial-body boundary or assume ColDet involvement. Preserve eleven validated semantic cores and ANIM-0004 route activation, diagnostics-off validated-animation-v3 delivery, original installed executable and dependencies. Independent Agent3 validates equivalent typed inputs/state/returns/callbacks/FP/ABI; root executes guarded disposable original comparisons and short natural test-copy scenarios. Unsupported state falls back before effects. Previously authorized test-copy UI control persists, but do not manipulate user-owned live processes. ColDet migration is outside this task.


## GAME-0002 authorization — 2026-10-07

User authorizes the next step after GAME-0001: delayed pending-hit consumption, timing, blocking, health changes and reaction animation. Agent1 must establish an actual bounded whole function or legitimate predicate, initially ordinary Z hit/no-block, hit/block and miss, without assuming distinct function boundaries or ColDet involvement. Fresh Agent2 implements approved behavioral specifications only; Agent3 independently validates state/callbacks/FP/ABI against root-owned original observations. Preserve twelve validated semantic cores including GAME-0001, ANIM-0004 activation, diagnostics-off validated-gameplay-v4 and installed original. Unsupported entries fall back before effects. Short owned test-copy UI authorization persists; no user-owned process manipulation. GitHub publication is a separate snapshot and must not receive unvalidated changes automatically.
