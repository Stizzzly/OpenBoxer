# GAME-0002 — Original movement controls for natural observation

Status: ANALYZED — approved original behavioral control metadata, not a replacement implementation contract or runtime validation PASS.
Confidence(key/state/vector mapping and conditions): CONFIRMED. Confidence(a particular hold duration achieving distance>=6 in a live match): UNKNOWN until observed.

Original program.exe SHA-25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, imagebase400000, original IDA session27d52951 on2026-10-08. Original player whole body41EF90; original WM_KEYDOWN/WM_KEYUP handling449BA6/449BB7 sets/clears keybyte576914+wParam. No replacement source inspected.

V, virtual-key0x56, keybyte57696A selects the negative movement-direction branch: negate(P+44), multiply byfloat32(10.5), copy four returned words toP+76, stateP+164=2 and movingbyteP+171=1. This is the original backward/retreat control. Mode2 uses V; it does not admit arrows for movement. Outside mode2, Left arrow VK0x25/keybyte576939 is an alternative to V on that branch.

B, VK0x42/keybyte576956, selects the positive direction branch P+44*10.5, state1 and movingbyte1. Outside mode2, Right arrow VK0x27/keybyte57693B is its alternative. B/Right is tested first and has priority if both movement directions are held. The geometric interpretation of P+44 as a guaranteed vector toward the opponent is not established by this control mapping; inspect actual position/distance rather than assuming screen displacement.

Both movement directions require deathbyte584794=0, activebyte58479D nonzero, menu577F90=0, orderedfloat(P+0)>0, playerattack5761C9=0 and playerblock5761D0=0. P+0 is retained as a raw observed field condition; do not infer its semantic name from the gate. Holding Z while its attack remains active blocks movement; holding Space while block is active also blocks it.

Backward movement additionally compares cached distance575EEC againstfloat32(8). When distance<8 it produces the negative vector above; distance>=8 constructs zeroXYZ atP+76 instead, preserving the opaque fourth word as prescribed, while still setting state2/movingbyte1. This permits a distance interval6..8 suitable for the existing pending0far admission but does not guarantee the opponent will remain there. Mode2 sites: gate41F22C, distance41F243, negateCALL41F258, multiplyCALL41F268. Other-mode sites: gate41F40B, distance41F422, negateCALL41F437, multiplyCALL41F447. Positive vector multiply sites41F1A1(mode2),41F374(other modes).

For an authorized owned natural observation, V is the mode-independent original retreat key. Release attack/block/positive-direction controls before using it; release V before the fresh Z checkpoint since GAME-0001 fresh-strike admission rejects movement keys. Verify actual entry distance>=6 and pending0 through the existing recorder. No particular hold duration, far-miss success, scene stability or crash explanation is claimed here.
