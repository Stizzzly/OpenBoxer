# REQ-GAME-0002 — Complete approved execution metadata

Related specification: GAME-0002-opponent-strike-consumption.md

Blocking: YES for complete behavioral implementation/native integration.

The semantic contract is approved. Root explicitly authorizes preliminary typed
scaffolding only until a separate READY message supplies completed metadata.

Required answers:

1. Which exact raw float32 values does each mode1 AIindex equality match write
   to actor+220? The current contract mentions corresponding values but does
   not enumerate them.
2. Supply approved callback-site/return identifiers, callback ABI metadata and
   patchable observer boundaries, including repeated loop sites and imported
   audio invocation. No addresses or boundaries will be inferred.
3. Supply the exact reached x87 comparison load/compare-pop relationship and
   conditional direction needed to reproduce raw status flags, with approved
   scalar callback FP/EAX capture and exit-state requirements.

Evidence needed: approved behavioral/ABI supplements and typed metadata;
no original implementation or disassembly may be supplied to Agent2.

Current candidate is only an unintegrated typed interface in damage.hpp;
there are no executable GAME-0002 effects, guards or hooks yet.
