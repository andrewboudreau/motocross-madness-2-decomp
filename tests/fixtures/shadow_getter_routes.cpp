// Executes actual scene getter/cache/bounds methods; no scripted dependencies.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/Math3D.h"
#include "shadow/D3DIMSoultreeShadow.h"

typedef char VecBytes[(sizeof(Vec3)==12 && sizeof(ShadowVec3)==12)?1:-1];
typedef char VecOffsets[(offsetof(ShadowVec3,x)==0 && offsetof(ShadowVec3,y)==4 && offsetof(ShadowVec3,z)==8)?1:-1];
typedef char MatrixBytes[(sizeof(Matrix4)==64 && sizeof(ShadowMatrix)==64)?1:-1];
typedef char MatrixOffsets[(offsetof(Matrix4,_41)==48 && offsetof(Matrix4,_44)==60)?1:-1];
typedef char ModelFields[(offsetof(SoultreeObject,localMatrix)==0xb8 && offsetof(SoultreeObject,worldValid)==0x138 && offsetof(SoultreeObject,subtreeDirty)==0x18c)?1:-1];
typedef char CasterField[(offsetof(D3DIMSoultreeShadow,caster)==0x2c)?1:-1];
union NodeStorage { double align; unsigned char bytes[5*sizeof(SoultreeObject)+32]; } actual, expected;
union Output { double align; unsigned char bytes[128]; } out, wantedOut;
static SoultreeObject* n=(SoultreeObject*)(actual.bytes+16);
static SoultreeObject* e=(SoultreeObject*)(expected.bytes+16);
static int cases;
static void Require(int ok,int line) { if(!ok){ printf("shadow getter failure %d case %d\n",line,cases); exit(1); } }
#define CHECK(x) Require((x),__LINE__)
static void Matrix(Matrix4& m,int seed,int i) {
    memset(&m,0,64);
    m._11=(seed&1)?-1.0f:1.0f; m._22=1; m._33=m._11; m._44=1;
    m._41=(float)(seed-12+i);m._42=(float)(i*3);m._43=(float)(7-seed-i);
}
static void Setup(int seed,int valid,int boxes) {
    memset(&actual,seed+73,sizeof(actual));
    for(int i=0;i<5;i++) {
        n[i].parent=0;n[i].firstChild=0;n[i].nextSibling=0;
        n[i].worldValid=(valid>>(i%3))&1;n[i].subtreeDirty=0;
        n[i].field_0x150=(boxes>>(i%2))&1;
        n[i].localBoundsA=Vec3((float)(i-2),(float)(seed%5),(float)(i+seed%3));
        n[i].localBoundsB=Vec3((float)(i+1),2.0f,3.0f);
        n[i].subtreeBoundsA=Vec3(101.0f,102.0f,103.0f);
        n[i].subtreeBoundsB=Vec3(201.0f,202.0f,203.0f);
        Matrix(n[i].localMatrix,seed,i); Matrix(n[i].worldMatrix,seed+5,i+1);
    }
    n[0].parent=&n[1];n[0].firstChild=&n[2];
    n[2].parent=&n[0];n[2].nextSibling=&n[3];n[2].firstChild=&n[4];
    n[3].parent=&n[0];n[4].parent=&n[2];
    memcpy(&expected,&actual,sizeof(actual));memset(&out,0x97,sizeof(out));wantedOut=out;
}
static int Index(SoultreeObject* p) { return (int)(p-n); }
static void World(int i) {
    int k;
    if(e[i].worldValid)return;
    if(e[i].parent) {
        int parent=Index(e[i].parent);World(parent);
        Matrix4 value;
        for(int row=0;row<4;row++)for(int col=0;col<4;col++) {
            double sum=0;
            for(k=0;k<4;k++)sum+=(double)e[i].localMatrix.m[row][k]*e[parent].worldMatrix.m[k][col];
            value.m[row][col]=(float)sum;
        }
        e[i].worldMatrix=value;
    } else e[i].worldMatrix=e[i].localMatrix;
    e[i].worldValid=1;
}
static void Grow(int i,int& have,double* lo,double* hi) {
    int k;
    if(e[i].field_0x150) {
        World(i);
        for(int corner=0;corner<8;corner++) {
            double local[3],world[3],point[3];
            for(k=0;k<3;k++)local[k]=e[i].localBoundsA[k]+((corner&(1<<k))?1:-1)*e[i].localBoundsB[k];
            for(int col=0;col<3;col++) {
                world[col]=e[i].worldMatrix.m[3][col];
                for(k=0;k<3;k++)world[col]+=local[k]*e[i].worldMatrix.m[k][col];
                world[col]-=e[0].worldMatrix.m[3][col];
            }
            for(int row=0;row<3;row++) {
                point[row]=0;
                for(k=0;k<3;k++)point[row]+=world[k]*e[0].worldMatrix.m[row][k];
            }
            for(k=0;k<3;k++) {
                if(!have || point[k]<lo[k])lo[k]=point[k];
                if(!have || point[k]>hi[k])hi[k]=point[k];
            }
            have=1;
        }
    }
    for(SoultreeObject* child=e[i].firstChild;child;child=e[Index(child)].nextSibling)Grow(Index(child),have,lo,hi);
}
static void Bounds() {
    int k;
    World(0);
    int have=e[0].field_0x150!=0;double lo[3]={0,0,0},hi[3]={0,0,0};
    if(have)for(k=0;k<3;k++) {lo[k]=e[0].localBoundsA[k]-e[0].localBoundsB[k];hi[k]=e[0].localBoundsA[k]+e[0].localBoundsB[k];}
    for(SoultreeObject* child=e[0].firstChild;child;child=e[Index(child)].nextSibling)Grow(Index(child),have,lo,hi);
    for(k=0;k<3;k++) {e[0].subtreeBoundsA[k]=(float)((hi[k]+lo[k])*0.5);e[0].subtreeBoundsB[k]=(float)((hi[k]-lo[k])*0.5);}
}
int main() {
    for(int seed=0;seed<32;seed++)for(int valid=0;valid<8;valid++)for(int boxes=0;boxes<4;boxes++) {
        for(int alias=0;alias<2;alias++) {
            Setup(seed,valid,boxes);World(0);
            Matrix4* result=alias?&n[0].worldMatrix:(Matrix4*)(out.bytes+16);
            if(!alias)memcpy(wantedOut.bytes+16,&e[0].worldMatrix,64);
            n[0].GetMatrixIn(0,result);
            CHECK(memcmp(&actual,&expected,sizeof(actual))==0);CHECK(memcmp(&out,&wantedOut,sizeof(out))==0);cases++;
        }
        for(int dirty=0;dirty<2;dirty++)for(int alias=0;alias<2;alias++) {
            Setup(seed,valid,boxes);n[0].subtreeDirty=e[0].subtreeDirty=dirty;
            if(dirty)Bounds();
            ShadowVec3* center=(ShadowVec3*)(out.bytes+16);
            ShadowVec3* extent=(ShadowVec3*)(out.bytes+(alias?16:40));
            memcpy(wantedOut.bytes+16,&e[0].subtreeBoundsA,12);
            memcpy(wantedOut.bytes+(alias?16:40),&e[0].subtreeBoundsB,12);
            n[0].GetSubtreeBounds(reinterpret_cast<Vec3*>(center),reinterpret_cast<Vec3*>(extent));
            CHECK(memcmp(&actual,&expected,sizeof(actual))==0);CHECK(memcmp(&out,&wantedOut,sizeof(out))==0);cases++;
        }
    }
    printf("Shadow scene getters: %d real matrix, bounds, cache and alias cases passed\n",cases);
    return 0;
}
