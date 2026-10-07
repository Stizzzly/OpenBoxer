# IO-0002 — BMP/DIB loader origin: GLaux/TK

Status: ANALYZED_STATIC. Confidence(origin): HIGH. Runtime equivalence: not tested.
Date: 2026-10-04. Role: Reverse Engineering.

## Original identity

program.exe SHA-256: 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6. Preferred imagebase 0x00400000, x86. IDA session 2e174258 reverified by survey_binary. Analysis only on unchanged private copy. No replacement source used or changed.

## Finding

The BMP/DIB decoder is attributable with HIGH confidence to the Windows GLaux/TK loader family, specifically auxDIBImageLoadA → tkDIBImageLoadAW plus DibNumColors and tkErrorPopups. It is statically linked into EXE. This conclusion uses multiple related functions and distinctive behavior, not a BMP signature alone.

Candidate mapping:

| Original VA | Proposed upstream identity | Confidence |
|---|---|---|
| 0x492EDE | auxDIBImageLoadA, ANSI forwarding wrapper | HIGH |
| 0x4934CC | tkDIBImageLoadAW | HIGH |
| 0x493468 | DibNumColors | HIGH |
| 0x4939C4 | tkErrorPopups | HIGH |
| 0x570BA8 | tkPopupEnable diagnostic control byte | HIGH |

Core decoder/helper classification: UPSTREAM_MATCH for the observed, compared behavior, confidence HIGH. Exact historical SDK/library build and all edge-case equivalence: UNKNOWN. This is not a byte-for-byte or whole-library match assertion. No game-specific modification has been identified in these compared decoder/helper paths.

## Distinctive original evidence

1. Main entry branches between CreateFileA and CreateFileW using a second argument, then uses a read-only file mapping. BMP header presence selects a 14-byte header skip and pixel offset from file +10; bare DIB takes a separate path.
2. Palette helper distinguishes 12-byte core headers from info headers; explicit color count takes priority, then bit depths 1/4/8 produce counts 2/16/256. The same helper is called at 0x49358C.
3. Main decoder converts 12-byte core headers and 40-byte info headers to a separate aligned BITMAPINFO. Core palettes expand triples to quads. Its allocation, default-field handling and branch shape match the comparison source.
4. GDI pipeline at 0x4936E9–0x4937AE: CreateCompatibleDC → CreateDIBSection for 24-bit output → SelectObject → SetDIBits → GdiFlush.
5. Pixel loop at 0x4937D6–0x49380D swaps blue/red triplet order and skips width modulo 4 padding per row. Result allocation at 0x49381E is 12 bytes: width at +0, height at +4, pixel pointer at +8.
6. Diagnostic cluster and cleanup match: memory/format/conversion/file-open messages, DeleteDC/DeleteObject/LocalFree/UnmapViewOfFile/CloseHandle, freeing pixel buffer when result creation fails.
7. Each popup path tests byte 0x570BA8. Function 0x4939C4 stores its byte argument there. Independently acquired TK header defines a popup guard, and TK source exposes a matching popup-control setter. This additional neighboring function strengthens attribution.

Static decompiler types are not an ABI contract. In particular, ANSI wrapper output was mislabeled LPCWSTR by propagation; the second argument selects ANSI behavior. Malformed-file behavior and top-down bitmap cases are not runtime verified.

## Game-side boundary

Two EXE wrapper routines, 0x425390 and 0x42A250, perform the same observable sequence: null filename returns null; attempt fopen in mode r; failure returns null; close the stream; call the ANSI GLaux entry. Calls occur at 0x4253E3 and 0x42A2A3. These wrappers are outside the decoder and are possible copied tutorial helpers; origin investigated separately alongside the NeHe model-loader comparison.

For future replacement work, preserve the GLaux decoder boundary or supply a deliberately specified compatible adapter. Do not count this third-party loader as original game logic. Texture selection/upload and object ownership at its callers remain independent game work items.

## Independent source provenance

Comparison repository: https://github.com/tdechaize/Glaux
Pinned revision: 78dfb19b946cebb2e3a7ff124c9daea6f5b78d5a.
Relevant files: src_bad/TKDIB.C, IMAGE.C, TK.H, TK.C, glaux.h.
TKDIB.C SHA-256: 8BAA45EE90517F851F296339038BED43379C35F4215508CA311D5E887798C9F2.
Direct comparison: https://github.com/tdechaize/Glaux/blob/78dfb19b946cebb2e3a7ff124c9daea6f5b78d5a/src_bad/TKDIB.C

This is a modern maintenance repository preserving historical source with explicitly marked edits, not an authenticated original Microsoft SDK distribution. Its header attributes the DIB module to Gilman Wong/Microsoft, 1994. Attribution of the code family is HIGH; exact package provenance remains UNKNOWN. Any future dependency reuse requires separately checking the applicable source license, not assuming all GLaux code has one license.

Private acquired source and full original analysis: C:\Users\ADMIN\Boxer-analysis\origins\glaux. No copied original pseudocode is included in this specification.

## Next verification

Confirm historical SDK binary/source revision if exact upstream build matters; verify the caller ownership/free policy; collect normal and failed BMP loads in the original before an adapter receives implementation approval.
