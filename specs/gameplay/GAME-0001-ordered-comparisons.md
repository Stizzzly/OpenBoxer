# GAME-0001 — Approved admitted-path comparison supplement

Status: ANALYZED, APPROVED. Confidence: CONFIRMED original order, operand widths/orientations, short circuits and pop/store boundaries. Source: program.exe SHA25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, body41EF90, IDA sessiondde3c884 inspected2026-10-06. This is behavioral/FP metadata, not copied original implementation.

This supplement resolves REQ-GAME-0001-ordered-entry-callbacks. The main spec's callback-order clarification remains required. P+12 and P+28 entry copies are direct four uint32 read/store pairs in ascending offsets, with no accessor calls. All eight initial accessor calls and both P+76 constructions must still occur.

## Comparison primitive

Each described comparison loads the left operand into x87 ST0, compares with the right memory operand and pops the loaded operand. There is no stored boolean/float result, no second FP operand pushed by the parent, and no remaining temporary after this comparison. Afterward the original samples full SW into AX for its integer branch decision; sampling SW does not change FP state. For finite ordered operands, C0/C2/C3 are (1,0,0) for left<right, (0,0,1) for equality and (0,0,0) for left>right; C1 is0, TOP returns to its prior value. Preserve sticky flags. A predicate computed only with integer/SSE comparisons is insufficient because original callbacks observe this x87 environment. Arithmetic/store C1/sticky effects between comparisons remain those of the main spec's ordered x87 contract.

## Entry and fresh-key eligibility

After position queries/eight accessors/distance sqrt+float32 store and entry P-to-global copies, outer active flags and mode are integer tests. With admitted B/V/arrows all0, forward/backward handling short-circuits on these key bytes before any menu/P+0 health comparison. Thus neither P+0-versus0 movement-health comparison nor backward-distance-versus8 comparison executes. Then the first zero construction(P+76) executes, mode2 site1F2C4 or other-mode site1F4A3. Integer movement-state tests see P+164=0 and make no reset writes.

Space0 short-circuits block activation. Block-release checks Space, P+164 and blockbyte in that order but make no write. No FP comparison executes in block handling.

Fresh-key checks execute sequentially Z then X then C. For each: key byte first, corresponding latch second, menu third, then opponent-health comparison **left=float32 VA576238, right=float32 +0** (Z compareVA41F5C5, X41F723, C41F881); reject if left<right. Then integer bytes are checked in order attack5761C9, recoil5761CA, incoming5761C8, lock5761F2, block5761D0. On admitted exactly-one-fresh-key input, zero key bytes skip all remaining checks including health comparison for those keys; only the selected key executes health comparison. Do not execute artificial comparisons for unselected keys.

After selected latch/type stores, compare **left=cached float32 distance575EEC, right=float32 1** (Z41F63C, X41F799, C41F8F7). If left<=right, skip upper comparison and hit effects. Otherwise compare **left=the then-live cached float32 distance, right=float32 upper bound4/5/6** (Z41F64F, X41F7AC, C41F90A). Hit requires left<right. Upper bound equality is a miss. These are two separate load/compare/pop operations when the lower gate passes; no load-once combined range check. After hit constructor/impulse and marker write where applicable, hand/state/attack writes are integer.

Recoil byte0 skips its timer-versus0.5 FP comparison; its subsequent alternative also observes byte0. Incoming byte0 skips duration*0.5-versusopponent timer comparison and the entire incoming-hit consequence branch. Absent B/V/arrows takes the direct P+171=0 and second P+76 zero-construction route; the alternative float584780-versus0 comparison does **not** execute. Consequently the second constructor sees the preceding hit callback/range-comparison FP state unchanged by those skipped checks.

## Duration ladder, including comparisons for earlier nonmatching types

After second construction and with fresh attackbyte1:

