# OpenBoxer

> **Validated bounded scope (2026-10-08):** GAME-0002 ordinary-Z damage, block and reaction passed final comparison and diagnostics-off delivery checks. Generic Damage remains original by default. See [report](validation/GAME-0002.md).

An experimental behavioral reimplementation of **Месть боксера. Московский криминалитет**, developed by [Stizzzly](https://github.com/Stizzzly) with **AI-assisted co-development by OpenAI Codex**.

This is an incremental replacement DLL and guarded launcher for the original 32-bit Windows game, **not a standalone rebuilt game**. Supported game-owned functions are replaced in a disposable test copy; unsupported states fall back to the untouched original function. See [credits](CREDITS.md) and the [Russian introduction](README.ru.md).

## Current scope

- MS3D loading; map loading and drawing; texture loading/upload; map lighting.
- Character pose/vertex processing; animation selection, frame progression and future-frame preparation.
- Verified activation for attack/block animations.
- Fresh Z/X/C strike registration within a bounded whole-player-update path. Strict distance intervals are `(1,4)`, `(1,5)`, `(1,6)`; misses still start the attack animation. Ordinary-Z delayed blocking/damage now have a bounded validated opt-in replacement.

GAME-0001 validation: 108 paired original/replacement scenarios, exact state/callback/FP comparisons, register/stack checks, admission rejection tests, real hit/miss observations and source-correlated replay. Fourteen CTest checks pass. These are bounded results, not proof that the entire game has been reconstructed. See [final independent report](validation/GAME-0001-independent-final.md).

## Source layout

- `replacement/ms3d/`: current independently implemented replacements, binary adapters, launcher and tests.
- `specs/`: behavioral contracts and evidence levels, without copied original implementation.
- `requests/`: unresolved and resolved research questions.
- `validation/`: written reports and independent checking tools. Large native observations and local capture files are excluded; historical reports may reference unavailable local evidence paths.
- `third_party/coldet/`: independently obtained ColDet 1.2 source, documentation and LGPL notices. The game has **not** been migrated to this build. ZIP download archives are not included; extracted upstream source is retained.

The old standalone prototype and bundled SDL binaries are excluded from this first publication; the top-level CMake entry builds the current replacement project.

## Build

Windows, CMake >= 3.20, Ninja, Clang and an i686 MinGW sysroot are required. The tested environment uses Clang 20.1.8 and MSYS2 at `C:/msys64`. Review `replacement/ms3d/toolchain-i686.cmake` for local toolchain paths.

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
cmake -S replacement/ms3d -B build -G Ninja "-DCMAKE_TOOLCHAIN_FILE=$PWD/replacement/ms3d/toolchain-i686.cmake" "-DCMAKE_BUILD_TYPE=Release"
cmake --build build
ctest --test-dir build --output-on-failure
```

Outputs include `ms3d_replacement.dll` and `ms3d_launcher.exe`. No game files are needed for the synthetic CTest suite. Differential fixtures and native testing require your own supported original game copy.

## Running and compatibility

The current native launcher deliberately requires the lab directory `C:/Users/ADMIN/Boxer-lab/ms3d`, its `MS3D_TEST_COPY.marker`, the supported executable identity and approved model assets. These developer-specific paths are retained in the unchanged validated source; this publication is not a portable installer. Read the launcher and guards before attempting setup. Use a separate copy of your own game, not the installed original.

Supported original executable SHA-256:
`77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`

From the lab directory, after making the required test-copy setup:

```powershell
& '<absolute build path>/ms3d_launcher.exe' '<absolute build path>/ms3d_replacement.dll' replace
```

Native diagnostics/captures are opt-in. Remove `OPENBOXER_CAPTURE`, `OPENBOXER_DIAGNOSTICS` and test-isolation environment settings for normal play. Removing the injected replacement launch and starting the original executable restores ordinary behavior; the installed original is not patched on disk.

## Licensing

Newly written project code is available under the [MIT License](LICENSE). The MIT grant excludes third-party material and original-game content. Third-party ColDet remains LGPL-2.0-or-later; see its retained notices. Original game code and assets are not granted by this repository.
