#pragma once

// A screen rectangle (RECT-shaped: left, top, right, bottom). RenderTarget,
// the dialogs and the controls pass it; Win32 calls take it as a RECT.
struct CameraRect { int left; int top; int right; int bottom; };