1. Compare **left=float32 timer5761CC, right=float32 duration5762A4+16*player index584784** atVA41FF51. Only if timer<=duration then test integer type==1. Admitted timer+0/duration>0 reaches this type test for every strike. Type1 stores its increment and skips the remaining duration ladder.
2. For type2/3, calculate x87 **duration(float32) * multiplier(float32 bits3FA66666)**, retaining53-bit intermediate with no float32 store, then compare/pop **left=this product, right=live float32 timer** atVA41FFA5. Only if product>=timer then test integer type==2. Type2 stores its increment and skips third comparison.
3. For type3, calculate x87 **duration(float32) * multiplier(float32 bits3FF33333)**, no intermediate float32 store, then compare/pop **left=this product, right=live float32 timer** atVA41FFF8. Only if product>=timer then test type==3; type3 stores its increment.

Each admitted increment loads float32 bits3C23D70A, multiplies live float32 dt576370, adds live float32 timer, and pops into a rounded float32 timer store. Thus type2 performs both first and second comparisons, and type3 all three. Removing the earlier comparison because the integer type is already known changes observable FP state. The expiry branch and its fatigue-versus100 comparison atVA420070 do not execute for the approved timer+0/positive-duration route.

## Mandatory post-duration checks before cursor callback

Execute the following in this order, even when no branch write results:

| Order | Left / right | Original comparison VA | Admitted action |
| --- | --- | --- | --- |
| 1 | live float32 fatigue5761F4 / float32 +0 | 42009B | If fatigue>0, compute 0.15(float32 bits3E19999A)*live dt in x87, subtract product from live fatigue, pop/store rounded float32 fatigue. If zero skip arithmetic/store. |
| 2 | then-live float32 fatigue / float32 99 | 4200C6 | Always compare. With admitted fatigue[0,99) and dt>=0, result<99; lock-setting writes are skipped. |
| 3 | live float32 health5761D4 / float32 +0 | 42011B | Always compare, even though nonnegative guard excludes death. Result>=0 skips death body. |
| 4 | live float32 combo5761FC / float32 0.6(bits3F19999A) | 42027A | Always compare; admitted combo+0 yields less, skipping combo consequences. This is the final parent comparison before cursor callback20381. |

Between rows2/3, lockbyte5761F2=0 short-circuits the fatigue-versus50 comparison at420101. Between rows3/4, victorybyte5761F0=0 skips victory-timer-versus0.35 comparison at4201FE; combobyte576208=0 skips combo-versus1 comparison and combo increment at42024F. Nevertheless row4 executes independently of that byte. These skipped FP checks must not be invented in the replacement.

At cursor callback entry, the parent last condition producer is combo+0 versus0.6: C0=1, C2=C3=C1=0 and TOP0; sticky exception bits reflect the preceding callbacks/ordered arithmetic. Do not reset SW to this condition pattern, because doing so loses sticky flags. Fixture captures should compare actual full callback-entry environment.

## Tail comparison checkpoints

After value-length/absolute result rounded to P+172 and Y zero store, compare/pop **left=x87(float32 P+172 * float32 1000), right=float32 1500** at4204C2; no intermediate float32 store. Result<1500 uses vector-add, otherwise vector-multiply. After pointer-length ST0 contribution, P+140 arithmetic/store, query/audio integer checks and finalXZ sqrt, compare/pop **left=sqrt ST0, right=float64 3** at42065F. Result>3 runs zeroY/scalar-first multiply/body callback, otherwise returns the incidental EAX formula. These orientations and exact FP/callback entry states apply to both branches. All arithmetic/store boundaries remain those specified in the main contract.

## Validation and scope

Capture CW/SW/TOP/tag at every dependency boundary, in addition to full post environment and EAX. At minimum test Z/X/C separately, lower-boundary miss (no upper comparison), upper-boundary miss (both comparisons), hit (both comparisons plus dependencies), zero/nonzero fatigue and duration multipliers with nontrivial float32 values. Compare state/callback order/environment EXACT. Callback mutation fixtures remain within the main invariant stability domain; do not broaden eligibility or replace skipped branches with after-effects fallback. No replacement source was accessed.
