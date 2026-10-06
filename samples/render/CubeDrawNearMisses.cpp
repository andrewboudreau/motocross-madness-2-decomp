// Near-miss cubedraw.cpp candidates, kept out of src/reconstructed until
// they match. See src/reconstructed/CubeDraw.h.
//
// 0x0043dc60 (895 bytes): the switch, the normal stores (chained
// assignments give retail's y/x/z order) and the cell copies are in place.
// VC6 here keeps 25600.0f in a register where retail stores immediates, and
// the stack slots of the five loop variables and the grid pointer's anchor
// (x here, y in retail) differ.
//
// 0x0043e0b0 (352 bytes): the texture-coordinate arithmetic matches once the
// low value is stored before the high one is computed. Retail keeps a row
// pointer (+0x300 per row) and ends the face loop on it; VC6 here folds the
// row into a single cell pointer (index, pointer, 3-D cast and explicit row
// pointer forms all fold, or keep the face counter).
//
// 0x0043e330 (398 bytes): the walk matches; eax and ecx swap roles for the
// render target and its camera, the face mask is addressed [face + cube]
// in retail and [cube + face] here, and the quad test's arguments are
// evaluated before the camera load in retail.
//
// Slot 14 0x0043e4c0 (886 bytes): the render-state sequence matches; retail
// keeps the zero arguments in esi (the face offset's initial value) and
// recomputes the cell's vertex and mapping addresses from face * 16 + i,
// where VC6 here pushes immediates and strength-reduces the addresses. Its
// static matrix's atexit thunk 0x0043e840 ($E26) matches.

#include <string.h>

#include "../../src/reconstructed/CubeDraw.h"

#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/Cube.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/Quantize.h"
#include "../../src/reconstructed/RenderTarget.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"

// CubeDraw.cpp's file statics (same names and addresses).
static Vector3 s_viewPoints[25];
static Vector3 s_faceNormals[6];
static Vector3 s_facePoints[6][25];
static UnknownCubeVertex s_cellVertices[6][16][6];
static UnknownCubeTextureMapping s_cellMappings[6][16];

// 0x0043dc60
int DrawableCube::UnknownFunction43dc60()
{
    UnknownCubeVertex grid[5][5];
    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 5; row++) {
            UnknownCubeVertex* point = grid[row];
            for (int column = 0; column < 5; column++, point++) {
                switch (face) {
                case 0:
                    s_faceNormals[0].x = s_faceNormals[0].y = 0.0f;
                    s_faceNormals[0].z = -1.0f;
                    point->position.x = (float)(column * 128 - 256) * 100.0f;
                    point->position.y = (float)(256 - row * 128) * 100.0f;
                    point->position.z = 25600.0f;
                    break;
                case 1:
                    point->position.x = -25600.0f;
                    s_faceNormals[1].z = s_faceNormals[1].y = 0.0f;
                    s_faceNormals[1].x = 1.0f;
                    point->position.y = (float)(256 - row * 128) * 100.0f;
                    point->position.z = (float)(column * 128 - 256) * 100.0f;
                    break;
                case 2:
                    s_faceNormals[2].x = s_faceNormals[2].y = 0.0f;
                    s_faceNormals[2].z = 1.0f;
                    point->position.x = (float)(256 - column * 128) * 100.0f;
                    point->position.y = (float)(256 - row * 128) * 100.0f;
                    point->position.z = -25600.0f;
                    break;
                case 3:
                    point->position.x = 25600.0f;
                    s_faceNormals[3].z = s_faceNormals[3].y = 0.0f;
                    s_faceNormals[3].x = -1.0f;
                    point->position.y = (float)(256 - row * 128) * 100.0f;
                    point->position.z = (float)(256 - column * 128) * 100.0f;
                    break;
                case 4:
                    s_faceNormals[4].x = s_faceNormals[4].z = 0.0f;
                    s_faceNormals[4].y = -1.0f;
                    point->position.x = (float)(column * 128 - 256) * 100.0f;
                    point->position.y = 25600.0f;
                    point->position.z = (float)(row * 128 - 256) * 100.0f;
                    break;
                case 5:
                    s_faceNormals[5].x = s_faceNormals[5].z = 0.0f;
                    s_faceNormals[5].y = 1.0f;
                    point->position.x = (float)(column * 128 - 256) * 100.0f;
                    point->position.y = -25600.0f;
                    point->position.z = (float)(256 - row * 128) * 100.0f;
                    break;
                }
                point->diffuse = 0xffffffff;
                point->tu = (float)column * 0.25f;
                point->tv = (float)row * 0.25f;
            }
        }
        for (int cellRow = 0; cellRow < 4; cellRow++) {
            for (int cellColumn = 0; cellColumn < 4; cellColumn++) {
                UnknownCubeVertex* v = s_cellVertices[face][cellRow * 4 + cellColumn];
                UnknownCubeVertex* p = &grid[cellRow][cellColumn];
                v[0].position = p[0].position;
                v[0].diffuse = 0xffffffff;
                v[0].specular = 0;
                v[0].tu = 0.0f;
                v[0].tv = 0.0f;
                v[1].position = p[1].position;
                v[1].diffuse = 0xffffffff;
                v[1].specular = 0;
                v[1].tu = 1.0f;
                v[1].tv = 0.0f;
                v[2].position = p[6].position;
                v[2].diffuse = 0xffffffff;
                v[2].specular = 0;
                v[2].tu = 1.0f;
                v[2].tv = 1.0f;
                v[3].position = p[0].position;
                v[3].diffuse = 0xffffffff;
                v[3].specular = 0;
                v[3].tu = 0.0f;
                v[3].tv = 0.0f;
                v[4].position = p[6].position;
                v[4].diffuse = 0xffffffff;
                v[4].specular = 0;
                v[4].tu = 1.0f;
                v[4].tv = 1.0f;
                v[5].position = p[5].position;
                v[5].diffuse = 0xffffffff;
                v[5].specular = 0;
                v[5].tu = 0.0f;
                v[5].tv = 1.0f;
            }
        }
        for (int i = 0; i < 25; i++)
            s_facePoints[face][i] = grid[0][i].position;
    }
    return 1;
}

