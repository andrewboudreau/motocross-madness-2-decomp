// Native execution of unchanged, dual-strict scene setters and their real helper closure.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "math/Math3D.h"
#include "../../src/reconstructed/MatrixUtil.h"

typedef char VectorBytes[(sizeof(Vector3) == 12 && sizeof(Vec3) == 12) ? 1 : -1];
typedef char VectorOffsets[(offsetof(Vector3,x)==0 && offsetof(Vector3,y)==4 && offsetof(Vector3,z)==8) ? 1 : -1];
typedef char NodeBytes[(sizeof(SoultreeObject)==0x1a4) ? 1 : -1];
typedef char LocalOffset[(offsetof(SoultreeObject,localMatrix)==0xb8) ? 1 : -1];
typedef char ValidOffset[(offsetof(SoultreeObject,worldValid)==0x138) ? 1 : -1];
typedef char DirtyOffset[(offsetof(SoultreeObject,subtreeDirty)==0x18c) ? 1 : -1];
extern void InitFastInvSqrtTable();
extern "C" void* GarageModelPrimary(void*);
extern "C" void* GarageGameObject(void*);
static int cases;
static void Check(int ok,int line) { if(!ok) { printf("garage setter failure %d case %d\n",line,cases); exit(1); } }
#define CHECK(x) Check((x),__LINE__)
union Storage { double align; unsigned char bytes[6*sizeof(SoultreeObject)]; } storage, expected;
static SoultreeObject* nodes = (SoultreeObject*)storage.bytes;
static SoultreeObject* wanted = (SoultreeObject*)expected.bytes;

static void Prepare(int seed,int state) {
    memset(&storage,(seed*17+39)&255,sizeof(storage));
    for(int i=0;i<6;i++) {
        nodes[i].parent=0; nodes[i].firstChild=0; nodes[i].nextSibling=0;
        nodes[i].worldValid=(state>>(i%3))&1;
        nodes[i].subtreeDirty=seed&1;
        nodes[i].localMatrix._41=(float)(seed-31);
        nodes[i].localMatrix._42=(float)(seed+7);
        nodes[i].localMatrix._43=(float)-seed;
    }
    // Ancestor, receiver, two children, grandchild, and receiver's sibling.
    nodes[1].parent=&nodes[0]; nodes[1].firstChild=&nodes[2]; nodes[1].nextSibling=&nodes[5];
    nodes[2].parent=&nodes[1]; nodes[2].nextSibling=&nodes[3]; nodes[2].firstChild=&nodes[4];
    nodes[3].parent=&nodes[1]; nodes[4].parent=&nodes[2];
    memcpy(&expected,&storage,sizeof(storage));
    wanted[0].subtreeDirty=wanted[1].subtreeDirty=1;
    if(nodes[1].worldValid) {
        wanted[1].worldValid=0;
        wanted[2].worldValid=0; wanted[3].worldValid=0;
        if(nodes[2].worldValid) wanted[4].worldValid=0;
    }
}
static double Length(double* v) { return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); }
static void Normalize(double* v) { double n=Length(v); for(int i=0;i<3;i++)v[i]/=n; }
static void Cross(double* r,const double* a,const double* b) {
    r[0]=a[1]*b[2]-a[2]*b[1]; r[1]=a[2]*b[0]-a[0]*b[2]; r[2]=a[0]*b[1]-a[1]*b[0];
}
int main() {
    InitFastInvSqrtTable();
    for(int seed=0;seed<64;seed++) for(int state=0;state<8;state++) {
        for(int alias=0;alias<4;alias++) {
            Prepare(seed,state);
            Vector3 external;
            unsigned long bits[3]={0x3f000001UL+seed*0x10001UL,0x80000000UL,0x7fc00000UL+(unsigned)seed};
            memcpy(&external,bits,12);
            const Vector3* input=&external;
            if(alias) input=(const Vector3*)((unsigned char*)&nodes[1].localMatrix+44+(alias-1)*4);
            const float* ref=alias ? (const float*)((unsigned char*)&wanted[1].localMatrix+44+(alias-1)*4) : (const float*)&external;
            // Sequential stores preserve the retail behavior for overlapping inputs.
            memcpy(&wanted[1].localMatrix._41,ref,4);
            memcpy(&wanted[1].localMatrix._42,ref+1,4);
            memcpy(&wanted[1].localMatrix._43,ref+2,4);
            CHECK(GarageModelPrimary(&nodes[1])==&nodes[1]);
            CHECK(GarageGameObject(&nodes[1])==(unsigned char*)&nodes[1]+12);
            nodes[1].SetPosition(*reinterpret_cast<const Vec3*>(input));
            CHECK(memcmp(&storage,&expected,sizeof(storage))==0); cases++;
        }
        for(int axes=0;axes<3;axes++) for(int flags=0;flags<4;flags++) {
            Prepare(seed,state);
            double z[3]={0,0,-1},y[3]={0,1,0},r[3],out[3][3];
            if(axes==1) { z[2]=-2; y[1]=3; }
            if(axes==2) { z[0]=1; z[1]=2; z[2]=-3; y[0]=2; y[1]=1; y[2]=1; }
            float zx=(float)z[0],zy=(float)z[1],zz=(float)z[2],yx=(float)y[0],yy=(float)y[1],yz=(float)y[2];
            if(flags&1) { Cross(r,y,z); if(flags&2)Cross(y,z,r); else Cross(z,r,y); }
            Normalize(y); Normalize(z); Cross(out[0],y,z);
            memcpy(out[1],y,sizeof(y)); memcpy(out[2],z,sizeof(z));
            nodes[1].SetAxes(zx,zy,zz,yx,yy,yz,flags&1,(flags>>1)&1);
            for(int row=0;row<3;row++) {
                for(int col=0;col<3;col++) {
                    CHECK(fabs(nodes[1].localMatrix.m[row][col]-out[row][col])<0.00002);
                    wanted[1].localMatrix.m[row][col]=nodes[1].localMatrix.m[row][col];
                }
                wanted[1].localMatrix.m[row][3]=0;
            }
            wanted[1].localMatrix._44=1;
            CHECK(memcmp(&storage,&expected,sizeof(storage))==0); cases++;
        }
    }
    printf("Garage setters: %d actual setter, alias, axes and cache cases passed\n",cases);
    return 0;
}
