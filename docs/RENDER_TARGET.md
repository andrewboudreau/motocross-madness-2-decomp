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

PCRenderTarget wraps a COM-style device at +0x50 and a surface at +0x48. The
method indices it uses line up with IDirect3DDevice7 and IDirectDrawSurface7:

- **Device:** 5/6 BeginScene/EndScene, 11 SetTransform, 13 SetViewport, 20/21
  Set/GetRenderState, 35 SetTexture, 36/37 Get/SetTextureStageState.
- **Surface:** 5 Blt, 32 Unlock.

That identity is inference from call shape (VC98 ships DirectX 5 headers
only), so `RenderInterfaces.h` keeps neutral names and notes the indices.

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

Slots 4, 10, 12, 13 and 15–18 and the helper `0x004c5d00` are not yet
reconstructed.
