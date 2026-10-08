# GAME-0002 v5 default-off root observation — 2026-10-08

Scope: root-owned test copy only, frozen DLL SHA-256 19838F07440A79D071A91C343AE2BBDA28D4B903A2D9190F2A78124C31BBEE47. All OPENBOXER environment overrides cleared; only CHARACTER_ASSETS_CONFIRMED=1 supplied. Damage mode, captures and diagnostics remain default-off. Original installed executable untouched.

Owned PID22124 showed rendered battle, pause and Continue into round2 (screenshots retained locally). Process exited before the late counter read; cause UNKNOWN, not classified as a crash or success exit. Second owned PID2540 supplied successful live counter/route reads and ordinary Z input; root stopped this exact PID after observations.

Corrected RPM symbol mapping uses frozen PE preferred ImageBase6E080000 and observed runtime base70FB0000/size2293760. An initial reader incorrectly used6E000000 and produced rejected data; rejected-wrong-preferred-base.json and early invalid snapshot remain archived, never used as evidence. Valid final snapshots explicitly use6E080000; independent validator must verify own symbol mapping.

Confirmed damage thunk remains original41C4E0; damage replacement/observer/counters are zero. Eleven prior semantic units have nonzero available replacement counters; clip/action counters become37/27 after Z. Strike has an enabled replacement flag and redirected thunk, but no persistent natural execution counter when diagnostics are off; no fabricated counter claim is made. Capture/original/diagnostic fourteen-byte flags and MS3D slot are recorded in final live snapshot.

Root directory file inventory finds no new runtime JSON/JSONL/bin capture and no modification except the root-generated prepatch observation JSON. GPU gfxcapture twice timed out for this legacy fullscreen window; fallback ddagrab captured only the independently verified foreground640x480 owned game client. Local raw evidence: C:/Users/ADMIN/Boxer-lab/ms3d/damage-default-off-stage5. This observation does not activate or validate GAME-0002 candidate damage.
