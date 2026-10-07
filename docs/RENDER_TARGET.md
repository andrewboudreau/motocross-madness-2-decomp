# RenderTarget and PCRenderTarget

RTTI: `PCRenderTarget : RenderTarget`. Canonical source is
`src/reconstructed/RenderTarget.{h,cpp}`, `PCRenderTarget.{h,cpp}` and
`RenderInterfaces.h`, with bindings files for each. PCRenderTarget.cpp is
literal evidence (its destructor passes `__FILE__`). RenderTarget's code sits
just before ResourceManager.cpp references; its file name is not attested.
Names are provisional.

The RenderTarget is the object Camera keeps at +0x18, called "the owner" in
the camera notes. Four facts connect them:

- Camera calls RenderTarget slot 12 with its viewport rectangle.
- Camera calls `0x004e8cf0`, which follows RenderTarget's destructor.
- PCCamera calls `0x004c5d00`, inside PCRenderTarget.cpp.
- PCCamera uses PCRenderTarget's device at +0x50 for its three matrices.

`Camera::Owner()` returns a `RenderTarget*` and `PCCamera::PCOwner()` a
`PCRenderTarget*`.

## RenderTarget

The vtable holds the destructor, `_purecall` in slots 1–17, and empty bodies
in slots 18 and 19. Those bodies are shared with other classes by
identical-code folding. All functions below are strict exact.

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| Constructor | `0x004e8c50` | 44 | vptr; zeroes +0x04, +0x08, +0x14..+0x1c, +0x28, +0x34..+0x44 |
| Destructor / wrapper | `0x004e8cb0` / `0x004e8c80` | 7 / 30 | Empty destructor |
| `0x004e8ca0` | `0x004e8ca0` | 10 | +0x04 = display object (its mode table gives Camera the aspect) |
| `0x004e8cc0` | `0x004e8cc0` | 40 | Frame index wraps at +0x14 unless the global's +0x0c->+0x6c is set; frame count always increments |
| `0x004e8cf0` | `0x004e8cf0` | 49 | Current camera = argument; refreshes it via `Camera::0x0042e550` when its cached size (+0x1c4/+0x1c8) differs from +0x0c/+0x10 |
| 18, 19 | `0x004806f0`, `0x0044d710` | 3 / 1 | Empty |

## PCRenderTarget

PCRenderTarget wraps a COM-style `device` at +0x50 and `renderSurface` at
+0x48; `deviceCaps` names the decoded capability word at +0x164. The
method indices it uses line up with IDirect3DDevice7 and IDirectDrawSurface7:

- **Device:** 5/6 BeginScene/EndScene, 11 SetTransform, 13 SetViewport, 20/21
  Set/GetRenderState, 35 SetTexture, 36/37 Get/SetTextureStageState.
- **Surface:** 5 Blt, 32 Unlock.

The game imports `DirectDrawCreateEx` and carries IID_IDirectDraw7 and
IID_IDirect3D7, and every decoded index and argument count lines up with the
DirectX 7 SDK declaration order, so `RenderInterfaces.h` names those methods
after the SDK (strong inference; VC98 ships DirectX 5 headers only) and
notes each index. Unused slots keep `UnknownMethodN`. +0x34 nonzero makes
slot 12 clear the target (`D3DCLEAR_TARGET`) as well as Z.

