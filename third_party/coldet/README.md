# ColDet dependency — BUILD-0001

This directory supplies the complete independently obtained ColDet 1.2 dependency for OpenBoxer. This choice restores the project's public collision API and a compilable dependency; it does not identify the original game's ColDet version or establish game equivalence.

## Provenance

Retrieved on 2026-10-04 directly from the project's SourceForge release distribution (no mirror substitution):

- 1.2 listing: https://sourceforge.net/projects/coldet/files/coldet/1.2/
- Selected archive: https://downloads.sourceforge.net/project/coldet/coldet/1.2/coldet12.zip
- SHA-256: `AA557DD1FAE02976D51DC90A36395B93E2D57D21DB1CFFC73E1C367ED4F8D7CF`
- Comparison candidate: https://downloads.sourceforge.net/project/coldet/coldet/1.0/coldet_10.zip
- 1.0 SHA-256: `E9638520F85DDA2EE741886B2047A3FF923610C26F16D8E18CACC08D0FAA9100`

Both downloaded archives are retained intact. `upstream-1.2/coldet` is the unmodified extracted package including documentation and notices. `src` is the build copy. 1.2 was chosen as the available complete release exposing the same source API used by the replacement's model loader and collision queries. No original-game files, IDA output, or reverse-engineering specifications were used.

## License and distribution

Copyright (C) 2000 Amir Geva. Upstream source headers grant GNU Library General Public License version 2 or any later version. `COPYING` and `upstream-readme.txt` are retained, as are notices in each source file. The complete upstream package is retained as requested by its README. ColDet remains LGPL licensed; static linking does not remove its license requirements. This build change is not a binary release or a legal determination about distribution requirements.

## Build and API changes

- The CMake `ColDet` static target compiles all eight translation units listed by upstream's makefile: coldet, coldet_bld, box, box_bld, math3d, sysdep, mytritri and tritri.
- `tritri.c` is compiled as C, matching the `extern "C"` declaration at the upstream caller. No collision algorithm was replaced or invented.
- The target defines `WIN32` and empty `EXPORT`, selecting the Windows timing API without upstream's DLL import/export attributes for a static library.
- The only local edit to the upstream build-copy sources is three forward declarations in `src/math3d.h`: PitchMatrix3D, YawMatrix3D and RollMatrix3D, before their first use. Modern C++ lookup requires them. Original function definitions and calculations are unchanged.
- Project `src/coldet.h` forwards to the upstream public header. The factory therefore uses the upstream C++ linkage consistently instead of the legacy project's manually added C linkage. Existing source consumers remain buildable. Binary compatibility with any prior DLL is not claimed.
- OpenBoxer links ColDet and does not compile legacy duplicate collision sources from project `src`. Those legacy files remain on disk for preservation.
- C++17 is required, extensions disabled. SDL2_PATH is a cache variable with the previous bundled distribution as its default. No gameplay or rendering source was changed.

## Reproduction and checks

PowerShell from the project root, using the installed UCRT Clang toolchain:

```powershell
$env:PATH='C:\msys64\ucrt64\bin;'+$env:PATH
cmake -S . -B cmake-build-coldet-clean -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/clang.exe -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-coldet-clean
ctest --test-dir cmake-build-coldet-clean --output-on-failure
```

The smoke executable constructs/finalizes a two-triangle planar mesh, checks separated and intersecting spheres, checks ray hit/miss, and checks the reported ray contact coordinates with absolute epsilon 1e-5 for this simple upstream fixture only. Failures return distinct nonzero codes; checks remain active in Release builds. It needs neither SDL nor game assets. This is an upstream dependency/build smoke check, not differential original-game validation.

For the library alone, after configuration:

```powershell
cmake --build cmake-build-coldet-clean --target ColDet
```

In CLion, select the installed Clang/UCRT toolchain, use Ninja and a fresh build directory, then select the OpenBoxer or ColDet target. The CMake profile may set `-DSDL2_PATH=...` if the SDL distribution moves. Old configured build folders may retain compiler/toolchain caches; the fresh directory above avoids those caches.

Implementation verification on 2026-10-04: fresh Debug configure PASS with Clang 20.1.8; full 14-step build PASS including `OpenBoxer.exe`, `libColDet.a` and `coldet_smoke.exe`; CTest 1/1 PASS. The OpenBoxer graphical process was not launched. Independent validator results are recorded separately under validation.
