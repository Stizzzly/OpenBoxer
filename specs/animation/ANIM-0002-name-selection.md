# ANIM-0002 — Lower-model animation selection by name

Status: VALIDATED_BOUNDED; see validation/ANIM-0002.md and implementation-freeze.json. Combined ANIM-0003 delivery validated in validated-animation-v2.
Confidence: CONFIRMED bounded body, ABI, ordered state and dependency boundary. Semantic identity as lower/legs model HIGH from explicit LEGS names and render route. Compiler/source provenance UNKNOWN.

Evidence target: program.exe SHA256 77F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, x86 base0x400000/image0x19F000. IDA session d283dbb8, analysis ready, full body406E40 through406F06, dependency4989C0 and lookup4097A0 inspected 2026-10-05. Classification GAME_SPECIFIC for selection; CRT comparator provenance UNKNOWN, preserved dependency.

## Boundary and ABI

Whole selection body VA0x406E40/RVA0x6E40; thunk VA0x401C26/RVA0x1C26 is a complete five-byte direct tail transfer. Metadata-only synthesized jump install/restore requires root-preverified module identity/target; no copied instructions are supplied. This is independent of future-frame preparation405D40 and advancement406810. VA405440 is an animation-file loader, not this selector.

thiscall: ECX owner O, one stack argument pointer to NUL-terminated requested name N, callee cleanup4, preserve nonvolatile registers; full uint32 EAX defined below. O is not model M: M=O+644. Adapters must perform this explicit mapping, never confuse owner fields with model fields. Normal stack-check dependency has no floating effects; abnormal ABI failure excluded.

## Layout / inputs / outputs

All offsets decimal. M has112-byte span established by RENDER-0006. Owner accesses are entirely within that lower model.

| Role | Owner / model offset | Type |
|---|---|---|
| Stored loop bound | O+684 / M+40 | live signed int32 |
| Selected record index | O+688 / M+44 | int32 output on match |
| Current frame | O+692 / M+48 | raw32 record start copied on match |
| Blend factor | O+720 / M+76 | raw float32 output0x3DCCCCCD (0.1) |
| Time anchor | O+724 / M+80 | raw32 current-time output |
| Embedded animation collection | O+728 / M+84 | lookup ECX |
| Collection begin/end/capacity | M+88/92/96 | pointer roles |
| Animation record |272-byte stride | name begins at record+0, start raw32 at+256 |
| Current-time global | VA0x57D904 / RVA0x17D904 | raw32 read only on match |

The bound is stored M+40, NOT a count callback and NOT an assumed vector count. There is no selection-time collection-count invocation. The collection used is exactly M+84, not an upper-model collection.

Dependencies: animation_lookup thunkRVA0x10B4/bodyRVA0x97A0, thiscall ECX=M+84, signed index stack, ret4, EAX record pointer. Original pure lookup obtains live collection begin then returns begin+272*index with32-bit arithmetic and no bounds check. Keep original dependency in natural runs. compare_names bodyRVA0x989C0, cdecl String1=returned record base, String2=N, caller cleanup8, signed EAX; zero selects, any nonzero fails. Keep original comparator, including its CRT locale behavior, rather than substituting host CRT/Unicode conversion. Its inspected C-locale route compares bytes with ASCII A..Z case folding; non-C locale routes through original tolower. Locale state and behavior beyond preserved callback are outside candidate logic.

Comparator observer seam: independently verified selection callsiteVA0x406E94/RVA0x6E94 is one complete five-byte direct CALL to bodyRVA0x989C0; returnRVA0x6E99. There is no required comparator entry trampoline/thunk. In isolated fixtures or explicitly opt-in native ordered capture, root verifies module identity, live opcodeE8 and decoded target989C0 before installing metadata-synthesized E8rel32 to a typed cdecl recorder, then restores a newly synthesized E8rel32 to the preserved body. No copied original instructions or body patch is needed. Native recorder forwards the exact two pointers to original body989C0 and returns its fullEAX without resetting FP state. Ordinary diagnostics-off launch must not patch this seam. A fixture recorder may provide controlled compare outputs/mutations; compare those effects explicitly. Candidate callback uses the same typed comparator dependency directly; observer installation is harness-only authorization.

## Ordered behavior

1. Start local signed index i=0 and return accumulator0xCCCCCCCC. Read live M+40 on each loop condition. If i>=that bound, finish immediately. Thus initial bound<=0 has no callbacks, no state writes, no time read, full EAX0xCCCCCCCC.
2. Invoke lookup(M+84,i), then compare_names(record,N). N is the original request pointer, not a copied/reinterpreted name; comparator sees any mutations made by lookup. Neither index nor bound is advanced before comparison.
3. If comparison is nonzero, increase local i by one with32-bit wrap; EAX accumulator becomes that integer. Recheck live M+40. A callback can shorten/extend the bound; do not cache it. Finite ordinary positive no-match returns final tested index, normally initial bound if unchanged. No writes by this unit on no-match.
4. On the first zero comparison, store i at M+44. Reread live M+44 as signed argument and call lookup a second time with ECX=M+84. The second lookup occurs after selector write and can observe it; its returned record may differ from the first lookup.
5. Read raw32 at the second record+256 and store to M+48. Then store literal bits0x3DCCCCCD to M+76. Then read global time bits once, after both lookup calls and comparison, and copy to M+80. Return full EAX owner O. No later search, count callback, progression, future-slot update, allocation, file operation or GL call occurs.

