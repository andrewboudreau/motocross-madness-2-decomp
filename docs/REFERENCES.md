# External references

These resources may be useful, but they are **not ground truth** for the MCM2 code decomp unless independently validated against the retail files/binary.

## Motocross Madness file formats

- Repository: https://github.com/AlexKimov/motocross-madness-file-formats
- Status: archived/read-only upstream.
- Scope: community reverse engineering of Motocross Madness series data formats.
- Potential MCM2 value: `DAT2.bt` plus `DecodeRES.1sc` / `unpackDAT2.1sc` for resource data.
- Policy: keep this as a light-touch asset/data-format reference; do not import its assumptions into C++ class/layout/function reconstruction without binary evidence.

## MSVC under Wine patterns

These are implementation references for the **general** “copy an installed MSVC tree to Linux and invoke it via Wine” workflow, not MCM2-specific evidence:

- https://github.com/fekir/wine-cl
- https://github.com/mstorsjo/msvc-wine

The bootstrap keeps its own minimal wrapper because VC6 is much older than the toolchains those projects primarily target.

## VC6 SP3 file-version reference

- Archived Microsoft KB Q230733: https://helparchive.huntertur.net/document/104797

Use it to check the SP3 compiler component versions, then rely on MCM2 byte matching as the final toolchain test.