// 0x0043e0b0
int DrawableCube::UnknownFunction43e0b0(Cube* cube, UnknownCubeTextureContext* context)
{
    field_0x2c = cube;
    cube->UnknownFunction43d630((UnknownTexturePalette*)field_0x30, context);
    if (UnknownFunction511850(field_0x2c->field_0x0c)) {
        for (int face = 0; face < 6; face++) {
            for (int row = 0; row < 4; row++) {
                UnknownCubeVertex* v = s_cellVertices[face][row * 4];
                for (int column = 0; column < 4; column++, v += 6) {
                    v[0].tu = 0.03125f;
                    v[0].tv = 0.03125f;
                    v[1].tu = 0.96875f;
                    v[1].tv = 0.03125f;
                    v[2].tu = 0.96875f;
                    v[2].tv = 0.96875f;
                    v[3].tu = 0.03125f;
                    v[3].tv = 0.03125f;
                    v[4].tu = 0.96875f;
                    v[4].tv = 0.96875f;
                    v[5].tu = 0.03125f;
                    v[5].tv = 0.96875f;
                }
            }
        }
        return 1;
    }
    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 4; row++) {
            UnknownCubeVertex* v = s_cellVertices[face][row * 4];
            for (int column = 0; column < 4; column++, v += 6) {
                int size = UnknownFunction43d080(field_0x2c->field_0x38[face].field_0x00, column, row);
                float low = 1.0f / (float)size;
                v[0].tu = low;
                v[0].tv = low;
                float high = 1.0f - low;
                v[1].tu = high;
                v[1].tv = low;
                v[2].tu = high;
                v[2].tv = high;
                v[3].tu = low;
                v[3].tv = low;
                v[4].tu = high;
                v[4].tv = high;
                v[5].tu = low;
                v[5].tv = high;
            }
        }
    }
    return 1;
}

// 0x0043e330
void DrawableCube::UnknownFunction43e330()
{
    Vector3 forward;
    forward.x = ((UnknownCubeCameraView*)Target()->field_0x08)->field_0xac.m[0][2];
    forward.y = ((UnknownCubeCameraView*)Target()->field_0x08)->field_0xac.m[1][2];
    forward.z = ((UnknownCubeCameraView*)Target()->field_0x08)->field_0xac.m[2][2];
    const Vector3& center = field_0x2c->field_0x14;
    const Vector3& eye = ((UnknownCubeCameraView*)Target()->field_0x08)->field_0x170;
    Vector3 offset(center.x - eye.x, center.y - eye.y, center.z - eye.z);
    for (int face = 0; face < 6; face++) {
        field_0x2c->field_0x38[face].field_0xe8 = 0;
        if (!g_UnknownCubeClipper575a98->UnknownFunction52f140(Target()->field_0x08, &forward,
                                                                &s_faceNormals[face]))
            continue;
        for (int k = 0; k < 25; k++)
        {
            const Vector3& point = s_facePoints[face][k];
            s_viewPoints[k] = Vector3(offset.x + point.x, offset.y + point.y, offset.z + point.z);
        }
        g_UnknownCubeClipper575a98->UnknownFunction52f0a0(
            s_viewPoints, s_viewPoints, &((UnknownCubeCameraView*)Target()->field_0x08)->field_0xac, 25);
        for (int i = 0; i < 16; i++) {
            int corner = i + i / 4;
            if (g_UnknownCubeClipper575a98->UnknownFunction52f4d0(Target()->field_0x08, s_viewPoints, corner,
                                                                  corner + 1, corner + 6, corner + 5))
                field_0x2c->field_0x38[face].field_0xe8 |= 1 << i;
        }
    }
}

