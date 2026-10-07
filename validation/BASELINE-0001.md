# BASELINE-0001 — Independent replacement audit

Date: 2026-10-04
Role: Agent 3 (Validator)
Scope: OpenBoxer replacement CMakeLists.txt, main.cpp and src C++ code. Assets and stb implementation excluded. No original executable, IDA, disassembly or decompiler material accessed. Replacement sources unchanged.

## Results

- CMake configuration: PASS (CONFIRMED).
- Translation-unit compilation: PASS, seven objects (CONFIRMED).
- Complete executable build: FAIL_LINK (CONFIRMED). No successful baseline executable produced.
- Behavioral equivalence: BLOCKED. No original observation traces, approved behavioral specifications, paired input vectors or comparison policies supplied. Static findings below are replacement defects/risks, not proven original-versus-replacement mismatches.
- Git: not a repository (CONFIRMED): git rev-parse --show-toplevel reports no .git in this directory or parents.

## Reproduction

PowerShell, project working directory, PATH prefixed with C:\msys64\ucrt64\bin:

```powershell
cmake -S . -B cmake-build-baseline -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/clang.exe -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-baseline
```

Toolchain observed: Clang 20.1.8, CMake 4.1.0, Ninja, bundled MinGW SDL2. Configuration completed; linker failed with undefined references to newCollisionModel3D, my_tri_tri_intersect(Triangle const&, Triangle const&), and tri_tri_intersect.

## Findings

1. **P1 — Missing collision implementation prevents linking.** CMakeLists.txt:17 lists translation units but omits src/mytritri.cpp, whose line 27 defines my_tri_tri_intersect. src/coldet.h:158 declares newCollisionModel3D, used at src/MS3DModel.cpp:32; no definition exists in supplied src C++ files. CollisionModel3DImpl constructor/finalize/addTriangle/setTransform and tree construction also have declarations without supplied definitions. Confidence: CONFIRMED for observed unresolved symbols; HIGH for completeness audit.
2. **P1 — C/C++ linkage mismatch prevents linking.** src/box.cpp:228 declares tri_tri_intersect with C linkage, whereas src/tritri.cpp:200 defines it with default C++ linkage. Confidence: CONFIRMED.
3. **P1 — Uninitialized owning pointer on construction.** src/MS3DModel.cpp:7-13 initializes other pointers but omits collisionModel; destructor at line 20 reads/deletes it. Constructing and destroying without Load produces undefined behavior. Confidence: CONFIRMED static analysis; runtime reproduction not executed.
4. **P1 — Unchecked binary reads and indices.** src/MS3DModel.cpp:40-82 and :101 ignore fread results; counts can remain indeterminate after truncation, texture strings need not terminate, and :125-130/:163-170 use triangle and vertex indices without bounds validation. Truncated or corrupt files can produce excessive allocations, invalid reads or crashes rather than a clean false return. Version read at :41 is not validated. Confidence: HIGH; malformed-file fixture execution pending build repair.
5. **P2 — Reload leaks and unsafe partial state.** src/MS3DModel.cpp:32, :46, :56, :70, :89-90 overwrite owning pointers without releasing prior resources. Failed reload can leave mixed old/new counts and resources. Default copy operations also duplicate owning pointers (src/Md3DModel.h:52), permitting double destruction. Confidence: HIGH static analysis.
6. **P2 — Working-directory-dependent asset startup.** main.cpp:52/:61 resolve base paths against CWD; supplied assets reside under src/base, while CMakeLists.txt post-build step only copies SDL2.dll. Launching from project root or clean baseline build directory cannot find the selected map. main.cpp:61 logs failure and continues collision queries instead of stopping or choosing documented fallback. Confidence: CONFIRMED for path/layout; resulting collision behavior UNKNOWN until missing implementation exists.
7. **P2 — GL resources outlive context.** main.cpp:60 creates level with scope through function return; :164 deletes GL context before level destructor invokes glDeleteTextures at src/MS3DModel.cpp:25. Confidence: CONFIRMED ordering; driver-specific failure mode UNKNOWN.
8. **P2 — Texture row/channel mismatch.** src/TextureLoader.h:14 preserves input channel count, but :28 treats every non-four-channel image as RGB. One/two-channel data cannot satisfy that upload format. :31 also leaves default GL_UNPACK_ALIGNMENT=4, so tightly packed RGB rows whose width*3 is not divisible by four are uploaded with incorrect row stride. Confidence: HIGH; rendering fixtures pending.
9. **P2 — Sphere contact normalization discarded.** src/coldet.cpp:391-394 calls P.Normalized() and ignores returned value; src/math3d.h:64 returns a new vector. Output point therefore depends on center distance instead of normalized direction. For C1=(0,0,0), C2=(2,0,0), r1=r2=2, observed source formula yields P=(-4,0,0), outside sphere 1. Confidence: CONFIRMED source semantics; intended original contact policy UNKNOWN.
10. **P2 — GL context failure unchecked.** main.cpp:34 may return null and subsequent GL calls proceed. SDL attribute errors are also ignored at :22-24. Confidence: HIGH.

## Next checkpoints

1. Restore complete clean linking, then repeat this exact Clang build. Keep build PASS distinct from equivalence PASS.
2. Startup from project/build/asset CWD; missing map, missing texture, failed GL context, quit/escape and resource teardown. Record exit result, messages and allocations.
3. Loader fixtures: minimal valid model, every truncated section, invalid signature/version, invalid triangle/vertex/material indices, unterminated texture name, reload and destruction-before-load. Check no invalid reads/leaks using an available memory instrument.
4. Collision vectors: separated/contact/penetrating sphere pairs; face/edge/vertex/degenerate triangles; ray origin on/in/behind shapes; exact tangency; negative/zero/non-finite inputs. Approved specifications must define expected cases before equivalence can be assessed.
5. Input/time checkpoints: equal held-key duration at several frame cadences. main.cpp currently adds fixed movement/rotation each frame (:71-72/:88-121) and delays 16ms (:154), so wall-clock motion is cadence dependent; compatibility policy UNKNOWN.
6. Agent 1 supplies original behavioral specifications and trace fixtures through the approved boundary. Compare return values, selected triangle, contact state, camera state, callback order and failure handling at short deterministic checkpoints. Integer/enums use EXACT; float policy and tolerances remain UNKNOWN until justified by evidence. Do not invent epsilon values.

No differential tests were run, and no claim of original behavioral compatibility is made.

## BUILD-0001 advice from replacement evidence only

Minimal direct fixes for two observed link errors: add src/mytritri.cpp to the executable target, and give tri_tri_intersect declaration and definition consistent linkage (the existing caller declares extern "C", while the definition is C++). Changing either side consistently is mechanically possible; a shared declaration is preferable to duplicated declarations.

A factory adapter `return new CollisionModel3DImpl(static_model);` matches the declared public factory and declared constructor, but is NOT a sufficient build repair: supplied source contains no constructor definition or complete implementations of finalize, addTriangle(Vector3D...), setTransform(Matrix3D...), BoxedTriangle construction and BoxTreeInnerNode construction methods. The adapter would move failure to these symbols/vtable dependencies. Restore the missing matching ColDet implementation from an approved source, or specify and implement this missing dependency explicitly. Do not label a stub that bypasses collision as a compatibility fix. Namespace macros in sysdep.h are empty for this WIN32 build, so namespace mismatch is not the observed cause.
