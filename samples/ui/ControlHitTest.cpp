// Complete nonexact menu hit tests; original TU ownership is unproven.
// Full retail extents: UIButton354 bytes, UIMultiState329 bytes.
// See docs/CONTROL_HIT_TEST.md.
#include <windows.h>
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/PCVideoCard.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004734c0: whether `point` is on an opaque pixel of the image.
// Session 19, all 256 unless noted: a shared `goto failed` for Lock and
// Unlock, an `int result = opaque` copy, Unlock without the if, `Unlock() == 0`
// with two returns, `if (!Unlock) return`, `return Unlock ? opaque : opaque`,
// a `format == 8 &&` pair, an empty then-arm; `format != 8` reversed 245, a
// switch on format 245, a ternary condition 249, Unlock and return inside the
// 8-bit arm 219.
// Wave 32, not applied (both forms need a statement with no evidence behind it):
// retail's `opaque` is a lone int whose address escapes. Two probes reproduce
// all 354 bytes of 0x004734c0:
//   (a) `int opaque; RECT rect; if (0) PtInRect((RECT*)&opaque, point);` (any
//       never-executed call that is handed `&opaque`; `(void)&opaque` and an
//       unused `int* p = &opaque` do not expose it), or
//   (b) `struct { int opaque; RECT rect; } s;` (the PtInRect argument exposes
//       the whole aggregate),
// each with this body, the opaque flag cleared after the rect copy:
//       rect = *(RECT*)screenArea;
//       opaque = 0;
//       if (PtInRect(&rect, point)) {
//           ...
//           if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
//               int offset = ...;
//               int result;
//               if (format == 8) {
//                   if (<8-bit pixel differs>) result = 1; else result = opaque;
//               } else {
//                   if (<16-bit pixel differs>) opaque = 1;
//                   result = opaque;
//               }
//               if (texture->systemSurface->Unlock(0))
//                   return result;
//               opaque = result;
//           }
//       } else
//           opaque = 0;
//       return opaque;
// What each part buys (struct form, strict of 354): the exposed flag alone with
// the old body 221; the separate `result` with the 8-bit hit assigned directly
// (or a `goto unlock` past `result = opaque`) 280: the memory store for the
// 16-bit hit only, `mov edi, 1; jmp` for the 8-bit one and the unfolded
// `mov eax, [opaque]` when Lock fails; the clear after the rect copy 315 (before
// it the store is scheduled first); `if (Unlock(0)) return result; opaque =
// result;` falling into the shared `return opaque` 331 (the dead `test eax,
// eax`; `return result` twice, a ternary, an empty if or a plain call all fold
// it away); `else opaque = 0;` with one final `return opaque` 354 (`return 0`
// after the block or in an else puts the Lock-failure return ahead of the
// zero return). Early returns on PtInRect 62-97. The else store is redundant
// (the flag is already 0) and nothing shows what took the flag's address, so
// the source below keeps the plain form.
// 0x00478e10 with form (a) and `int format = ...; int opaque = 0;` reaches
// 318/329, one difference: retail stores the flag's 0 and then the spilled
// format ([esp + 0x1c], [esp + 0x20]); flag first in the source schedules the
// 0 store ahead of the renderTarget load (301), a TrackGame* local and the
// probe between the two declarations change nothing. Its frame (flag at 0,
// the format spill at 4, the description at 8) rules out an aggregate there:
// `struct { int opaque; DDSurfaceDesc2 desc; }` puts the spill below the pair
// (315), with `format` as a member VC6 reloads it through eax (173-174, or
// 250-284 with the local read before the Lock). A union, a one-member struct,
// a constructor-initialised wrapper and `volatile` (148-151) do not expose it.
int UIButton::ContainsPoint(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->surfacePixelFormat;
    RECT rect = *(RECT*)screenArea;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        DDSurfaceDesc2 desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)GetStateTexture(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / PixelFormatBytesPerPixel(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.srcBltColorKey[0])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.srcBltColorKey[0])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x00478e10: the same test as UIButton slot 28 (0x004734c0), but on the
// control's own area (no RECT copy: retail passes &screenArea to PtInRect).
int UIMultiState::ContainsPoint(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->surfacePixelFormat;
    if (PtInRect((RECT*)screenArea, point)) {
        point.x -= ((RECT*)screenArea)->left;
        point.y -= ((RECT*)screenArea)->top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        DDSurfaceDesc2 desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)GetStateTexture(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / PixelFormatBytesPerPixel(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.srcBltColorKey[0])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.srcBltColorKey[0])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}
