# Initial publication

Date: 2026-10-07. Publication requested by the project owner for public repository Stizzzly/OpenBoxer. MIT selected for independently written project code; third-party licenses remain intact.

The initial source snapshot is exported from the validated replacement tree without editing replacement behavior. It excludes original game files/assets, raw analysis and IDA databases, native memory/callback captures, build outputs, bundled runtime binaries, upstream ZIP downloads, and the historical standalone prototype.

`um publish check` against the installed original game reported zero failures and no copied game files, secret patterns or decompiler code fingerprints. Absolute-path warnings are consciously retained: source guards and diagnostic tools currently refer to the developer lab. README documents that limitation; source paths have not been silently rewritten or advertised as portable.

Archived validation reports describe earlier local runs. Their omitted observations and absolute local paths are historical evidence references, not shipped files. The published synthetic CTest tests can run without original game assets.

Credits are in CREDITS.md and both README files. Codex is credited as an AI assistant, not as a separate human or an invented GitHub identity. The initial commit uses an AI-Assisted-by trailer without claiming a fabricated co-author email.

Clean publication build: Windows/i686 Clang 20.1.8, Release, 157 build steps completed; CTest 14/14 PASS. All twelve semantic source files are byte-identical to the local validated implementation before Git serialization. Staged file-extension audit excludes binaries, game models/images, raw native captures and analysis databases.
