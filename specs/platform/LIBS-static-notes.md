# Static component inventory — original program.exe

Agent 1, 2026-10-04. Bounded fresh investigation; no replacement implementation or legacy OpenBoxer source/assets used.

## Provenance and limits

Target original `C:\Program Files (x86)\Alligator Friends\Месть боксера. Московский криминалитет\program.exe`; SHA-256 `77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6`. Fresh IDA session a6e1ce44 on unchanged copy under `C:\Users\ADMIN\Boxer-analysis\fresh-coldet`. Survey reverified hash and preferred base 0x00400000 (CONFIRMED). Addresses below are preferred VAs. IDA library names are signature-derived identifications, not recovered source symbols; exact library versions and modifications remain UNKNOWN without independent upstream comparisons.

## Findings

| Component | Evidence | Conclusion / confidence |
|---|---|---|
| Microsoft CRT, including debug heap | IDA names `_fread` 0x4955D0, `_fopen` 0x495890, `_malloc` 0x4962D0, `__malloc_dbg` 0x4962F0, `__CrtCheckMemory` 0x4973F0, `__CrtDumpMemoryLeaks` 0x498010, `__CrtDbgReport` 0x49CC20 | Statically present Microsoft runtime family HIGH; exact compiler/runtime release UNKNOWN |
| Microsoft C++ exception runtime | `___CxxFrameHandler3` 0x495E70, `___InternalCxxFrameHandler` 0x49E620 | Statically present C++ EH runtime HIGH; cannot infer uniform compiler provenance from this alone |
| C++ standard library locale/iostream | `std::ios_base::clear` 0x4AEB80, `std::locale::classic` 0x4AFAF0, `std::locale::_Getfacet` 0x4AFBD0; many decorated std stream-buffer/file-buffer routines around 0x4B3000–0x4B5000 | Static C++ standard library family HIGH; exact upstream/vendor release UNKNOWN |
| Separate older iostream API family | MS3D routine calls names `ifstream::ifstream(char const*,int,int)`, `istream::tellg`, `istream::seekg`, without std namespace | Older iostream API presence HIGH; mixed library provenance hypothesis MEDIUM, version UNKNOWN |
| MS3D model parsing | `MS3D000000` at 0x562218 referenced at 0x415AA9 inside 0x4159B0. Routine opens filename via ifstream, reads file buffer, starts record parsing at byte 14, walks 15-byte first records and 70-byte second records. Map path `base/maps/1/map.ms3d` at 0x5636DC references game function 0x42E970 | MS3D parsing capability HIGH; third-party loader identity UNKNOWN. Format use does not prove a named library |
| BMP/DIB image conversion | 0x4934CC maps filename with CreateFileA/W, CreateFileMappingA, MapViewOfFile; tests 0x4D42 (`BM`), processes DIB, uses CreateDIBSection/SetDIBits/GdiFlush. `Image file conversion error.` string 0x56525C referenced at 0x4937A4. Caller 0x492EDE | Static Windows BMP/DIB loader HIGH. GLaux origin is a comparison candidate only (MEDIUM hypothesis); exact upstream match UNKNOWN |
| TGA loading | Numerous original `.tga` path literals, e.g. 0x562524 (`base/textures/tex06.tga`) | Asset naming CONFIRMED; actual decoder/library identity UNKNOWN |

CRT, EH, and standard-library routines belong in a runtime-support category. Their presence and IDA function counts must not be reported as reconstructed game logic or a named game engine.

## Unresolved candidates

Bounded IDA cached-string regex `jpeg|jfif|libpng|zlib|inflate|deflate|glut|glaux|SDL|IDP3|Huffman|Premature|Bogus|SOI` returned no matches. Bounded function-name regex `jpeg|png|inflate|deflate|glut|aux|sdl|ms3d|md3` returned no names. Confidence in these tool results CONFIRMED; conclusion for static JPEG/IJG, PNG/libpng, zlib, GLUT, SDL and MD3 presence remains UNKNOWN. Stripped/optimized code or indirect loading can evade these searches. No absence claim is justified.

Vorbis diagnostic string at 0x564F5C was observed but is insufficient to classify a statically linked decoder: it could belong to game error reporting around a dynamically imported library. Coordinator's independent import inventory is the appropriate companion evidence.

## Next small investigation boundaries

1. MS3D boundary 0x4159B0: trace incoming call through thunk 0x401C21, determine the game caller and file-open/failure semantics, then record one original map-loading checkpoint. Do not infer validation of the magic string merely from the strncmp call; branch/result use still requires instruction-level confirmation.
2. BMP/DIB boundary 0x4934CC and wrapper 0x492EDE: independently obtain historical GLaux source and compare public wrapper/API/output record layout plus multiple loader helpers before attribution. Capture one original width/height/pixel-output observation to create an approved image contract.
3. Search characteristic decoder constants/control-flow if JPEG/PNG/zlib identification remains relevant after the dynamic dependency inventory. No guessed decoder behavior should enter implementation specifications.

Upstream classifications for non-runtime static loaders: UNKNOWN. Runtime signature families HIGH, exact version match UNKNOWN. This inventory alone does not approve replacement behavior or establish differential PASS.

## Reproducibility

IDA survey_binary(a6e1ce44, minimal); find_regex for component strings; func_query for library-name filters; xrefs_to(0x562218, 0x56525C, 0x5636DC); analyze_function(0x4159B0, 0x4934CC). Analysis-bearing pseudocode was examined only by Agent 1 and was not copied into this document. Raw database stays outside repository.

## Origin investigation update — 2026-10-04

The earlier UNKNOWN attribution has been refined through independent source comparisons:
- MS3D: MODIFIED_UPSTREAM, HIGH, Brett Porter/PortaLib3D/NeHe Lesson 31 family. Exact fork UNKNOWN. See specs/io/IO-0001-ms3d-origin.md.
- BMP/DIB: HIGH GLaux/TK family attribution based on main loader, palette helper, ANSI wrapper and popup-control correspondence. Exact SDK build UNKNOWN. See specs/io/IO-0002-bmp-origin.md.

These replace the earlier origin hypotheses; they do not establish runtime equivalence or approve wholesale substitution of original layouts with upstream layouts.
