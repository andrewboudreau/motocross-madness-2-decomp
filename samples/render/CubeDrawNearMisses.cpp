// Near-miss cubedraw.cpp candidates, kept out of src/reconstructed until
// they match. See src/reconstructed/CubeDraw.h.
//
// 0x0043dc60 (895 bytes): the switch, the normal stores (chained
// assignments give retail's y/x/z order) and the cell copies are in place.
// VC6 here keeps 25600.0f in a register where retail stores immediates, and
// the stack slots of the five loop variables and the grid pointer's anchor
// (x here, y in retail) differ. Retail's grid walk runs one pointer through
// all 25 points (biased by +4, x at -4); its cell copy resets the vertex
// pointer from a row pointer (+0x300 per row) and advances the grid pointer
// by 0x80 per row plus 0x20 after it (as if indexed `cell + cellRow`), and
// writes each vertex's position, diffuse, specular, tu and tv in source
// order where VC6 here hoists the constant stores. Tried in session 10: a
// single grid pointer for the face (81-82 of 1052), `&grid[row][column]`
// or `grid[0][row * 5 + column]` per point (64), the grid index as
// `cell + cellRow` with the vertex pointer from `cell` (137 of 1068 bytes
// but 152 instruction diffs against 148; the row reset still folds), slot
// 14's flat `first = cell * 6` vertex form and
// `grid[0][cellRow * 5 + cellColumn]` (121).
// Session 15 reference diff: the three 100.0f loads retail has and this lacks are
// the case tails VC6 cross-jumps here (fild; fmul 100.0f; fstp [eax+8] shared by
// cases 1, 3 and 4), and 0x0057a220 against 0x0057a224 is the +4 pointer bias.
//
// 0x0043e0b0 (352 bytes): the texture-coordinate arithmetic matches once the
// low value is stored before the high one is computed. Retail keeps a row
// pointer (+0x300 per row) and ends the face loop on it; VC6 here folds the
// row into a single cell pointer (index, pointer, 3-D cast and explicit row
// pointer forms all fold, or keep the face counter). Also tried: a rowStart
// index, a 4-D array cast, a loop bounded by the row pointer and a
// post-incremented vertex pointer (39 of 352 at best). Session 10 also
// tried slot 14's flat form (cell = face * 16 + row * 4 + column, first =
// cell * 6 on the global array), a per-row first-vertex index, the cell
// index in the innermost loop, a down-counting column loop, rows stepped
// by 4 cells, a byte-offset cell address and a column-offset cell pointer:
// all compile to the same 30 of 352 (the row reset still folds away).

#include <string.h>

#include "../../src/reconstructed/CubeDraw.h"

#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/Cube.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/Quantize.h"
#include "../../src/reconstructed/RenderTarget.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"

// CubeDraw.cpp's tables (same names and addresses); one storage shared with CubeDraw.cpp.
extern Vector3 s_viewPoints[25];
extern Vector3 s_faceNormals[6];
extern Vector3 s_facePoints[6][25];
extern UnknownCubeVertex s_cellVertices[6][16][6];
extern UnknownCubeTextureMapping s_cellMappings[6][16];

// 0x0043dc60
int DrawableCube::BuildFaceGeometry()
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
                UnknownCubeVertex* cell = s_cellVertices[face][cellRow * 4 + cellColumn];
                UnknownCubeVertex* corner = &grid[cellRow][cellColumn];
                cell[0].position = corner[0].position;
                cell[0].diffuse = 0xffffffff;
                cell[0].specular = 0;
                cell[0].tu = 0.0f;
                cell[0].tv = 0.0f;
                cell[1].position = corner[1].position;
                cell[1].diffuse = 0xffffffff;
                cell[1].specular = 0;
                cell[1].tu = 1.0f;
                cell[1].tv = 0.0f;
                cell[2].position = corner[6].position;
                cell[2].diffuse = 0xffffffff;
                cell[2].specular = 0;
                cell[2].tu = 1.0f;
                cell[2].tv = 1.0f;
                cell[3].position = corner[0].position;
                cell[3].diffuse = 0xffffffff;
                cell[3].specular = 0;
                cell[3].tu = 0.0f;
                cell[3].tv = 0.0f;
                cell[4].position = corner[6].position;
                cell[4].diffuse = 0xffffffff;
                cell[4].specular = 0;
                cell[4].tu = 1.0f;
                cell[4].tv = 1.0f;
                cell[5].position = corner[5].position;
                cell[5].diffuse = 0xffffffff;
                cell[5].specular = 0;
                cell[5].tu = 0.0f;
                cell[5].tv = 1.0f;
            }
        }
        for (int i = 0; i < 25; i++)
            s_facePoints[face][i] = grid[0][i].position;
    }
    return 1;
}

// 0x0043e0b0
int DrawableCube::LoadTexturesAndCoords(Cube* cube, UnknownCubeTextureContext* context)
{
    cubeObject = cube;
    cube->LoadMissingTextures((UnknownTexturePalette*)loadPalette, context);
    if (HasMipLevels(cubeObject->fileFormat)) {
        for (int face = 0; face < 6; face++) {
            for (int row = 0; row < 4; row++) {
                UnknownCubeVertex* cell = s_cellVertices[face][row * 4];
                for (int column = 0; column < 4; column++, cell += 6) {
                    cell[0].tu = 0.03125f;
                    cell[0].tv = 0.03125f;
                    cell[1].tu = 0.96875f;
                    cell[1].tv = 0.03125f;
                    cell[2].tu = 0.96875f;
                    cell[2].tv = 0.96875f;
                    cell[3].tu = 0.03125f;
                    cell[3].tv = 0.03125f;
                    cell[4].tu = 0.96875f;
                    cell[4].tv = 0.96875f;
                    cell[5].tu = 0.03125f;
                    cell[5].tv = 0.96875f;
                }
            }
        }
        return 1;
    }
    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 4; row++) {
            UnknownCubeVertex* cell = s_cellVertices[face][row * 4];
            for (int column = 0; column < 4; column++, cell += 6) {
                int size = DecodeTextureSize(cubeObject->faces[face].sizeCodes, column, row);
                float low = 1.0f / (float)size;
                cell[0].tu = low;
                cell[0].tv = low;
                float high = 1.0f - low;
                cell[1].tu = high;
                cell[1].tv = low;
                cell[2].tu = high;
                cell[2].tv = high;
                cell[3].tu = low;
                cell[3].tv = low;
                cell[4].tu = high;
                cell[4].tv = high;
                cell[5].tu = low;
                cell[5].tv = high;
            }
        }
    }
    return 1;
}
