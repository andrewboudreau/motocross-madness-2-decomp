# Port planning

Notes for running a rebuilt MCM2 on current Windows, Linux and Steam Deck.
Nothing here is built yet. The matching build stays VC6 SP3 against the
retail imports; a port is a separate build target on top of reconstructed
source, and needs far more of the game reconstructed than exists today.

## What the retail executable links

The import table of the retail demo `mcm2.exe` (SHA `31fde4cc…`) lists 15
DLLs. The counts, names and ordinals are confirmed from the PE. Ordinal
names come from [REBUILD_GAPS.md](../docs/REBUILD_GAPS.md).

| # | DLL | Imports | Used for |
|---|---|---:|---|
| 1 | KERNEL32 | 121 | Win32 core and the VC6 CRT underneath |
| 2 | USER32 | 54 | Window, messages, input |
| 3 | GDI32 | 18 | Pens, brushes, fonts, `TextOutA`, `GetTextExtentPoint32A` |
| 4 | ADVAPI32 | 9 | Registry (`RegOpenKeyExA`, `RegSetValueExA`, …), `GetUserNameA` |
| 5 | SHELL32 | 1 | `ShellExecuteA` |
| 6 | ole32 | 3 | `CoInitialize`, `CoCreateInstance`, `CoUninitialize` |
| 7 | blade | 2 | `DirectDrawEnumerateA`, `DirectDrawCreateEx` |
| 8 | d3drm | 1 | `D3DRMVectorRotate` only (FollowCamera) |
| 9 | WINMM | 8 | `timeGetTime`, timer period, CD audio via `mciSendCommandA`, aux volume |
| 10 | MSVFW32 | 2 | `ICClose`, `ICSendMessage` ([VFWDECO.md](../docs/VFWDECO.md)) |
| 11 | IMM32 | 11 | Input Method Editor (East Asian text entry) |
| 12 | DSOUND | 2 | Ordinals 1 `DirectSoundCreate`, 2 `DirectSoundEnumerateA` |
| 13 | WSOCK32 | 10 | TCP client: `socket`, `connect`, `send`, `gethostbyname`, … |
| 14 | DINPUT | 1 | `DirectInputCreateEx` |
| 15 | DPLAYX | 1 | Ordinal 4 `DirectPlayLobbyCreateA` ([NET.md](../docs/NET.md)) |

There is no `ddraw.dll` import. `blade.dll` ships in the game folder,
identifies itself as the Microsoft Blade Software Rasterizer, and exports the
DirectDraw entry points, wrapping the system ddraw
([PROVENANCE.md](../docs/PROVENANCE.md), [runtime/README.md](../runtime/README.md)).
The game reaches Direct3D 7 Immediate Mode through that DirectDraw object;
hardware D3D7 and the Blade software device are both selectable
([PICKDEVICE.md](../docs/PICKDEVICE.md)). d3drm supplies one maths helper and
does not mean the Retained Mode renderer is used.

## Windows rebuild

Most imports still exist on Windows 11 and can be linked unchanged:

| DLL | Status on Windows 11 |
|---|---|
| 1–6, WINMM, MSVFW32, IMM32, DSOUND, DINPUT | Shipped |
| WSOCK32 | Shipped as a shim over `ws2_32`; moving to `ws2_32` is trivial |
| DPLAYX | Optional legacy feature; the loader blocks on an install prompt. `runtime/dplayx` stubs it. Lobby servers are gone. |
| d3drm | Not shipped since Vista. Replace with our own `D3DRMVectorRotate` (`runtime/d3drm` already does). |
| blade | Game-folder DLL, not ours to redistribute. Link the hardware path to the system `ddraw.dll`; keep Blade only if the software device matters. |

The practical problems are not missing DLLs:

- Exclusive 640x480x16 full screen fails on current Windows. The runtime
  emulates it ([runtime/README.md](../runtime/README.md#display-emulation));
  a rebuild can render windowed or 32-bit directly.
- CD audio through MCI needs the CD. Rip tracks to files and play them through
  the mixer.
- The videos need their VfW codec installed. The system shim database's
  INDEO5 registry fix suggests Indeo 5 (inference; the files are not checked).

## Linux and Steam Deck

### Route A: run the Windows build under Proton/Wine

Wine implements all 15 imports, including DirectDraw, Direct3D 7, d3drm and
DirectPlay. Steam Deck runs Windows games through Proton anyway. This is the
cheapest way to play on Deck and needs no port code. Untested.

### Route B: native port

Each Windows layer needs a native replacement. SDL (2 or 3) covers most of
them.

| Windows piece | Native replacement |
|---|---|
| USER32 window and messages | SDL window and events |
| DINPUT | SDL gamepad and keyboard (Deck controls included) |
| DSOUND | SDL audio or a mixer such as miniaudio |
| WINMM timers | SDL timers / `std::chrono` |
| WINMM CD audio | Ogg/FLAC tracks through the mixer |
| IMM32 | SDL text input, or drop it |
| GDI32 text | Bitmap font or stb_truetype drawn by the renderer |
| ADVAPI32 registry | Config file under `$XDG_CONFIG_HOME` |
| SHELL32 `ShellExecuteA` | `xdg-open` / `SDL_OpenURL` |
| ole32 | Not needed; COM only creates DirectX objects |
| blade + DirectDraw + Direct3D 7 | New renderer: OpenGL, Vulkan or SDL_GPU. The largest job. |
| d3drm `D3DRMVectorRotate` | A few lines of vector maths |
| MSVFW32 video decode | ffmpeg, or a decoder for the actual codec |
| WSOCK32 | POSIX sockets (nearly the same API) |
| DPLAYX / DirectPlay | ENet, Steam networking, or cut multiplayer |
| KERNEL32 files, threads, sync | Standard C++, POSIX or SDL |

Other native-port issues:

- File paths are case-sensitive; game data uses DOS-style names and `\`.
- A 64-bit build changes pointer size. Any struct read straight from data
  files or sent over the network must use fixed-width fields.
- VC6-specific source (non-standard scoping, MSVC intrinsics, `__declspec`)
  must also compile with GCC/Clang. The x86 byte order carries over unchanged.

## Structure

Keep the matching source calling Win32/DirectX exactly as retail does. Put
platform calls behind a thin interface in reconstructed source so the VC6
build supplies the Win32/DX implementation and a port build supplies an SDL
one. Port-only code lives here, never in `src/reconstructed/`.

## Open questions

- How many reconstructed and unreconstructed functions call each import, which
  sizes the platform layer. The DirectX, VfW, IMM and socket imports go
  through thunks at `0x00533040..0x005330ee` plus the DPLAYX thunk at
  `0x00534420` ([REBUILD_GAPS.md](../docs/REBUILD_GAPS.md)); system DLL calls
  need an IAT cross-reference instead.
- How much rendering goes through DirectDraw 2D (blits, surfaces) versus
  Direct3D 7 draw calls.
- Which codec and container the videos use.
- Whether the demo runs under Proton unmodified, or needs the
  `runtime/` d3drm and dplayx replacements there too.
