# BUILD-0001 — Independent ColDet build validation

Date: 2026-10-04. Role: Agent 3 Validator. Specification: specs/build/BUILD-0001.md.

## Result

**PASS** for authorized dependency/build task (CONFIRMED).

- Independent fresh Debug configure: PASS.
- ColDet static library, OpenBoxer executable and coldet_smoke executable: PASS, all 14 Ninja actions completed.
- CTest: PASS, 1/1 test, 0 failures; fixture ran in 0.02 seconds.
- Original-game behavioral equivalence: BLOCKED, outside this task. No original implementation, IDA or reverse-engineering specifications were accessed.

No replacement source was changed by this validator. Validation output resides in cmake-build-validation. No application runtime was required or executed.

## Exact reproduction

PowerShell from C:\Users\ADMIN\CLionProjects\OpenBoxer:

```powershell
$env:PATH='C:\msys64\ucrt64\bin;'+$env:PATH
cmake -S . -B cmake-build-validation -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/clang.exe -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build cmake-build-validation
ctest --test-dir cmake-build-validation --output-on-failure -V
```

Observed Clang C/C++ 20.1.8 and CMake 4.1.0. CTest printed `ColDet construction/finalize/sphere/ray smoke PASS`. Clean configuration and build exit codes were zero. Legacy collision translation units are excluded from OpenBoxer; target links the separate ColDet static library (CMakeLists.txt:8-21). The library includes the complete eight upstream translation units, including construction files and C-compiled tritri.c.

## Independent smoke assessment

Meaningful geometry checks in tests/unit/coldet_smoke.cpp (CONFIRMED):

- Factory creates an owned non-null collision model; reserves two triangles, adds a square in z=0 spanning x,y=[-2,2], and finalizes it.
- Sphere at (0.5,0,0.25), radius 0.5 intersects the plane within the square; sphere at (0.5,0,2), radius 0.5 misses.
- Downward ray from (0.5,0,2) hits and reports (0.5,0,0); ray from (5,0,2) misses the square.
- Exact boolean expectations; ray point uses absolute epsilon 1e-5 per coordinate. These are simple binary-exact geometric inputs, and the tolerance is conservative for an isolated short float calculation. This is a fixture tolerance, not an original-game physics tolerance.
- Checks use explicit failure returns instead of assert, so they remain enabled independently of NDEBUG. RAII destroys the constructed model.

Coverage limits: one identity-transform planar mesh, sphere hit/miss and closest-ray hit/miss/contact. Does not exercise model-model collisions, translated/rotated/static models, segment boundaries, degenerate triangles, tangency, negative/non-finite inputs, timeout behavior, malformed game assets, SDL/OpenGL startup or gameplay. The test establishes usable construction/query integration, not comprehensive library correctness or version equivalence with the original game.

## Provenance and portability audit

third_party/coldet/README.md records SourceForge release 1.2 URL, archive identity, license and API/build changes. Independently calculated archive SHA-256 matches the recorded value:

`AA557DD1FAE02976D51DC90A36395B93E2D57D21DB1CFFC73E1C367ED4F8D7CF`

COPYING, original readme, intact archive and extracted upstream package are present. This validates local retained provenance consistency; this validator did not independently re-download the remote archive.

Compared every corresponding build-copy source file against retained upstream source using git diff --no-index. Only math3d.h differs: three inline rotation-helper forward declarations before their first use. No algorithm/body changes observed (CONFIRMED). C compilation of tritri.c preserves the upstream caller's C linkage. Empty EXPORT and WIN32 select static Windows compilation; forwarding project src/coldet.h restores upstream factory C++ linkage consistently. API consumers compile and link; previous DLL ABI compatibility is not claimed.

No blocking findings for BUILD-0001 scope. Historical prototype risks in BASELINE-0001 remain outside this dependency repair and have not been retested or declared fixed.