The class also keeps a 300-entry render-state cache at +0x264. The
constructor fills it with `{i, 0}`, and slot 8 skips a state whose cached
value already matches unless forced.

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| Constructor | `0x004c4ee0` | 113 | `memset` of the +0x54 rectangle (sharing the zero register with the cache `memset`), device/surface pointers cleared, +0x250 = 3, state cache filled |
| Destructor / wrapper | `0x004c5320` / `0x004c4f60` | 176 / 30 | `/GX` frame; debug frees of +0x258 and +0x260 at lines 196 and 210; Release of +0x50 and +0x4c; `~RenderTarget` |
| 1, 2 | `0x004c53d0`, `0x004c53e0` | 15 each | Device methods 5 / 6 succeeded |
| 3 | `0x004c53f0` | 42 | Surface method 5 with the source's +0x70 surface succeeded |
| 5 | `0x004c5490` | 25 | Surface method 32 succeeded |
| 6, 7 | `0x004c54b0`, `0x004c54d0` | 30 each | Device methods 36 / 37 result |
| 8 | `0x004c56e0` | 54 | Cached render-state set |
| 9 | `0x004c5720` | 22 | Device method 21 result |
| 11 | `0x004c5930` | 22 | Device method 35 with a null texture |
| 14 | `0x004c54f0` | 22 | Device method 13 succeeded (Camera's viewport) |
| 19 | `0x004c5ed0` | 42 | Render states 0x19 = 5, 0x18 = 0, 0x0f = 0, unforced |

Also strict exact (12 more functions, not yet calibration cases):

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| `0x004c4f80` | `0x004c4f80` | 681 | Attach: display, device GUID (+0x54), surface, frame modulus; size and pixel format from the surface; with `zbuffer`, a Z surface of the same depth from the enumerated list (fails on `0x8876017c`); CreateDevice, caps into +0x164..+0x24f, texture formats; deletes itself on failure |
| Z/texture format callbacks | `0x004c52a0` / `0x004c5230` | 114 / 105 | stdcall enumeration callbacks; append the 0x20-byte format to +0x260/+0x25c (Z, flag 0x400 only; line 39) or +0x258/+0x254 (line 20) with `DebugRealloc` |
| 4 | `0x004c5420` | 99 | Lock: returns the bits (0 on failure) and the pitch |
| 10 | `0x004c5740` | 488 | Stage-0 colour/alpha operations for blend modes 1-8 (switch; case 8 jumps into case 3's tail) |
| 13 | `0x004c5640` | 151 | Whether a texture format matching flags/FourCC/bit count/G/A masks was enumerated |
| 15, 16, 17 | `0x004c5b20`, `0x004c5bd0`, `0x004c5c70` | 164 / 152 / 144 | DrawIndexedPrimitive, DrawPrimitive, DrawIndexedPrimitiveVB (device 26/25/32); count vertices of formats 0x112/0x1e2 (+0x38) and points/lines/triangles (+0x3c/+0x40/+0x44) |
| 18 | `0x004c5e60` | 112 | By caps +0x1b8 bit 0x10 or 0x20: render states 0x18, 0x0f, 0x19 through slot 8 |
| `0x004c5950` | `0x004c5950` | 453 | Texture-memory probe: creates 256x256 then 32x32 surfaces (caps 0x10005000) until failure or system memory; bytes = count<<17 + count<<11 |
| `0x004c5d00` | `0x004c5d00` | 351 | Screenshot: first free `"%s%05d.TGA"` (computer name, counter `0x0068995c`), lock, Tgafile writer for 16/24/32 bits; `MAX_PATH` buffer and an inline file-exists test give retail's frame and unrotated loop |

RenderTarget fields: +0x20 Z depth, +0x24 memory caps (0x4000/0x800), +0x28
pixel format, +0x2c Z clear value (float 1.0), +0x30 clear colour, +0x34
stencil flag, +0x38..+0x44 per-frame primitive counters (reset by slot 12).

Near miss (`samples/render/PCRenderTargetNearMisses.cpp`): slot 12
`0x004c5510` (Clear), 44/275 — register allocation and the merging of the
three Clear calls.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x004c5230` `EnumTextureFormatCallback`
- `0x004c52a0` `EnumZBufferFormatCallback`
- `0x004c5950` `MeasureTextureMemory`
- `0x004c4f80` `InitializeRenderTarget`
- `0x004c5d00` `SaveScreenshot`

## Direct3D constants

`src/reconstructed/D3DConstants.h` spells the DirectX 7 SDK values the
renderer passes (render states, compare functions, texture-stage states,
primitive types, FVF codes, primitive caps, DDSD/DDSCAPS/DDLOCK flags and
the two DDERR codes the code tests). A value is named only where the
receiving method is identified by its vtable index: PCRenderTarget slot 8
forwards to device method 20 (SetRenderState), slot 7 to method 37
(SetTextureStageState), slots 15/16/17 to methods 26/25/32 (draw calls).
GetCaps (method 3) fills PCRenderTarget+0x164..+0x250 in the
D3DDEVICEDESC7 layout: `triRasterCaps` (+0x1a8), `triAlphaCmpCaps`
(+0x1b8), `triTextureCaps` (+0x1c0) and `triTextureFilterCaps` (+0x1c4)
are dpcTriCaps members, and the bits tested against them (fog vertex /
table / range, dither, antialias, GREATER / NOTEQUAL, TRANSPARENCY,
LINEAR / LINEARMIPLINEAR) are the SDK's.
