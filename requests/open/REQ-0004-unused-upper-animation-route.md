# REQ-0004 — Reachability of unused upper-model selector

Related: ANIM-0004.
Blocking: NO for approved actual whole-character strike/block selection activation; YES before implementing or activating upper-model selection.
Status: OPEN.

Question: Does any computed indirect route in this exact build invoke thunk4015E1/body406D40 or load actual owner+532 upper geometry, and what original lifetime/asset/caller witnesses establish it?

Current evidence: SHA25677F9AC7B4C4517F7D1776D7C2AFDAC19440169977BDC5393F58A1A0FBB7BD7D6, IDA session736dce9b 2026-10-06; zero thunkxrefs, independent raw PE relative CALL/JMP and literal-pointer scan only finds4015E1->406D40. Loader404F60 formats upper/head paths but only calls144C for owner+644 lowerpath. Installedfighters files are eight lower.bhm/eight animation.cfg, no upper/head. Active437110 invokes lowerselector1C26/6E40 for LEGS strike/block names. Confidence facts CONFIRMED, absence of every possible indirect route UNKNOWN.

Resolution requires an original actual call observation plus original loaded data provenance and bounds. Do not fabricate assets, callerguards or infer upper lifetime from registeredlower. ANIM-0004 proceeds through verified active lower/whole-character selection without waiting on this excluded question.
