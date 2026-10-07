# RENDER-0003 map lighting

`lighting.cpp` implements both approved stages from
`specs/render/RENDER-0003-map-lighting.md`. It receives only the documented raw
model layout and typed callbacks. Original consumers and natural OpenGL calls
remain in the game. Existing MS3D, WORLD, reload and upload algorithms are unchanged.

Installation synthesizes relative tail jumps at approved thunks 0x1690 and
0x1974. Original bodies 0x27AF0 and 0x35D90 remain callable for fallback and
isolated comparison. The existing explicit test-copy, executable hash and module
guards apply. The coordinator independently verifies prepatch routes. This
implementation never reads, saves or translates original instructions.

Stage A copies material indices, positions, UVs and normals into 116-byte flat
records, preserving order, duplicates and the four-byte tail. Ordered x87
addition followed by division computes centers. Stage B writes the documented
44-byte prefix of each 56-byte global record, preserving its twelve-byte tail.
Its ordered center uses the specified float multiplier. The otherwise unused
material doubling retains observable masked x87 exception flags.

Independent semantic x87 adapters preserve the approved 53-bit intermediate
precision without float stores between arithmetic steps. Copies and GL argument
passing preserve raw float bits. A separate ABI bridge captures the final void
color callback's EAX residue. Each post-color diffuse component rereads both the
flat material index and model material-array pointer independently.

`OPENBOXER_LIGHTING_MODE=replace` enables supported native inputs; other modes
call the original bodies. Native guards require readable nonoverlapping source
arrays, valid memberships and indices, matching membership/triangle totals,
finite supported arithmetic inputs and x87 control 0x027F. Capacity is 5000 flat
records for A and three global records for B. Unsupported inputs fall back
before writes. Synthetic fixtures additionally exercise specified zero and
negative count artifacts through the installed hook under isolated mode.

## Isolated validation

The coordinator runs the disposable initialized test-copy process:

```powershell
$p='C:/Users/ADMIN/CLionProjects/OpenBoxer/replacement/ms3d'
& "$p/build/ms3d_launcher.exe" --lighting-fixture "$p/build/ms3d_replacement.dll"
```

The launcher parks main before injection and always terminates its owned fixture
process afterward. Typed GL recorders avoid GPU calls. Twenty-eight independent
synthetic scenarios cover count edges, group ordering and duplicates, rounding
and cancellation, overflow flags, repeated writes and live callback mutations.
The synthetic installed-schema case is independently authored structural input.
Two additional comparisons parse the approved map1 light fixture into separate
original-heap models through the approved original loader callback, then compare
A and B. Cleanup uses the approved buffer release adapter, avoiding GL destructors.

Comparisons invoke intact original bodies versus actual installed candidate
thunks and verify genuine replacement counters. They compare full normalized
model/source/global state, callback order and raw arguments, EAX, ESP,
nonvolatile-register sentinels, x87 control/exception bits/stack balance and
MXCSR. Binary witnesses and summaries are written in the explicit lab directory.
Offline CTest runs the independent synthetic oracle; original-process parity is
reported separately by the coordinator and Validator.

## Natural observations

`lighting-replacement.log` records route counts, centers, material/color bits,
global count and FP metadata before downstream consumers execute. Native
`lighting-native-A/B-NNNN.bin` files contain nine little-endian uint32 header
fields: magic 0x33474C52, unit 1/2, call number, saved EAX, record count, x87
control, x87 status, MXCSR and payload byte length. A captures the first up to
three produced 116-byte records; B captures the full three-record 168-byte
global window. Logging returns the saved result, including B's driver residue.
