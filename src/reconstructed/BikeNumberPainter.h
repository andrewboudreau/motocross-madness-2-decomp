#pragma once

#include "CameraRect.h"
#include "PCTextureMap.h"

class D3DIMSoultreeObject;

// Provisional unit (tier 3 name and file): the plate number painter whose code
// runs 0x00417500..0x00417aff, between BikeCamera's code and BikeRace's
// constructor helpers at 0x00417b00. No __FILE__ or RTTI names the unit; the
// "BikeNumbers128.tga" literal (0x005679dc) and the renderer's "PROCEDURAL"
// (0x005679f0) sit among BikeAI.cpp's and bikerace.cpp's strings. SelectGamePicProcs.h and KrustyUI.cpp keep opaque
// views of the same 0x2c-byte object.
//
// Layout (tier 1 from the constructor 0x00417500): the digit sheet at +0x00
// and ten digit widths at +0x04 (19 pixels, 13 for the digit 1).
class UnknownBikeNumberPainter {
public:
    UnknownBikeNumberPainter(TextureMapManager* textures); // 0x00417500
    ~UnknownBikeNumberPainter();                            // 0x00417570
    // 0x00417580: copies the digit's 32x32 cell (a 4x3 grid, width from
    // digitWidths) onto `target` at x + 4, y 4.
    void UnknownFunction417580(int digit, int x, PCTextureMap* target);
    // 0x00417670: paints `number` on the model's "PROCEDURAL" textures and
    // fits the plate surfaces' texture coordinates to its digit count.
    void UnknownFunction417670(D3DIMSoultreeObject* model, int number);

    PCTextureMap* sheet;  // +0x00: BikeNumbers128.tga
    int digitWidths[10];  // +0x04
};
