# Independent IO-0003 loader replacement

This isolated x86 Clang project implements only the approved behavioral contract
`specs/io/IO-0003-ms3d-replacement-contract.md` and its sixteen immutable fixture
identities. It does not consume the legacy loader, game executable contents,
IDA databases, disassembly, or decompiled implementations as source material.
Module hashes, callback RVAs, object offsets and the slot RVA are supplied ABI
data. No original instructions are patched or copied.

## Build and independent tests

```powershell
$p='C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d'
& C:/msys64/ucrt64/bin/cmake.exe -S $p -B "$p/build" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$p/toolchain-i686.cmake" -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/ninja.exe
& C:/msys64/ucrt64/bin/cmake.exe --build "$p/build"
& C:/msys64/ucrt64/bin/ctest.exe --test-dir "$p/build" --output-on-failure
```

Clang 20.1.8 targets i686-w64-windows-gnu using mingw32 headers/libraries.
Executables and the DLL link their C++/pthread runtime statically. The DLL imports
ADVAPI32.dll, KERNEL32.dll and msvcrt.dll. Both pointer-size and field-block layout
have compile-time assertions. GNU ld currently emits duplicate RTTI-name-section
warnings for some executable stdlib exception types; build and actual execution
pass. Original ABI callbacks use explicit cdecl/thiscall attributes; no original
STL or exception objects are constructed by the replacement.

CTest covers an independently authored synthetic fixture with signed material
index, raw bone byte, unsigned 65535 index, exact position/material copies,
1-minus-t conversion, unmodified padding/texture ID, raw filename, unrelated
object state, ownership release, seven staged allocation failures, and parser
truncation guards. Those guards define only the independent parser's refusal;
unsupported original files are delegated to the preserved original loader.

## Launch test copy

Only `C:/Users/ADMIN/Boxer-lab/ms3d/program.exe` is accepted. The coordinator must
prepare that copy and `MS3D_TEST_COPY.marker`. The installed executable is never
started or modified by this launcher.

```powershell
& "$p/build/ms3d_launcher.exe" "$p/build/ms3d_replacement.dll" replace
```

Modes: `original` starts without injection; `pass-through` installs the slot and
calls the original; `shadow` preserves the original object and compares a
separately constructed scratch candidate; `replace` substitutes approved empty
objects with approved fixture bytes and ordinary numerical controls. All other
cases delegate to the preserved callable. Original-owned buffers exclusively
use the original allocate/release callbacks. A staging guard releases candidate
buffers on independent exceptions or null allocations before committing fields.
Allocator failure/foreign exception behavior and exact stream-runtime allocation
side effects remain outside the contract.

The launcher starts its own process under the Win32 debugger, arms a hardware
execution breakpoint at the supplied CRT-ready checkpoint RVA 0x4CD90, and
continues startup debug events. At that checkpoint it explicitly suspends the
main thread, removes the hardware breakpoint, and detaches the debugger with
kill-on-exit disabled. Detach drains queued startup events and retries within a
three-second bound to avoid the observed loader-event race. It then injects LoadLibrary, waits for bootstrap identity
and slot guards, changes only the virtual slot with VirtualProtect, and resumes
the primary thread. No original instructions are read or modified. Bootstrap
does not call the original allocator before CRT initialization. Failure terminates
only the launcher's own newly created process. Original and replacement C++ heaps
are never mixed for payload ownership.

## Differential fixture worker

The coordinator must first park an already initialized original test process at
the approved WinMain checkpoint after CRT initialization, with its main thread
sleeping. The injector does not create that checkpoint and does not inspect
original instructions. It rejects any PID whose image path differs from the
explicit test copy.

```powershell
& "$p/build/ms3d_launcher.exe" --attach <PID> "$p/build/ms3d_replacement.dll" --fixture
```

This requests replace mode explicitly. The worker creates two original-heap,
original-constructor initialized empty objects per manifest fixture, invokes the
preserved original callable on one and the installed virtual slot on the other,
and compares exact payload bytes with relocated pointers normalized. It verifies
actual replacement/fallback counters, slot identity, return values, unchanged
unrelated object bytes, ESP restoration, x87 control and MXCSR controls. The
seventeenth case exercises a nonexistent path through original fallback and
requires unchanged state and false results. It releases each buffer using the
approved non-GL cleanup adapter and then releases scratch objects. It never calls
the original destructor on uninitialized texture IDs.

Outputs in the test lab: `ms3d-fixtures.txt`, `ms3d-replacement.log`, and seventeen
pairs `fixture-NN-original.bin` / `fixture-NN-candidate.bin`. The binary format is
little-endian uint32 byte-length followed by bytes for each chunk: four counts
(groups/materials/triangles/vertices), vertex array, triangle array, then two
chunks per group (first eight record bytes and indices), then two per material
(first 76 bytes and null-terminated texture name). It includes untouched spans;
only pointer values are omitted. An independent validator owns report/dump review.

For a reproducible run without an external debugger, use
`ms3d_launcher.exe --fixture <absolute DLL path>`. It creates its own test process,
parks it at the same CRT-ready checkpoint, injects and runs the worker while the
main thread remains suspended, and terminates that owned process after completion.

Coordinator reported the first original-process run with sixteen exact matches
and the missing-path false result, x87=027F/MXCSR=00001F80. This statement is a
reported observation, not independent validation by the implementation agent.
Nonvolatile-register sentinel instrumentation, allocation-exception paths,
nonempty replacement, alternate floating environments, and long-running gameplay
remain separate validation scope.
