// Near-miss cubedraw.cpp candidate, kept out of src/reconstructed until it
// matches. See src/reconstructed/CubeDraw.h.
//
// 0x0043dc60 (1078 bytes plus its 26-byte switch table): 1100 of 1104 strict,
// 259 of 261 instructions aligned. The only difference left is the order of
// two column-loop spills at +0x228: retail stores the `256 - column * 128`
// induction value ([esp+0x28]) before `column * 128 - 256` ([esp+0x18])
// around the last grid store; VC6 here the other way. Swapping case blocks
// or the tu/tv stores is worse, and `-column * 128 + 256` is the same.
// What moved VC6 to retail's shape:
// - a flat 25-point grid written through one running `point` index
//   (retail's single biased pointer through all 25 points);
// - a separate running corner index `first`, stepped per cell and once more
//   per row (retail's +0x80 at the row top and +0x20 at the bottom), and a
//   running cell index `index` that is not reset per row (the row-exit
//   `mov eax, edi; add edi, 0x300`);
// - direct `s_cellVertices[face][index][k]` stores, which keep each vertex's
//   position, diffuse, specular, tu and tv in source order;
// - case 5's normal stores after its position stores: otherwise VC6
//   cross-jumps the `fmul 100.0f; fstp z` tails of cases 1, 3 and 4 into it
//   (only case 5's placement matters);
// - `first` declared before `index` and the increments written
//   `cellColumn++, first++, index++`.
// Ruled out (equal or worse): `default: break;` at either end of the switch;
// the coordinate spelled (X) * 100.0f, 100.0f * (float)X, (column - 2) * 128
// or -256 + column * 128 (all equal), (float)((X) * 100) (worse); every
// position-store order in each arm and every diffuse/tu/tv order; normals
// first in cases 0-4 (neutral); `point` declared in the for-init or stepped
// in the body; `grid[5][5]` indexed `grid[0][point]` (same); declaration
// orders of face/row/column/point; reusing row/column for the cell loops; a
// running grid pointer (86) or corner pointer (121); a local cell pointer
// with direct corner indexing (worse); and from earlier sessions
// `&grid[row][column]`, `grid[row * 5 + column]` (64), `cell + cellRow`
// (137) and slot 14's flat `first = cell * 6` form (121).
// Behaviour audit 2026-11-11 (linked game): every case's coordinates, normal
// stores and constants, the diffuse/tu/tv grid stores, the six vertices of
// each cell (corner 0/1/6/0/6/5 with their tu/tv) and the 25 face points agree
// with retail; nothing writes `reserved` or the grid's specular in either.
//
// 0x0043e0b0 matched with the same per-face running cell index and is in
// src/reconstructed/CubeDraw.cpp.

#include <string.h>

#include "../../src/reconstructed/CubeDraw.h"

#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/Cube.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/Quantize.h"
#include "../../src/reconstructed/RenderTarget.h"
#include "../../src/reconstructed/TextureMap.h"

// CubeDraw.cpp's tables (same names and addresses); one storage shared with CubeDraw.cpp.
extern Vector3 s_viewPoints[25];
extern Vector3 s_faceNormals[6];
extern Vector3 s_facePoints[6][25];
extern UnknownCubeVertex s_cellVertices[6][16][6];
extern UnknownCubeTextureMapping s_cellMappings[6][16];

