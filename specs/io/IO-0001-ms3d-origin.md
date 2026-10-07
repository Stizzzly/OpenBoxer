# IO-0001 — MS3D loader origin and original map-loading boundary

Status: ANALYZED_PARTIAL. Agent 1, 2026-10-04. Static investigation; runtime equivalence unverified.

## Conclusion

Classification: MODIFIED_UPSTREAM, confidence HIGH, for the Brett Porter / PortaLib3D / NeHe Lesson 31 model-loader family. This is a family attribution based on several independent implementation choices and companion routines, not proof of the exact historical fork or direct copying from a particular archive revision. Exact original class names, original source revision, and complete ancestry: UNKNOWN. It is demonstrably not an unmodified instance of the compared VC Lesson 31 source.

## Evidence isolation and provenance

Original program.exe SHA-256: `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`. Original install: `C:\Program Files (x86)\Alligator Friends\Месть боксера. Московский криминалитет`. Fresh IDA session a6e1ce44 survey rechecked hash; preferred base 0x00400000. Original analysis database remains under `C:\Users\ADMIN\Boxer-analysis\fresh-coldet`. No legacy OpenBoxer source/assets were read or used; no replacement code was written.

Upstream obtained independently by cloning the [GameDev official NeHe archive](https://github.com/gamedev-net/nehe-opengl) into `C:\Users\ADMIN\Boxer-analysis\upstream-nehe`, commit `9f073e5b092ad8dbcb21393871a2855fe86a65c6`. Compared `vc/Lesson31/MilkshapeModel.cpp`, `Model.h`, `Model.cpp`. Their header identifies Brett Porter, copyright 2000; retained-notice condition is recorded, not interpreted as an unrestricted license. The [author's tutorial](https://nehe.gamedev.net/tutorial/model_loading/16004/index.html) independently identifies PortaLib3D origin. Its old VC ZIP link returned 404; the official source archive supplied the files instead. Archive commit is evidence of retrieved content, not the game's creation date.

Local upstream file SHA-256:

- MilkshapeModel.cpp: D6F3673A92BDF3A9BF8270C6D672B331EF550F6F90E341B4DCF8B24A553B20BE
- Model.cpp: 76A39A495CCF0FDDC3C56492ABD323C3456FC434022E01E2AABB5C490CC301E9
- Model.h: B27D4E13C5BAB859777F6AE873EB6C981FEE854F6F3E34DC69C59B60110D958C

## Distinctive comparisons

Confidence HIGH for static behavioral comparisons unless stated otherwise. Format section order alone is weak evidence; the combination below supports attribution.

| Fingerprint | Original observations | Comparison result |
|---|---|---|
| File acquisition | 0x4159FC old ifstream API; seek end/tell/seek start, allocate entire length, read/close, parse temporary buffer | Same acquisition sequence and older stream family as VC Lesson 31 |
| Vertex runtime representation | 0x415B33 copies bone byte first; 0x415B53 copies three floats to runtime +4; 15-byte disk stride, 16-byte runtime stride | Same non-packed runtime Vertex arrangement and assignment sequence |
| Triangle conversion | 0x415BD7–0x415C22 widens three indices into temporary integers and creates three temporary `1-t` values; ordered normal/s/t/index copies at 0x415C43/68/8D/AF | Same temporary-array strategy and copy order, beyond simply reading MS3D |
| Group representation | 0x415D39 skips flags/name, allocates 32-bit index list and widens disk indices, stores material index/count/pointer in 12-byte runtime record | Same runtime Mesh representation and parsing strategy |
| Material ownership | 0x415F97 strlen of disk texture name, allocate length+1, strcpy to owned pointer; disk stride 361 | Same owned texture-filename strategy; runtime structure modified |
| Base constructor | 0x4162F0 zeros eight count/pointer fields in mesh/material/triangle/vertex order | Same field-pair initialization order |
| Base destructor | 0x416420 frees group-index lists, then material filenames, then mesh/material/triangle/vertex arrays with count/pointer resets | Same companion ownership cleanup structure; added GL deletion |
| Texture reload | 0x416680 loops material filenames, nonempty calls loader via 0x401799; empty stores zero | Same reloadTextures decision structure |
| Derived interface | ctor 0x415890 delegates to base, then sets table 0x56220C. Table +4 points to loader thunk 0x401C21 | Consistent with virtual destructor then loadModelData interface; exact names UNKNOWN |

## Confirmed differences from compared upstream

1. Original calls strncmp at 0x415AB2 but discards its result. Instruction review at 0x415AB7–0x415AC2 confirms no conditional branch and that return EAX is overwritten. No version-3/4 rejection exists in this complete loader. Confidence CONFIRMED for ignored comparison result, HIGH for version-check absence within this routine. Malformed-file outcomes outside readable memory remain UNKNOWN.
2. Runtime triangle allocation stride is 76, whereas compared upstream structure accounts for 72 bytes on the ordinary 32-bit layout; meaning of additional original storage UNKNOWN.
3. Runtime material allocation stride is 80; texture identifier +72, filename pointer +76. Compared upstream layout places these after four float[4] arrays and shininess (76-byte ordinary 32-bit structure). Original copies 24 bytes from disk +80 into runtime +48, preserving additional disk material data; semantic use of added field UNKNOWN.
4. Original loader returns after material parsing and buffer cleanup; it does not call texture reload inside 0x4159B0. Game caller invokes reload separately through 0x401A64 -> 0x416680.
5. Original base destructor calls glDeleteTextures at 0x4164C2; compared upstream destructor does not.
6. Original model object allocation is 0x8D9C4, with count/pointer pairs near +0x8D9A4 onward. The large preceding storage and its purpose are game-specific candidates, meaning UNKNOWN; original ABI is not the small upstream Model layout.

## Inputs, outputs, and side effects of bounded loader

Proposed behavioral interface: load a static MS3D model into an existing object from a filename. Exact class name UNKNOWN.

Inputs: object pointer and filename. Return: byte/boolean-like zero on observed stream failure, one on completed parsing. Side effects: file access, temporary whole-file allocation, vertex/triangle/group/material allocations, object count/pointer writes, owned filename allocation, buffer cleanup. Allocation failure, truncated input, integer overflow, repeated loading, and constructor/destructor failure behavior: UNKNOWN pending a dedicated contract.

Loader stops after materials; it does not parse joints/keyframes or transform/animate vertices in this routine (HIGH). Bone identifiers are stored, but this does not demonstrate animation support elsewhere. Do not infer absence of game-wide skeletal animation. Compared Lesson 31 source likewise does not load joint/keyframe sections despite declaring disk joint structures. Rendering/animation routines have not been compared in this bounded task.

No original MilkshapeModel/Model/PortaLib/Brett RTTI or attribution string was found by the cached-string search; actual class identity remains UNKNOWN. Error text comments in upstream source are not original binary diagnostics and must not be copied into the game contract.

## Confirmed game boundary

Independent supporting NeHe correlation: archive `vc/Lesson31/Lesson31.cpp` LoadBMP helper tests null filename, uses text-mode fopen("r") only as an existence probe, closes it, then calls auxDIBImageLoad. Original 0x425390 independently analyzed here has that same unusual two-open decision sequence, invoking 0x492EDE after fclose. Confidence HIGH for sequence match; the short helper alone would be weak source attribution. In combination with loader/ownership fingerprints it supports the NeHe family conclusion. Root coordinator separately investigates the GLaux internals; this unit does not claim an independently completed GLaux source comparison.

Confidence HIGH: game function 0x42E970 allocates 0x8D9C4 objects, constructs via 0x401956 -> 0x415890, stores pointers in globals 0x585560 and 0x585564, calls virtual slot +4 for paths `base/maps/N/map.ms3d` and `base/maps/N/light.ms3d` for N=1..8, then separately reloads textures via 0x401A64. Derived vtable 0x56220C +4 contains 0x401C21 (CONFIRMED integer read); thunk targets 0x4159B0. First map slot call is 0x42EA23, first light call 0x42EB96. Return checking appears absent in the observed caller; validate complete caller before defining failure propagation.

Another constructor/reload caller 0x448060 is a candidate for object/prop loading, confidence LOW for semantic label; investigate independently.

## Remaining work and validation

Exact upstream fork and compiler provenance UNKNOWN. Rendering and transform behavior UNKNOWN. Next unit: observe one original map load at 0x42E970 with N=1, snapshot counts, record input file hash and material texture requests, and verify failure propagation. Record position values as original float32 bits initially (EXACT extraction); cross-compiler numerical tolerance remains UNKNOWN until arithmetic checkpoint testing. Implementation readiness BLOCKED for malformed-input compatibility and complete lifecycle; family attribution does not approve implementation from pseudocode.

Reproducible evidence: IDA survey, decompile(0x4159B0), disasm at ignored comparison result, ctor/destructor/reload analyses (0x415890, 0x415960, 0x4162F0, 0x416420, 0x416680), thunk/vtable xrefs and get_int(0x562210), analyze_function(0x42E970), independently retrieved upstream files. No assembly or decompiler body is included in this specification.
