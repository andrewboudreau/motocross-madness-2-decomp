// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// 0x00510910 is ManagedTexture::RemapTexCoordsIfMoved (defined in
// ManagedTexture.cpp). Griddraw.h's GridManagedTexture is a boundary view of
// the same object (mapFlags at +0x68), so the view forwards to the provider.

// ManagedTexture.h's provider, declared only as far as this call needs.
class ManagedTexture {
public:
    int RemapTexCoordsIfMoved(float* scale, float* offsetU, float* offsetV, float* u, float* v, int count,
                              unsigned int stride);  // 0x00510910
};

class GridManagedTexture {
public:
    int RemapTexCoordsIfMoved(float* a, float* b, float* c, float* u, float* v, int count,
                              unsigned int stride);  // 0x00510910 (ret 0x1c)
};

int GridManagedTexture::RemapTexCoordsIfMoved(float* a, float* b, float* c, float* u, float* v, int count,
                                              unsigned int stride)
{
    return reinterpret_cast<ManagedTexture*>(this)->RemapTexCoordsIfMoved(a, b, c, u, v, count, stride);
}
