// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// 0x00510910 is ManagedTexture::RemapTexCoordsIfMoved (defined in
// ManagedTexture.cpp). Griddraw.h's GridManagedTexture is a boundary view of
// the same object (mapFlags at +0x68), so the view forwards to the provider.

#include "Griddraw.h"        // GridManagedTexture
#include "ManagedTexture.h"  // ManagedTexture

int GridManagedTexture::RemapTexCoordsIfMoved(float* scale, float* offsetU, float* offsetV, float* u, float* v, int count,
                                              unsigned int stride)
{
    return reinterpret_cast<ManagedTexture*>(this)->RemapTexCoordsIfMoved(scale, offsetU, offsetV, u, v, count, stride);
}
