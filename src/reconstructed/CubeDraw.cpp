// CubeDraw.cpp -- reconstruction of D:\aardvark\VC\krusty2\cubedraw.cpp.
// See CubeDraw.h for the TU extent and the class. The geometry builder
// 0x0043dc60, 0x0043e0b0 and slot 14 0x0043e4c0 (with its static matrix and
// atexit thunk 0x0043e840) are near misses in
// samples/render/CubeDrawNearMisses.cpp.

#include <string.h>

#include "CubeDraw.h"
#include "../krusty2/visibility/VisibilityClipper.h"

#include "Camera.h"
#include "Cube.h"
#include "D3DConstants.h"
#include "DebugAlloc.h"
#include "ManagedTexture.h"
#include "MatrixUtil.h"
#include "Parameterblocks.h"
#include "Quantize.h"
#include "RenderTarget.h"
#include "TextureMap.h"

// The four vector constants that open many retail files. Strong inference:
// this file's copies are 0x00579920, 0x00579a60, 0x0057a200 and 0x005798a8,
// built by 0x0043e870..0x0043e9ab (see CubeDraw.h).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00579930: the current face's grid points relative to the camera.
Vector3 s_viewPoints[25];
// 0x00579a70: the face normals.
Vector3 s_faceNormals[6];
// 0x00579af8: each face's 5x5 grid of points.
Vector3 s_facePoints[6][25];
// 0x0057a210: two triangles per cell, 16 cells per face.
UnknownCubeVertex s_cellVertices[6][16][6];
// 0x0057ea10
UnknownCubeTextureMapping s_cellMappings[6][16];
// The cell arrays viewed flat: cell = face * 16 + i, whose first vertex is
// cell * 6. Slot 14 indexes them this way (retail recomputes the addresses
// from the cell and its first vertex instead of stepping pointers).
#define CELL_VERTEX(n) ((&s_cellVertices[0][0][0])[n])
#define CELL_MAPPING(n) ((&s_cellMappings[0][0])[n])
// 0x0057ee94 / 0x0057ee98: released by the destructor (line 0x192).
static void* s_UnknownGlobal57ee94;
static int s_UnknownGlobal57ee98;

// 0x0043d910
DrawableCube::DrawableCube(int flags) : GameObject(flags)
{
    cubeObject = 0;
    loadPalette = 0;
    field_0x40 = 0;
    memset(s_cellMappings, 0, sizeof(s_cellMappings));
    for (int i = 0; i < 96; i++)
        (&s_cellMappings[0][0])[i].scale = 1.0f;
}

// 0x0043d980
DrawableCube* DrawableCube::Load(void* value, UnknownTextureStream* stream,
                                                  UnknownCubeTextureContext* context)
{
    int offset;
    int paletteCount;
    int start;
    int cubeCount;
    char mode;
    int i;

    GameObject::SetRenderContext(value);
    textureGroup = context->textureGroup;
    if (!stream)
        goto fail;
    if (stream->innerStream)
        stream->Seek(stream->sliceOffset, 0, 0);
    start = stream->Tell();
    if (stream->Read(&paletteCount, 4, 1) != 1)
        goto fail;
    if (stream->Read(&cubeCount, 4, 1) != 1)
        goto fail;
    for (i = 0; i < paletteCount; i++) {
        if (stream->Read(&offset, 4, 1) != 1)
            goto fail;
        int position = stream->Tell();
        mode = ((UnknownParameterStream*)stream)->GetTextMode();
        if (stream->Seek(offset + start, 0, 1))
            goto fail;
        if (loadPalette)
            loadPalette->Release();
        loadPalette = new (__FILE__, 0x5f) ColorMapper(stream);
        if (stream->Seek(position, 0, 1))
            goto fail;
        ((UnknownParameterStream*)stream)->SetTextMode(mode);
    }
    for (i = 0; i < cubeCount; i++) {
        if (stream->Read(&offset, 4, 1) != 1)
            goto fail;
        int position = stream->Tell();
        mode = ((UnknownParameterStream*)stream)->GetTextMode();
        if (stream->Seek(offset + start, 0, 1))
            goto fail;
        if (cubeObject)
            cubeObject->Release();
        cubeObject = (new (__FILE__, 0x6d) Cube)->Load(stream, 0, textureGroup, start);
        if (!cubeObject)
            goto fail;
        if (stream->Seek(position, 0, 1))
            goto fail;
        ((UnknownParameterStream*)stream)->SetTextMode(mode);
    }
    BuildFaceGeometry();
    LoadTexturesAndCoords(cubeObject, context);
    loadPalette->Release();
    loadPalette = 0;
    return this;
fail:
    Release();
    return 0;
}

// 0x0043e210
int DrawableCube::RecordTextureUse()
{
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 16; i++) {
            if (textureGroup && (cubeObject->faces[face].visibleCells & (1 << i))) {
                TextureMap* texture = cubeObject->faces[face].textures[i];
                if (texture && (texture->mapFlags & 1))
                    ((ManagedTexture*)texture)->RecordUse(7.0f);
            }
        }
    }
    return 1;
}

// 0x0043e290
DrawableCube::~DrawableCube()
{
    if (s_UnknownGlobal57ee94)
        DebugFree(s_UnknownGlobal57ee94, __FILE__, 0x192);
    s_UnknownGlobal57ee94 = 0;
    s_UnknownGlobal57ee98 = 0;
    if (loadPalette)
        loadPalette->Release();
    if (cubeObject)
        cubeObject->Release();
}

