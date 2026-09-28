# Repository policy

This repository contains clean-room reverse-engineering tooling, reconstructed source candidates, and derived metadata only.

Do **not** commit or redistribute:

- Motocross Madness / Motocross Madness 2 executables, installers, assets, CABs, ISOs, or other copyrighted game data.
- Microsoft Visual C++ 6 / Visual Studio 6 executables, libraries, headers, installer media, or service-pack payloads.
- Generated object/debug files containing material copied from either proprietary distribution.

Users supply their own legally obtained `MCM2PCG.exe` and VC6 installation locally. Runtime inputs belong under ignored directories such as `input/`, `work/`, and `toolchains/`.

Committed `analysis/` files are derived facts/metadata used to make the project reproducible and reviewable. Nearby source-file attributions are evidence/hints unless explicitly proven.