The unit's exact own writes on match are ordered M+44, M+48, M+76, M+80. Callback effects are separate and may alter any approved fixture state. The first record's name controls match; the second record's start controls frame. Read the global only after factor write; neither preflight nor copying initial time authorizes caching it for the body. M+52/+60/+64/+68/+72 remain unchanged by this unit.

## Floating and return policy

Selection uses only integer data copies, including factor/time bits. It adds no floating arithmetic or comparisons and changes no CW/SW/TOP/tag/MXCSR itself. Preserve dependency-induced environment changes; do not reset status between callbacks. Compare full FP environment EXACT for identical initial environments/dependency behavior. Arbitrary time bits, including NaN/signedzero, are copied as raw bits; finite time is a natural safety restriction only, not the behavioral contract. EAX on success is owner pointer, normalized as owner role in cross-address captures; synthetic same-address original/candidate runs compare exact32 bits. Empty count sentinel is observed original debug residue and must be explicit, not guessed bool/void return. EFLAGS/volatile ECX/EDX after return are not compatibility outputs.

## Bounded native activation and observation

Proposed first scope: registered live RENDER-0006 owner O with unchanged model/source/lifetime witnesses, O==modulebase+RVA0x17ACD0, M==O+644; caller returnRVA0x371BE (stand),0x371DF (run),0x37200 (run backward). These three callsites in gameplay state-update body437110 explicitly load owner57ACD0 and names LEGS_STAND/LEGS_RUN/LEGS_RUNB. Initial setup body41C040 uses the same owner at returnRVA0x1C15F but can precede draw registration: leave it original. Other actors/routes remain original until separately witnessed. The static callsite relationship does not claim a live registered model exists; root must capture it and prove guard hits naturally after registration.

Read-only guard before any body callback/write: stored count positive and bounded<=256; valid begin/end/capacity272-byte collection with count>=stored bound; all candidate record names NUL-terminated within the first256 bytes, request readable and NUL-terminated within256 bytes; selected natural record starts in0..748; original comparator/accessors unchanged; owner and route witnesses valid. Preserve original comparator rather than preflight case-folding to decide a match. Guard may conservatively require every scanned record start0..748. Unsupported input falls back before replacement effects. Live asynchronous mutation unsupported. Guards must preserve FP environment. Finite global time may be required for native route consistency with ANIM-0001; callback-free raw-bit selection fixtures may exercise all bits.

Capture pre/post112-byte lower model, normalized owner/model/collection/request roles, full selected collection data records, bounded requested bytes, global time bits before/after, full EAX, entry/exit FP state and ordered callback roles/indices/string bytes/returned roles. Include first/last/duplicate-name match and natural later state switch, plus no-match synthetic case. Ordinary diagnostics remain off; preserve nine prior semantic cores and F1D6 baseline lineage. Root alone handles disposable/native launches.

Natural switch input evidence supplement (CONFIRMED static mapping, natural execution pending): original window-message handling atVA449BA6/449BB7 handles message256/257 by setting/clearing byte arrayVA576914 indexed by wParam. Player updateVA41EF90 is called atVA42D89D with ECX584910; its statefield+164 therefore is5849B4, consumed by437110. B (VK0x42/66) maps576956 and selects state1 at41F1C8 in mode584790==2, or41F39B in other mode; V (VK0x56/86) maps57696A and selects state2 at41F2A2/41F481. Other mode also accepts right arrow(VK0x27/39=>57693B) for state1 and left arrow(VK0x25/37=>576939) for state2. Release all applicable movement keys returns state0 at41F2FB/41F4F1. With actorvariant584784==0,437110 routes states0/1/2 to verified stand/run/runb callsites. Preconditions include !byte584794 && byte58479D, menu/state577F90==0, positive player health, no byte5761C9 attack and no byte5761D0 block. Candidate natural script is a short B-down/up then V-down/up after real draw registration and active gameplay; capture actual matching selection calls rather than declaring success from keyboard injection. Backward translation can be blocked by distance>=8 while state2 selection still occurs. This mapping is original evidence, not permission to alter game state or implement controls in this task.

## Differential cases

Count0/negative sentinel; positive no-match; first/middle/last match; duplicate case-insensitive names first wins; mixed ASCII case with original comparator; preserved non-C comparator route where available; repeated identical request still resets frame/factor/time; lookup returns different second record; lookup/comparator changes count, selector, requested bytes, collection begin, record start/global time; comparator returns arbitrary nonzero values; empty request/record name; raw globaltime0/-0/NaN patterns. Bounded fixtures terminate despite mutable count; invalid pointer/nontermination/crash behavior stays outside scope. Compare all model112 bytes, approved external state, fullEAX, exact callback order/arguments and FP environment. Distinguish FAIL_STATE/FAIL_RETURN_VALUE/FAIL_CALLBACK_ORDER/FAIL_FLOAT; insufficient natural provenance is BLOCKED.

## Unknowns

UNKNOWN: intended meanings of bound versus actual vector count mismatch, game source/compiler, unsupported malformed strings/pointers, unbounded callback-driven loops, other-owner native lifetime registration. These do not block the described finite guarded unit. No replacement source was viewed or written by Agent1.


