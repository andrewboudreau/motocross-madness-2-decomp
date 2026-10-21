// Native source-only checks of in-place transforms; no reference-code loading.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "math/Math3D.h"
#include "../../src/reconstructed/CollisionTransforms.h"

static int Close(float a, float b) { return fabs(a - b) <= 0.0001f; }
static int Same(const Vec3& a, const Vec3& b) {
    return Close(a.x,b.x) && Close(a.y,b.y) && Close(a.z,b.z);
}
static int CheckBox(Vec3 center, Vec3 half, const Matrix4& matrix) {
    // Independent geometric oracle: union all original and transformed corners.
    Vec3 low=center-half, high=center+half;
    for (int i=0;i<8;++i) {
        float corner[3]={center.x+((i&1)?half.x:-half.x),
                         center.y+((i&2)?half.y:-half.y),
                         center.z+((i&4)?half.z:-half.z)};
        for (int axis=0;axis<3;++axis) {
            float value=matrix.m[3][axis];
            for (int row=0;row<3;++row) value+=corner[row]*matrix.m[row][axis];
            if (value<low[axis]) low[axis]=value;
            if (value>high[axis]) high[axis]=value;
        }
    }
    Vec3 actualCenter=center, actualHalf=half;
    MoveBox(&actualCenter,&actualHalf,&matrix);
    return Same(actualCenter,(low+high)*0.5f) && Same(actualHalf,(high-low)*0.5f);
}
int main() {
    int checks=0;
    for (int n=0;n<64;++n) {
        Matrix4 m; memset(&m,0,sizeof(m)); m._44=1;
        m._11=(float)(n%5-2); m._12=0.25f; m._21=-0.5f;
        m._22=(float)(n%3); m._33=(float)(n%7-3);
        m._41=(float)(n-32); m._42=(float)(9-n); m._43=3.5f;
        Vec3 c((float)(n%3),-2.0f,7.0f), h(1.0f,(float)(n%4),3.0f);
        if (!CheckBox(c,h,m)) return 1;
        ++checks;
        // Identical output pointers retain the half-extents store, which is last.
        Vec3 shared(1,2,3), separateCenter=shared, separateHalf=shared;
        MoveBox(&separateCenter,&separateHalf,&m);
        MoveBox(&shared,&shared,&m);
        if (!Same(shared,separateHalf)) return 2;
        ++checks;
        // Translation-row input overlaps the output center. Matrix reads finish
        // before output stores; the matrix's first three rows must stay intact.
        Matrix4 overlap=m; Vec3 center(m._41,m._42,m._43), expectedHalf=h;
        MoveBox(&center,&expectedHalf,&m);
        MoveBox((Vec3*)&overlap._41,&h,&overlap);
        if (!Same(center,*(Vec3*)&overlap._41) || !Same(h,expectedHalf) ||
            memcmp(&overlap,&m,48)) return 3;
        ++checks;
    }
    for (int turn=0;turn<4;++turn) for (int offset=-4;offset<=4;++offset) {
        Matrix4 original; memset(&original,0,sizeof(original));
        const float cs[4]={1,0,-1,0}, sn[4]={0,1,0,-1};
        original._11=original._22=cs[turn]; original._12=sn[turn]; original._21=-sn[turn];
        original._33=original._44=1; original._41=(float)offset;
        original._42=(float)(offset*2); original._43=-3;
        Matrix4 inverse=original; InvertRigidTransform(&inverse);
        for (int row=0;row<4;++row) for (int col=0;col<4;++col) {
            float product=0;
            for (int k=0;k<4;++k) product+=original.m[row][k]*inverse.m[k][col];
            if (!Close(product,row==col?1.0f:0.0f)) return 4;
        }
        InvertRigidTransform(&inverse);
        if (memcmp(&inverse,&original,sizeof(original))) {
            // Signed zero is numerically equal but may change under negation.
            for (int i=0;i<16;++i) if (!Close(((float*)&inverse)[i],((float*)&original)[i])) return 5;
        }
        ++checks;
    }
    // The helper assumes rigid input; it does not compute a general inverse.
    // Preserve the fourth column even for non-affine diagnostic input.
    Matrix4 arbitrary;
    for (int element=0;element<16;++element) ((float*)&arbitrary)[element]=(float)(element+1);
    InvertRigidTransform(&arbitrary);
    const float expected[16]={1,5,9,4,2,6,10,8,3,7,11,12,-86,-254,-422,16};
    for (int e=0;e<16;++e) if (((float*)&arbitrary)[e]!=expected[e]) return 6;
    ++checks;
    printf("Collision transforms: %d geometry and alias cases passed\n",checks);
    return 0;
}