// 0x0043e4c0
int DrawableCube::UnknownVirtualSlot14()
{
    static Matrix4 s_world = IdentityMatrix();
    int fog;
    Target()->UnknownVirtualSlot9(9, &fog);
    s_world.m[0][0] = 2.0f;
    s_world.m[1][1] = 2.0f;
    s_world.m[2][2] = 2.0f;
    s_world.m[3][0] = field_0x2c->field_0x14.x;
    s_world.m[3][1] = field_0x2c->field_0x14.y;
    s_world.m[3][2] = field_0x2c->field_0x14.z;
    Target()->field_0x08->UnknownVirtualSlot30(&s_world);
    Target()->UnknownVirtualSlot8(0x1b, 0, 0);
    Target()->UnknownVirtualSlot8(0x29, 0, 0);
    Target()->UnknownVirtualSlot8(0xf, 0, 0);
    Target()->UnknownVirtualSlot8(0xe, 0, 0);
    Target()->UnknownVirtualSlot8(7, 0, 0);
    Target()->UnknownVirtualSlot7(0, 1, 2);
    Target()->UnknownVirtualSlot7(0, 2, 2);
    Target()->UnknownVirtualSlot7(0, 4, 1);
    int face;
    int i;
    for (face = 0; face < 6; face++) {
        for (i = 0; i < 16; i++) {
            if (!(field_0x2c->field_0x38[face].field_0xe8 & (1 << i)))
                continue;
            if (!(field_0x2c->field_0x38[face].field_0x04 & (1 << i)))
                continue;
            if (!field_0x2c->field_0x38[face].field_0x08[i]->UnknownVirtualSlot7())
                continue;
            if (field_0x34 && (field_0x2c->field_0x38[face].field_0x08[i]->field_0x68 & 1)) {
                UnknownCubeTextureMapping* mapping = &s_cellMappings[face][i];
                ((ManagedTexture*)field_0x2c->field_0x38[face].field_0x08[i])
                    ->UnknownFunction510910(&mapping->scale, &mapping->offsetU, &mapping->offsetV,
                                            &s_cellVertices[face][i][0].tu, &s_cellVertices[face][i][0].tv, 6,
                                            0x20);
            }
            field_0x2c->field_0x38[face].field_0x08[i]->UnknownVirtualSlot19();
            if (!Target()->UnknownVirtualSlot16(4, 0x1e2, (int)s_cellVertices[face][i], 6, 0))
                return 0;
        }
    }
    Target()->UnknownVirtualSlot8(9, 1, 0);
    Target()->UnknownVirtualSlot7(0, 4, 1);
    Target()->UnknownVirtualSlot7(0, 1, 1);
    Target()->UnknownVirtualSlot11(0);
    for (face = 0; face < 6; face++) {
        for (i = 0; i < 16; i++) {
            if (!(field_0x2c->field_0x38[face].field_0xe8 & (1 << i)))
                continue;
            if (field_0x2c->field_0x38[face].field_0x04 & (1 << i))
                continue;
            if (face >= 4 || i <= 7)
                continue;
            UnknownCubeVertex* v = s_cellVertices[face][i];
            v[0].diffuse = Target()->field_0x30;
            v[1].diffuse = Target()->field_0x30;
            v[2].diffuse = Target()->field_0x30;
            v[3].diffuse = Target()->field_0x30;
            v[4].diffuse = Target()->field_0x30;
            v[5].diffuse = Target()->field_0x30;
            if (!Target()->UnknownVirtualSlot16(4, 0x1e2, (int)v, 6, 0))
                return 0;
        }
    }
    Target()->UnknownVirtualSlot8(0xe, 1, 0);
    Target()->UnknownVirtualSlot8(7, 1, 0);
    Target()->UnknownVirtualSlot8(9, fog, 0);
    return 1;
}