// 0x0043dc60
int DrawableCube::BuildFaceGeometry()
{
    UnknownCubeVertex grid[25];
    for (int face = 0; face < 6; face++) {
        int point = 0;
        for (int row = 0; row < 5; row++) {
            for (int column = 0; column < 5; column++, point++) {
                switch (face) {
                case 0:
                    grid[point].position.x = (float)(column * 128 - 256) * 100.0f;
                    grid[point].position.y = (float)(256 - row * 128) * 100.0f;
                    grid[point].position.z = 25600.0f;
                    s_faceNormals[0].x = s_faceNormals[0].y = 0.0f;
                    s_faceNormals[0].z = -1.0f;
                    break;
                case 1:
                    grid[point].position.x = -25600.0f;
                    grid[point].position.y = (float)(256 - row * 128) * 100.0f;
                    grid[point].position.z = (float)(column * 128 - 256) * 100.0f;
                    s_faceNormals[1].z = s_faceNormals[1].y = 0.0f;
                    s_faceNormals[1].x = 1.0f;
                    break;
                case 2:
                    grid[point].position.x = (float)(256 - column * 128) * 100.0f;
                    grid[point].position.y = (float)(256 - row * 128) * 100.0f;
                    grid[point].position.z = -25600.0f;
                    s_faceNormals[2].x = s_faceNormals[2].y = 0.0f;
                    s_faceNormals[2].z = 1.0f;
                    break;
                case 3:
                    grid[point].position.x = 25600.0f;
                    grid[point].position.y = (float)(256 - row * 128) * 100.0f;
                    grid[point].position.z = (float)(256 - column * 128) * 100.0f;
                    s_faceNormals[3].z = s_faceNormals[3].y = 0.0f;
                    s_faceNormals[3].x = -1.0f;
                    break;
                case 4:
                    grid[point].position.x = (float)(column * 128 - 256) * 100.0f;
                    grid[point].position.y = 25600.0f;
                    grid[point].position.z = (float)(row * 128 - 256) * 100.0f;
                    s_faceNormals[4].x = s_faceNormals[4].z = 0.0f;
                    s_faceNormals[4].y = -1.0f;
                    break;
                case 5:
                    grid[point].position.x = (float)(column * 128 - 256) * 100.0f;
                    grid[point].position.y = -25600.0f;
                    grid[point].position.z = (float)(256 - row * 128) * 100.0f;
                    s_faceNormals[5].x = s_faceNormals[5].z = 0.0f;
                    s_faceNormals[5].y = 1.0f;
                    break;
                }
                grid[point].diffuse = 0xffffffff;
                grid[point].tu = (float)column * 0.25f;
                grid[point].tv = (float)row * 0.25f;
            }
        }
        int first = 0;
        int index = 0;
        for (int cellRow = 0; cellRow < 4; cellRow++, first++) {
            for (int cellColumn = 0; cellColumn < 4; cellColumn++, first++, index++) {
                UnknownCubeVertex* corner = &grid[first];
                s_cellVertices[face][index][0].position = corner[0].position;
                s_cellVertices[face][index][0].diffuse = 0xffffffff;
                s_cellVertices[face][index][0].specular = 0;
                s_cellVertices[face][index][0].tu = 0.0f;
                s_cellVertices[face][index][0].tv = 0.0f;
                s_cellVertices[face][index][1].position = corner[1].position;
                s_cellVertices[face][index][1].diffuse = 0xffffffff;
                s_cellVertices[face][index][1].specular = 0;
                s_cellVertices[face][index][1].tu = 1.0f;
                s_cellVertices[face][index][1].tv = 0.0f;
                s_cellVertices[face][index][2].position = corner[6].position;
                s_cellVertices[face][index][2].diffuse = 0xffffffff;
                s_cellVertices[face][index][2].specular = 0;
                s_cellVertices[face][index][2].tu = 1.0f;
                s_cellVertices[face][index][2].tv = 1.0f;
                s_cellVertices[face][index][3].position = corner[0].position;
                s_cellVertices[face][index][3].diffuse = 0xffffffff;
                s_cellVertices[face][index][3].specular = 0;
                s_cellVertices[face][index][3].tu = 0.0f;
                s_cellVertices[face][index][3].tv = 0.0f;
                s_cellVertices[face][index][4].position = corner[6].position;
                s_cellVertices[face][index][4].diffuse = 0xffffffff;
                s_cellVertices[face][index][4].specular = 0;
                s_cellVertices[face][index][4].tu = 1.0f;
                s_cellVertices[face][index][4].tv = 1.0f;
                s_cellVertices[face][index][5].position = corner[5].position;
                s_cellVertices[face][index][5].diffuse = 0xffffffff;
                s_cellVertices[face][index][5].specular = 0;
                s_cellVertices[face][index][5].tu = 0.0f;
                s_cellVertices[face][index][5].tv = 1.0f;
            }
        }
        for (int i = 0; i < 25; i++)
            s_facePoints[face][i] = grid[i].position;
    }
    return 1;
}