// 0x0043e330: flags the cells of each camera-facing face whose quads are on
// screen. The camera is read through the cast each time (the Target()
// accessor's expansion swaps eax/ecx), and the eye reference is bound before
// the cube centre, which is the order retail loads them in.
void DrawableCube::MarkVisibleCells()
{
    Vector3 forward;
    forward.x = ((UnknownCubeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewMatrix.m[0][2];
    forward.y = ((UnknownCubeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewMatrix.m[1][2];
    forward.z = ((UnknownCubeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewMatrix.m[2][2];
    const Vector3& eye = ((UnknownCubeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
    const Vector3& center = cubeObject->center;
    Vector3 offset(center.x - eye.x, center.y - eye.y, center.z - eye.z);
    for (int face = 0; face < 6; face++) {
        cubeObject->faces[face].visibleCells = 0;
        if (!reinterpret_cast<VisibilityClipper*>(g_visibilityClipper)->TestDot(
                reinterpret_cast<const VisibilityCamera*>(((RenderTarget*)renderContext)->currentCamera),
                reinterpret_cast<const VisibilityBoxVec*>(&forward),
                reinterpret_cast<const VisibilityBoxVec*>(&s_faceNormals[face])))
            continue;
        for (int k = 0; k < 25; k++)
        {
            const Vector3& point = s_facePoints[face][k];
            s_viewPoints[k] = Vector3(offset.x + point.x, offset.y + point.y, offset.z + point.z);
        }
        reinterpret_cast<VisibilityClipper*>(g_visibilityClipper)->TransformVectors(
            reinterpret_cast<const VisibilityBoxVec*>(s_viewPoints),
            reinterpret_cast<VisibilityBoxVec*>(s_viewPoints),
            reinterpret_cast<const VisibilityMatrix*>(&((UnknownCubeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewMatrix), 25);
        for (int i = 0; i < 16; i++) {
            int corner = i + i / 4;
            if (reinterpret_cast<VisibilityClipper*>(g_visibilityClipper)->CullQuad(
                    reinterpret_cast<const VisibilityCamera*>(((RenderTarget*)renderContext)->currentCamera),
                    reinterpret_cast<const VisibilityBoxVec*>(s_viewPoints), corner,
                    corner + 1, corner + 6, corner + 5))
                cubeObject->faces[face].visibleCells |= 1 << i;
        }
    }
}

// 0x0043e4c0: draws the cube around the camera (scaled by 2, centred on the
// cube's position, depth off): the textured cells, then the bare cells of
// the four side faces' lower halves in the target's colour. Returns 0 when a
// draw fails. The static matrix is built once from IdentityMatrix; its
// atexit thunk is 0x0043e840.
int DrawableCube::UnknownVirtualSlot14()
{
    static Matrix4 s_world = IdentityMatrix();
    int fog;
    Target()->GetRenderState(9, &fog);
    s_world.m[0][0] = 2.0f;
    s_world.m[1][1] = 2.0f;
    s_world.m[2][2] = 2.0f;
    s_world.m[3][0] = cubeObject->center.x;
    s_world.m[3][1] = cubeObject->center.y;
    s_world.m[3][2] = cubeObject->center.z;
    Target()->currentCamera->SetWorldMatrix(&s_world);
    Target()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
    Target()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
    Target()->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
    Target()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0, 0);
    Target()->SetRenderState(D3DRENDERSTATE_ZENABLE, 0, 0);
    Target()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    Target()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    Target()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    int face;
    int i;
    for (face = 0; face < 6; face++) {
        for (i = 0; i < 16; i++) {
            if (!(cubeObject->faces[face].visibleCells & (1 << i)))
                continue;
            if (!(cubeObject->faces[face].presentTextures & (1 << i)))
                continue;
            int cell = face * 16 + i;
            int first = cell * 6;
            if (!cubeObject->faces[face].textures[i]->IsResident())
                continue;
            if (textureGroup && (cubeObject->faces[face].textures[i]->mapFlags & 1)) {
                ((ManagedTexture*)cubeObject->faces[face].textures[i])
                    ->RemapTexCoordsIfMoved(&CELL_MAPPING(cell).scale, &CELL_MAPPING(cell).offsetU,
                                            &CELL_MAPPING(cell).offsetV, &CELL_VERTEX(first).tu,
                                            &CELL_VERTEX(first).tv, 6, 0x20);
            }
            cubeObject->faces[face].textures[i]->Bind();
            if (!Target()->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)&CELL_VERTEX(first), 6, 0))
                return 0;
        }
    }
    Target()->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
    Target()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    Target()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    Target()->ClearTexture(0);
    for (face = 0; face < 6; face++) {
        for (i = 0; i < 16; i++) {
            if (!(cubeObject->faces[face].visibleCells & (1 << i)))
                continue;
            if (cubeObject->faces[face].presentTextures & (1 << i))
                continue;
            if (face >= 4 || i <= 7)
                continue;
            int first = (face * 16 + i) * 6;
            CELL_VERTEX(first).diffuse = Target()->clearColor;
            CELL_VERTEX(first + 1).diffuse = Target()->clearColor;
            CELL_VERTEX(first + 2).diffuse = Target()->clearColor;
            CELL_VERTEX(first + 3).diffuse = Target()->clearColor;
            CELL_VERTEX(first + 4).diffuse = Target()->clearColor;
            CELL_VERTEX(first + 5).diffuse = Target()->clearColor;
            if (!Target()->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)s_cellVertices[face][i], 6, 0))
                return 0;
        }
    }
    Target()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 1, 0);
    Target()->SetRenderState(D3DRENDERSTATE_ZENABLE, 1, 0);
    Target()->SetRenderState(D3DRENDERSTATE_SHADEMODE, fog, 0);
    return 1;
}

// 0x0043e850
int DrawableCube::PrepareGeometry()
{
    MarkVisibleCells();
    RecordTextureUse();
    return 1;
}

// 0x00467ae0 (shared `return 1` body)
int DrawableCube::RenderPre3D()
{
    return 1;
}
